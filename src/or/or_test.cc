#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "or_u8.h"
#include "or_u16.h"
#include "or_u32.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src0_buffer,
                        struct halide_buffer_t *_src1_buffer,
                        struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        const std::vector<int> extents{width, height, depth};
        auto input0 = mk_rand_buffer<T>(extents);
        auto input1 = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input0.set_host_dirty();
        input1.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input0, input1, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int c=0; c<depth; ++c) {
            for (int j=0; j<height; ++j) {
                for (int i=0; i<width; ++i) {
                    T expect = (input0(i, j, c) | input1(i, j, c));
                    T actual = output(i, j, c);
                    if (expect != actual) {
                        throw std::runtime_error(fmt::format("Error at ({}, {}, {}): expect={}, actual={}",
                                                             i, j, c, expect, actual));
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
    test<uint8_t>(or_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(or_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(or_u32);
#endif
}
