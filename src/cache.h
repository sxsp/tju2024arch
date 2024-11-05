#ifndef CACHE_H
#define CACHE_H

struct cache_line
{
    unsigned int tag;
    unsigned int index;

    unsigned int SET_code;
    unsigned int LRU_count;
    unsigned int LFU_count;

    bool is_dirty;
    bool is_valid;
};

struct cache_set
{
    unsigned int start_cache_i;
    unsigned int LFU_ages;
    unsigned int LFU_min_i;
    unsigned int LRU_max_i;
};

void init_cache(cache_line *cache, cache_set *set);
void print_a_cache_line(cache_line cachei, unsigned int i);
void print_whole_cache(cache_line *cache);
unsigned int search_in_set_for_hit(cache_line *cache, unsigned int cur_tag, unsigned int start_i, bool &is_hit);
unsigned int search_in_set_for_free_line(cache_line *cache, cache_set *set, unsigned int index, bool &have_free_line);
void update_BLOCK_COUNT_for_hit(cache_line *cache, cache_set *set, unsigned int index, unsigned int hit_i);
void update_set_for_add_line(cache_line *cache, cache_set *set, unsigned int cache_i, unsigned int index, unsigned int tag);
void update_set_for_exchange_line(cache_line *cache, cache_set *set, unsigned int cache_i, unsigned int index, unsigned int tag);

#endif