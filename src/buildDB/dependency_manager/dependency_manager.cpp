#include "dependency_manager.hpp"
#include "dependency_data.hpp"
#include "count_size.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../config.hpp"
#include "dependency_tables/dependency_tables.hpp"
#include "../../frac_num/frac_num.hpp"
#include <set>
#include <stdio.h>
#include <stdexcept>

static bool load_dependency(Entry entry)
{
    printf("loading dependency (%d,%d) ...\n", entry.red_set, entry.blue_set);
    char filename[32];
    int piece_num = from_equivalent_piece_set[entry.red_set].count() + from_equivalent_piece_set[entry.blue_set].count();
    long long db_size = calculate_size(entry);
    if ((memory_used_bytes + db_size * sizeof(frac_num)) > MEMORY_LIMIT_BYTES)
        return false;
    loaded_dependency[entry.red_set][entry.blue_set] = new frac_num[db_size];

    sprintf(filename, "%s/%d/%d_%d", DB_PREFIX, piece_num, entry.red_set, entry.blue_set);
    FILE *fp = fopen(filename, "rb");
    fread(loaded_dependency[entry.red_set][entry.blue_set], sizeof(frac_num), db_size, fp);
    fclose(fp);
    loaded_dependency_entry_set.insert(entry);
    memory_used_bytes += db_size * sizeof(frac_num);
    return true;
}
static void del_dependency(Entry entry)
{
    long long db_size = calculate_size(entry);
    delete loaded_dependency[entry.red_set][entry.blue_set];
    loaded_dependency[entry.red_set][entry.blue_set] = nullptr;
    memory_used_bytes -= db_size * sizeof(frac_num);
    loaded_dependency_entry_set.erase(entry);
}

static Entry find_next_entry(unsigned int piece_number, int current_smaller_set_id, int current_greater_set_id)
{
    bool resume_loop = true;
    for (int smaller_set_id = 1; smaller_set_id < 34; smaller_set_id++)
    {
        for (int greater_set_id = smaller_set_id; greater_set_id < 34; greater_set_id++)
        {
            if (resume_loop)
            {
                greater_set_id = current_greater_set_id;
                smaller_set_id = current_smaller_set_id;
                resume_loop = false;
                continue;
            }
            if ((from_equivalent_piece_set[smaller_set_id].count() + from_equivalent_piece_set[greater_set_id].count()) == piece_number)
                return make_Entry(smaller_set_id, greater_set_id);
        }
    }
    return make_Entry(-1, -1);
}
static EntrySet build_dependency_set(int smaller_set_id, int greater_set_id)
{
    EntrySet dependency_entry_set;
    for (int i = 0; i < 6; i++)
    {
        int child_set = piece_set_childs[smaller_set_id][i];
        if (child_set == 0)
            break;
        dependency_entry_set.insert(make_Entry(greater_set_id, child_set));
        dependency_entry_set.insert(make_Entry(child_set, greater_set_id));
    }
    for (int i = 0; i < 6; i++)
    {
        int child_set = piece_set_childs[greater_set_id][i];
        if (child_set == 0)
            break;
        dependency_entry_set.insert(make_Entry(smaller_set_id, child_set));
        dependency_entry_set.insert(make_Entry(child_set, smaller_set_id));
    }
    return dependency_entry_set;
}
// return false when memory full
void read_all_dependencies(int smaller_set_id, int greater_set_id)
{
    EntrySet dependency_entry_set = build_dependency_set(smaller_set_id, greater_set_id);
    EntrySet difference_with_existing_dependency = set_difference(dependency_entry_set, loaded_dependency_entry_set);
    long long required_size_bytes = calculate_set_size(difference_with_existing_dependency) * sizeof(frac_num);
    if (memory_used_bytes + required_size_bytes > MEMORY_LIMIT_BYTES)
    {
        char errormsg[128];
        sprintf(errormsg, "memory exceeds limit when loading dependencies for (%d %d)\nlimit = %lld, used = %lld\n",
                smaller_set_id, greater_set_id, MEMORY_LIMIT_BYTES, memory_used_bytes + required_size_bytes);
        throw std::runtime_error(errormsg);
    }
    for (auto it : difference_with_existing_dependency)
        load_dependency(it);
}

long long calculate_dependencies_size(int smaller_set_id, int greater_set_id)
{
    EntrySet dependency_entry_set = build_dependency_set(smaller_set_id, greater_set_id);
    dependency_entry_set.insert(make_Entry(smaller_set_id, greater_set_id));
    dependency_entry_set.insert(make_Entry(greater_set_id, smaller_set_id));
    return (calculate_set_size(dependency_entry_set) * sizeof(frac_num));
}

