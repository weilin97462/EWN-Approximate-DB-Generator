#include <utility>
#include <algorithm>
#include <assert.h>
#include "perm_encode.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../config.hpp"

int perm_encode(char *permutation, int size)
{
    int position[32];
    for (int i = 0; i < size; i++)
        position[permutation[i]] = i;
    int perm_enc_val = 0;
    // set size bits to 1
    std::bitset<6> position_bit = (1 << (size)) - 1;
    for (int i = 0; i < size; i++)
    {
        int shift = position[i];
        position_bit[shift] = 0;
        perm_enc_val += ((position_bit >> shift).count()) * factorial[size - 1 - i];
    }
    return perm_enc_val;
}
// sort piece numbers to permutations
void sort_to_perm_array(char perm_array[2][6], char piece_position[2][6], std::bitset<6> piece_bits[2])
{
    std::pair<char, char> sorted_positions[2][6];
    for (int color = 0; color < 2; color++)
    {
        for (int num = 0; num < 6; num++)
        {
            char position = piece_position[color][num];
            if (position == -1)
                continue;
            char sorted_rank = sort_piece_bits[piece_bits[color].to_ulong()][num];
#ifdef ENABLE_SAFETY_CHECK
            assert((sorted_rank >= 0) && (sorted_rank <= 6));
#endif
            sorted_positions[color][sorted_rank].first = sorted_rank;
            sorted_positions[color][sorted_rank].second = position;
        }
    }
    int rnum = piece_bits[RED].count();
    int bnum = piece_bits[BLUE].count();
#ifdef ENABLE_SAFETY_CHECK
    assert((rnum <= 6) && (bnum <= 6));
#endif
    std::sort(sorted_positions[RED], sorted_positions[RED] + rnum, [](std::pair<char, char> a, std::pair<char, char> b)
              { return a.second > b.second; });
    std::sort(sorted_positions[BLUE], sorted_positions[BLUE] + bnum, [](std::pair<char, char> a, std::pair<char, char> b)
              { return a.second > b.second; });
    for (int i = 0; i < rnum; i++)
        perm_array[RED][sorted_positions[RED][i].first] = i;
    for (int i = 0; i < bnum; i++)
        perm_array[BLUE][sorted_positions[BLUE][i].first] = i;
}