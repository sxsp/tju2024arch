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
	cache_line *cache = (cache_line *)malloc(L1_SIZE * sizeof(cache_line));
	init_cache(cache);
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
		printf("tag: %u, index: %u, offset: %u\n", cur_tag, cur_index, cur_offset);
	}

	fclose(traces);
	free(cache);
	return 0;
}
