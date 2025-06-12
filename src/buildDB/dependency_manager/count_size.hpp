#ifndef __count_size__
#define __count_size__ 1
#include "dependency_data.hpp"
long long calculate_size(Entry entry);
long long calculate_set_size(EntrySet entry_set);
long long calculate_offset_MD(MDEntry entry);
long long calculate_size_MD(MDEntry entry);
long long calculate_set_size_MD(MDEntrySet entry_set);
#endif