#include <cstdlib>
#include <iostream>

#include "convolution.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

    try {
        constexpr int width = 512;
        constexpr int height = 512;
        constexpr int depth = 3;
        Buffer<uint8_t> input = mk_const_buffer<uint8_t>({width, height, depth}, 1);

        using fixed16_t = int16_t;
        constexpr uint32_t frac_bits = 10;
        const fixed16_t kv = static_cast<fixed16_t>(round(1.0f * (1 << frac_bits)));
        fixed16_t kernel_data[5][5] = {
            {kv, kv, kv,  0, 0},
            {kv, kv, kv,  0, 0},
            {kv, kv, kv,  0, 0},
            { 0,  0,  0,  0, 0},
            { 0,  0,  0,  0, 0}
        };

        Buffer<fixed16_t> kernel(reinterpret_cast<fixed16_t*>(kernel_data), 5, 5);

        input.set_host_dirty();
        kernel.set_host_dirty();

        Buffer<uint8_t> output(width, height, depth);

        const auto &result = benchmark([&]() {
            convolution(input, kernel, 3, output); });
        fmt::print("Execution time: {}ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int c = 0; c < depth; ++c) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    uint8_t ev = input(x, y, c) * 9;
                    uint8_t av = output(x, y, c);
                    if (ev != av) {
                        const auto s =
                            fmt::format("Error: expect({}, {}, {}) = {}, "
                                        "actual({}, {}, {}) = {}",
                                        x, y, c, ev, x, y, c, av).c_str();
                        throw std::runtime_error(s);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        fmt::print("Error: {}\n", e.what());
        return 1;
    }

    printf("Success!\n");
    return 0;
}
