#include <algorithm>
#include <functional>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "encode.hpp"
#include "../permutation_encoding/perm_encode.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../tables/encode_tables/with_cut/diag_symm_tables.hpp"
#include "../combination_encoding/combination_encoding.hpp"
#include "../../config.hpp"

#ifdef ENABLE_SAFETY_CHECK
#include <stdexcept>
#endif
static void diag_flip_piecepos(char piece_position[2][6])
{
    for (char color = 0; color < 2; color++)
        for (char num = 0; num < 6; num++)
            if (piece_position[color][num] >= 0)
                piece_position[color][num] = flip[piece_position[color][num]];
}
// calculate small comb mapping
static void remap_comb(short comb_mapping[25], char *removing_positions, char big_side)
{
    char need_remove[25] = {0, 0, 0, -1, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, -1, 0, 0, 0};
    for (int i = 0; i < 6; i++)
    {
        char pos = removing_positions[i];
        if (pos == -1)
            continue;
        need_remove[pos] = 1;
    }
    if (big_side == RED)
        need_remove[0] = 1;
    else
        need_remove[24] = 1;
    short count = 0;
    for (int i = 0; i < 25; i++)
    {
        if (need_remove[i] == 0)
        {
            comb_mapping[i] = count;
            count++;
        }
    }
}
// return the length of the sorted list
static bool calculate_sorted_list(char *piece_posisition_color, short *sorted_list, short *comb_mapping)
{
    bool in_corner = false;
    int piece_count = 0;
    for (short pce_num = 0; pce_num < 6; pce_num++)
    {
        char position = piece_posisition_color[pce_num];
        if (position == -1)
            continue;
        short mapped_value = comb_mapping[position];
        if (mapped_value == -1)
        {
            in_corner = true;
            continue;
        }
        sorted_list[piece_count] = mapped_value;
        piece_count++;
    }
    std::sort(sorted_list, sorted_list + piece_count, std::greater());
    return in_corner;
}
// calculate the combination encode value
//  -1  0  1 -1 -1
//   2  3  4  5 -1
//   6  7  8  9 10
//  -1 11 12 13 14
//  -1 -1 15 16 -1
static long long combination_encode_val(char piece_position[2][6], int big_num, int small_num, char big_side)
{
    long long comb_enc_val = 0;
    // encode big side
    short sorted_list[6];
    short comb_mapping[25] = {-1, 0, 1, -2, -2, 2, 3, 4, 5, -2, 6, 7, 8, 9, 10, -2, 11, 12, 13, 14, -2, -2, 15, 16, -1};
    bool in_corner = calculate_sorted_list(piece_position[big_side], sorted_list, comb_mapping);
    long long big_enc_val, big_compressed_enc_val;
    int small_comb_size;
    if (in_corner)
    {
        big_enc_val = encode_comb(sorted_list, big_num - 1);
        big_compressed_enc_val = (long long)convert_to_compressed_cut_comb_index[big_num - 1][big_enc_val];
        if (big_compressed_enc_val == -1)
        {
            diag_flip_piecepos(piece_position);
            calculate_sorted_list(piece_position[big_side], sorted_list, comb_mapping);
            big_enc_val = encode_comb(sorted_list, big_num - 1);
            big_compressed_enc_val = (long long)convert_to_compressed_cut_comb_index[big_num - 1][big_enc_val];
        }
#ifdef ENABLE_SAFETY_CHECK
        assert((big_compressed_enc_val != -1));
#endif
        comb_enc_val = (long long)compressed_cut_comb_count[big_num] * C[18 - big_num][small_num];
        small_comb_size = 19 - big_num;
        comb_enc_val += big_compressed_enc_val * C[small_comb_size][small_num];
    }
    else
    {
        big_enc_val = encode_comb(sorted_list, big_num);
        big_compressed_enc_val = (long long)convert_to_compressed_cut_comb_index[big_num][big_enc_val];
        if (big_compressed_enc_val == -1)
        {
            diag_flip_piecepos(piece_position);
            calculate_sorted_list(piece_position[big_side], sorted_list, comb_mapping);
            big_enc_val = encode_comb(sorted_list, big_num);
            big_compressed_enc_val = (long long)convert_to_compressed_cut_comb_index[big_num][big_enc_val];
        }
#ifdef ENABLE_SAFETY_CHECK
        assert((big_compressed_enc_val != -1));
#endif
        small_comb_size = 18 - big_num;
        comb_enc_val = (long long)convert_to_compressed_cut_comb_index[big_num][big_enc_val] * C[small_comb_size][small_num];
    }
    // encode small side
    char small_side = big_side ^ 1;
    remap_comb(comb_mapping, piece_position[big_side], big_side);
    calculate_sorted_list(piece_position[small_side], sorted_list, comb_mapping);
    long long small_enc_val = encode_comb(sorted_list, small_num);
    comb_enc_val += small_enc_val;
    return comb_enc_val;
}

void encode_with_cut(char piece_position[2][6], std::bitset<6> piece_bits[2], Query &q)
{
#ifdef ENABLE_SAFETY_CHECK
    bool is_cut[25] = {0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0};
    for (int color = 0; color < 2; color++)
    {
        for (int num = 0; num < 6; num++)
        {
            char pos = piece_position[color][num];
            if ((pos != -1) && (is_cut[pos]))
                throw std::runtime_error("Some pieces are in cut positions!");
        }
    }
#endif
    // acquire combination value
    long long combination_val;
    if (q.r_num >= q.b_num)
        combination_val = combination_encode_val(piece_position, q.r_num, q.b_num, RED);
    else
        combination_val = combination_encode_val(piece_position, q.b_num, q.r_num, BLUE);

    // acquire permutation value
    q.red_set_id = to_equivalent_piece_set[piece_bits[0].to_ulong()];
    q.blue_set_id = to_equivalent_piece_set[piece_bits[1].to_ulong()];
    char perm_array[2][6];
    sort_to_perm_array(perm_array, piece_position, piece_bits);
    int red_perm_num = factorial[q.r_num];
    int blue_perm_num = factorial[q.b_num];
    bool r_symm = is_equivalent_piece_set_symmetric[q.red_set_id];
    bool b_symm = is_equivalent_piece_set_symmetric[q.blue_set_id];
    int red_perm_enc_val = perm_encode(perm_array[RED], q.r_num);
    if (r_symm)
    {
        red_perm_num /= 2;
        if (red_perm_enc_val >= red_perm_num)
            red_perm_enc_val = (factorial[q.r_num] - red_perm_enc_val - 1);
    }
    int blue_perm_enc_val = perm_encode(perm_array[BLUE], q.b_num);
    if (b_symm)
    {
        blue_perm_num /= 2;
        if (blue_perm_enc_val >= blue_perm_num)
            blue_perm_enc_val = (factorial[q.b_num] - blue_perm_enc_val - 1);
    }
    q.ID = (combination_val * red_perm_num * blue_perm_num + red_perm_enc_val * blue_perm_num + blue_perm_enc_val);
}