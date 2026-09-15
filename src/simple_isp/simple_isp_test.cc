#include <cstdlib>
#include <fstream>
#include <iostream>

#include <unistd.h>
#include <dlfcn.h>

#include "HalideRuntime.h"
#include "HalideBuffer.h"
#include "halide_benchmark.h"

#include "simple_isp.h"

#include "test_common.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

Buffer<uint16_t> fill_bayer_pattern(int width, int height)
{
    Buffer<uint16_t> buffer(width, height);
    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            if (x < width/2) {
                if (y < height/2) {
                    if (x%2 == 0 && y%2 == 0) {
                        buffer(x, y) = 0x03FF;
                    } else {
                        buffer(x, y) = 0;
                    }
                } else {
                    if ((x%2 == 0 && y%2 == 1) || (x%2 == 1 && y%2 == 0)) {
                        buffer(x, y) = 0x03FF;
                    } else {
                        buffer(x, y) = 0;
                    }
                }
            } else {
                if (y < height/2) {
                    if ((x%2 == 0 && y%2 == 1) || (x%2 == 1 && y%2 == 0)) {
                        buffer(x, y) = 0x03FF;
                    } else {
                        buffer(x, y) = 0;
                    }
                } else {
                    if (x%2 == 1 && y%2 == 1) {
                        buffer(x, y) = 0x03FF;
                    } else {
                        buffer(x, y) = 0;
                    }
                }
            }
        }
    }
    return buffer;
}

int main(int argc, char **argv) {
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

    try {
        constexpr int width = 3280;
        constexpr int height = 2486;

        Buffer<uint16_t> input = fill_bayer_pattern(width, height);
        Buffer<uint8_t> output(4, width, height);

        input.set_host_dirty();

        const uint16_t optical_black_clamp_value = 16;
        const float gamma_value = 1.0f/1.8f;
        const float saturation_value = 0.6f;

        const auto result = benchmark([&]() {
            simple_isp(input, optical_black_clamp_value, gamma_value,
                       saturation_value, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        save_ppm("out.ppm", output);
    } catch (const std::exception& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}
