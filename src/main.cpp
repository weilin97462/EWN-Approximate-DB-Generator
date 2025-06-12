#include <stdio.h>
#include "buildDB/buildDB.hpp"
#include "config.hpp"
#include "buildDB/checkpoint/checkpoint.hpp"
#include "buildDB/trial_calculation/trial_calculation.hpp"
#include <filesystem>

int main()
{
    if (!trial_calculation())
        return 0;
    unsigned int DB_piece_num;
    printf("input piece num (please make sure you computer spec meet the above requirements):\n");
    scanf("%d", &DB_piece_num);
    if (DB_piece_num < 2 || DB_piece_num > 12)
    {
        printf("illegal piece num\n");
        return 0;
    }
    std::filesystem::create_directory(DB_PREFIX);
    std::filesystem::permissions(DB_PREFIX, DEFAULT_RW_PERMISSION, std::filesystem::perm_options::replace);
    checkpoint.load_checkpoint();
    for (; checkpoint.cur_pce_num <= DB_piece_num; checkpoint.cur_pce_num++)
        build_DB(checkpoint.cur_pce_num);
}