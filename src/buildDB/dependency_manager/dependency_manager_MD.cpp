#include "dependency_manager.hpp"
#include "dependency_data.hpp"
#include "count_size.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../tables/encode_tables/with_sorted_md/md_encode_table.hpp"
#include "../../tables/DB_tables/md_size_table.hpp"
#include "../../config.hpp"
#include "dependency_tables/dependency_tables.hpp"
#include "../../frac_num/frac_num.hpp"
#include "../with_MD/MD_bound/MD_bound.hpp"
#include <set>
#include <stdio.h>
#ifdef USE_PORTABLE_FALLOCATE
#include <filesystem>
#else
#include <fcntl.h>
#endif
#include <errno.h>
#include <stdexcept>

static bool load_dependency_MD(MDEntry entry)
{
    char filename[32];
    int piece_num = from_equivalent_piece_set[entry.red_set].count() + from_equivalent_piece_set[entry.blue_set].count();
    long long db_size = calculate_size_MD(entry);
    if ((memory_used_bytes + db_size * sizeof(frac_num)) > MEMORY_LIMIT_BYTES)
        return false;
    loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md] = new frac_num[db_size];
    sprintf(filename, "%s/%d/%d_%d", DB_PREFIX, piece_num, entry.red_set, entry.blue_set);
    FILE *fp = fopen(filename, "rb");
    long long file_offset = calculate_offset_MD(entry) * sizeof(frac_num);
    fseeko64(fp, file_offset, SEEK_SET);
    fread(loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md], sizeof(frac_num), db_size, fp);
    fclose(fp);
    loaded_dependency_md_entry_set.insert(entry);
    memory_used_bytes += db_size * sizeof(frac_num);
    return true;
}
static void del_dependency_MD(MDEntry entry)
{
    long long db_size = calculate_size_MD(entry);
    delete loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md];
    loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md] = nullptr;
    memory_used_bytes -= db_size * sizeof(frac_num);
    loaded_dependency_md_entry_set.erase(entry);
}

static MDEntry find_next_entry_MD(MDEntry entry)
{
    int red_set_num = from_equivalent_piece_set[entry.red_set].count();
    int blue_set_num = from_equivalent_piece_set[entry.blue_set].count();
    int big_num, small_num;
    if (red_set_num < blue_set_num)
        big_num = blue_set_num, small_num = red_set_num;
    else
        big_num = red_set_num, small_num = blue_set_num;
    int small_max_md = md_range[small_num][1];
    int big_min_md = md_range[big_num][0];
    int md = entry.small_md + entry.big_md;
    const int small_end = std::min(small_max_md, md - big_min_md);
    entry.small_md++;
    entry.big_md--;
    if (entry.small_md > small_end)
        return make_MDEntry(-1, -1, -1, -1);
    else
        return entry;
}

static void check_dependency_and_insert(MDEntrySet &dependency_entry_set, int red_set_id, int blue_set_id, int red_md, int blue_md)
{
    int red_num = from_equivalent_piece_set[red_set_id].count();
    int blue_num = from_equivalent_piece_set[blue_set_id].count();
    // find big and small side
    int big_md, small_md;
    int big_num, small_num;
    if (red_num >= blue_num)
    {
        big_md = red_md;
        small_md = blue_md;
        big_num = red_num;
        small_num = blue_num;
    }
    else
    {
        big_md = blue_md;
        small_md = red_md;
        big_num = blue_num;
        small_num = red_num;
    }
    if (big_md < 0 || small_md < 0)
        return;
    long long size = md_size[big_num - 5][small_num - 3][big_md][small_md];
    if (size == 0)
        return;
    dependency_entry_set.insert(make_MDEntry(red_set_id, blue_set_id, big_md, small_md));
}

