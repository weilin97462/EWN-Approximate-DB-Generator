#include <stdlib.h>
#include <utility>
#include <algorithm>
#include "perm_decode.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"

static int count_ith_1[64][6] = {
    {-1, -1, -1, -1, -1, -1},
    {0, -1, -1, -1, -1, -1},
    {1, -1, -1, -1, -1, -1},
    {0, 1, -1, -1, -1, -1},
    {2, -1, -1, -1, -1, -1},
    {0, 2, -1, -1, -1, -1},
    {1, 2, -1, -1, -1, -1},
    {0, 1, 2, -1, -1, -1},
    {3, -1, -1, -1, -1, -1},
    {0, 3, -1, -1, -1, -1},
    {1, 3, -1, -1, -1, -1},
    {0, 1, 3, -1, -1, -1},
    {2, 3, -1, -1, -1, -1},
    {0, 2, 3, -1, -1, -1},
    {1, 2, 3, -1, -1, -1},
    {0, 1, 2, 3, -1, -1},
    {4, -1, -1, -1, -1, -1},
    {0, 4, -1, -1, -1, -1},
    {1, 4, -1, -1, -1, -1},
    {0, 1, 4, -1, -1, -1},
    {2, 4, -1, -1, -1, -1},
    {0, 2, 4, -1, -1, -1},
    {1, 2, 4, -1, -1, -1},
    {0, 1, 2, 4, -1, -1},
    {3, 4, -1, -1, -1, -1},
    {0, 3, 4, -1, -1, -1},
    {1, 3, 4, -1, -1, -1},
    {0, 1, 3, 4, -1, -1},
    {2, 3, 4, -1, -1, -1},
    {0, 2, 3, 4, -1, -1},
    {1, 2, 3, 4, -1, -1},
    {0, 1, 2, 3, 4, -1},
    {5, -1, -1, -1, -1, -1},
    {0, 5, -1, -1, -1, -1},
    {1, 5, -1, -1, -1, -1},
    {0, 1, 5, -1, -1, -1},
    {2, 5, -1, -1, -1, -1},
    {0, 2, 5, -1, -1, -1},
    {1, 2, 5, -1, -1, -1},
    {0, 1, 2, 5, -1, -1},
    {3, 5, -1, -1, -1, -1},
    {0, 3, 5, -1, -1, -1},
    {1, 3, 5, -1, -1, -1},
    {0, 1, 3, 5, -1, -1},
    {2, 3, 5, -1, -1, -1},
    {0, 2, 3, 5, -1, -1},
    {1, 2, 3, 5, -1, -1},
    {0, 1, 2, 3, 5, -1},
    {4, 5, -1, -1, -1, -1},
    {0, 4, 5, -1, -1, -1},
    {1, 4, 5, -1, -1, -1},
    {0, 1, 4, 5, -1, -1},
    {2, 4, 5, -1, -1, -1},
    {0, 2, 4, 5, -1, -1},
    {1, 2, 4, 5, -1, -1},
    {0, 1, 2, 4, 5, -1},
    {3, 4, 5, -1, -1, -1},
    {0, 3, 4, 5, -1, -1},
    {1, 3, 4, 5, -1, -1},
    {0, 1, 3, 4, 5, -1},
    {2, 3, 4, 5, -1, -1},
    {0, 2, 3, 4, 5, -1},
    {1, 2, 3, 4, 5, -1},
    {0, 1, 2, 3, 4, 5}};
// not used, using an precompute table is faster
// static int bit_rank(unsigned int rank_bit, int rank)
// {
//     int rank_count = 0;
//     for (int i = 0; i < 6; i++)
//     {
//         if ((rank_bit & (1 << i)))
//             rank_count++;
//         if (rank_count - 1 == rank)
//             return i;
//     }
//     return -1;
// }
void perm_decode(int enc_val, int *permutation, int size)
{
    int position[32];
    std::bitset<6> position_bit = (1 << (size)) - 1;
    for (char i = 0; i < size; i++)
    {
        div_t div_result = div(enc_val, factorial[size - i - 1]);
        enc_val = div_result.rem;
        position[i] = count_ith_1[position_bit.to_ulong()][size - i - 1 - div_result.quot];
        position_bit[position[i]] = 0;
    }
    for (int i = 0; i < size; i++)
        permutation[position[i]] = i;
}