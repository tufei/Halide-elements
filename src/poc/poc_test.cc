#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <exception>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "poc.h"
#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

using std::string;
using std::vector;

int main()
{
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

    try {
        int ret = 0;

        //
        // Run
        //
        constexpr int n{16};
        Buffer<float> input1 = mk_rand_real_buffer<float>({n, n}, 0.0f, 1.0f);
        Buffer<float> input2(n, n);
        Buffer<float> output(n, n);

        constexpr int shift_x{3};
        constexpr int shift_y{4};

        for (int y=0; y<n; ++y) {
            for (int x=0; x<n; ++x) {
                if (y - shift_y < n && y - shift_y >= 0 &&
                    x - shift_x < n && x - shift_x >= 0) {
                    input2(x, y) = input1(x - shift_x, y - shift_y);
                } else {
                    input2(x, y) = 0.0f;
                }
            }
        }

        input1.set_host_dirty();
        input2.set_host_dirty();

        const auto result = benchmark([&]() {
            poc(input1, input2, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        int max_x = 0;
        int max_y = 0;
        float max_v = output(0, 0);
        for (int y=0; y<n; ++y) {
            for (int x=0; x<n; ++x) {
                if (max_v < output(x, y)) {
                    max_x = x;
                    max_y = y;
                    max_v = output(x, y);
                }
            }
        }

        if (shift_x != max_x || shift_y != max_y) {
            throw std::runtime_error(fmt::format("Error: expect({}, {}), actual({}, {})",
                                                 shift_x, shift_y, max_x, max_y));
        }

    } catch (const std::exception& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}

