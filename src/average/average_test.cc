#include <climits>
#include <cmath>
#include <cstdlib>
#include <string>
#include <exception>

#include "halide_benchmark.h"

#include "average_u8.h"
#include "average_u16.h"

#include "test_common.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_dst_buffer))
{

    try {
        using upper_t = typename Halide::Element::Upper<T>::type;

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

        const int wx_lower = -window_width / 2;
        const int wx_upper = wx_lower + window_width;
        const int wy_lower = -window_height / 2;
        const int wy_upper = wy_lower + window_height;
        const int window_area = window_width * window_height;

        for (int c = 0; c < depth; ++c) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int ax, ay;
                    upper_t f = 0;

                    for (int wy = wy_lower; wy < wy_upper; wy++) {
                        for (int wx = wx_lower; wx < wx_upper; wx++) {
                            ax = x + wx;
                            ay = y + wy;

                            if (ax < 0) ax = 0;
                            if (ay < 0) ay = 0;
                            if (width <= ax) ax = width - 1;
                            if (height <= ay) ay = height - 1;

                            f += input(ax, ay, c);

                        }
                    }
                    T expect = static_cast<T>(roundf(static_cast<float>(f) / static_cast<float>(window_area)));

                    T actual = output(x, y, c);

                    if (expect != actual) {
                        throw std::runtime_error(fmt::format("Error at ({}, {}, {}): expect={}, actual={}",
                                                             x, y, c, expect, actual));
                    }
                }
            }
        }
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
    test<uint8_t>(average_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(average_u16);
#endif
}
