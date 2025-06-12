#include "EWN_board.hpp"
#include "tables.hpp"
#include "../tables/basic_tables/basic_tables.hpp"
#include "../frac_num/frac_num.hpp"
#include "../buildDB/dependency_manager/dependency_manager.hpp"
#include "../encode/encode.hpp"
#include <string.h>
#include <algorithm>

void EWN_Board::init_with_piecepos(char input_piecepos[2][6], char input_color)
{
    color = input_color;
    memcpy(piecepos, input_piecepos, 12);
    memset(board, -1, 25);
    piece_bits[RED].reset();
    piece_bits[BLUE].reset();
    for (char temp_color = 0; temp_color < 2; temp_color++)
    {
        for (char temp_piece = 0; temp_piece < 6; temp_piece++)
        {
            char position = input_piecepos[temp_color][temp_piece];
            if (position >= 0)
            {
                board[position] = temp_piece + 6 * (temp_color);
                piece_bits[temp_color][temp_piece] = 1;
            }
        }
    }
    piece_num = piece_bits[RED].count() + piece_bits[BLUE].count();
}
// very fast!
void EWN_Board::move(int id)
{
    int start = moves[id][0], end = moves[id][1];
    int piece = board[start], end_piece = board[end];
    // undo
    board[start] = -1;
    if (end_piece >= 0)
    {
        piece_num--;
        if (end_piece < 6)
        {
            piece_bits[RED][end_piece] = 0;
            piecepos[RED][end_piece] = -1;
        }
        else
        {
            piece_bits[BLUE][end_piece - 6] = 0;
            piecepos[BLUE][end_piece - 6] = -1;
        }
    }
    piecepos[color][piece % 6] = end;
    board[end] = piece;
    color ^= 1;
}
void EWN_Board::find_steps(int dice)
{
    int movable_piece1, movable_piece2;
    int *result1, *result2;
    int pos1, pos2 = 0;
    movable_piece1 = dice2num[piece_bits[color].to_ulong()][dice][0];
    movable_piece2 = dice2num[piece_bits[color].to_ulong()][dice][1];
    if (piece_num < CUT_THRESHOLD)
    {
        if (movable_piece2 == -1)
        {
            pos1 = piecepos[color][movable_piece1];
            result1 = (int *)pos2steps[color][pos1];
            result2 = nullptr;
            move_count = result1[3];
        }
        else
        {
            pos1 = piecepos[color][movable_piece1];
            pos2 = piecepos[color][movable_piece2];
            result1 = (int *)pos2steps[color][pos1];
            result2 = (int *)pos2steps[color][pos2];
            move_count = result1[3] + result2[3];
        }
    }
    else
    {
        if (movable_piece2 == -1)
        {
            pos1 = piecepos[color][movable_piece1];
            result1 = (int *)pos2steps_cut[color][pos1];
            result2 = nullptr;
            move_count = result1[3];
        }
        else
        {
            pos1 = piecepos[color][movable_piece1];
            pos2 = piecepos[color][movable_piece2];
            result1 = (int *)pos2steps_cut[color][pos1];
            result2 = (int *)pos2steps_cut[color][pos2];
            move_count = result1[3] + result2[3];
        }
    }
    // gen moves
    int i = 0;
    for (; i < result1[3]; i++)
    {
        moves[i][0] = pos1;
        moves[i][1] = result1[i];
    }
    for (; i < move_count; i++)
    {
        moves[i][0] = pos2;
        moves[i][1] = result2[i - result1[3]];
    }
    return;
}
numerator_int_t EWN_Board::search_and_sum_win_rate()
{
    numerator_int_t sum = 0;
    int blank_length = 0;
    numerator_int_t prev_max_val = 0;
    for (char dice = 0; dice < 6; dice++)
    {
        if (piece_bits[color][dice] == 0)
        {
            blank_length++;
            continue;
        }
        find_steps(dice);
        EWN_Board child;
        numerator_int_t max_val = 0;
        for (int i = 0; i < move_count; i++)
        {
            numerator_int_t ret = 0;
            child = (*this);
            child.move(i);
            if (child.check_winner())
                ret = DENOM_VALUE;
            else
                // negate win rate here
                ret = DENOM_VALUE - child.read_win_rate_from_dependency();
            if (ret > max_val)
                max_val = ret;
        }
        sum += max_val;
        if (blank_length > 0)
            sum += blank_length * std::max(max_val, prev_max_val);
        blank_length = 0;
        prev_max_val = max_val;
    }
    sum += blank_length * prev_max_val;
    sum = sum / 6;
    return sum;
}

bool EWN_Board::check_winner()
{
    return ((piece_bits[color] == 0) || (board[0] >= 6) || ((0 <= board[24]) && (board[24] < 6)));
}

// #include <stdio.h>
// static void print_piece_pos(char piece_position[2][6])
// {
//     char board[25];
//     memset(board, -1, 25);
//     for (int color = 0; color < 2; color++)
//     {
//         for (int num = 0; num < 6; num++)
//         {
//             board[piece_position[color][num]] = num + 6 * color;
//         }
//     }
//     for (int x = 0; x < 5; x++)
//     {
//         for (int y = 0; y < 5; y++)
//         {
//             int num = board[5 * x + y];
//             if (num == -1)
//                 printf("_ ");
//             else if (num < 6)
//                 printf("%c ", 'A' + num);
//             else
//                 printf("%d ", num - 5);
//         }
//         printf("\n");
//     }
//     printf("\n");
//     for(int color = 0;color<2;color++)
//     {
//         for(int num=0;num<6;num++)
//         {
//             printf("%3d",piece_position[color][num]);
//         }
//         printf("\n");
//     }
// }

numerator_int_t EWN_Board::read_win_rate_from_dependency()
{
    if (color == BLUE)
        rotate();
    Query q;
    // print_piece_pos(piecepos);
    encode(piecepos, q);
    if (piece_num >= MD_THRESHOLD)
        return read_dependency_MD(make_MDEntry(q.red_set_id, q.blue_set_id, q.bmd, q.smd), q.md_ID);
    else
        return read_dependency(make_Entry(q.red_set_id, q.blue_set_id), q.ID);
}

void EWN_Board::rotate()
{
    // rotate board
    char piece_pos_copy[2][6];
    memcpy(piece_pos_copy, piecepos, sizeof(piecepos));
    for (char i = 0; i < 6; i++)
    {
        if (piece_pos_copy[BLUE][i] >= 0)
            piecepos[RED][i] = 24 - piece_pos_copy[BLUE][i];
        else
            piecepos[RED][i] = -1;
    }
    for (char i = 0; i < 6; i++)
    {
        if (piece_pos_copy[RED][i] >= 0)
            piecepos[BLUE][i] = 24 - piece_pos_copy[RED][i];
        else
            piecepos[BLUE][i] = -1;
    }
}