#ifndef __md_bound__
#define __md_bound__ 1
class MD_bound
{
public:
    int smaller_set_id;
    int greater_set_id;
    int small_min_md;
    int small_max_md;
    int big_min_md;
    int big_max_md;
    int min_md;
    int max_md;
    void calculate_bound(int smaller_set_id, int greater_set_id);
    int small_md_start(int md);
    int small_md_end(int md);
};
#endif