#ifndef __dep_data__
#define __dep_data__ 1

#include <set>
#include <algorithm>
#include <utility>
#include "../../frac_num/frac_num.hpp"

class Entry
{
public:
    int red_set;
    int blue_set;
    bool operator<(const Entry &other) const;
};
Entry make_Entry(int int1, int int2);
typedef std::set<Entry> EntrySet;
extern EntrySet loaded_dependency_entry_set;
extern frac_num *loaded_dependency[34][34];

class MDEntry
{
public:
    int red_set;
    int blue_set;
    int big_md;
    int small_md;
    bool operator<(const MDEntry &other) const;
};
MDEntry make_MDEntry(int int1, int int2, int int3, int int4);
typedef std::set<MDEntry> MDEntrySet;
extern MDEntrySet loaded_dependency_md_entry_set;
extern frac_num *loaded_dependency_md[34][34][41][41];

extern long long memory_used_bytes;
extern bool read_ahead_completed;

template <typename T>
std::set<T> set_difference(const std::set<T> &a, const std::set<T> &b)
{
    std::set<T> result;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.begin()));
    return result;
}

#endif