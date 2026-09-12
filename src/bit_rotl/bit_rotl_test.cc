#include <cstdlib>
#include <iostream>
#include <climits>

#include "halide_benchmark.h"

#include "bit_rotl_f32_u32.h"
#include "bit_rotl_f64_u64.h"

#include "test_common.h"

using namespace Halide::Tools;

template<typename T, typename I>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     size_t _value,
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
        constexpr size_t rotate_left_bits{1};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input.set_host_dirty();

        const auto &result = benchmark([&]() {
            func(input, rotate_left_bits, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int c = 0; c < depth; ++c) {
            for (int y = 0; y < height; ++y) {
                for (int x =0; x < width; ++x) {
                    T actual = output(x, y, c);
                    const auto shift_value = rotate_left_bits % (sizeof(I) * 8);
                    I i = *reinterpret_cast<I*>(&input(x, y, c));
                    i = i << shift_value | i >> (sizeof(I) * 8 - shift_value);
                    T expect = *reinterpret_cast<T*>(&i);

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

#ifdef TYPE_f32_u32
    test<float, uint32_t>(bit_rotl_f32_u32);
#endif
#ifdef TYPE_f64_u64
    test<double, uint64_t>(bit_rotl_f64_u64);
#endif
}
