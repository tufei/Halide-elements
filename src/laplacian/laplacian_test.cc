#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "halide_benchmark.h"

#include "laplacian_u8.h"
#include "laplacian_u16.h"

#include "test_common.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        constexpr int width{8};
        constexpr int height{8};
        constexpr int depth{3};
        constexpr int window_width{3};
        constexpr int window_height{3};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input.set_host_dirty();

        const auto &result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });

        fmt::print("Execution time: {}ms\n", double(result) * 1e3);

        output.copy_to_host();

        constexpr double kernel[3][3] = {{-1., -1., -1.},
                                         {-1.,  8., -1.},
                                         {-1., -1., -1.}};

        for (int c = 0; c < depth; c++) {
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    double sum{0.};
                    for (int k = -1; k <= 1; k++) {
                        for (int l = -1; l <= 1; l++) {
                            const int y = std::clamp(i + k, 0, height - 1);
                            const int x = std::clamp(j + l, 0, width - 1);
                            sum += static_cast<double>(input(x, y, c)) *
                                   kernel[k + 1][l + 1];
                        }
                    }
                    if (sum < 0.) sum = -sum;
                    if (sum > (std::numeric_limits<T>::max)())
                        sum = (std::numeric_limits<T>::max)();
                    T expect = static_cast<T>(sum);
                    T actual = output(j, i, c);

                    if (abs(expect - actual) > 1) {
                        const auto s =
                            fmt::format("Error: expect({}, {}, {}) = {}, "
                                        "actual({}, {}, {}) = {}\n",
                                        j, i, c, expect, j, i, c, actual);
                        throw std::runtime_error(s);
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
    test<uint8_t>(laplacian_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(laplacian_u16);
#endif
}
