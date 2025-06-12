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
#include "../../tables/basic_tables/basic_tables.hpp"
#include "../../config.hpp"
#include "../../query.hpp"

static std::mutex public_query_lock;
static Query public_query;

static void set_public_query(unsigned int piece_num, int red_set_id, int blue_set_id)
{
    int red_num = from_equivalent_piece_set[red_set_id].count();
    int blue_num = from_equivalent_piece_set[blue_set_id].count();
    public_query.red_set_id = red_set_id;
    public_query.blue_set_id = blue_set_id;
    public_query.piece_num = piece_num;
    public_query.r_num = red_num;
    public_query.b_num = blue_num;
    public_query.ID = 0;
}

static void build_MD_thread(long long max_id)
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
            private_query.ID = cur_ID;
            char piecepos[2][6];
            decode(piecepos, private_query);
            int MD = compute_manhattan_distance(piecepos);
            // value > than DENOM_VALUE is undefined, can use it to store MD without using extra memory
            write_dependency(make_Entry(private_query.red_set_id, private_query.blue_set_id), cur_ID, MD + DENOM_VALUE);
        }
    }
}

static void build_MD(unsigned int piece_num, int smaller_set_id, int greater_set_id)
{
    long long size = calculate_size(make_Entry(smaller_set_id, greater_set_id));
    // create threads
    std::thread threads[THREAD_NUM];
    // set public query
    set_public_query(piece_num, smaller_set_id, greater_set_id);
    // run threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id] = std::thread(build_MD_thread, size);
    // recycle threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id].join();
    // return if two sets are equal
    if (smaller_set_id == greater_set_id)
        return;
    // set public query with switched sets
    set_public_query(piece_num, greater_set_id, smaller_set_id);
    // run threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id] = std::thread(build_MD_thread, size);
    // recycle threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id].join();
}

// build the instances that its mahnattan distance == specified_md
static void build_new_DB_MD_thread(long long max_id, int specified_md)
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
            private_query.ID = cur_ID;
            Entry entry = make_Entry(private_query.red_set_id, private_query.blue_set_id);
            if (read_dependency(entry, cur_ID) != (DENOM_VALUE + specified_md))
                continue;
            char piecepos[2][6];
            decode(piecepos, private_query);
            EWN_Board decoded_board;
            decoded_board.init_with_piecepos(piecepos, RED);
            numerator_int_t win_rate = decoded_board.search_and_sum_win_rate();
            write_dependency(entry, cur_ID, win_rate);
        }
    }
}

// build the instances that its mahnattan distance == specified_md
static void build_new_DB_MD(unsigned int piece_num, int smaller_set_id, int greater_set_id, int specified_md)
{
    long long size = calculate_size(make_Entry(smaller_set_id, greater_set_id));
    // create threads
    std::thread threads[THREAD_NUM];
    // set public query
    set_public_query(piece_num, smaller_set_id, greater_set_id);
    // run threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id] = std::thread(build_new_DB_MD_thread, size, specified_md);
    // recycle threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id].join();
    // return if two sets are equal
    if (smaller_set_id == greater_set_id)
        return;
    // set public query with switched sets
    set_public_query(piece_num, greater_set_id, smaller_set_id);
    // run threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id] = std::thread(build_new_DB_MD_thread, size, specified_md);
    // recycle threads
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
        threads[thread_id].join();
}

static void build_new_DB(unsigned int piece_num, int smaller_set_id, int greater_set_id)
{
    build_MD(piece_num, smaller_set_id, greater_set_id);
    int red_num = from_equivalent_piece_set[smaller_set_id].count();
    int blue_num = from_equivalent_piece_set[greater_set_id].count();
    int min_man_dist = md_range[red_num][0] + md_range[blue_num][0];
    int max_man_dist = md_range[red_num][1] + md_range[blue_num][1];
    // build the database from the smallest manhattan distance to the biggest manhattan distance
    for (int md = min_man_dist; md <= max_man_dist; md++)
        build_new_DB_MD(piece_num, smaller_set_id, greater_set_id, md);
}

static void build_DB_file(unsigned int piece_num, int smaller_set_id, int greater_set_id)
{
    printf("building file %d_%d \n", greater_set_id, smaller_set_id);
    // need to real all dependencies if read ahead is incomplete
    if (read_ahead_completed == false)
        read_all_dependencies(smaller_set_id, greater_set_id);
    // allocate new_DB
    allocate_new_DB(smaller_set_id, greater_set_id);
    // do read ahead in another thread
    std::thread read_ahead_thread = std::thread(dependencies_read_ahead, piece_num, smaller_set_id, greater_set_id);
    // build the MD lookup table in new_DB
    build_MD(piece_num, smaller_set_id, greater_set_id);
    // then build new_DB
    build_new_DB(piece_num, smaller_set_id, greater_set_id);
    // need to wait read ahead to end
    read_ahead_thread.join();
    // prevent IO conjestion, write result after read ahead ends
    write_new_DB_to_file(piece_num, smaller_set_id, greater_set_id);
    // clear dependency
    clear_unused_dependencies(piece_num, smaller_set_id, greater_set_id);
}

void build_DB_no_MD(unsigned int piece_number)
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
                build_DB_file(piece_number, smaller_set_id, greater_set_id);
            }
        }
    }
}