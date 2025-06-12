#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <functional>

#include "decode.hpp"
#include "../combination_decoding/combination_decoding.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../tables/encode_tables/with_sorted_md/md_encode_table.hpp"
#include "../../tables/decode_tables/with_sorted_md/md_decode_table.hpp"
#include "../permutation_decoding/perm_decode.hpp"
static int map_to_original_board_index[2][18] = {
    {23, 19, 22, 18, 14, 17, 13, 16, 12, 8, 11, 7, 10, 6, 2, 5, 1, 0},
    {1, 5, 2, 6, 10, 7, 11, 8, 12, 16, 13, 17, 14, 18, 22, 19, 23, 24}};

static void calculate_inverse_map(short inv_comb_mapping[18], short *big_side_sorted_list, char big_num, char small_side)
{
    // O(n), which is very fast!
    char need_remove[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // the piece is in corner, no need to subtract
    if (big_side_sorted_list[0] < 17)
    {
        need_remove[16 - big_side_sorted_list[0]] = 1;
    }
    for (int i = 1; i < big_num; i++)
    {
        need_remove[16 - big_side_sorted_list[i]] = 1;
    }
    for (int i = 0, shift = 0; i < 18; i++)
    {
        if (need_remove[i] == 1)
            shift++;
        else
            inv_comb_mapping[i - shift] = map_to_original_board_index[small_side][i];
    }
}

void decode_with_sorted_md(char piece_position[2][6], Query q)
{
    memset(piece_position, -1, 12);
    // get red blue number
    char red_num = q.r_num;
    char blue_num = q.b_num;

    // calculate permutation number
    int red_perm_num = factorial[red_num];
    int blue_perm_num = factorial[blue_num];
    bool r_symm = is_equivalent_piece_set_symmetric[q.red_set_id];
    bool b_symm = is_equivalent_piece_set_symmetric[q.blue_set_id];
    if (r_symm)
        red_perm_num /= 2;
    if (b_symm)
        blue_perm_num /= 2;

    char big_side, small_side;
    char big_num, small_num;
    if (red_num >= blue_num)
    {
        big_side = RED;
        small_side = BLUE;
        big_num = red_num;
        small_num = blue_num;
    }
    else
    {
        big_side = BLUE;
        small_side = RED;
        big_num = blue_num;
        small_num = red_num;
    }
    // first perform binary search in md_to_index
    // to find big_md and small_md
    long long comb_index, perm_index;
    auto div_result = div(q.ID, (long long)red_perm_num * blue_perm_num);
    comb_index = div_result.quot;
    perm_index = div_result.rem;
    int min_small_md = md_range[small_num][0];
    int big_md;
    int left = md_range[big_num][0], mid, right = md_range[big_num][1];
    // first find big_md
    if (comb_index >= md_to_index[big_num - 5][small_num - 3][right][min_small_md])
    {
        big_md = right;
    }
    else
    {
        while (1)
        {
            mid = (left + right) / 2;
            if (md_to_index[big_num - 5][small_num - 3][mid + 1][min_small_md] <= comb_index)
            {
                left = mid + 1;
            }
            else if (comb_index < md_to_index[big_num - 5][small_num - 3][mid][min_small_md])
            {
                right = mid;
            }
            else
            {
                break;
            }
        }
        big_md = mid;
    }
    // then find small_md
    int small_md;
    left = min_small_md, right = md_range[small_num][1];
    if (comb_index >= md_to_index[big_num - 5][small_num - 3][big_md][right])
    {
        small_md = right;
    }
    else
    {
        while (1)
        {
            mid = (left + right) / 2;
            if (md_to_index[big_num - 5][small_num - 3][big_md][mid + 1] <= comb_index)
            {
                left = mid + 1;
            }
            else if (comb_index < md_to_index[big_num - 5][small_num - 3][big_md][mid])
            {
                right = mid;
            }
            else
            {
                break;
            }
        }
        small_md = mid;
    }
    // find big_compresssed_id and small_side_raw_id
    int big_compressed_id, small_side_raw_id;
    left = big_side_start_by_manhattan_distance[big_num - 5][big_md];
    right = big_side_start_by_manhattan_distance[big_num - 5][big_md + 1] - 1;
    if (comb_index >= big_side_to_index[big_num - 5][small_num - 3][right][small_md])
    {
        big_compressed_id = right;
    }
    else
    {
        while (1)
        {
            mid = (left + right) / 2;
            if (big_side_to_index[big_num - 5][small_num - 3][mid + 1][small_md] <= comb_index)
            {
                left = mid + 1;
            }
            else if (comb_index < big_side_to_index[big_num - 5][small_num - 3][mid][small_md])
            {
                right = mid;
            }
            else
            {
                break;
            }
        }
        big_compressed_id = mid;
    }
    small_side_raw_id = comb_index - big_side_to_index[big_num - 5][small_num - 3][big_compressed_id][small_md];
    // then calculate sorted_position_list
    char sorted_position_list[2][6];
    // then decompress the big_compress_id
    int big_comb_id = decompress_big_side[big_num - 5][big_compressed_id];
    short sorted_list[6];
    decode_comb(big_comb_id, sorted_list, big_num, 18);
    for (int i = 0; i < big_num; i++)
    {
        sorted_position_list[big_side][i] = map_to_original_board_index[big_side][sorted_list[i]];
    }
    std::sort(sorted_position_list[big_side], sorted_position_list[big_side] + big_num, std::greater());
    // and convert small_raw_id to small_comb_id
    short inv_comb_mapping[18];
    calculate_inverse_map(inv_comb_mapping, sorted_list, big_num, small_side);
    int small_comb_id = index_to_small_side_comb[big_side_to_subtraction_type[big_num - 5][big_compressed_id]][small_num - 3][small_md][small_side_raw_id];
    decode_comb(small_comb_id, sorted_list, small_num, 19 - big_num);
    for (int i = 0; i < small_num; i++)
    {
        sorted_position_list[small_side][i] = inv_comb_mapping[sorted_list[i]];
    }
    std::sort(sorted_position_list[small_side], sorted_position_list[small_side] + small_num, std::greater());

    // lehmer decode: restore the permutation
    div_result = div(perm_index, (long long)blue_perm_num);
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