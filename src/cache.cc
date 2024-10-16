#include <stdio.h>
#include "cache.h"

extern unsigned int L1_BLOCKSIZE;
extern unsigned int L1_SIZE;
extern unsigned int L1_ASSOC;
extern unsigned int L1_REPLACEMENT_POLICY;
extern unsigned int L1_WRITE_POLICY;

void init_cache(cache_line *cache)
{
    for (unsigned int i = 0; i < L1_SIZE; i++)
    {
        cache[i].is_value = false;
        cache[i].is_dirty = false;
        cache[i].SET_code = i / L1_ASSOC;
        cache[i].LRU_count = i % L1_ASSOC;
    }
}

void print_a_cache_line(cache_line cachei, unsigned int i)
{
    printf("the %uth cache line:\n", i);
    printf("tag: %u\n", cachei.tag);
    printf("index: %u\n", cachei.index);
    printf("SET_code: %u\n", cachei.SET_code);
    printf("LRU_count: %u\n", cachei.LRU_count);
    printf("is_dirty: %d\n", cachei.is_dirty);
    printf("is_value: %d\n", cachei.is_value);
}
void print_whole_cache(cache_line *cache)
{
    for (unsigned int i = 0; i < L1_SIZE; i++)
    {
        print_a_cache_line(cache[i], i);
    }
}