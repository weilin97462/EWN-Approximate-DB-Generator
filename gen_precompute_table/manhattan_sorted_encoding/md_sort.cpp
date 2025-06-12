// do 9~12 encoding
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "../combination_encoding/combination_encoding.hpp"
#define RMAX 18564
#define R_COMPRESS_MAX 9366

// no need to write
short hash_to_subtraction_type[16384];
short subtraction_type_to_hash[1018];
short subtraction_count[3];
short subtraction_count_cdf[3];
short index_to_small_side_comb_counter[1018][4][41] = {};
static short encode_subtraction_type(short md_numbers[7])
{
    int base = 1;
    short encode_val = 0;
    for (int i = 6; i >= 0; i--)
    {
        encode_val += md_numbers[i] * base;
        base *= 4;
    }
    return encode_val;
}
static void decode_subtraction_type(short subtraction_type_id, short md_numbers[7])
{
    for (int i = 6; i >= 0; i--)
    {
        md_numbers[i] = subtraction_type_id % 4;
        subtraction_type_id /= 4;
    }
    return;
}
static short big_side_to_subtraction_type_func(short big_side_id, short len)
{
    short sorted_list[6];
    static short md[18] = {1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8};
    short md_count[7] = {2, 3, 2, 3, 2, 3, 2};
    decode_comb(big_side_id, sorted_list, len, 19);
    // the md of final subtraction type is in blue perspective
    for (int i = 0; i < len; i++)
    {
        if (md[sorted_list[i]] != 8)
        {
            md_count[md[sorted_list[i]] - 1]--;
        }
    }
    return encode_subtraction_type(md_count);
}
// compress_big_side[piece_num-5][comb_id]=compress_id
// sort by manhattan distance
short compress_big_side[2][18564] = {};
// decompress_big_side[piece_num-5][compress_id]=bigger_comb_id
short decompress_big_side[2][9366] = {};
// md_range[piece_num]={min,max}
short md_range[7][2] = {};
// list_big_side_by_manhattan_distance[piece_num-5][manhattan_distance]={id1,id2,id3.....}
short big_side_start_by_manhattan_distance[2][42] = {};
// md_to_index
int md_to_index[2][4][41][41] = {};
int md_size[2][4][41][41] = {};
// big_side_to_index[big_piece_num-5][small_piece_num-3][compress_id][small_manhattan_distance-8]
int big_side_to_index[2][4][9366][41] = {};
// big_side_to_subtraction_type[big_piece_num-5][big_side_id]
short big_side_to_subtraction_type[2][9366];
// small_side_comb_to_index[subtraction_type][small_comb_id]
// sort by manhattan distance
short small_side_comb_to_index[1018][4][2002];
// index_to_small_side_comb[subtraction_type][small_pce_num-3][small_manhattan_distance][index]
short index_to_small_side_comb[1018][4][41][242];

