#include "checkpoint.hpp"
#include "../../config.hpp"
#include <stdio.h>
#include <filesystem>
bool piece_set_restored = false;
bool md_restored = false;
CheckPoint checkpoint;
FILE *checkpoint_file;

void CheckPoint::save_checkpoint()
{
    fseek(checkpoint_file, 0, SEEK_SET);
    fwrite(&checkpoint, sizeof(CheckPoint), 1, checkpoint_file);
    fflush(checkpoint_file);
}
void CheckPoint::load_checkpoint()
{
    char filename[128];
    sprintf(filename, "%s/checkpoint.bin", DB_PREFIX);
    if (std::filesystem::exists(filename))
    {
        checkpoint_file = fopen(filename, "rb+");
        fread(&checkpoint, sizeof(CheckPoint), 1, checkpoint_file);
        piece_set_restored = false;
        if (checkpoint.cur_pce_num > MD_THRESHOLD)
            md_restored = false;
        else
            md_restored = true;
    }
    else
    {
        printf("No previous check point is found, start building from beginning.");
        checkpoint_file = fopen(filename, "wb");
        checkpoint.cur_pce_num = 2;
        checkpoint.greater_set_id = 1;
        checkpoint.smaller_set_id = 1;
        checkpoint.manhattan_distance = 1;
        piece_set_restored = true;
        md_restored = true;
    }
}

// int main()
// {
//     checkpoint.load_checkpoint();
//     checkpoint.cur_pce_num = 10;
//     checkpoint.greater_set_id = 6;
//     checkpoint.smaller_set_id = 1;
//     checkpoint.manhattan_distance = 1;
//     checkpoint.save_checkpoint();
// }