static MDEntrySet build_dependency_set_MD(MDEntry entry)
{
    int red_num = from_equivalent_piece_set[entry.red_set].count();
    int blue_num = from_equivalent_piece_set[entry.blue_set].count();
    int red_md, blue_md;
    if (red_num >= blue_num)
    {
        red_md = entry.big_md;
        blue_md = entry.small_md;
    }
    else
    {
        red_md = entry.small_md;
        blue_md = entry.big_md;
    }
    MDEntrySet dependency_entry_set;
    // no capture
    int new_blue_set = entry.red_set;
    int new_red_set = entry.blue_set;
    int new_red_md = blue_md;
    for (int new_blue_md = red_md - 1; new_blue_md >= red_md - 2; new_blue_md--)
    {
        check_dependency_and_insert(dependency_entry_set, new_red_set, new_blue_set, new_red_md, new_blue_md);
    }
    // red capture red
    // grow child from red, then put it to new_blue_md
    new_red_set = entry.blue_set;
    for (int child_id = 0; child_id < 6; child_id++)
    {
        new_blue_set = piece_set_childs[entry.red_set][child_id];
        if (new_blue_set == 0)
            break;
        for (int new_blue_md = red_md - 2; new_blue_md >= red_md - 8; new_blue_md--)
        {
            check_dependency_and_insert(dependency_entry_set, new_red_set, new_blue_set, new_red_md, new_blue_md);
        }
    }
    // red capture blue
    // grow child from blue, then put it into new_red_md
    new_blue_set = entry.red_set;
    for (int child_id = 0;; child_id++)
    {
        char new_red_set = piece_set_childs[entry.blue_set][child_id];
        if (new_red_set == 0)
            break;
        for (int new_blue_md = red_md - 1; new_blue_md >= red_md - 2; new_blue_md--)
        {
            for (int new_red_md = blue_md - 1; new_red_md >= blue_md - 7; new_red_md--)
            {
                check_dependency_and_insert(dependency_entry_set, new_red_set, new_blue_set, new_red_md, new_blue_md);
            }
        }
    }
    return dependency_entry_set;
}
// return false when memory full
void read_all_dependencies_MD(MDEntry entry)
{
    MDEntrySet dependency_entry_set = build_dependency_set_MD(entry);
    MDEntrySet difference_with_existing_dependency = set_difference(dependency_entry_set, loaded_dependency_md_entry_set);
    long long required_size_bytes = calculate_set_size_MD(difference_with_existing_dependency) * sizeof(frac_num);
    if (memory_used_bytes + required_size_bytes > MEMORY_LIMIT_BYTES)
    {
        char errormsg[128];
        sprintf(errormsg, "memory exceeds limit when loading dependencies for (%d %d)\nlimit = %lld, used = %lld\n",
                entry.red_set, entry.blue_set, MEMORY_LIMIT_BYTES, memory_used_bytes + required_size_bytes);
        throw std::runtime_error(errormsg);
    }
    for (auto it : difference_with_existing_dependency)
        load_dependency_MD(it);
}

long long calculate_dependencies_size_MD(MDEntry entry)
{
    MDEntrySet dependency_entry_set = build_dependency_set_MD(entry);
    dependency_entry_set.insert(entry);
    return calculate_set_size_MD(dependency_entry_set) * sizeof(frac_num);
}

void clear_unused_dependencies_MD(MDEntry entry)
{
    MDEntry next_entry = find_next_entry_MD(entry);
    MDEntrySet unused_dependencies;
    if ((next_entry.red_set == -1) && (next_entry.blue_set == -1))
        unused_dependencies = loaded_dependency_md_entry_set;
    else
    {
        MDEntrySet next_dependency_entry_set = build_dependency_set_MD(next_entry);
        unused_dependencies = set_difference(loaded_dependency_md_entry_set, next_dependency_entry_set);
    }
    for (auto it : unused_dependencies)
        del_dependency_MD(it);
}

void dependencies_read_ahead_MD(MDEntry entry)
{
    read_ahead_completed = false;
    MDEntry next_entry = find_next_entry_MD(entry);
    if ((next_entry.red_set == -1) && (next_entry.blue_set == -1))
        return;
    MDEntrySet next_dependency_entry_set = build_dependency_set_MD(next_entry);
    MDEntrySet difference_with_existing_dependency = set_difference(next_dependency_entry_set, loaded_dependency_md_entry_set);
    for (auto it : difference_with_existing_dependency)
        if (load_dependency_MD(it) == false)
            return;
    read_ahead_completed = true;
}

