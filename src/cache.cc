#include <stdio.h>
#include "cache.h"

extern unsigned int L1_BLOCKSIZE;
extern unsigned int L1_SIZE;
extern unsigned int L1_ASSOC;
extern unsigned int L1_REPLACEMENT_POLICY;
extern unsigned int L1_WRITE_POLICY;

extern unsigned SET_NUM;

void init_cache(cache_line *cache, cache_set *set)
{
    for (unsigned int i = 0; i < L1_SIZE; i++)
    {
        cache[i].is_valid = false;
        cache[i].is_dirty = false;
        cache[i].SET_code = i / L1_ASSOC;
        cache[i].LRU_count = L1_ASSOC - 1;
        cache[i].LFU_count = 0;
        cache[i].tag = 0;
    }
    for (unsigned int i = 0; i < SET_NUM; i++)
    {
        set[i].start_cache_i = i * L1_ASSOC;
        set[i].LFU_ages = 0;
        set[i].LFU_min_i = i * L1_ASSOC;
        set[i].LRU_max_i = i * L1_ASSOC;
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
    printf("is_valid: %d\n", cachei.is_valid);
}
void print_whole_cache(cache_line *cache)
{
    for (unsigned int i = 0; i < L1_SIZE; i++)
    {
        print_a_cache_line(cache[i], i);
    }
}

// 查询是否命中，若命中返回cache命中位置的下标
unsigned int search_in_set_for_hit(cache_line *cache, unsigned int cur_tag, unsigned int start_i, bool &is_hit)
{
    // 在组中查找，是否hit
    for (unsigned int i = 0; i < L1_ASSOC; i++)
    {
        if (cache[start_i + i].is_valid && cache[start_i + i].tag == cur_tag)
        {
            is_hit = true;
            return start_i + i;
        }
    }
    is_hit = false;
    return 0;
}

// 命中的情况下，更新LRU和LFU计数信息
void update_BLOCK_COUNT_for_hit(cache_line *cache, cache_set *set, unsigned int index, unsigned int hit_i)
{
    cache[hit_i].LFU_count++;
    unsigned int hit_LRU_value = cache[hit_i].LRU_count;
    cache[hit_i].LRU_count = 0;
    for (unsigned int i = set[index].start_cache_i; i < set[index].start_cache_i + L1_ASSOC; i++)
    {
        // // 更新组内最小LFU块的位置
        // if (cache[i].is_valid && (cache[set[index].LFU_min_i].LFU_count > cache[i].LFU_count))
        // {
        //     set[index].LFU_min_i = i;
        // }

        // 更新各个块的LRU
        if (i == hit_i || !cache[i].is_valid)
        {
            continue;
        }
        if (cache[i].LRU_count < hit_LRU_value)
        {
            cache[i].LRU_count++;
        }

        // 更新组内最大LRU块的位置
        if (cache[set[index].LRU_max_i].LRU_count < cache[i].LRU_count)
        {
            set[index].LRU_max_i = i;
        }
    }
}

// 寻找是否有空行，若有返回第一个空行的下标，若没有返回替换行的下标
unsigned int search_in_set_for_free_line(cache_line *cache, cache_set *set, unsigned int index, bool &have_free_line)
{
    for (unsigned int i = set[index].start_cache_i; i < set[index].start_cache_i + L1_ASSOC; i++)
    {
        if (!cache[i].is_valid)
        {
            have_free_line = true;
            return i;
        }
    }
    have_free_line = false;
    // return L1_REPLACEMENT_POLICY == 0 ? set[index].LRU_max_i : set[index].LFU_min_i;
    if (L1_REPLACEMENT_POLICY == 0)
    {
        return set[index].LRU_max_i;
    }
    unsigned int ans = set[index].start_cache_i;
    for (unsigned int i = set[index].start_cache_i; i < set[index].start_cache_i + L1_ASSOC; i++)
    {
        if (cache[i].LFU_count < cache[ans].LFU_count)
        {
            ans = i;
        }
    }
    return ans;
}

// 调入新行，更新组
void update_set_for_add_line(cache_line *cache, cache_set *set, unsigned int cache_i, unsigned int index, unsigned int tag)
{
    cache[cache_i].is_valid = true;
    cache[cache_i].tag = tag;
    // 更新LFU
    set[index].LFU_ages = cache[cache_i].LFU_count;
    cache[cache_i].LFU_count = set[index].LFU_ages + 1;

    // 更新LRU
    unsigned int cache_i_LRU_value = cache[cache_i].LRU_count;
    set[index].LRU_max_i = set[index].start_cache_i;
    cache[cache_i].LRU_count = 0;
    for (unsigned int i = set[index].start_cache_i; i < set[index].start_cache_i + L1_ASSOC; i++)
    {
        // // 更新组内最小LFU块的位置
        // if (cache[i].is_valid && (cache[set[index].LFU_min_i].LFU_count > cache[i].LFU_count))
        // {
        //     set[index].LFU_min_i = i;
        // }
        if (i == cache_i || !cache[i].is_valid)
        {
            continue;
        }
        if (cache[i].LRU_count < cache_i_LRU_value)
        {
            cache[i].LRU_count++;
        }
        // 更新组内最大LRU块的位置
        if (cache[set[index].LRU_max_i].LRU_count < cache[i].LRU_count)
        {
            set[index].LRU_max_i = i;
        }
    }
}

// 替换行，更新组
void update_set_for_exchange_line(cache_line *cache, cache_set *set, unsigned int cache_i, unsigned int index, unsigned int tag)
{

    // cache[cache_i].is_valid = true;
    cache[cache_i].tag = tag;

    // 更新组的LRU_age信息
    set[index].LFU_ages = cache[cache_i].LFU_count;
    // 更新LFU
    cache[cache_i].LFU_count = set[index].LFU_ages + 1;

    // 更新LRU
    unsigned int cache_i_LRU_value = cache[cache_i].LRU_count;
    cache[cache_i].LRU_count = 0;
    set[index].LRU_max_i = set[index].start_cache_i;
    for (unsigned int i = set[index].start_cache_i; i < set[index].start_cache_i + L1_ASSOC; i++)
    {
        // // 更新组内最小LFU块的位置
        // if (cache[i].is_valid && (cache[set[index].LFU_min_i].LFU_count > cache[i].LFU_count))
        // {
        //     set[index].LFU_min_i = i;
        // }
        if (i == cache_i || !cache[i].is_valid)
        {
            continue;
        }
        if (cache[i].LRU_count < cache_i_LRU_value)
        {
            cache[i].LRU_count++;
        }
        // 更新组内最大LRU块的位置
        if (cache[set[index].LRU_max_i].LRU_count < cache[i].LRU_count)
        {
            set[index].LRU_max_i = i;
        }
    }
}