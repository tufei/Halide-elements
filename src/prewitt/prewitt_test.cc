#include <iostream>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "prewitt_u8.h"
#include "prewitt_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_input_buffer, struct halide_buffer_t *_output_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);
        T *tmp = new T[width*height*depth];
        T (*expect)[height][depth] = reinterpret_cast<T (*)[height][depth]>(tmp);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                int yy0 = std::max(0, y - 1);
                int yy1 = y;
                int yy2 = std::min(y + 1, height - 1);
                for (int x=0; x<width; ++x) {
                    int xx0 = std::max(0, x - 1);
                    int xx1 = x;
                    int xx2 = std::min(x + 1, width - 1);

                    float diff_x = -input(xx0, yy0, c) + input(xx2, yy0, c) +
                                  -input(xx0, yy1, c) + input(xx2, yy1, c) +
                                  -input(xx0, yy2, c) + input(xx2, yy2, c);

                    float diff_y = -input(xx0, yy0, c) + input(xx0, yy2, c) +
                                  -input(xx1, yy0, c) + input(xx1, yy2, c) +
                                  -input(xx2, yy0, c) + input(xx2, yy2, c);

                    expect[x][y][c] = static_cast<T>(sqrt(diff_x*diff_x + diff_y*diff_y));
                }
            }
        }

        const auto &result = benchmark([&]() {
            func(input, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T actual = output(x, y, c);
                    if ((expect[x][y][c] - actual) > 1.0) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d", x, y, c, expect[x][y][c], x, y, c, actual).c_str());
                    }
                }
            }
        }
        delete[] tmp;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    printf("Success!\n");
    return 0;
}

int main()
{
#ifdef TYPE_u8
    test<uint8_t>(prewitt_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(prewitt_u16);
#endif
}

