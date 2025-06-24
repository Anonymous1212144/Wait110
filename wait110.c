#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    const size_t intmax_size = sizeof(uintmax_t);
    const uintmax_t intmax_shift = (intmax_size << 3) - 1;

    // Set up an initial array of size 2. Assuming size_t can hold the maximum
    // addressable size, this is always the best we can do
    size_t size = 2;
    uintmax_t *arr = (uintmax_t *)malloc(size * intmax_size);

    // Convert nanotime into 2 groups of uintmax_t. Assuming both seconds and
    // nanoseconds are at most uintmax_t, 2 is always sufficient
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    uint64_t t_low = (ts.tv_sec & UINT32_MAX) * 1000000000ULL + ts.tv_nsec;
    uintmax_t t_high = (ts.tv_sec >> 32) * 1000000000 + (t_low >> 32);
    arr[0] = (t_high << 32) + (t_low & UINT32_MAX);
    arr[1] = t_high >> (intmax_shift - 31);

    const int rule = 110;

    size_t o_size = 0;
    char *output = NULL;
    uintmax_t l = step - 1;
    while (1)
    {
        // Prints the current array as binary numbers
        if (++l == step)
        {
            l = 0;
            uint8_t zero = 0;
            size_t pos = 0;
            size_t needed = size * (intmax_shift + 1) + 1;
            if (needed > o_size)
            {
                output = realloc(output, needed);
                o_size = needed;
            }
            for (size_t i = size; i > 0; i--)
            {
                uintmax_t n = arr[i - 1];
                uintmax_t s = ((uintmax_t)1) << intmax_shift;
                for (uintmax_t j = 0; j <= intmax_shift; j++)
                {
                    if (n & s)
                    {
                        output[pos++] = '1';
                        zero = 1;
                    }
                    else if (zero)
                        output[pos++] = '0';
                    s >>= 1;
                }
            }
            output[pos++] = '\n';
            fwrite(output, 1, pos, stdout);
        }

        // Rule 110 logic
        uintmax_t out = arr[0] & 1;
        for (size_t i = 0; i < size; i++)
        {
            uintmax_t cell = arr[i];
            uintmax_t s = 2;
            for (uintmax_t j = 1; j < intmax_shift; j++)
            {
                if (rule & (1 << (cell & 7)))
                    out |= s;
                cell >>= 1;
                s <<= 1;
            }
            if (i + 1 < size)
            {
                cell |= (arr[i + 1] & 3) << 2;
                if (rule & (1 << (cell & 7)))
                    out |= ((uintmax_t)1) << intmax_shift;
                arr[i] = out;
                cell >>= 1;
                if (rule & (1 << cell))
                    out = 1;
                else
                    out = 0;
            }
            else
            {
                if (cell)
                    out |= ((uintmax_t)1) << intmax_shift;

                // If we need more space and we have more space, double the
                // allocated space. If this results in warping to 0, set it to
                // the maximum of size_t instead
                if ((cell & 2) && (~size))
                {
                    size <<= 1;
                    if (!size)
                        size--;
                    arr = realloc(arr, size * intmax_size);
                    arr[i + 1] = 1;
                    for (size_t j = i + 2; j < size; j++)
                        arr[j] = 0;
                }
                arr[i] = out;
                break;
            }
        }
    }
    return 0;
}
