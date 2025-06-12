#ifndef __basic_tables__
#define __basic_tables__ 1
#include <bitset>
// basic definition
#define RED 0
#define BLUE 1
// binomial
extern const int C[26][26];
extern const int factorial[7];
extern const char flip[25];

extern const char to_equivalent_piece_set[64];
extern const std::bitset<6> from_equivalent_piece_set[34];
extern const bool is_equivalent_piece_set_symmetric[34];
extern const char sort_piece_bits[64][6];
extern const char piece_set_to_nums[34][6];

extern const short md_range[7][2];
#endif