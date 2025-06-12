#ifndef __checkpoint__
#define __checkpoint__ 1
#include <stdio.h>
class CheckPoint
{
public:
    unsigned int cur_pce_num;
    int greater_set_id;
    int smaller_set_id;
    int manhattan_distance;
    void save_checkpoint();
    void load_checkpoint();
};
extern CheckPoint checkpoint;
extern bool piece_set_restored;
extern bool md_restored;
extern FILE *checkpoint_file;
#endif