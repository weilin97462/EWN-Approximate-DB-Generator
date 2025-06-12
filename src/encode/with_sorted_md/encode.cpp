#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "encode.hpp"
#include "../permutation_encoding/perm_encode.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../tables/encode_tables/with_sorted_md/md_encode_table.hpp"
#include "../../manhattan_distance/manhattan_distance.hpp"
#include "../combination_encoding/combination_encoding.hpp"
#include "../../config.hpp"

static int map_to_original_board_index[2][18] = {
    {23, 19, 22, 18, 14, 17, 13, 16, 12, 8, 11, 7, 10, 6, 2, 5, 1, 0},
    {1, 5, 2, 6, 10, 7, 11, 8, 12, 16, 13, 17, 14, 18, 22, 19, 23, 24}};

static void diag_flip_piecepos(char piece_position[2][6])
{
    for (char color = 0; color < 2; color++)
        for (char num = 0; num < 6; num++)
            if (piece_position[color][num] >= 0)
                piece_position[color][num] = flip[piece_position[color][num]];
}

void encode_with_sorted_md(char piece_position[2][6], std::bitset<6> piece_bits[2], Query &q)
{
    q.red_set_id = to_equivalent_piece_set[piece_bits[0].to_ulong()];
    q.blue_set_id = to_equivalent_piece_set[piece_bits[1].to_ulong()];
    // encode:
    //  big_side_id=compress_big_side[piece_num-5][big_comb]
    //  comb_id=big_side_to_index[big_piece_num-5][small_piece_num-3][big_side_id][small_md]+small_side_comb_to_index[big_side_to_subtraction_type[big_piece_num-5][big_side_id]][small_comb_id]
    //  comb_id*a!b!+perm_id
    char big_side, small_side;
    char big_num, small_num;
    if (q.r_num >= q.b_num)
    {
        big_side = RED;
        small_side = BLUE;
        big_num = q.r_num;
        small_num = q.b_num;
    }
    else
    {
        big_side = BLUE;
        small_side = RED;
        big_num = q.b_num;
        small_num = q.r_num;
    }
    // it stores only colors, no numbers
    // -1=blank,0=red,1=blue
    char color_board[25];
    memset(color_board, -1, 25);
    // red
    for (char num = 0; num < 6; num++)
    {
        char pos = piece_position[RED][num];
        if (pos >= 0)
            color_board[pos] = RED;
    }
    // blue
    for (char num = 0; num < 6; num++)
    {
        char pos = piece_position[BLUE][num];
        if (pos >= 0)
            color_board[pos] = BLUE;
    }

    short big_side_sorted_list[6];
    // build the big_side_sorted_list
    for (char big_side_encode_seq_id = 17, sorted_list_back = 0; big_side_encode_seq_id >= 0; big_side_encode_seq_id--)
    {
        char pce_color = color_board[map_to_original_board_index[big_side][big_side_encode_seq_id]];
        if (pce_color == big_side)
        {
            big_side_sorted_list[sorted_list_back] = big_side_encode_seq_id;
            sorted_list_back++;
            if (sorted_list_back == big_num)
                break;
        }
    }
    long long big_side_comb_id = encode_comb(big_side_sorted_list, big_num);
    // left-bottom top-right symmetry
    //-1 means need to flip & recalculate the big_side_comb_id
    long long compress_big_side_id = compress_big_side[big_num - 5][big_side_comb_id];
    if (compress_big_side_id < 0)
    {
        diag_flip_piecepos(piece_position);
        memset(color_board, -1, 25);
        // red
        for (char num = 0; num < 6; num++)
        {
            char pos = piece_position[RED][num];
            if (pos >= 0)
                color_board[pos] = RED;
        }
        // blue
        for (char num = 0; num < 6; num++)
        {
            char pos = piece_position[BLUE][num];
            if (pos >= 0)
                color_board[pos] = BLUE;
        }
        // rebuild the big_side_sorted_list
        for (char big_side_encode_seq_id = 17, sorted_list_back = 0; big_side_encode_seq_id >= 0; big_side_encode_seq_id--)
        {
            char pce_color = color_board[map_to_original_board_index[big_side][big_side_encode_seq_id]];
            if (pce_color == big_side)
            {
                big_side_sorted_list[sorted_list_back] = big_side_encode_seq_id;
                sorted_list_back++;
                if (sorted_list_back == big_num)
                    break;
            }
        }
        big_side_comb_id = encode_comb(big_side_sorted_list, big_num);
        compress_big_side_id = compress_big_side[big_num - 5][big_side_comb_id];
    }
// after flipping, it should get a non-negative compressed big-side-id
// if not, there is a bug.
#ifdef ENABLE_SAFETY_CHECK
    assert((compress_big_side_id != -1));
#endif

    // next: calculate big_md small_md
    short big_md = 0, small_md = 0;
    // big_md
    for (int num = 0; num < 6; num++)
    {
        char pos = piece_position[big_side][num];
        if (pos >= 0)
        {
            big_md += manhattan_distance[big_side][pos];
        }
    }
    q.bmd = big_md;
    for (int num = 0; num < 6; num++)
    {
        char pos = piece_position[small_side][num];
        if (pos >= 0)
        {
            small_md += manhattan_distance[small_side][pos];
        }
    }
    q.smd = small_md;

    // next: calculate small_encoding_sequence by removing elements belong to big_side_sorted_list
    // O(n), which is very fast!
    char need_remove[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char small_seq_len;
    memset(need_remove, 0, 18);
    // the piece is in corner, no need to subtract
    if (big_side_sorted_list[0] < 17)
    {
        need_remove[16 - big_side_sorted_list[0]] = 1;
        small_seq_len = 18 - big_num;
    }
    else
    {
        small_seq_len = 19 - big_num;
    }
    int small_encoding_sequence[18];
    for (int i = 1; i < big_num; i++)
    {
        need_remove[16 - big_side_sorted_list[i]] = 1;
    }
    for (int small_encoding_seq_id = 0, shift = 0; small_encoding_seq_id < 18; small_encoding_seq_id++)
    {
        if (need_remove[small_encoding_seq_id] == 1)
            shift++;
        else
            small_encoding_sequence[small_encoding_seq_id - shift] = map_to_original_board_index[small_side][small_encoding_seq_id];
    }
    short small_side_sorted_list[6];
    for (char small_side_encode_seq_id = small_seq_len - 1, sorted_list_back = 0; small_side_encode_seq_id >= 0; small_side_encode_seq_id--)
    {
        char pce_color = color_board[small_encoding_sequence[small_side_encode_seq_id]];
        if (pce_color == small_side)
        {
            small_side_sorted_list[sorted_list_back] = small_side_encode_seq_id;
            sorted_list_back++;
            if (sorted_list_back == small_num)
                break;
        }
    }
    long long small_side_comb_id = encode_comb(small_side_sorted_list, small_num);

    // calculate the combination encode value
    long long comb_encode_value = (long long)big_side_to_index[big_num - 5][small_num - 3][compress_big_side_id][small_md] + (long long)small_side_comb_to_index[big_side_to_subtraction_type[big_num - 5][compress_big_side_id]][small_num - 3][small_side_comb_id];
    // calculate permutation: red_perm*(red_num)!+blue_perm
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
    q.ID = comb_encode_value * red_perm_num * blue_perm_num + red_perm_enc_val * blue_perm_num + blue_perm_enc_val;
    q.md_ID = q.ID - (long long)md_to_index[big_num - 5][small_num - 3][q.bmd][q.smd] * red_perm_num * blue_perm_num;
}