#ifndef __query__
#define __query__ 1
typedef struct _Query
{
    char r_num,b_num;
    char piece_num;
    char red_set_id;
    char blue_set_id;
    long long ID;
    //added to perform 9 to 12 pce search
    //md_ID means the id within the (smd,bmd) section
    char smd;
    char bmd;
    long long md_ID;
} Query;
#endif