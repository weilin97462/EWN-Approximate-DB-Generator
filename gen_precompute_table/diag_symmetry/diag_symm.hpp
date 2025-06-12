#ifndef __diag_symm__
#define __diag_symm__ 1
extern int convert_to_compressed_comb_index[7][100947];
extern int decompress_comb_index[7][100947];
extern int compressed_comb_count[7];
void gen_diag_symm_tables();
extern int convert_to_compressed_cut_comb_index[7][12376];
extern int decompress_cut_comb_index[7][12376];
extern int compressed_cut_comb_count[7];
void gen_diag_symm_tables_cut();
#endif