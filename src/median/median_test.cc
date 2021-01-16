#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include "median_u8.h"
#include "median_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

#define BORDER_INTERPOLATE(x, l) (x < 0 ? 0 : (x >= l ? l - 1 : x))

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const int window_width = 3;
        const int window_height = 3;
        const std::vector<int32_t> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        func(input, output);
        T *tmp = new T[depth * height * width];
        T (*expect)[height][width] = reinterpret_cast<T (*)[height][width]>(tmp);

        const int offset_x = window_width / 2;
        const int offset_y = window_height / 2;
        const int window_area = window_width * window_height;
        size_t table_size = static_cast<size_t>(window_area);
        T table[table_size];

        for (int c = 0; c < depth; c++) {
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    for (int k = -offset_y; k <= offset_y; k++) {
                        for (int l = -offset_x; l <= offset_x; l++) {
                            table[(k + offset_y) * window_width + (l + offset_x)] =
                                input(BORDER_INTERPOLATE(j + l, width), BORDER_INTERPOLATE(i + k, height), c);
                        }
                    }
                    std::sort(table, table + table_size);
                    expect[c][i][j] = table[table_size / 2];
                }
            }
        }

        const auto &result = benchmark([&]() {
            func(input, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T expect_ = expect[c][y][x];
                    T actual = output(x, y, c);
                    if (expect_ != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d", x, y, c, expect_, x, y, c, actual).c_str());
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
    test<uint8_t>(median_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(median_u16);
#endif
}
