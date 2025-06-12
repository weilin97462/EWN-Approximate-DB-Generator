#include "MD_bound.hpp"
#include "../../../tables/basic_tables/basic_tables.hpp"
#include <algorithm>

void MD_bound::calculate_bound(int smaller_set_id_input, int greater_set_id_input)
{
    smaller_set_id = smaller_set_id_input;
    greater_set_id = greater_set_id_input;
    // calculate bounds
    int smaller_set_num = from_equivalent_piece_set[smaller_set_id].count();
    int greater_set_num = from_equivalent_piece_set[greater_set_id].count();
    int big_num, small_num;
    if (smaller_set_num < greater_set_num)
        big_num = greater_set_num, small_num = smaller_set_num;
    else
        big_num = smaller_set_num, small_num = greater_set_num;
    small_min_md = md_range[small_num][0];
    small_max_md = md_range[small_num][1];
    big_min_md = md_range[big_num][0];
    big_max_md = md_range[big_num][1];
    min_md = small_min_md + big_min_md;
    max_md = small_max_md + big_max_md;
}

int MD_bound::small_md_start(int md)
{
    return std::max(small_min_md, md - big_max_md);
}
int MD_bound::small_md_end(int md)
{
    return std::min(small_max_md, md - big_min_md);
}