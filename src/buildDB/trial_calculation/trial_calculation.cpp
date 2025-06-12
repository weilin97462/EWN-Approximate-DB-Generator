#include "trial_calculation.hpp"
#include "../checkpoint/checkpoint.hpp"
#include "../../config.hpp"
#include "../dependency_manager/dependency_manager.hpp"
#include "../dependency_manager/dependency_data.hpp"
#include "../dependency_manager/count_size.hpp"
#include "../with_MD/MD_bound/MD_bound.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

static long long max_ram_usage = -1, max_file_size = -1, total_file_size = 0;

static void calculate_no_MD_entry(int smaller_set_id, int greater_set_id)
{
    long long mem_size = calculate_dependencies_size(smaller_set_id, greater_set_id);
    long long file_size = calculate_size(make_Entry(smaller_set_id, greater_set_id)) * sizeof(frac_num);
    if (mem_size > max_ram_usage)
        max_ram_usage = mem_size;
    if (file_size > max_file_size)
        max_file_size = file_size;
    total_file_size += file_size;
    if (smaller_set_id != greater_set_id)
        total_file_size += file_size;
}

static void calculate_no_MD(unsigned int piece_num)
{
    for (int smaller_set_id = 1; smaller_set_id < 34; smaller_set_id++)
    {
        for (int greater_set_id = smaller_set_id; greater_set_id < 34; greater_set_id++)
        {
            if ((from_equivalent_piece_set[smaller_set_id].count() + from_equivalent_piece_set[greater_set_id].count()) == piece_num)
            {
                calculate_no_MD_entry(smaller_set_id, greater_set_id);
            }
        }
    }
}

static void calculate_with_MD_entry(unsigned int piece_number, int smaller_set_id, int greater_set_id)
{
    long long mem_size;
    MD_bound bound;
    bound.calculate_bound(smaller_set_id, greater_set_id);
    for (int md = bound.min_md; md <= bound.max_md; md++)
    {
        const int small_start = bound.small_md_start(md);
        const int small_end = bound.small_md_end(md);
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            MDEntry entry = make_MDEntry(smaller_set_id, greater_set_id, md - small_md, small_md);
            mem_size = calculate_dependencies_size_MD(entry);
            if (mem_size > max_ram_usage)
                max_ram_usage = mem_size;
        }
        if (smaller_set_id == greater_set_id)
            continue;
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            MDEntry entry = make_MDEntry(greater_set_id, smaller_set_id, md - small_md, small_md);
            mem_size = calculate_dependencies_size_MD(entry);
            if (mem_size > max_ram_usage)
                max_ram_usage = mem_size;
        }
    }
    long long file_size = calculate_size(make_Entry(smaller_set_id, greater_set_id)) * sizeof(frac_num);
    if (file_size > max_file_size)
        max_file_size = file_size;
    total_file_size += file_size;
    if (smaller_set_id != greater_set_id)
        total_file_size += file_size;
}

static void calculate_with_MD(unsigned int piece_num)
{
    for (int smaller_set_id = 1; smaller_set_id < 34; smaller_set_id++)
    {
        for (int greater_set_id = smaller_set_id; greater_set_id < 34; greater_set_id++)
        {
            if ((from_equivalent_piece_set[smaller_set_id].count() + from_equivalent_piece_set[greater_set_id].count()) == piece_num)
            {
                calculate_with_MD_entry(piece_num, smaller_set_id, greater_set_id);
            }
        }
    }
}

static void calculate_piece_num(unsigned int piece_num)
{
    if (piece_num <= MD_THRESHOLD)
        calculate_no_MD(piece_num);
    else
        calculate_with_MD(piece_num);
}

static std::string bytes_to_human_readable(long long bytes)
{
    std::stringstream str_stream;
    long long unit = 1000000000000000LL;
    if (bytes >= unit)
    {
        str_stream << bytes << std::setprecision(5) << "B (" << static_cast<long double>(bytes) / static_cast<long double>(unit) << "PB)";
        return str_stream.str();
    }
    unit = 1000000000000LL;
    if (bytes >= unit)
    {
        str_stream << bytes << std::setprecision(5) << "B (" << static_cast<long double>(bytes) / static_cast<long double>(unit) << "TB)";
        return str_stream.str();
    }
    unit = 1000000000LL;
    if (bytes >= unit)
    {
        str_stream << bytes << std::setprecision(5) << "B (" << static_cast<long double>(bytes) / static_cast<long double>(unit) << "GB)";
        return str_stream.str();
    }
    unit = 1000000LL;
    if (bytes >= unit)
    {
        str_stream << bytes << std::setprecision(5) << "B (" << static_cast<long double>(bytes) / static_cast<long double>(unit) << "MB)";
        return str_stream.str();
    }
    unit = 1000LL;
    if (bytes >= unit)
    {
        str_stream << bytes << std::setprecision(5) << "B (" << static_cast<long double>(bytes) / static_cast<long double>(unit) << "KB)";
        return str_stream.str();
    }
    str_stream << bytes << "B";
    return str_stream.str();
}

bool trial_calculation()
{
    unsigned int cur_pce_num = checkpoint.cur_pce_num;
    std::cout << "==================================================================\n";
    std::cout << "Please check you computer specifications meet the following needs:\n";
    std::cout << "e.g. ext4 only supports maximum 17.592 TB for a single file.\n";
    std::cout << "==================================================================\n";
    for (cur_pce_num = 2; cur_pce_num <= 12; cur_pce_num++)
    {
        calculate_piece_num(cur_pce_num);
        std::cout << "piece num = " << cur_pce_num << ":\n";
        std::cout << "minimal required memory:\t" << bytes_to_human_readable(max_ram_usage) << std::endl;
        std::cout << "maximum file size:\t\t" << bytes_to_human_readable(max_file_size) << std::endl;
        std::cout << "total file size:\t\t" << bytes_to_human_readable(total_file_size) << std::endl;
        std::cout << "==================================================================\n";
    }
    while (true)
    {
        char y_n_input;
        printf("If you make sure your computer specification satisfies the above limit, enter y to proceed, n to cancel:(y/n)\n");
        while (true)
        {
            scanf("%c", &y_n_input);
            if (!((y_n_input == '\n') || (y_n_input == '\r') || (y_n_input == '\0')))
                break;
        }
        if (y_n_input == 'y' || y_n_input == 'Y')
            return true;
        else if (y_n_input == 'n' || y_n_input == 'N')
            return false;
        printf("invalid enter: %c\n", y_n_input);
    }
}