#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdint>

#include "gaussian_u8.h"
#include "gaussian_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(Buffer<T>*, double, Buffer<T>*))
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
        const double sigma{1.0};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, sigma, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        double kernel_sum = 0;
        for (int i = -(window_width/2); i < -(window_width/2) + window_width; i++) {
            for (int j = -(window_height/2); j < -(window_height/2) + window_height; j++) {
                kernel_sum += exp(-(i * i + j * j) / (2 * sigma * sigma));
            }
        }

        // Verify element-wise correctness
        for (int c = 0; c < depth; ++c) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    double expect_f = 0.0f;
                    for (int j = -(window_height/2); j < -(window_height/2) + window_height; j++) {
                        int yy = std::min(std::max(0, y + j), height - 1);
                        for (int i = -(window_width/2); i < -(window_width/2) + window_width; i++) {
                            int xx = std::min(std::max(0, x + i), width - 1);
                            expect_f += exp(-(i * i + j * j) / (2 * sigma * sigma)) * input(xx, yy, c);
                        }
                    }
                    expect_f /= kernel_sum;
                    T expect = round_to_nearest_even<T>(expect_f);
                    T actual = output(x, y, c);

                    // HLS backend の C-simulation と LLVM backend で丸めの方法とexpの実装が異なるため、1以内の誤差を許している
                    // (C-simulation は round half away from zero だが、LLVM 版は round half to even)
                    if (abs(expect - actual) > 1) {
                        fmt::print(stderr, "dst({}, {}, {}) = {} = round_f32({})\n", x, y, c, expect, expect_f);
                        throw std::runtime_error(
                            fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                        x, y, c, expect, x, y, c, actual));
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
    test<uint8_t>(gaussian_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(gaussian_u16);
#endif
}