static void allocate_DB_file_MD(Entry entry)
{
    int piece_num = from_equivalent_piece_set[entry.red_set].count() + from_equivalent_piece_set[entry.blue_set].count();
    char filename[512];
    sprintf(filename, "%s/%d/%d_%d", DB_PREFIX, piece_num, entry.red_set, entry.blue_set);
    FILE *fp = fopen(filename, "wb+");
    long long size = calculate_size(make_Entry(entry.red_set, entry.blue_set)) * sizeof(frac_num);
#ifdef USE_PORTABLE_FALLOCATE
    try
    {
        std::filesystem::resize_file(filename, size);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        throw std::runtime_error("Failed to resize file: " + std::string(e.what()));
    }
#else
    if (fallocate64(fileno(fp), FALLOC_FL_KEEP_SIZE, 0, size) == -1)
    {
        char errmsg[128];
        snprintf(errmsg, sizeof(errmsg),
                 "Unable to allocate %lld bytes in your file system",
                 size);
        throw std::runtime_error(errmsg);
    }
#endif
    fclose(fp);
}
void allocate_DB_files_MD(Entry entry)
{
    allocate_DB_file_MD(entry);
    if (entry.red_set == entry.blue_set)
        return;
    allocate_DB_file_MD(make_Entry(entry.blue_set, entry.red_set));
}
void allocate_new_DB_MD(MDEntry entry)
{
    long long size = calculate_size_MD(entry);
    if (memory_used_bytes + size * sizeof(frac_num) > MEMORY_LIMIT_BYTES)
    {
        char errormsg[128];
        sprintf(errormsg, "memory exceeds limit when allocating temp DB for (%d %d) - md(%d %d) \nlimit = %lld, used = %lld\n",
                entry.red_set, entry.blue_set, entry.big_md, entry.small_md, MEMORY_LIMIT_BYTES, memory_used_bytes + size * sizeof(frac_num));
        throw std::runtime_error(errormsg);
    }
    loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md] = new frac_num[size];
    memory_used_bytes += size * sizeof(frac_num);
}
// only used in build connecting DB
void allocate_all_new_DB_MD(MD_bound bound)
{
    for (int md = bound.min_md; md <= bound.max_md; md++)
    {
        const int small_start = bound.small_md_start(md);
        const int small_end = bound.small_md_end(md);
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            allocate_new_DB_MD(make_MDEntry(bound.smaller_set_id, bound.greater_set_id, md - small_md, small_md));
        }
        if (bound.smaller_set_id == bound.greater_set_id)
            continue;
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            allocate_new_DB_MD(make_MDEntry(bound.greater_set_id, bound.smaller_set_id, md - small_md, small_md));
        }
    }
}

void write_new_DB_to_file_MD(unsigned int piece_num, MDEntry entry)
{
    long long size = calculate_size_MD(entry);
    long long offset = calculate_offset_MD(entry) * sizeof(frac_num);
    char filename[64];
    sprintf(filename, "%s/%d/%d_%d", DB_PREFIX, piece_num, entry.red_set, entry.blue_set);
    FILE *fp = fopen(filename, "rb+");
    fseeko64(fp, offset, SEEK_SET);
    fwrite(loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md], sizeof(frac_num), size, fp);
    fclose(fp);
    delete loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md];
    loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md] = nullptr;
    memory_used_bytes -= size * sizeof(frac_num);
}

void write_all_new_DB_to_file_MD(unsigned int piece_num, MD_bound bound)
{
    for (int md = bound.min_md; md <= bound.max_md; md++)
    {
        const int small_start = bound.small_md_start(md);
        const int small_end = bound.small_md_end(md);
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            write_new_DB_to_file_MD(piece_num, make_MDEntry(bound.smaller_set_id, bound.greater_set_id, md - small_md, small_md));
        }
        if (bound.smaller_set_id == bound.greater_set_id)
            continue;
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            write_new_DB_to_file_MD(piece_num, make_MDEntry(bound.greater_set_id, bound.smaller_set_id, md - small_md, small_md));
        }
    }
}

numerator_int_t read_dependency_MD(MDEntry entry, long long index)
{
    return (loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md][index]).getint();
}

void write_dependency_MD(MDEntry entry, long long index, numerator_int_t numerator)
{
    (loaded_dependency_md[entry.red_set][entry.blue_set][entry.big_md][entry.small_md][index]).assignint(numerator);
}