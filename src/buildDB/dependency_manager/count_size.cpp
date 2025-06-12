#include "dependency_data.hpp"
#include "count_size.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../tables/encode_tables/without_cut/diag_symm_tables.hpp"
#include "../../tables/encode_tables/with_cut/diag_symm_tables.hpp"
#include "../../tables/encode_tables/with_sorted_md/md_encode_table.hpp"
#include "../../tables/DB_tables/md_size_table.hpp"
#include "../../config.hpp"
#include <stdexcept>

long long calculate_size(Entry entry)
{
    int red_num = from_equivalent_piece_set[entry.red_set].count();
    int blue_num = from_equivalent_piece_set[entry.blue_set].count();
    int piece_num = red_num + blue_num;
    char big_num, small_num;
    if (red_num >= blue_num)
        big_num = red_num, small_num = blue_num;
    else
        big_num = blue_num, small_num = red_num;

    int red_perm_num = factorial[red_num];
    int blue_perm_num = factorial[blue_num];
    bool r_symm = is_equivalent_piece_set_symmetric[entry.red_set];
    bool b_symm = is_equivalent_piece_set_symmetric[entry.blue_set];
    if (r_symm)
        red_perm_num /= 2;
    if (b_symm)
        blue_perm_num /= 2;

    long long DB_size = 0;
    if (piece_num < CUT_THRESHOLD)
    {
        DB_size += (long long)compressed_comb_count[big_num] * C[24 - big_num][small_num];
        DB_size += (long long)compressed_comb_count[big_num - 1] * C[25 - big_num][small_num];
    }
    else
    {
        DB_size += (long long)compressed_cut_comb_count[big_num] * C[18 - big_num][small_num];
        DB_size += (long long)compressed_cut_comb_count[big_num - 1] * C[19 - big_num][small_num];
    }
    DB_size *= (red_perm_num * blue_perm_num);
    return DB_size;
}

long long calculate_set_size(EntrySet entry_set)
{
    long long sum = 0;
    for (auto it : entry_set)
        sum += calculate_size(it);
    return sum;
}

long long calculate_offset_MD(MDEntry entry)
{
    int red_num = from_equivalent_piece_set[entry.red_set].count();
    int blue_num = from_equivalent_piece_set[entry.blue_set].count();
    int piece_num = red_num + blue_num;
    if (piece_num < MD_THRESHOLD)
        throw std::runtime_error("Piece number is lower than MD_THRESHOLD in calculate_size_MD.\n");
    char big_num, small_num;
    if (red_num >= blue_num)
        big_num = red_num, small_num = blue_num;
    else
        big_num = blue_num, small_num = red_num;

    int red_perm_num = factorial[red_num];
    int blue_perm_num = factorial[blue_num];
    bool r_symm = is_equivalent_piece_set_symmetric[entry.red_set];
    bool b_symm = is_equivalent_piece_set_symmetric[entry.blue_set];
    if (r_symm)
        red_perm_num /= 2;
    if (b_symm)
        blue_perm_num /= 2;
    long long DB_offset = md_to_index[big_num - 5][small_num - 3][entry.big_md][entry.small_md];
    DB_offset *= (red_perm_num * blue_perm_num);
    return DB_offset;
}
long long calculate_size_MD(MDEntry entry)
{
    int red_num = from_equivalent_piece_set[entry.red_set].count();
    int blue_num = from_equivalent_piece_set[entry.blue_set].count();
    int piece_num = red_num + blue_num;
    if (piece_num < MD_THRESHOLD)
        throw std::runtime_error("Piece number is lower than MD_THRESHOLD in calculate_size_MD.\n");
    char big_num, small_num;
    if (red_num >= blue_num)
        big_num = red_num, small_num = blue_num;
    else
        big_num = blue_num, small_num = red_num;

    int red_perm_num = factorial[red_num];
    int blue_perm_num = factorial[blue_num];
    bool r_symm = is_equivalent_piece_set_symmetric[entry.red_set];
    bool b_symm = is_equivalent_piece_set_symmetric[entry.blue_set];
    if (r_symm)
        red_perm_num /= 2;
    if (b_symm)
        blue_perm_num /= 2;
    long long DB_size = md_size[big_num - 5][small_num - 3][entry.big_md][entry.small_md];
    DB_size *= (red_perm_num * blue_perm_num);
    return DB_size;
}

long long calculate_set_size_MD(MDEntrySet entry_set)
{
    long long sum = 0;
    for (auto it : entry_set)
        sum += calculate_size_MD(it);
    return sum;
}