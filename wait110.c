/*  Zero-instruction Turing-complete "programming language"
    Copyright (C) 2026 Anonymous1212144

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uintmax_t calc_bitwidth()
{
    // find bit width of uintmax_t in O(log n) because maybe in the future
    // uintmax_t can have googol bits or something
    const uintmax_t i = (uintmax_t)-1;
    uintmax_t j = 1;
    do
    {
        j <<= 1;
    } while (i >> j >> j);
    uintmax_t k = j >> 1;
    do
    {
        if (i >> j >> k)
            j |= k;
        k >>= 1;
    } while (k);
    return j + 1;
}

uintmax_t high_bit_bsearch(const uintmax_t t, uintmax_t i0, uintmax_t i1)
{
    // find bit width of input in O(log n) because maybe in the future
    // uintmax_t can have googol bits or something
    uintmax_t j0;
    uintmax_t j1 = (i0 + i1) >> 1;
    do
    {
        if ((uintmax_t)-1 << j1 & t)
            i0 = j1;
        else
            i1 = j1;
        j0 = j1;
        j1 = (i0 + i1) >> 1;
    } while (j0 != j1);
    return i1;
}

uintmax_t calc_next(const uintmax_t b, const uintmax_t prev_high, const uintmax_t next_low)
{
    // Rule 110 logic
    const uintmax_t a = b << 1 | prev_high;
    const uintmax_t c = b >> 1 | next_low;
    return (a ^ b) ^ (a & b & ~c);
}

void print(
    const uintmax_t *const arr,
    char *const out_buffer,
    const size_t size,
    const uintmax_t width,
    const uintmax_t intmax_width)
{
    // Prints the current array as binary numbers
    const size_t s = size - 1;
    size_t index = 0;
    uintmax_t k;

    for (size_t i = 0; i != s; i++)
    {
        k = arr[i];
        size_t j = 1;
        do
        {
            out_buffer[index++] = '0' + (k & 1);
            k >>= 1;
        } while (j++ != intmax_width);
    }

    k = arr[s];
    for (size_t j = 0; j != width; j++)
    {
        out_buffer[index++] = '0' + (k & 1);
        k >>= 1;
    }

    out_buffer[index] = '\n';
    fwrite(out_buffer, 1, (size - 1) * intmax_width + width + 1, stdout);
}

int main(int argc, char *argv[])
{
    // The first argument is number of step before printing
    uintmax_t step;
    if (argc > 1)
        step = strtoumax(argv[1], NULL, 10);
    else
        step = 1;

    // Constants regarding intmax size, because we don't know what this might
    // be changed to in the future
    const uintmax_t intmax_width = calc_bitwidth();
    const uintmax_t intmax_shift = intmax_width - 1;

    // Set up an initial array of size 2. Assuming size_t can hold the maximum
    // addressable size, this is always the best we can do
    size_t size = 2;
    size_t capacity = 3;
    uintmax_t *arr = (uintmax_t *)malloc(3 * sizeof(uintmax_t));
    if (!arr)
        exit(1);
    char *out_buffer = (char *)malloc(3 * intmax_width);
    if (!out_buffer)
        exit(1);

    // Convert nanotime into 2 groups of uintmax_t. Assuming both seconds and
    // nanoseconds are at most uintmax_t, 2 is always sufficient
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    uint64_t t_low = (ts.tv_sec & UINT32_MAX) * 1000000000ULL + ts.tv_nsec;
    uintmax_t t_high = (ts.tv_sec >> 32) * 1000000000 + (t_low >> 32);
    arr[0] = (t_high << 32) + (t_low & UINT32_MAX);
    arr[1] = t_high >> (intmax_shift - 31);

    uintmax_t width = high_bit_bsearch(arr[1], 0, intmax_width);
    if (!width)
    {
        size = 1;
        width = high_bit_bsearch(arr[0], 0, intmax_width);
    }

    uintmax_t count = step - 1;
    while (1)
    {
        if (++count == step) {
            print(arr, out_buffer, size, width, intmax_width);
            count = 0;
        }

        // Rule 110 logic
        uintmax_t prev_high = 0;
        size_t s = size - 1;
        for (size_t i = 0; i < s; i++)
        {
            uintmax_t next_low = (arr[i + 1] & 1) << intmax_shift;
            uintmax_t high = arr[i] >> intmax_shift;
            arr[i] = calc_next(arr[i], prev_high, next_low);
            prev_high = high;
        }
        arr[s] = calc_next(arr[s], prev_high, 0);

        // Increase allocated space if we need more
        if (++width == intmax_width)
        {
            width = 0;
            ++size;
            if (size > capacity)
            {
                capacity = capacity << 1 | 1;
                arr = (uintmax_t *)realloc(arr, capacity * sizeof(uintmax_t));
                if (!arr)
                    exit(1);
                out_buffer = (char *)realloc(out_buffer, capacity * intmax_width);
                if (!out_buffer)
                    exit(1);
            }
            
        }
    }
    return 0;
}
