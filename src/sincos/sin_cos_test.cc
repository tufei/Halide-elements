#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>
#include <iomanip>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "sin_cos.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

int test(int (*func)(struct halide_buffer_t *_src_buffer1, struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        constexpr int width = 1024;
        constexpr int height = 768;
        constexpr int depth = 3;
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_real_buffer<float>(extents, 0, 1);
        auto output = mk_null_buffer<float>(extents);

        input.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        double diff_max = 0.0;
        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    float expect = (x + y) % 2 == 0 ? sin(input(x, y, c)) : cos(input(x, y, c));
                    float actual = output(x, y, c);
                    double diff = fabs(expect - actual);
                    diff_max = std::max(diff, diff_max);
                    if (diff > 0.0000001) {
                        throw std::runtime_error(fmt::format("Error: expect: {}({:.10f}) = {:.10f}, actual: {}({:.10f}) = {:.10f}, diff = {:.10f}",
                                                         (x + y) % 2 == 0 ? "sin" : "cos", input(x, y, c), expect,
                                                         (x + y) % 2 == 0 ? "sin" : "cos", input(x, y, c), actual, diff));
                    }
                }
            }
        }
        fmt::print("Max diff = {}\n", diff_max);

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
    if (test(sin_cos)) return 1;
    return 0;
}
