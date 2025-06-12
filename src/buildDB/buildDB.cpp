#include "buildDB.hpp"
#include "../config.hpp"
#include "no_MD/buildDB.hpp"
#include "with_MD/buildDB.hpp"
#include <stdio.h>
#include <stdexcept>
#include <filesystem>

// build every possible database for inputed piece_number
void build_DB(unsigned int piece_number)
{
    char filename[128];
    sprintf(filename, "%s/%d", DB_PREFIX, piece_number);
    std::filesystem::create_directory(filename);
    std::filesystem::permissions(filename, DEFAULT_RW_PERMISSION, std::filesystem::perm_options::replace);
    if (piece_number < MD_THRESHOLD)
        build_DB_no_MD(piece_number);
    else
        build_DB_with_MD(piece_number);
}
