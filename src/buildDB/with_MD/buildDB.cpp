#include <stdio.h>
#include <set>
#include <algorithm>
#include <utility>
#include <thread>
#include <mutex>
#include "../../manhattan_distance/manhattan_distance.hpp"
#include "../checkpoint/checkpoint.hpp"
#include "../../decode/decode.hpp"
#include "../../EWN_board/EWN_board.hpp"
#include "buildDB.hpp"
#include "../dependency_manager/dependency_data.hpp"
#include "../dependency_manager/dependency_manager.hpp"
#include "../dependency_manager/count_size.hpp"
#include "MD_bound/MD_bound.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../config.hpp"
#include "../../query.hpp"

static std::mutex public_query_lock;
static Query public_query;

static void setup_public_query(MDEntry entry)
{
    int red_num = from_equivalent_piece_set[entry.red_set].count();
    int blue_num = from_equivalent_piece_set[entry.blue_set].count();
    public_query.red_set_id = entry.red_set;
    public_query.blue_set_id = entry.blue_set;
    public_query.piece_num = red_num + blue_num;
    public_query.r_num = red_num;
    public_query.b_num = blue_num;
    public_query.ID = 0;
    public_query.bmd = entry.big_md;
    public_query.smd = entry.small_md;
}
static void build_new_DB_MD_thread(long long max_id, long long md_offset, MDEntry entry)
{
    Query private_query;
    while (1)
    {
        public_query_lock.lock();
        if (public_query.ID >= max_id)
        {
            public_query_lock.unlock();
            return;
        }
        private_query = public_query;
        public_query.ID += THREAD_BATCH;
        public_query_lock.unlock();
        long long ID_start = private_query.ID, ID_end = ID_start + THREAD_BATCH;
        if (ID_end > max_id)
            ID_end = max_id;
        for (long long cur_ID = ID_start; cur_ID < ID_end; cur_ID++)
        {
            private_query.ID = cur_ID + md_offset;
            char piecepos[2][6];
            decode(piecepos, private_query);
            EWN_Board decoded_board;
            decoded_board.init_with_piecepos(piecepos, RED);
            numerator_int_t win_rate = decoded_board.search_and_sum_win_rate();
            write_dependency_MD(entry, cur_ID, win_rate);
        }
    }
}

// build the instances according to entry
static void build_new_DB_MD(MDEntry entry)
{
    long long size = calculate_size_MD(entry);
    long long offset = calculate_offset_MD(entry);
    // create threads
    std::thread threads[THREAD_NUM];
    // set public query
    setup_public_query(entry);
    // run threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id] = std::thread(build_new_DB_MD_thread, size, offset, entry);
    // recycle threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id].join();
}

static void build_DB_file_MD(unsigned int piece_num, int red_set_id, int blue_set_id, int md, int small_md)
{
    printf("building %d_%d (%d,%d) \n", red_set_id, blue_set_id, md, small_md);
    int big_md = md - small_md;
    MDEntry entry = make_MDEntry(red_set_id, blue_set_id, big_md, small_md);
    // need to real all dependencies if read ahead is incomplete
    if (read_ahead_completed == false)
        read_all_dependencies_MD(entry);
    // allocate new_DB
    allocate_new_DB_MD(entry);
    // do read ahead in another thread
    std::thread read_ahead_thread = std::thread(dependencies_read_ahead_MD, entry);
    // then build new_DB
    build_new_DB_MD(entry);
    // need to wait read ahead to end
    read_ahead_thread.join();
    // prevent IO conjestion, write result after read ahead ends
    write_new_DB_to_file_MD(piece_num, entry);
    // clear dependency
    clear_unused_dependencies_MD(entry);
}

