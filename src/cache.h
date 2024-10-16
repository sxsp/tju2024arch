#ifndef CACHE_H
#define CACHE_H

struct cache_line
{
    unsigned int tag;
    unsigned int index;

    unsigned int SET_code;
    unsigned int LRU_count;
    bool is_dirty;
    bool is_value;
};

void init_cache(cache_line *cache);
void print_a_cache_line(cache_line cachei, unsigned int i);
void print_whole_cache(cache_line *cache);

#endif