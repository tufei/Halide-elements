#include <cstdlib>
#include <iostream>
#include <climits>

#include "halide_benchmark.h"

#include "add_scalar_u8.h"
#include "add_scalar_u16.h"
#include "add_scalar_u32.h"

#include "test_common.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     double _value,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret{0};

        //
        // Run
        //
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        constexpr double value{128.0};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input.set_host_dirty();

        const auto &result = benchmark([&]() {
            func(input, value, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int c = 0; c < depth; ++c) {
            for (int y = 0; y < height; ++y) {
                for (int x =0; x < width; ++x) {
                    T actual = output(x, y, c);
                    double f = static_cast<double>(input(x, y, c)) + value;
                    f = std::min(static_cast<double>(std::numeric_limits<T>::max()), f);
                    f = std::max(static_cast<double>(0.0f), f);
                    T expect = round_to_nearest_even<T>(f);

                    // HLS backend の C-simulation と LLVM backend で丸めの方法が異なるため、1以内の誤差を許している
                    // (C-simulation は round half away from zero だが、LLVM 版は round half to even)
                    if (abs(expect - actual) > 1.) {
                        const auto s =
                            fmt::format("Error: expect({}, {}, {}) = {}, "
                                        "actual({}, {}, {}) = {}\n",
                                        x, y, c, expect, x, y, c, actual);
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
    test<uint8_t>(add_scalar_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(add_scalar_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(add_scalar_u32);
#endif
}