static void build_DB_file(int piece_number, int smaller_set_id, int greater_set_id)
{
    // if respawn from checkpoint, don't allocate file again
    if (md_restored)
    {
        printf("building file %d_%d \n", greater_set_id, smaller_set_id);
        allocate_DB_files_MD(make_Entry(greater_set_id, smaller_set_id));
    }
    MD_bound bound;
    bound.calculate_bound(smaller_set_id, greater_set_id);
    // then build the database
    for (int md = bound.min_md; md <= bound.max_md; md++)
    {
        if (!md_restored)
        {
            md = checkpoint.manhattan_distance;
            printf("md = %d\n", md);
            md_restored = true;
        }
        checkpoint.manhattan_distance = md;
        checkpoint.save_checkpoint();
        const int small_start = bound.small_md_start(md);
        const int small_end = bound.small_md_end(md);
        for (int small_md = small_start; small_md <= small_end; small_md++)
            build_DB_file_MD(piece_number, smaller_set_id, greater_set_id, md, small_md);
        if (smaller_set_id == greater_set_id)
            continue;
        for (int small_md = small_start; small_md <= small_end; small_md++)
            build_DB_file_MD(piece_number, greater_set_id, smaller_set_id, md, small_md);
    }
}

static void build_connecting_DB_file_MD(unsigned int piece_num, int red_set_id, int blue_set_id, int md, int small_md)
{
    printf("building %d_%d (%d,%d) \n", blue_set_id, red_set_id, md, small_md);
    // need to real all dependencies if read ahead is incomplete
    MDEntry entry = make_MDEntry(red_set_id, blue_set_id, md - small_md, small_md);
    // then build new_DB
    build_new_DB_MD(entry);
}

static void build_connecting_DB_file(int piece_number, int smaller_set_id, int greater_set_id)
{
    // if respawn from checkpoint, don't allocate file again
    if (md_restored)
        allocate_DB_files_MD(make_Entry(greater_set_id, smaller_set_id));
    if (read_ahead_completed == false)
        read_all_dependencies(smaller_set_id, greater_set_id);
    MD_bound bound;
    bound.calculate_bound(smaller_set_id, greater_set_id);
    // allocate new_DB
    allocate_all_new_DB_MD(bound);
    // do read ahead in another thread
    std::thread read_ahead_thread = std::thread(dependencies_read_ahead, piece_number, smaller_set_id, greater_set_id);
    // then build the database
    for (int md = bound.min_md; md <= bound.max_md; md++)
    {
        if (!md_restored)
        {
            md = checkpoint.manhattan_distance;
            printf("md = %d\n", md);
            md_restored = true;
        }
        checkpoint.manhattan_distance = md;
        checkpoint.save_checkpoint();
        const int small_start = bound.small_md_start(md);
        const int small_end = bound.small_md_end(md);
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            build_connecting_DB_file_MD(piece_number, smaller_set_id, greater_set_id, md, small_md);
        }
        if (smaller_set_id == greater_set_id)
            continue;
        for (int small_md = small_start; small_md <= small_end; small_md++)
        {
            build_connecting_DB_file_MD(piece_number, greater_set_id, smaller_set_id, md, small_md);
        }
    }
    // need to wait read ahead to end
    read_ahead_thread.join();
    // write results
    write_all_new_DB_to_file_MD(piece_number, bound);
    // clear dependency
    clear_unused_dependencies(piece_number, smaller_set_id, greater_set_id);
}

void build_DB_with_MD(unsigned int piece_number)
{
    for (int smaller_set_id = 1; smaller_set_id < 34; smaller_set_id++)
    {
        for (int greater_set_id = smaller_set_id; greater_set_id < 34; greater_set_id++)
        {
            if (!piece_set_restored)
            {
                smaller_set_id = checkpoint.smaller_set_id;
                greater_set_id = checkpoint.greater_set_id;
                printf("restored from checkpoint!\n");
                printf("piece_num = %d, smaller_set = %d, greater_set = %d\n", checkpoint.cur_pce_num, smaller_set_id, greater_set_id);
                piece_set_restored = true;
            }

            if ((from_equivalent_piece_set[smaller_set_id].count() + from_equivalent_piece_set[greater_set_id].count()) == piece_number)
            {
                checkpoint.smaller_set_id = smaller_set_id;
                checkpoint.greater_set_id = greater_set_id;
                checkpoint.save_checkpoint();
                if (piece_number > MD_THRESHOLD)
                    build_DB_file(piece_number, smaller_set_id, greater_set_id);
                else if (piece_number == MD_THRESHOLD)
                    build_connecting_DB_file(piece_number, smaller_set_id, greater_set_id);
                else
                    throw std::runtime_error("Invalid piece number at build_DB_with_MD!\n");
            }
        }
    }
}