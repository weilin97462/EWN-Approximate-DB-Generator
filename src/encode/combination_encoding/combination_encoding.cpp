#include "combination_encoding.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
int encode_comb(short *sorted_list, int length)
{
    int ret = 0;
    for (int i = 0; i < length; i++)
    {
        ret += C[sorted_list[i]][length - i];
    }
    return ret;
}