#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "init.h"
#include "utils.h"
#include "cache.h"

unsigned int L1_BLOCKSIZE = 0;
unsigned int L1_SIZE = 0;
unsigned int L1_ASSOC = 0;
unsigned int L1_REPLACEMENT_POLICY = 0;
unsigned int L1_WRITE_POLICY = 0;
char *trace_file;

unsigned SET_NUM = 0;
unsigned int TAG_LENGTH = 0;
unsigned int INDEX_LENGTH = 0;
unsigned int OFFSET_LENGTH = 0;

unsigned int L1_READS = 0;
unsigned int L1_READ_MISSES = 0;
unsigned int L1_WRITES = 0;
unsigned int L1_WRITE_MISSES = 0;
double L1_MISS_RATE = 0.0;
unsigned int L1_WRITEBACK = 0;
unsigned int TOTAL_MEM_TRAFFIC = 0;
double AVERAGE_ACCESS_TIME = 0.0;

int main(int argc, char *argv[])
{
	if (arg_init(argc, argv) == 1)
	{
		return EXIT_FAILURE;
	}
	FILE *traces;
	traces = fopen(trace_file, "r");
	if (traces == NULL)
	{
		printf("Error: open %s failed.\n", trace_file);
		return EXIT_FAILURE;
	}
	print_init_arg_info();

	// printf("%u %u %u %u %u %s\n", L1_BLOCKSIZE, L1_SIZE, L1_ASSOC, L1_REPLACEMENT_POLICY, L1_WRITE_POLICY, trace_file);
	cache_line *cache = (cache_line *)malloc(L1_SIZE * sizeof(cache_line));
	cache_set *set = (cache_set *)malloc(SET_NUM * sizeof(cache_set));
	init_cache(cache, set);
	// print_whole_cache(cache);

	char line[64];
	char cur_op;
	char cur_addr_str[9];
	unsigned int cur_addr;
	unsigned int cur_i = 0;
	unsigned int cur_tag = 0;
	unsigned int cur_index = 0;
	unsigned int cur_offset = 0;
	char *endptr;
	bool is_hit = false;
	unsigned int hit_i = 0;
	while (fgets(line, sizeof(line), traces) != NULL)
	{
		cur_i++;
		cur_op = line[0];
		strncpy(cur_addr_str, line + 2, strlen(line) - 3);
		cur_addr_str[strlen(line) - 3] = '\0';
		cur_addr = strtoul(cur_addr_str, &endptr, 16);
		if (*endptr != '\0')
		{
			printf("Error: failed to parse line%u: %s\n", cur_i, line);
			fclose(traces);
			free(cache);
			return EXIT_FAILURE;
		}
		get_tag_index_offset(cur_addr, cur_tag, cur_index, cur_offset);
		// 上面的步骤从地址中解析出了tag, index, offset

		is_hit = false;

		// 读
		if (cur_op == 'r')
		{
			L1_READS++;
			hit_i = search_in_set_for_hit(cache, cur_tag, set[cur_index].start_cache_i, is_hit);

			// 命中
			if (is_hit)
			{
				update_BLOCK_COUNT_for_hit(cache, set, cur_index, hit_i);
			}
			// 未命中，检查需要调入还是替换
			else
			{
				L1_READ_MISSES++;

				bool have_free_line = false;
				unsigned int exchange_or_add_i = search_in_set_for_free_line(cache, set, cur_index, have_free_line);

				// 有可用行，调入新行
				if (have_free_line)
				{
					update_set_for_add_line(cache, set, exchange_or_add_i, cur_index, cur_tag);
				}
				// 需要替换
				else
				{
					// WBWA策略，若是脏块，需要先写回
					if (L1_WRITE_POLICY == 0)
					{
						if (cache[exchange_or_add_i].is_dirty == true)
						{
							L1_WRITEBACK++;
							cache[exchange_or_add_i].is_dirty = false;
						}
						update_set_for_exchange_line(cache, set, exchange_or_add_i, cur_index, cur_tag);
					}

					// WTNA策略，直接替换
					else
					{
						update_set_for_exchange_line(cache, set, exchange_or_add_i, cur_index, cur_tag);
					}
				}
			}
		}

		// 写
		else
		{
			L1_WRITES++;
			hit_i = search_in_set_for_hit(cache, cur_tag, set[cur_index].start_cache_i, is_hit);
			// 命中
			if (is_hit)
			{
				update_BLOCK_COUNT_for_hit(cache, set, cur_index, hit_i);

				// WBWA策略，写命中先设为脏，不更新下一级存储
				if (L1_WRITE_POLICY == 0)
				{
					cache[hit_i].is_dirty = true;
				}
			}

			// 未命中
			else
			{
				L1_WRITE_MISSES++;

				bool have_free_line = false;
				unsigned int exchange_or_add_i = search_in_set_for_free_line(cache, set, cur_index, have_free_line);

				// 有可用行
				if (have_free_line)
				{
					// WBWA策略，调入新行，设新行为脏
					if (L1_WRITE_POLICY == 0)
					{
						update_set_for_add_line(cache, set, exchange_or_add_i, cur_index, cur_tag);
						// cache[exchange_or_add_i].is_valid = true;
						cache[exchange_or_add_i].is_dirty = true;
					}
				}
				// 需要替换
				else
				{
					// WBWA策略，若是脏块，需要先写回
					if (L1_WRITE_POLICY == 0)
					{
						if (cache[exchange_or_add_i].is_dirty == true)
						{
							L1_WRITEBACK++;
							cache[exchange_or_add_i].is_dirty = false;
						}
						update_set_for_exchange_line(cache, set, exchange_or_add_i, cur_index, cur_tag);
						cache[exchange_or_add_i].is_dirty = true;
					}
				}
			}
		}
	}
	printf("\n===== L1 contents =====\n");
	for (unsigned int i = 0; i < SET_NUM; i++)
	{
		printf("set %u:\t", i);
		for (unsigned int j = 0; j < L1_ASSOC; j++)
		{
			printf("%x ", cache[i * L1_ASSOC + j].tag);
			if (L1_WRITE_POLICY == 0)
			{
				printf("%c", cache[i * L1_ASSOC + j].is_dirty ? 'D' : ' ');
			}
			printf("\t");
		}
		printf("\n");
	}
	printf("\n  ====== Simulation results (raw) ======\n");
	printf("  a. number of L1 reads: %u\n", L1_READS);
	printf("  b. number of L1 read misses: %u\n", L1_READ_MISSES);
	printf("  c. number of L1 writes: %u\n", L1_WRITES);
	printf("  d. number of L1 write misses: %u\n", L1_WRITE_MISSES);
	printf("  e. L1 miss rate: %0.4f\n", ((double)(L1_READ_MISSES + L1_WRITE_MISSES) / (L1_READS + L1_WRITES)));
	printf("  f. number of writebacks from L1: %u\n", L1_WRITEBACK);
	TOTAL_MEM_TRAFFIC = (L1_WRITE_POLICY == 0) ? L1_READ_MISSES + L1_WRITE_MISSES + L1_WRITEBACK : L1_READ_MISSES + L1_WRITES;
	printf("  g. total memory traffic: %u\n", TOTAL_MEM_TRAFFIC);

	printf("\n  ==== Simulation results (performance) ====\n");
	double hit_time = 0.25 + 2.5 * L1_SIZE / 512 / 1024 + 0.025 * L1_BLOCKSIZE / 16 + 0.025 * L1_ASSOC;
	double miss_time = 20 + 0.5 * L1_BLOCKSIZE / 16;
	AVERAGE_ACCESS_TIME = hit_time + miss_time * (L1_READ_MISSES + L1_WRITE_MISSES) / (L1_READS + L1_WRITES);
	printf("  1. average access time: %0.4f ns\n", AVERAGE_ACCESS_TIME);
	fclose(traces);
	free(cache);
	free(set);
	return 0;
}
