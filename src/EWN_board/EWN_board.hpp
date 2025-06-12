#ifndef __cboard__
#define __cboard__ 1
#include "../frac_num/frac_num.hpp"
#include <bitset>
class EWN_Board
{
    // win: one of the piece_bits becomes 0
    std::bitset<6> piece_bits[2];
    char piecepos[2][6];
    // blank is -1
    char board[25];
    char moves[6][2];
    char move_count;
    char piece_num;
    char color;
    numerator_int_t read_win_rate_from_dependency();
    void move(int id);
    void find_steps(int dice);
    void rotate();
public:
    void init_with_piecepos(char input_piecepos[2][6], char input_color);
    numerator_int_t search_and_sum_win_rate();
    bool check_winner();
};
#endif