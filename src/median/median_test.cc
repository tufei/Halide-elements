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
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        constexpr int window_width{3};
        constexpr int window_height{3};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

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

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T expect_ = expect[c][y][x];
                    T actual = output(x, y, c);
                    if (expect_ != actual) {
                        throw std::runtime_error(fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                                             x, y, c, expect_, x, y, c, actual));
                    }
                }
            }
        }
        delete[] tmp;
    } catch (const std::exception& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}

int main()
{
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

#ifdef TYPE_u8
    test<uint8_t>(median_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(median_u16);
#endif
}
