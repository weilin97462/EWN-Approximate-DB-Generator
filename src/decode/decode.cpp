#include "decode.hpp"
#include "../query.hpp"
#include "../config.hpp"
#include "without_cut/decode.hpp"
#include "with_cut/decode.hpp"
#include "with_sorted_md/decode.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>

void decode(char piece_position[2][6], Query q)
{
    if (q.piece_num < CUT_THRESHOLD)
        decode_without_cut(piece_position, q);
    else if (q.piece_num < MD_THRESHOLD)
        decode_with_cut(piece_position, q);
    else if (q.piece_num <= 12)
        decode_with_sorted_md(piece_position, q);
    else
        throw std::runtime_error("encode error: invalid piece number");
}