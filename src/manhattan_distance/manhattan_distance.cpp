#include "manhattan_distance.hpp"
char manhattan_distance[2][25] = {
    {8, 7, 6, 5, 4,
     7, 6, 5, 4, 3,
     6, 5, 4, 3, 2,
     5, 4, 3, 2, 1,
     4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4,
     1, 2, 3, 4, 5,
     2, 3, 4, 5, 6,
     3, 4, 5, 6, 7,
     4, 5, 6, 7, 8}};
char compute_manhattan_distance(char piecepos[2][6])
{
    char md = 0;
    for (char color = 0; color < 2; color++)
    {
        for (char pce_num = 0; pce_num < 6; pce_num++)
        {
            if (piecepos[color][pce_num] >= 0)
                md += manhattan_distance[color][piecepos[color][pce_num]];
        }
    }
    return md;
}