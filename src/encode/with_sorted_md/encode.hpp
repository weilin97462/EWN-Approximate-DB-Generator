#ifndef __md_encode__
#define __md_encode__ 1
#include "../../query.hpp"
#include <bitset>
void encode_with_sorted_md(char piecepos[2][6], std::bitset<6> piece_comb[2], Query &q);
#endif