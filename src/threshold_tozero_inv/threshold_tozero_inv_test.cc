#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "threshold_tozero_inv_u8.h"
#include "threshold_tozero_inv_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
Halide::Runtime::Buffer<T>& tozero_inv_ref(Halide::Runtime::Buffer<T>& dst,
                                const Halide::Runtime::Buffer<T>& src,
                                const int32_t width, const int32_t height,
                                const int32_t depth, const T threshold)
{
    for(int c=0; c<depth; c++) {
        for(int y=0; y<height; y++){
            for(int x=0; x<width; x++){
                T val = src(x, y, c);
                dst(x, y, c) = val > threshold ? 0 : val;
            }
        }
    }
    return dst;
}

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     T _threshold,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};

        const T threshold = mk_rand_scalar<T>();
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, threshold, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        auto expect = mk_rand_buffer<T>(extents);
        expect = tozero_inv_ref(expect, input, width, height, depth, threshold);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T actual = output(x, y, c);
                    if (expect(x, y, c) != actual) {
                        throw std::runtime_error(
                            fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                       x, y, c, expect(x, y, c), x, y, c, actual));
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
    if (test<uint8_t>(threshold_tozero_inv_u8)) return 1;
#endif
#ifdef TYPE_u16
    if (test<uint16_t>(threshold_tozero_inv_u16)) return 1;
#endif
    return 0;
}
