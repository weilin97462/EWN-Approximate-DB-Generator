#ifndef __config__
#define __config__ 1

#define DB_PREFIX "EWN_DB"

// when piece number >= CUT_THRESHOLD, cut the board
#define CUT_THRESHOLD 8
// threshold to apply manhattan distance sorting min:9, max:13
#define MD_THRESHOLD 9

// safety check during encoding is quite expensive
// uncomment this if you want safety check
// #define ENABLE_SAFETY_CHECK 1

#define THREAD_NUM 124
#define THREAD_BATCH 30000

// default is 3 bytes
// you can choose 3,4,5,6,7,8,12,16
// for value greater than 8, you need your compiler to support unsigned __int128
// if your filesystem is ext4, the maximum supported value is 12
#define FRAC_SIZE_BYTES 3

// modify this to fit your computer spec!
// make sure this value is not larger than your actual memory size!
#define MEMORY_LIMIT_BYTES 1500000000000LL

#include <filesystem>
// if you are using os other than linux
// uncomment this
// #define USE_PORTABLE_FALLOCATE
constexpr auto DEFAULT_RW_PERMISSION = std::filesystem::perms::owner_all | std::filesystem::perms::group_read | std::filesystem::perms::others_read;

// check defined value
static_assert((MD_THRESHOLD >= 9));

#endif
