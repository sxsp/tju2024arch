#include "utils.h"
#include <stdio.h>

extern unsigned int L1_SIZE;
extern unsigned int TAG_LENGTH;
extern unsigned int INDEX_LENGTH;
extern unsigned int OFFSET_LENGTH;

unsigned int log2(unsigned int a)
{
    int count = 0;
    while (1)
    {
        if (a & 1)
        {
            return count;
        }
        a >>= 1;
        count++;
    }
}
unsigned int nmask(unsigned int a)
{
    unsigned int mask = 0;
    while (a--)
    {
        mask = (mask << 1) + 1;
    }
    return mask;
}

void get_tag_index_offset(unsigned int a, unsigned int &tag, unsigned int &index, unsigned int &offset)
{
    offset = a & nmask(OFFSET_LENGTH);
    a >>= OFFSET_LENGTH;
    index = a & nmask(INDEX_LENGTH);
    a >>= INDEX_LENGTH;
    tag = a & nmask(TAG_LENGTH);
}