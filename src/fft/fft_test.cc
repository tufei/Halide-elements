#include <cstdlib>
#include <string>
#include <vector>
#include <exception>

#include "fft.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

struct float2 {
    float x;
    float y;
};

bool validate(const std::vector<float2>& src, const std::vector<float2>& dst, const int n)
{
    std::vector<float2> ref_dst(n);

    for (int i=0; i<n; ++i) {
        float2 d;
        d.x = 0;
        d.y = 0;
        for (int j=0; j<n; ++j) {
            float2 w;
            w.x = cos(- 2 * M_PI * static_cast<float>(i) * static_cast<float>(j) / static_cast<float>(n));
            w.y = sin(- 2 * M_PI * static_cast<float>(i) * static_cast<float>(j) / static_cast<float>(n));
            float2 s = src[j];
            d.x += s.x * w.x - s.y * w.y;
            d.y += s.x * w.y + s.y * w.x;
        }
        ref_dst[i] = d;
    }

    bool passed = true;
    for (int i=0; i<n; ++i) {
        float2 expect = ref_dst[i];
        float2 actual = dst[i];
        if ((fabs(expect.x - actual.x) > 1e-3f) || (fabs(expect.y - actual.y) > 1e-3f)) {
            passed = false;
            fmt::print(stderr,
                "{:4d}=expect.x({:+7e}), actual.x({:+7e}), expect.y({:+7e}), actual.y({:+7e})\n",
                i, expect.x, actual.x, expect.y, actual.y);
        }
    }

    return passed;
}

int main() {
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

    try {
        constexpr int c{2};
        constexpr int n{256};
        constexpr int batch_size{4};
        Buffer<float> input = mk_rand_real_buffer<float>({c, n, batch_size}, -1.0f, 1.0f);
        Buffer<float> output(c, n, batch_size);
        input.set_host_dirty();

        const auto result = benchmark([&]() {
            fft(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int i=0; i<batch_size; ++i) {
            std::vector<float2> src(n);
            std::vector<float2> dst(n);
            for (int j=0; j<n; ++j) {
                src[j].x = input(0, j, i);
                src[j].y = input(1, j, i);
                dst[j].x = output(0, j, i);
                dst[j].y = output(1, j, i);
            }
            if (!validate(src, dst, n)) {
                throw std::runtime_error("Failed!");
            }
        }
    } catch (const std::exception& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}
