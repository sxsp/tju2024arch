#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <unistd.h>
// #include <io.h>
#include "utils.h"

extern unsigned int L1_BLOCKSIZE;
extern unsigned int L1_SIZE;
extern unsigned int L1_ASSOC;
extern unsigned int L1_REPLACEMENT_POLICY;
extern unsigned int L1_WRITE_POLICY;
extern char *trace_file;

extern unsigned SET_NUM;
extern unsigned int TAG_LENGTH;
extern unsigned int INDEX_LENGTH;
extern unsigned int OFFSET_LENGTH;

int arg_init(int argc, char *argv[])
{
	if (argc < 7)
	{
		printf("Error: expected command: sim_cache <L1_BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L1_REPLACEMENT_POLICY> <L1_WRITE_POLICY> <trace_file>.\n");
		return 1;
	}
	char *endptr;
	L1_BLOCKSIZE = strtoul(argv[1], &endptr, 10); // 10 表示十进制
	if (*endptr != '\0' || L1_BLOCKSIZE <= 0)
	{
		printf("Error: %s is not a valid value.\n", argv[1]);
		return 1;
	}
	L1_SIZE = strtoul(argv[2], &endptr, 10);
	if (*endptr != '\0' || L1_BLOCKSIZE <= 0)
	{
		printf("Error: %s is not a valid value.\n", argv[2]);
		return 1;
	}
	L1_ASSOC = strtoul(argv[3], &endptr, 10);
	if (*endptr != '\0' || L1_BLOCKSIZE <= 0)
	{
		printf("Error: %s is not a valid value.\n", argv[3]);
		return 1;
	}
	L1_REPLACEMENT_POLICY = strtoul(argv[4], &endptr, 10);
	if (*endptr != '\0')
	{
		printf("Error: %s is not a valid value.\n", argv[4]);
		return 1;
	}
	if (L1_REPLACEMENT_POLICY != 0 && L1_REPLACEMENT_POLICY != 1)
	{
		printf("Error: L1_REPLACEMENT_POLICY should be 0(LRU) or 1(LFU).\n");
		return 1;
	}
	L1_WRITE_POLICY = strtoul(argv[5], &endptr, 10);
	if (*endptr != '\0')
	{
		printf("Error: %s is not a valid value.\n", argv[5]);
		return 1;
	}
	if (L1_WRITE_POLICY != 0 && L1_WRITE_POLICY != 1)
	{
		printf("Error: L1_WRITE_POLICY should be 0(WBWA) or 1(WTNA).\n");
		return 1;
	}
	trace_file = argv[6];
	// if (access(trace_file, F_OK) == -1)
	// {
	// 	printf("Error: file %s does not exist.\n", trace_file);
	// 	return 1;
	// }
	SET_NUM = (unsigned int)(L1_SIZE / (L1_BLOCKSIZE * L1_ASSOC));
	OFFSET_LENGTH = log2(L1_BLOCKSIZE);
	INDEX_LENGTH = log2(SET_NUM);
	TAG_LENGTH = 32 - OFFSET_LENGTH - INDEX_LENGTH;
	return 0;
}

void print_init_arg_info()
{
	printf("  ===== Simulator configuration =====\n");
	printf("  L1_BLOCKSIZE: %u\n", L1_BLOCKSIZE);
	printf("  L1_SIZE: %u\n", L1_SIZE);
	printf("  L1_ASSOC: %u\n", L1_ASSOC);
	printf("  L1_REPLACEMENT_POLICY: %u\n", L1_REPLACEMENT_POLICY);
	printf("  L1_WRITE_POLICY: %u\n", L1_WRITE_POLICY);
	const char *last_slash = strrchr(trace_file, '/'); // strrchr函数查找最后一次出现字符的指针
	if (last_slash != NULL)
	{
		// 如果找到了斜杠，打印斜杠之后的字符串
		printf("  trace_file: %s\n", last_slash + 1);
	}
	else
	{
		// 如果没有找到斜杠，意味着trace_file不包含路径，直接打印
		printf("  trace_file: %s\n", trace_file);
	}
	// printf("  trace_file: %s\n", trace_file);
	printf("  ===================================\n");
}