// encode:
//  big_side_id=compress_big_side[piece_num-5][big_comb]
//  comb_id=big_side_to_index[big_piece_num-5][small_piece_num-3][big_side_id][small_md]+small_side_comb_to_index[big_side_to_subtraction_type[big_piece_num-5][big_side_id]][small_comb_id]
//  comb_id*a!b!+perm_id
FILE *encode_table = fopen("src/tables/encode_tables/with_sorted_md/md_encode_table.cpp", "w+");
FILE *encode_header = fopen("src/tables/encode_tables/with_sorted_md/md_encode_table.hpp", "w+");
FILE *decode_table = fopen("src/tables/decode_tables/with_sorted_md/md_decode_table.cpp", "w+");
FILE *decode_header = fopen("src/tables/decode_tables/with_sorted_md/md_decode_table.hpp", "w+");
FILE *DB_table = fopen("src/tables/DB_tables/md_size_table.cpp", "w+");
FILE *DB_header = fopen("src/tables/DB_tables/md_size_table.hpp", "w+");
void build_md_tables()
{
    short mirror[18] = {1, 0, 4, 3, 2, 6, 5, 9, 8, 7, 11, 10, 14, 13, 12, 16, 15, 17};
    short md[18] = {1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8};
    short count = 0;
    short md_count[41] = {};

    // build subtraction_type
    // sum from 11~13
    // calculate subtraction_count first
    short md_numbers[7];
    for (md_numbers[0] = 0; md_numbers[0] < 3; md_numbers[0]++)
    {
        for (md_numbers[1] = 0; md_numbers[1] < 4; md_numbers[1]++)
        {
            for (md_numbers[2] = 0; md_numbers[2] < 3; md_numbers[2]++)
            {
                for (md_numbers[3] = 0; md_numbers[3] < 4; md_numbers[3]++)
                {
                    for (md_numbers[4] = 0; md_numbers[4] < 3; md_numbers[4]++)
                    {
                        for (md_numbers[5] = 0; md_numbers[5] < 4; md_numbers[5]++)
                        {
                            for (md_numbers[6] = 0; md_numbers[6] < 3; md_numbers[6]++)
                            {
                                short sum = md_numbers[0] + md_numbers[1] + md_numbers[2] + md_numbers[3] + md_numbers[4] + md_numbers[5] + md_numbers[6];
                                if (11 <= sum && sum <= 13)
                                {
                                    subtraction_count[sum - 11]++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // calculate subtraction_count_cdf
    for (int i = 0; i < 2; i++)
    {
        subtraction_count_cdf[i + 1] = subtraction_count_cdf[i] + subtraction_count[i];
        subtraction_count[i] = 0;
    }
    subtraction_count[2] = 0;
    for (md_numbers[0] = 0; md_numbers[0] < 3; md_numbers[0]++)
    {
        for (md_numbers[1] = 0; md_numbers[1] < 4; md_numbers[1]++)
        {
            for (md_numbers[2] = 0; md_numbers[2] < 3; md_numbers[2]++)
            {
                for (md_numbers[3] = 0; md_numbers[3] < 4; md_numbers[3]++)
                {
                    for (md_numbers[4] = 0; md_numbers[4] < 3; md_numbers[4]++)
                    {
                        for (md_numbers[5] = 0; md_numbers[5] < 4; md_numbers[5]++)
                        {
                            for (md_numbers[6] = 0; md_numbers[6] < 3; md_numbers[6]++)
                            {
                                short sum = md_numbers[0] + md_numbers[1] + md_numbers[2] + md_numbers[3] + md_numbers[4] + md_numbers[5] + md_numbers[6];
                                if (11 <= sum && sum <= 13)
                                {
                                    short hash_val = encode_subtraction_type(md_numbers);
                                    short type = subtraction_count_cdf[sum - 11] + subtraction_count[sum - 11];
                                    hash_to_subtraction_type[hash_val] = type;
                                    subtraction_type_to_hash[type] = hash_val;
                                    subtraction_count[sum - 11]++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // calculate compress_big_side & decompress_big_side & big_side_to_subtraction_type
    for (short big_piece_num = 5; big_piece_num <= 6; big_piece_num++)
    {
        for (int i = 0; i < 41; i++)
        {
            md_count[i] = 0;
        }
        for (short red_id = 0; red_id < C[18][big_piece_num]; red_id++)
        {
            short sorted_list[6];
            short sorted_list_mirror[6];
            decode_comb(red_id, sorted_list, big_piece_num, 19);
            for (short i = 0; i < big_piece_num; i++)
            {
                sorted_list_mirror[i] = mirror[sorted_list[i]];
            }
            std::sort(sorted_list_mirror, sorted_list_mirror + big_piece_num, [](int a, int b)
                      { return a > b; });
            if (encode_comb(sorted_list_mirror, big_piece_num) <= red_id)
            {
                count++;
                short md_sum = 0;
                for (short i = 0; i < big_piece_num; i++)
                {
                    md_sum += md[sorted_list[i]];
                }
                md_count[md_sum]++;
            }
        }
        for (int i = 0; i < 41; i++)
        {
            big_side_start_by_manhattan_distance[big_piece_num - 5][i + 1] = big_side_start_by_manhattan_distance[big_piece_num - 5][i] + md_count[i];
        }
        for (int i = 0; i < 41; i++)
        {
            if (md_count[i] == 0 && big_side_start_by_manhattan_distance[big_piece_num - 5][i] == 0)
            {
                big_side_start_by_manhattan_distance[big_piece_num - 5][i] = -1;
            }
            else if (md_count[i] == 0 && big_side_start_by_manhattan_distance[big_piece_num - 5][i] != 0)
            {
                big_side_start_by_manhattan_distance[big_piece_num - 5][i + 1] = -1;
            }
            md_count[i] = 0;
        }
        md_count[40] = 0;
        for (short red_id = 0; red_id < C[18][big_piece_num]; red_id++)
        {
            short sorted_list[big_piece_num];
            short sorted_list_mirror[big_piece_num];
            decode_comb(red_id, sorted_list, big_piece_num, 19);
            for (short i = 0; i < big_piece_num; i++)
            {
                sorted_list_mirror[i] = mirror[sorted_list[i]];
            }
            std::sort(sorted_list_mirror, sorted_list_mirror + big_piece_num, [](int a, int b)
                      { return a > b; });
            short mirror_id = encode_comb(sorted_list_mirror, big_piece_num);
            if (mirror_id <= red_id)
            {
                short md_sum = 0;
                for (short i = 0; i < big_piece_num; i++)
                {
                    md_sum += md[sorted_list[i]];
                }
                short compressed_id = big_side_start_by_manhattan_distance[big_piece_num - 5][md_sum] + md_count[md_sum];
                md_count[md_sum]++;
                compress_big_side[big_piece_num - 5][mirror_id] = -1;
                compress_big_side[big_piece_num - 5][red_id] = compressed_id;
                decompress_big_side[big_piece_num - 5][compressed_id] = red_id;
                big_side_to_subtraction_type[big_piece_num - 5][compressed_id] = hash_to_subtraction_type[big_side_to_subtraction_type_func(red_id, big_piece_num)];
            }
        }
    }

    // calculate index_to_small_side_comb allocation & allocate small_side_comb_to_index
    for (short sub_type = 0; sub_type < 1018; sub_type++)
    {
        short subtraction_type_hash = subtraction_type_to_hash[sub_type];
        short md_numbers_local[7];
        decode_subtraction_type(subtraction_type_hash, md_numbers_local);
        short subtracted_md[18];
        short subtracted_md_index = 0;
        for (int i = 6; i >= 0; i--)
        {
            for (int j = 0; j < md_numbers_local[i]; j++)
            {
                subtracted_md[subtracted_md_index] = 7 - i;
                subtracted_md_index++;
            }
        }
        subtracted_md[subtracted_md_index] = 8;
        short remain_pce = subtracted_md_index + 1;
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (remain_pce + small_piece_num > 19)
                continue;
            for (short small_id = 0; small_id < C[remain_pce][small_piece_num]; small_id++)
            {
                short sorted_list[6];
                decode_comb(small_id, sorted_list, small_piece_num, 19);
                short md_sum = 0;
                for (short i = 0; i < small_piece_num; i++)
                {
                    md_sum += subtracted_md[sorted_list[i]];
                }
                index_to_small_side_comb_counter[sub_type][small_piece_num - 3][md_sum]++;
            }
        }
    }
    // allocate index_to_small_side_comb
    for (short sub_type = 0; sub_type < 1018; sub_type++)
    {
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            for (short small_md = 0; small_md < 41; small_md++)
            {
                index_to_small_side_comb_counter[sub_type][small_piece_num - 3][small_md] = 0;
            }
        }
    }
    // calculate index_to_small_side_comb & small_side_comb_to_index
    for (short sub_type = 0; sub_type < 1018; sub_type++)
    {
        short subtraction_type_hash = subtraction_type_to_hash[sub_type];
        short md_numbers_local[7];
        decode_subtraction_type(subtraction_type_hash, md_numbers_local);
        short subtracted_md[18];
        short subtracted_md_index = 0;
        for (int i = 6; i >= 0; i--)
        {
            for (int j = 0; j < md_numbers_local[i]; j++)
            {
                subtracted_md[subtracted_md_index] = 7 - i;
                subtracted_md_index++;
            }
        }
        subtracted_md[subtracted_md_index] = 8;
        short remain_pce = subtracted_md_index + 1;
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (remain_pce + small_piece_num > 19)
                continue;
            for (short small_id = 0; small_id < C[remain_pce][small_piece_num]; small_id++)
            {

                short sorted_list[6];
                decode_comb(small_id, sorted_list, small_piece_num, 19);
                short md_sum = 0;
                for (short i = 0; i < small_piece_num; i++)
                {
                    md_sum += subtracted_md[sorted_list[i]];
                }
                small_side_comb_to_index[sub_type][small_piece_num - 3][small_id] = index_to_small_side_comb_counter[sub_type][small_piece_num - 3][md_sum];
                index_to_small_side_comb[sub_type][small_piece_num - 3][md_sum][index_to_small_side_comb_counter[sub_type][small_piece_num - 3][md_sum]] = small_id;
                index_to_small_side_comb_counter[sub_type][small_piece_num - 3][md_sum]++;
            }
        }
    }

    // calculate md_range
    for (short md = 1; md <= 6; md++)
    {
        short pos_to_md[18] = {1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8};
        short min_md_sum = 0, max_md_sum = 0;
        short max_md_id = 17;
        for (short i = 0; i < md; i++)
        {
            min_md_sum += pos_to_md[i];
            max_md_sum += pos_to_md[max_md_id];
            max_md_id--;
        }
        md_range[md][0] = min_md_sum;
        md_range[md][1] = max_md_sum;
    }

    // calculate md_size & md_to_index & big_side_to_index
    // calculate md_size
    for (short big_piece_num = 5; big_piece_num <= 6; big_piece_num++)
    {
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (big_piece_num + small_piece_num < 9 || small_piece_num > big_piece_num)
                continue;
            for (short big_md = md_range[big_piece_num][0]; big_md <= md_range[big_piece_num][1]; big_md++)
            {
                for (short small_md = md_range[small_piece_num][0]; small_md <= md_range[small_piece_num][1]; small_md++)
                {
                    short start = big_side_start_by_manhattan_distance[big_piece_num - 5][big_md];
                    short end = big_side_start_by_manhattan_distance[big_piece_num - 5][big_md + 1];
                    for (short big_side_id = start; big_side_id < end; big_side_id++)
                    {
                        md_size[big_piece_num - 5][small_piece_num - 3][big_md][small_md] += index_to_small_side_comb_counter[big_side_to_subtraction_type[big_piece_num - 5][big_side_id]][small_piece_num - 3][small_md];
                    }
                }
            }
        }
    }
    // calculate md_to_index
    for (short big_piece_num = 5; big_piece_num <= 6; big_piece_num++)
    {
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (big_piece_num + small_piece_num < 9 || small_piece_num > big_piece_num)
                continue;
            int sum = 0;
            for (short big_md = md_range[big_piece_num][0]; big_md <= md_range[big_piece_num][1]; big_md++)
            {
                for (short small_md = md_range[small_piece_num][0]; small_md <= md_range[small_piece_num][1]; small_md++)
                {
                    md_to_index[big_piece_num - 5][small_piece_num - 3][big_md][small_md] = sum;
                    sum += md_size[big_piece_num - 5][small_piece_num - 3][big_md][small_md];
                }
            }
        }
    }
    // calculate big_side_to_index
    for (short big_piece_num = 5; big_piece_num <= 6; big_piece_num++)
    {
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (big_piece_num + small_piece_num < 9 || small_piece_num > big_piece_num)
                continue;
            for (short big_md = md_range[big_piece_num][0]; big_md <= md_range[big_piece_num][1]; big_md++)
            {
                for (short small_md = md_range[small_piece_num][0]; small_md <= md_range[small_piece_num][1]; small_md++)
                {
                    short start = big_side_start_by_manhattan_distance[big_piece_num - 5][big_md];
                    short end = big_side_start_by_manhattan_distance[big_piece_num - 5][big_md + 1];
                    int sum = 0;
                    for (short big_side_id = start; big_side_id < end; big_side_id++)
                    {
                        big_side_to_index[big_piece_num - 5][small_piece_num - 3][big_side_id][small_md] = md_to_index[big_piece_num - 5][small_piece_num - 3][big_md][small_md] + sum;
                        sum += index_to_small_side_comb_counter[big_side_to_subtraction_type[big_piece_num - 5][big_side_id]][small_piece_num - 3][small_md];
                    }
                }
            }
        }
    }

    fprintf(encode_table, "#include\"md_encode_table.hpp\"\n");
    fprintf(encode_header, "#ifndef __md_enc_table__\n#define __md_enc_table__ 1\n");
    fprintf(decode_table, "#include\"md_decode_table.hpp\"\n");
    fprintf(decode_header, "#ifndef __md_dec_table__\n#define __md_dec_table__ 1\n");
    fprintf(DB_table, "#include\"md_size_table.hpp\"\n");
    fprintf(DB_header, "#ifndef __md_size_table__\n#define __md_size_table__ 1\n");
    // write compress_big_side
    fprintf(encode_header, "extern const short compress_big_side[2][18564];\n");
    fprintf(encode_table, "const short compress_big_side[2][18564]={\n");
    for (int i = 0; i < 2; i++)
    {
        fprintf(encode_table, "\t{");
        for (int j = 0; j < RMAX; j++)
        {
            fprintf(encode_table, "%d,", compress_big_side[i][j]);
        }
        fprintf(encode_table, "},\n");
    }
    fprintf(encode_table, "};\n");

    // write decompress_big_side
    fprintf(decode_header, "extern const short decompress_big_side[2][9366];\n");
    fprintf(decode_table, "const short decompress_big_side[2][9366]={\n");
    for (int i = 0; i < 2; i++)
    {
        fprintf(decode_table, "\t{");
        for (int j = 0; j < 9366; j++)
        {
            fprintf(decode_table, "%d,", decompress_big_side[i][j]);
        }
        fprintf(decode_table, "},\n");
    }
    fprintf(decode_table, "};\n");

    // write big_side_to_subtraction_type
    fprintf(encode_header, "extern const short big_side_to_subtraction_type[2][9366];\n");
    fprintf(encode_table, "const short big_side_to_subtraction_type[2][9366]={\n");
    for (int i = 0; i < 2; i++)
    {
        fprintf(encode_table, "\t{");
        for (int j = 0; j < 9366; j++)
        {
            fprintf(encode_table, "%d,", big_side_to_subtraction_type[i][j]);
        }
        fprintf(encode_table, "},\n");
    }
    fprintf(encode_table, "};\n");

    // write md_range
    // fprintf(decode_header, "extern const short md_range[7][2];\n");
    // fprintf(decode_table, "const short md_range[7][2]={\n\t");
    // for (int i = 0; i < 7; i++)
    // {
    //     fprintf(decode_table, "{%d,%d},", md_range[i][0], md_range[i][1]);
    // }
    // fprintf(decode_table, "\n};\n");

    // write md_size
    fprintf(DB_header, "extern const int md_size[2][4][41][41];\n");
    fprintf(DB_table, "const int md_size[2][4][41][41]={\n");
    for (int big_piece_num = 5; big_piece_num <= 6; big_piece_num++)
    {
        fprintf(DB_table, "{\n");
        for (int small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (big_piece_num + small_piece_num < 9 || small_piece_num > big_piece_num)
            {
                fprintf(DB_table, "\t{{}},\n");
                continue;
            }
            fprintf(DB_table, "\t{\n");
            for (int bmd = 0; bmd < 41; bmd++)
            {
                fprintf(DB_table, "\t\t{");
                for (int smd = 0; smd < 41; smd++)
                {
                    fprintf(DB_table, "%d,", md_size[big_piece_num - 5][small_piece_num - 3][bmd][smd]);
                }
                fprintf(DB_table, "},\n");
            }
            fprintf(DB_table, "\t},\n");
        }
        fprintf(DB_table, "},\n");
    }
    fprintf(DB_table, "\n};\n");

    // write md_to_index
    fprintf(encode_header, "extern const int md_to_index[2][4][41][41];\n");
    fprintf(encode_table, "const int md_to_index[2][4][41][41]={\n");
    for (int big_piece_num = 5; big_piece_num <= 6; big_piece_num++)
    {
        fprintf(encode_table, "{\n");
        for (int small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (big_piece_num + small_piece_num < 9 || small_piece_num > big_piece_num)
            {
                fprintf(encode_table, "\t{{}},\n");
                continue;
            }
            fprintf(encode_table, "\t{\n");
            for (int bmd = 0; bmd < 41; bmd++)
            {
                fprintf(encode_table, "\t\t{");
                for (int smd = 0; smd < 41; smd++)
                {
                    fprintf(encode_table, "%d,", md_to_index[big_piece_num - 5][small_piece_num - 3][bmd][smd]);
                }
                fprintf(encode_table, "},\n");
            }
            fprintf(encode_table, "\t},\n");
        }
        fprintf(encode_table, "},\n");
    }
    fprintf(encode_table, "\n};\n");

    // write big_side_start_by_manhattan_distance
    fprintf(decode_header, "extern const short big_side_start_by_manhattan_distance[2][42];\n");
    fprintf(decode_table, "const short big_side_start_by_manhattan_distance[2][42]={\n");
    for (int i = 0; i < 2; i++)
    {
        fprintf(decode_table, "\t{");
        for (int j = 0; j < 42; j++)
        {
            fprintf(decode_table, "%d,", big_side_start_by_manhattan_distance[i][j]);
        }
        fprintf(decode_table, "},\n");
    }
    fprintf(decode_table, "};\n");

    // write big_side_to_index
    fprintf(encode_header, "extern const int big_side_to_index[2][4][9366][41];\n");
    fprintf(encode_table, "const int big_side_to_index[2][4][9366][41]={\n");
    for (int big_piece_num = 5; big_piece_num <= 6; big_piece_num++)
    {
        fprintf(encode_table, "{\n");
        for (int small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (big_piece_num + small_piece_num < 9 || small_piece_num > big_piece_num)
            {
                fprintf(encode_table, "\t{{}},\n");
                continue;
            }
            fprintf(encode_table, "\t{\n");
            for (int b_id = 0; b_id < big_side_start_by_manhattan_distance[big_piece_num - 5][md_range[big_piece_num][1] + 1]; b_id++)
            {
                fprintf(encode_table, "\t\t{");
                for (int smd = 0; smd < 41; smd++)
                {
                    fprintf(encode_table, "%d,", big_side_to_index[big_piece_num - 5][small_piece_num - 3][b_id][smd]);
                }
                fprintf(encode_table, "},\n");
            }
            fprintf(encode_table, "\t},\n");
        }
        fprintf(encode_table, "},\n");
    }
    fprintf(encode_table, "\n};\n");

    // write small_side_comb_to_index
    fprintf(encode_header, "extern const short small_side_comb_to_index[1018][4][2002];\n");
    fprintf(encode_table, "const short small_side_comb_to_index[1018][4][2002]={\n");
    for (short sub_type = 0; sub_type < 1018; sub_type++)
    {
        short subtraction_type_hash = subtraction_type_to_hash[sub_type];
        short md_numbers_local[7];
        decode_subtraction_type(subtraction_type_hash, md_numbers_local);
        short subtracted_md_index = 0;
        for (int i = 6; i >= 0; i--)
        {
            for (int j = 0; j < md_numbers_local[i]; j++)
            {
                subtracted_md_index++;
            }
        }
        short remain_pce = subtracted_md_index + 1;
        fprintf(encode_table, "\t{\n");
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            if (remain_pce + small_piece_num > 19)
            {
                fprintf(encode_table, "\t\t0,\n");
                continue;
            }

            fprintf(encode_table, "\t\t{");
            for (short small_id = 0; small_id < C[remain_pce][small_piece_num]; small_id++)
            {
                fprintf(encode_table, "%d,", small_side_comb_to_index[sub_type][small_piece_num - 3][small_id]);
            }
            fprintf(encode_table, "},\n");
        }
        fprintf(encode_table, "\t},\n");
    }
    fprintf(encode_table, "};\n");

    // write index_to_small_side_comb
    fprintf(decode_header, "extern const short index_to_small_side_comb[1018][4][41][242];\n");
    fprintf(decode_table, "const short index_to_small_side_comb[1018][4][41][242]={\n");
    for (short sub_type = 0; sub_type < 1018; sub_type++)
    {
        fprintf(decode_table, "\t{\n");
        for (short small_piece_num = 3; small_piece_num <= 6; small_piece_num++)
        {
            fprintf(decode_table, "\t\t{");
            for (short small_md = 0; small_md < 41; small_md++)
            {
                if (index_to_small_side_comb_counter[sub_type][small_piece_num - 3][small_md] == 0)
                {
                    fprintf(decode_table, "{},");
                    continue;
                }
                fprintf(decode_table, "{");
                for (short small_index = 0; small_index < index_to_small_side_comb_counter[sub_type][small_piece_num - 3][small_md]; small_index++)
                {
                    fprintf(decode_table, "%d,", index_to_small_side_comb[sub_type][small_piece_num - 3][small_md][small_index]);
                }
                fprintf(decode_table, "},");
            }
            fprintf(decode_table, "},\n");
        }
        fprintf(decode_table, "\t},\n");
    }
    fprintf(decode_table, "};\n");

    fprintf(encode_header, "#endif\n");
    fclose(encode_header);
    fclose(encode_table);
    fprintf(decode_header, "#endif\n");
    fclose(decode_header);
    fclose(decode_table);
    fprintf(DB_header, "#endif\n");
    fclose(DB_header);
    fclose(DB_table);
}