#include <stdio.h>
#include <algorithm>
#include "combination_encoding/combination_encoding.hpp"
#include "diag_symmetry/diag_symm.hpp"
#include "manhattan_sorted_encoding/md_sort.hpp"
#include "write_to_files/write_to_files.hpp"
#include "config.hpp"
#include <bitset>

int main()
{
    gen_diag_symm_tables();
    gen_diag_symm_tables_cut();
    build_md_tables();
    write_to_files();
}