void clear_unused_dependencies(unsigned int piece_num, int smaller_set_id, int greater_set_id)
{
    Entry next_entry = find_next_entry(piece_num, smaller_set_id, greater_set_id);
    EntrySet unused_dependencies;
    if ((next_entry.red_set == -1) && (next_entry.blue_set == -1))
        unused_dependencies = loaded_dependency_entry_set;
    else
    {
        EntrySet next_dependency_entry_set = build_dependency_set(next_entry.red_set, next_entry.blue_set);
        unused_dependencies = set_difference(loaded_dependency_entry_set, next_dependency_entry_set);
    }
    for (auto it : unused_dependencies)
        del_dependency(it);
}

void dependencies_read_ahead(unsigned int piece_num, int smaller_set_id, int greater_set_id)
{
    read_ahead_completed = false;
    Entry next_entry = find_next_entry(piece_num, smaller_set_id, greater_set_id);
    if ((next_entry.red_set == -1) && (next_entry.blue_set == -1))
        return;
    EntrySet next_dependency_entry_set = build_dependency_set(next_entry.red_set, next_entry.blue_set);
    EntrySet difference_with_existing_dependency = set_difference(next_dependency_entry_set, loaded_dependency_entry_set);
    for (auto it : difference_with_existing_dependency)
        if (load_dependency(it) == false)
            return;
    read_ahead_completed = true;
}

void allocate_new_DB(int smaller_set_id, int greater_set_id)
{
    long long size = calculate_size(make_Entry(smaller_set_id, greater_set_id));
    if (memory_used_bytes + size * sizeof(frac_num) > MEMORY_LIMIT_BYTES)
    {
        char errormsg[128];
        sprintf(errormsg, "memory exceeds limit when allocating temp DB for (%d %d)\nlimit = %lld, used = %lld\n",
                smaller_set_id, greater_set_id, MEMORY_LIMIT_BYTES, memory_used_bytes + size * sizeof(frac_num));
        throw std::runtime_error(errormsg);
    }
    loaded_dependency[smaller_set_id][greater_set_id] = new frac_num[size];
    memory_used_bytes += size * sizeof(frac_num);
    if (smaller_set_id == greater_set_id)
        return;
    if (memory_used_bytes + size * sizeof(frac_num) > MEMORY_LIMIT_BYTES)
    {
        char errormsg[128];
        sprintf(errormsg, "memory exceeds limit when allocating temp DB for (%d %d)\nlimit = %lld, used = %lld\n",
                smaller_set_id, greater_set_id, (long long)MEMORY_LIMIT_BYTES, memory_used_bytes + size * sizeof(frac_num));
        throw std::runtime_error(errormsg);
    }
    loaded_dependency[greater_set_id][smaller_set_id] = new frac_num[size];
    memory_used_bytes += size * sizeof(frac_num);
}

void write_new_DB_to_file(unsigned int piece_num, int smaller_set_id, int greater_set_id)
{
    long long size = calculate_size(make_Entry(smaller_set_id, greater_set_id));
    char filename[64];
    sprintf(filename, "%s/%d/%d_%d", DB_PREFIX, piece_num, smaller_set_id, greater_set_id);
    FILE *fp = fopen(filename, "wb+");
    fwrite(loaded_dependency[smaller_set_id][greater_set_id], sizeof(frac_num), size, fp);
    fclose(fp);
    delete loaded_dependency[smaller_set_id][greater_set_id];
    loaded_dependency[smaller_set_id][greater_set_id] = nullptr;
    memory_used_bytes -= size * sizeof(frac_num);
    if (smaller_set_id == greater_set_id)
        return;
    sprintf(filename, "%s/%d/%d_%d", DB_PREFIX, piece_num, greater_set_id, smaller_set_id);
    fp = fopen(filename, "wb+");
    fwrite(loaded_dependency[greater_set_id][smaller_set_id], sizeof(frac_num), size, fp);
    fclose(fp);
    delete loaded_dependency[greater_set_id][smaller_set_id];
    loaded_dependency[greater_set_id][smaller_set_id] = nullptr;
    memory_used_bytes -= size * sizeof(frac_num);
}

numerator_int_t read_dependency(Entry entry, long long index)
{
    return (loaded_dependency[entry.red_set][entry.blue_set][index]).getint();
}

void write_dependency(Entry entry, long long index, numerator_int_t numerator)
{
    (loaded_dependency[entry.red_set][entry.blue_set][index]).assignint(numerator);
}