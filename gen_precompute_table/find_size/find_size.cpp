#include "find_size.hpp"
#include "../diag_symmetry/diag_symm.hpp"
#include "../combination_encoding/combination_encoding.hpp"
#include "../config.hpp"
long long factorial[7] = {1, 1, 2, 6, 24, 120, 720};
long long find_size(int big_num, int small_num)
{
    long long total = 0;
    if (big_num + small_num < 20)
    {
        total += (long long)compressed_comb_count[big_num] * C[24 - big_num][small_num] * factorial[big_num] * factorial[small_num];
        total += (long long)compressed_comb_count[big_num - 1] * C[25 - big_num][small_num] * factorial[big_num] * factorial[small_num];
    }
    else
    {
        total += (long long)compressed_cut_comb_count[big_num] * C[18 - big_num][small_num] * factorial[big_num] * factorial[small_num];
        total += (long long)compressed_cut_comb_count[big_num - 1] * C[19 - big_num][small_num] * factorial[big_num] * factorial[small_num];
    }
    // total += (long long)C[23][big_num] * C[24 - big_num][small_num] * factorial[big_num] * factorial[small_num];
    // total += (long long)C[23][big_num - 1] * C[25 - big_num][small_num] * factorial[big_num] * factorial[small_num];
    return total;
}