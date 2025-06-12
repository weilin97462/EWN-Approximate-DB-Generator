#include <stdio.h>
#include <string.h>
#include <stdexcept>

#include "decode/decode.hpp"
#include "encode/encode.hpp"
#include "query.hpp"
#include "config.hpp"
#include "tables/basic_tables/basic_tables.hpp"
#include "tables/encode_tables/without_cut/diag_symm_tables.hpp"
#include "tables/decode_tables/without_cut/diag_symm_tables.hpp"
#include "tables/encode_tables/with_cut/diag_symm_tables.hpp"
#include "tables/decode_tables/with_cut/diag_symm_tables.hpp"
#include "decode/combination_decoding/combination_decoding.hpp"

void print_piece_pos(char piece_position[2][6])
{
    char board[25];
    memset(board, -1, 25);
    for (int color = 0; color < 2; color++)
    {
        for (int num = 0; num < 6; num++)
        {
            board[piece_position[color][num]] = num + 6 * color;
        }
    }
    for (int x = 0; x < 5; x++)
    {
        for (int y = 0; y < 5; y++)
        {
            int num = board[5 * x + y];
            if (num == -1)
                printf("_ ");
            else if (num < 6)
                printf("%c ", 'A' + num);
            else
                printf("%d ", num - 5);
        }
        printf("\n");
    }
    printf("\n");
}

void verify_set(char r_set, char b_set, char r_num, char b_num)
{
    int pce_num = r_num + b_num;
    int red_perm_num = factorial[r_num];
    int blue_perm_num = factorial[b_num];
    bool r_symm = is_equivalent_piece_set_symmetric[r_set];
    bool b_symm = is_equivalent_piece_set_symmetric[b_set];
    int big_num, small_num;
    if (r_num >= b_num)
        big_num = r_num, small_num = b_num;
    else
        big_num = b_num, small_num = r_num;
    if (r_symm)
        red_perm_num /= 2;
    if (b_symm)
        blue_perm_num /= 2;
    long long ID_maximum = 0;
    if (pce_num < CUT_THRESHOLD)
    {
        ID_maximum += (long long)compressed_comb_count[big_num] * C[24 - big_num][small_num];
        ID_maximum += (long long)compressed_comb_count[big_num - 1] * C[25 - big_num][small_num];
    }
    else
    {
        ID_maximum += (long long)compressed_cut_comb_count[big_num] * C[18 - big_num][small_num];
        ID_maximum += (long long)compressed_cut_comb_count[big_num - 1] * C[19 - big_num][small_num];
    }
    ID_maximum *= (red_perm_num * blue_perm_num);
    Query q;
    q.red_set_id = r_set;
    q.blue_set_id = b_set;
    q.r_num = r_num;
    q.b_num = b_num;
    q.piece_num = r_num + b_num;
    char piece_pos[2][6];
    printf("max = %lld\n", ID_maximum);
    for (long long ID = 0; ID < ID_maximum; ID++)
    {
        q.ID = ID;
        decode(piece_pos, q);
        encode(piece_pos, q);
        if (q.ID != ID)
        {
            print_piece_pos(piece_pos);
            char buff[512];
            sprintf(buff, "ERROR at (%d,%d) %lld => %lld\n", r_set, b_set, ID, q.ID);
            throw std::runtime_error(buff);
        }
    }
}

int main()
{
    // 236053440
    short sorted_index[6];
    int comb_idx = convert_to_compressed_comb_index[4][1898];
    int decode_comb_idx = decompress_comb_index[4][comb_idx];
    char piece_position[2][6] = {
        {0, 5, 10, -1, 6, 11},
        {18, 24, 16, 23, 19, 17}};
    print_piece_pos(piece_position);
    Query q;
    encode(piece_position, q);
    printf("Query: %hhd %hhd %hhd %hhd %lld\n", q.r_num, q.b_num, q.red_set_id, q.blue_set_id, q.ID);
    decode(piece_position, q);
    print_piece_pos(piece_position);
    for (int piece_num = 8; piece_num < 9; piece_num++)
    {
        printf("piece_num = %d\n", piece_num);
        for (int red_id = 1; red_id <= 33; red_id++)
        {
            std::bitset<6> r_pce_bits = from_equivalent_piece_set[red_id];
            for (int blue_id = 1; blue_id <= 33; blue_id++)
            {
                std::bitset<6> b_pce_bits = from_equivalent_piece_set[blue_id];
                if (r_pce_bits.count() + b_pce_bits.count() == piece_num)
                {
                    printf("%d,%d\n", red_id, blue_id);
                    verify_set(red_id, blue_id, r_pce_bits.count(), b_pce_bits.count());
                }
            }
        }
    }
}