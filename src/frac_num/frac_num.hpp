#ifndef __frac__
#define __frac__ 1
#include "../config.hpp"

#if FRAC_SIZE_BYTES == 3
using numerator_int_t = unsigned int;
constexpr numerator_int_t DENOM_VALUE = 10077696; // 6^9
#elif FRAC_SIZE_BYTES == 4
using numerator_int_t = unsigned long long;
constexpr numerator_int_t DENOM_VALUE = 2176782336LL; // 6^12
#elif FRAC_SIZE_BYTES == 5
using numerator_int_t = unsigned long long;
constexpr numerator_int_t DENOM_VALUE = 470184984576LL; // 6^15
#elif FRAC_SIZE_BYTES == 6
using numerator_int_t = unsigned long long;
constexpr numerator_int_t DENOM_VALUE = 101559956668416LL; // 6^18
#elif FRAC_SIZE_BYTES == 7
using numerator_int_t = unsigned long long;
constexpr numerator_int_t DENOM_VALUE = 21936950640377856LL; // 6^21
#elif FRAC_SIZE_BYTES == 8
using numerator_int_t = unsigned long long;
constexpr numerator_int_t DENOM_VALUE = 789730223053602816LL; // 6^23
#elif FRAC_SIZE_BYTES == 12
using numerator_int_t = unsigned __int128;
constexpr numerator_int_t DENOM_VALUE = ((static_cast<numerator_int_t>(61886548790943LL) * 1000000000000000LL) + 213277031694336LL); // 6^37
#elif FRAC_SIZE_BYTES == 16
using numerator_int_t = unsigned __int128;
constexpr numerator_int_t DENOM_VALUE = (((static_cast<numerator_int_t>(22452257LL) * 1000000000000000LL) + 707354557240087LL) * 1000000000000000LL) + 211123792674816LL; // 6^48
#else
static_assert(false, "Invalid size for rational number!");
#endif

class frac_num
{
    unsigned char data[FRAC_SIZE_BYTES];

public:
    numerator_int_t getint();
    long double getval();
    void assignint(numerator_int_t input);
    void assign(double input);
};
#endif
