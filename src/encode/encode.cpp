#include "encode.hpp"
#include "without_cut/encode.hpp"
#include "with_cut/encode.hpp"
#include "with_sorted_md/encode.hpp"

#include "../query.hpp"
#include "../config.hpp"
#include "../tables/basic_tables/basic_tables.hpp"

#include <stdio.h>
#include <string.h>
#include <stdexcept>

void encode(char piece_position[2][6], Query &q)
{
    std::bitset<6> piece_bits[2] = {0, 0};
    for (char color = 0; color < 2; color++)
    {
        for (char num = 0; num < 6; num++)
        {
            if (piece_position[color][num] < 0)
                continue;
            if ((0 <= piece_position[color][num]) && (piece_position[color][num] <= 24))
                piece_bits[color][num] = 1;
        }
    }
    q.r_num = piece_bits[RED].count();
    q.b_num = piece_bits[BLUE].count();
    q.piece_num = q.r_num + q.b_num;
#ifdef ENABLE_SAFETY_CHECK
    for (int num = 0; num < 6; num++)
    {
        if (piece_position[RED][num] == 24)
            throw std::runtime_error("RED already wins!");
    }
    for (int num = 0; num < 6; num++)
    {
        if (piece_position[BLUE][num] == 0)
            throw std::runtime_error("BLUE already wins!");
    }
    if (q.b_num == 0)
        throw std::runtime_error("RED already wins!");
    if (q.r_num == 0)
        throw std::runtime_error("BLUE already wins!");
#endif
    if (q.piece_num < CUT_THRESHOLD)
        encode_without_cut(piece_position, piece_bits, q);
    else if (q.piece_num < MD_THRESHOLD)
        encode_with_cut(piece_position, piece_bits, q);
    else if (q.piece_num <= 12)
        encode_with_sorted_md(piece_position, piece_bits, q);
    else
        throw std::runtime_error("encode error: invalid piece number!");
}