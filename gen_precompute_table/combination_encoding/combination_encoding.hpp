#ifndef __comb_enc__
#define __comb_enc__ 1
extern int C[26][26];
void decode_comb(int index,short *sorted_list,int length,int pool_size);
int encode_comb(short *sorted_list, int length);
#endif