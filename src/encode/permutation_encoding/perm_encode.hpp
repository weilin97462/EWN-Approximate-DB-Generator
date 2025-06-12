#ifndef __perm_encode__
#define __perm_encode__ 1
#include <bitset>
int perm_encode(char *permutation, int size);
void sort_to_perm_array(char perm_array[2][6], char piece_position[2][6], std::bitset<6> piece_bits[2]);
#endif