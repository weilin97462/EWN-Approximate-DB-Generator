#include <string.h>
#include "frac_num.hpp"

numerator_int_t frac_num::getint()
{
    numerator_int_t a = 0;
    memcpy(&a, data, FRAC_SIZE_BYTES);
    return a;
}
long double frac_num::getval()
{
    return static_cast<long double>(getint()) / static_cast<long double>(DENOM_VALUE);
}
void frac_num::assignint(numerator_int_t input)
{
    memcpy(data, &input, FRAC_SIZE_BYTES);
}
void frac_num::assign(double input)
{
    assignint((numerator_int_t)(input * DENOM_VALUE));
}