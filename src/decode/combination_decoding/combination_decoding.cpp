#include "combination_decoding.hpp"
#include "../../tables/basic_tables/basic_tables.hpp"
void decode_comb(int index, short *sorted_list, int length, int pool_size)
{
    int current_max = pool_size;
    for (int i = 0; i < length; i++)
    {
        int left = length - i - 1, right = current_max, mid;
        while (1)
        {
            mid = (left + right) / 2;
            if (C[mid][length - i] <= index)
            {
                if (index < C[mid + 1][length - i])
                    break;
                else
                    left = mid + 1;
            }
            else
                right = mid - 1;
        }
        current_max = mid;
        sorted_list[i] = mid;
        index -= C[mid][length - i];
    }
}