#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.hpp"
#include "../combination_decoding/combination_decoding.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../tables/encode_tables/without_cut/diag_symm_tables.hpp"
#include "../../tables/decode_tables/without_cut/diag_symm_tables.hpp"
#include "../permutation_decoding/perm_decode.hpp"
#define COMB_SPACE_SIZE 23

static void calculate_inverse_map(short inv_comb_mapping[25], char *removing_positions, char len, char big_color)
{
    char need_remove[25];
    memset(need_remove, 0, 25);
    for (int i = 0; i < len; i++)
    {
        char pos = removing_positions[i];
        if (pos == -1)
            continue;
        need_remove[pos] = 1;
    }
    if (big_color == RED)
        need_remove[0] = 1;
    else
        need_remove[24] = 1;
    short count = 0;
    for (int i = 0; i < 25; i++)
    {
        if (need_remove[i] == 0)
        {
            inv_comb_mapping[count] = i;
            count++;
        }
    }
}

static void decode_comb_val(char sorted_position_list[2][6], long long combination_val, char r_num, char b_num)
{
    char big_num, small_num;
    char big_color, small_color;
    if (r_num >= b_num)
        big_num = r_num, small_num = b_num, big_color = RED, small_color = BLUE;
    else
        big_num = b_num, small_num = r_num, big_color = BLUE, small_color = RED;
    long long not_corner_num = (long long)compressed_comb_count[big_num] * C[24 - big_num][small_num];
    int small_comb_size;
    short sorted_list[6];
    short inv_comb_mapping[25];
    long long big_comb_val, small_comb_val;
    // in corner
    if (combination_val >= not_corner_num)
    {
        combination_val -= not_corner_num;
        small_comb_size = 25 - big_num;
        lldiv_t div_result = div(combination_val, (long long)C[small_comb_size][small_num]);
        big_comb_val = decompress_comb_index[big_num - 1][div_result.quot];
        small_comb_val = div_result.rem;
        decode_comb(big_comb_val, sorted_list, big_num - 1, COMB_SPACE_SIZE);
        if (big_color == RED)
        {
            for (int i = 0; i < big_num - 1; i++)
                sorted_position_list[RED][i] = sorted_list[i] + 1;
            sorted_position_list[RED][big_num - 1] = 0;
        }
        else
        {
            for (int i = 1; i < big_num; i++)
                sorted_position_list[BLUE][i] = sorted_list[i - 1] + 1;
            sorted_position_list[BLUE][0] = 24;
        }
    }
    else
    {
        small_comb_size = 24 - big_num;
        lldiv_t div_result = div(combination_val, (long long)C[small_comb_size][small_num]);
        big_comb_val = decompress_comb_index[big_num][div_result.quot];
        small_comb_val = div_result.rem;
        decode_comb((int)big_comb_val, sorted_list, big_num, COMB_SPACE_SIZE);
        for (int i = 0; i < big_num; i++)
            sorted_position_list[big_color][i] = sorted_list[i] + 1;
    }

    calculate_inverse_map(inv_comb_mapping, sorted_position_list[big_color], big_num, big_color);
    decode_comb(small_comb_val, sorted_list, small_num, small_comb_size);
    // map to position
    for (int i = 0; i < small_num; i++)
        sorted_position_list[small_color][i] = inv_comb_mapping[sorted_list[i]];
}

void decode_without_cut(char piece_position[2][6], Query q)
{
    memset(piece_position, -1, 12);
    int red_perm_num = factorial[q.r_num];
    int blue_perm_num = factorial[q.b_num];
    bool r_symm = is_equivalent_piece_set_symmetric[q.red_set_id];
    bool b_symm = is_equivalent_piece_set_symmetric[q.blue_set_id];
    if (r_symm)
        red_perm_num /= 2;
    if (b_symm)
        blue_perm_num /= 2;
    long long remain_ID = q.ID;
    lldiv_t div_result;
    div_result = div(remain_ID, (long long)blue_perm_num * red_perm_num);
    long long combination_val = div_result.quot;
    remain_ID = div_result.rem;
    div_result = div(remain_ID, (long long)blue_perm_num);
    // decode combination
    char sorted_position_list[2][6];
    memset(sorted_position_list, 0, 12);
    decode_comb_val(sorted_position_list, combination_val, q.r_num, q.b_num);
    // decode permutation
    div_result = div(remain_ID, (long long)blue_perm_num);
    int red_perm_val = div_result.quot;
    int blue_perm_val = div_result.rem;
    int permutation[6];
    perm_decode(red_perm_val, permutation, q.r_num);
    for (int i = 0; i < q.r_num; i++)
    {
        int pce_num = piece_set_to_nums[q.red_set_id][i];
        piece_position[RED][pce_num] = sorted_position_list[RED][permutation[i]];
    }
    perm_decode(blue_perm_val, permutation, q.b_num);
    for (int i = 0; i < q.b_num; i++)
    {
        int pce_num = piece_set_to_nums[q.blue_set_id][i];
        piece_position[BLUE][pce_num] = sorted_position_list[BLUE][permutation[i]];
    }
}