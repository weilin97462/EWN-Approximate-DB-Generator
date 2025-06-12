#ifndef __dep_manager__
#define __dep_manager__ 1
#include "dependency_data.hpp"
#include "../with_MD/MD_bound/MD_bound.hpp"

// without MD
void read_all_dependencies(int smaller_set_id, int greater_set_id);
long long calculate_dependencies_size(int smaller_set_id, int greater_set_id);
void dependencies_read_ahead(unsigned int piece_num, int smaller_set_id, int greater_set_id);
void clear_unused_dependencies(unsigned int piece_num, int smaller_set_id, int greater_set_id);

void allocate_new_DB(int smaller_set_id, int greater_set_id);
void write_new_DB_to_file(unsigned int piece_num, int smaller_set_id, int greater_set_id);

numerator_int_t read_dependency(Entry entry, long long index);
void write_dependency(Entry entry, long long index, numerator_int_t numerator);

// with MD
void read_all_dependencies_MD(MDEntry entry);
long long calculate_dependencies_size_MD(MDEntry entry);
void clear_unused_dependencies_MD(MDEntry entry);
void dependencies_read_ahead_MD(MDEntry entry);

void allocate_DB_files_MD(Entry entry);
void allocate_new_DB_MD(MDEntry entry);
void allocate_all_new_DB_MD(MD_bound bound);

void write_new_DB_to_file_MD(unsigned int piece_num, MDEntry entry);
void write_all_new_DB_to_file_MD(unsigned int piece_num, MD_bound bound);

numerator_int_t read_dependency_MD(MDEntry entry, long long index);
void write_dependency_MD(MDEntry entry, long long index, numerator_int_t numerator);
#endif