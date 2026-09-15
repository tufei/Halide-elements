#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "scale_NN_u8.h"
#include "scale_NN_u16.h"
#include "scale_NN_i16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
Halide::Runtime::Buffer<T>& ref_NN(Halide::Runtime::Buffer<T>& dst, const Halide::Runtime::Buffer<T>& src,
                const int src_width, const int src_height,
                const int dst_width, const int dst_height, const int depth)
{
    float scale_w = static_cast<float>(src_width) / static_cast<float>(dst_width);
    float scale_h = static_cast<float>(src_height) / static_cast<float>(dst_height);

    for (int c = 0; c < depth; ++c) {
        for (int i = 0; i < dst_height; ++i) {
            for (int j = 0; j < dst_width; ++j) {
                float src_x = (static_cast<float>(j) + 0.5f) * scale_w;
                float src_y = (static_cast<float>(i) + 0.5f) * scale_h;
                float copy = src_x;

                int src_i = static_cast<int>(src_y);
                int src_j = static_cast<int>(src_x);
                src_j = src_j < src_width ? src_j : src_width - 1;
                src_i = src_i < src_height ? src_i : src_height - 1;
                dst(j, i, c) = src(src_j, src_i, c);
            }
        }
    }
    return dst;
}

template <typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_dst_buffer))
{
    try {
        constexpr int32_t depth = 3;
        constexpr int32_t in_width = 1024;
        constexpr int32_t in_height = 768;
        const std::vector<int> in_extents{in_width, in_height, depth};

        constexpr int32_t out_width = 500;
        constexpr int32_t out_height = 500;
        const std::vector<int> out_extents{out_width, out_height, depth};
        auto input = mk_rand_buffer<T>(in_extents);
        auto output = mk_null_buffer<T>(out_extents);

        input.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        auto expect = mk_null_buffer<T>(out_extents);

        expect = ref_NN(expect, input, in_width, in_height, out_width, out_height, depth);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<out_height; ++y) {
                for (int x=0; x<out_width; ++x) {
                    T actual = output(x, y, c);
                    if (abs(expect(x,y,c) - actual) > 0) {
                        throw std::runtime_error(fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
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
    test<uint8_t>(scale_NN_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(scale_NN_u16);
#endif
#ifdef TYPE_i16
    test<int16_t>(scale_NN_i16);
#endif
}
