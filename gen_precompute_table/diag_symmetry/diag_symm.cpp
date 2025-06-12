#include <algorithm>
#include <functional>
#include <stdio.h>
#include "../combination_encoding/combination_encoding.hpp"
#include "diag_symm.hpp"
#include "../config.hpp"
// [length][index]
int convert_to_compressed_comb_index[7][100947] = {};
int decompress_comb_index[7][100947] = {};
int compressed_comb_count[7] = {};

void gen_diag_symm_tables()
{
    int mirror[POOL_SIZE] = {4, 9, 14, 19, 0, 5, 10, 15, 20, 1, 6, 11, 16, 21, 2, 7, 12, 17, 22, 3, 8, 13, 18};
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        int count = 0;
        for (int big_id = 0; big_id < C[POOL_SIZE][big_num]; big_id++)
        {
            short sorted_list[6];
            short sorted_list_mirror[6];
            decode_comb(big_id, sorted_list, big_num, POOL_SIZE);
            for (int i = 0; i < big_num; i++)
                sorted_list_mirror[i] = mirror[sorted_list[i]];
            std::sort(sorted_list_mirror, sorted_list_mirror + big_num, std::greater());
            int mirror_id = encode_comb(sorted_list_mirror, big_num);
            if (mirror_id <= big_id)
            {
                convert_to_compressed_comb_index[big_num][mirror_id] = -1;
                convert_to_compressed_comb_index[big_num][big_id] = count;
                decompress_comb_index[big_num][count] = big_id;
                count++;
            }
        }
        compressed_comb_count[big_num] = count;
    }
}

// [length][index]
int convert_to_compressed_cut_comb_index[7][12376] = {};
int decompress_cut_comb_index[7][12376] = {};
int compressed_cut_comb_count[7] = {};

//     0  1
//  2  3  4  5
//  6  7  8  9 10
//    11 12 13 14
//       15 16

void gen_diag_symm_tables_cut()
{
    int mirror[POOL_SIZE_CUT] = {2, 6, 0, 3, 7, 11, 1, 4, 8, 12, 15, 5, 9, 13, 16, 10, 14};
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        int count = 0;
        for (int big_id = 0; big_id < C[POOL_SIZE_CUT][big_num]; big_id++)
        {
            short sorted_list[6];
            short sorted_list_mirror[6];
            decode_comb(big_id, sorted_list, big_num, POOL_SIZE_CUT);
            for (int i = 0; i < big_num; i++)
                sorted_list_mirror[i] = mirror[sorted_list[i]];
            std::sort(sorted_list_mirror, sorted_list_mirror + big_num, std::greater());
            int mirror_id = encode_comb(sorted_list_mirror, big_num);
            if (mirror_id <= big_id)
            {
                convert_to_compressed_cut_comb_index[big_num][mirror_id] = -1;
                convert_to_compressed_cut_comb_index[big_num][big_id] = count;
                decompress_cut_comb_index[big_num][count] = big_id;
                count++;
            }
        }
        compressed_cut_comb_count[big_num] = count;
    }
}