#include "dependency_data.hpp"

Entry make_Entry(int int1, int int2)
{
    Entry new_entry;
    new_entry.red_set = int1;
    new_entry.blue_set = int2;
    return new_entry;
}
bool Entry::operator<(const Entry &other) const
{
    if (red_set != other.red_set)
        return red_set < other.red_set;
    return blue_set < other.blue_set;
}
MDEntry make_MDEntry(int int1, int int2, int int3, int int4)
{
    MDEntry new_entry;
    new_entry.red_set = int1;
    new_entry.blue_set = int2;
    new_entry.big_md = int3;
    new_entry.small_md = int4;
    return new_entry;
}
bool MDEntry::operator<(const MDEntry &other) const
{
    if (red_set != other.red_set)
        return red_set < other.red_set;

    if (blue_set != other.blue_set)
        return blue_set < other.blue_set;

    if (big_md != other.big_md)
        return big_md < other.big_md;

    return small_md < other.small_md;
}
EntrySet loaded_dependency_entry_set;
frac_num *loaded_dependency[34][34] = {};
MDEntrySet loaded_dependency_md_entry_set;
frac_num *loaded_dependency_md[34][34][41][41] = {};

long long memory_used_bytes = 0;
bool read_ahead_completed = false;