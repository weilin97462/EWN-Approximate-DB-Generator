#ifndef __with_cut_encode__
#define __with_cut_encode__ 1
#include "../../query.hpp"
#include <bitset>
void encode_with_cut(char piece_position[2][6],std::bitset<6> piece_bits[2], Query &q);
#endif