#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>
#include <cstring>
#include <vector>
#include <cstdint>

#include "histogram_u8.h"
#include "histogram_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        constexpr int hist_width{std::numeric_limits<T>::max() + 1};
        const std::vector<int> extents{width, height, depth}, extents_hist{hist_width, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<uint32_t>(extents_hist);
        uint32_t expect[hist_width * depth];
        constexpr uint32_t hist_size = std::numeric_limits<T>::max() + 1;
        uint32_t hist[hist_size * depth];
        constexpr int bin_size = (hist_size + hist_width - 1) / hist_width;

        input.set_host_dirty();

        memset(hist, 0, sizeof(hist));
        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    hist[input(x, y, c) * depth + c]++;
                }
            }
        }

        int idx = 0;
        for (int i = 0; i < hist_width; i++, idx++) {
            for (int c = 0; c < depth; ++c) {
                uint32_t sum = 0;
                for (int k = 0; k < bin_size && idx < hist_size; ++k) {
                    sum += hist[idx * depth + c];
                }
                expect[i * depth + c] = sum;
            }
        }

        const auto result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        // Verify element-wise correctness
        for (int c=0; c<depth; ++c) {
            for (int x=0; x<hist_width; ++x) {
                uint32_t actual = output(x, c);
                if (expect[x * depth + c] != actual) {
                    throw std::runtime_error(
                        fmt::format("Error: expect({}, {}) = {}, actual({}, {}) = {}",
                                    x, c, expect[x * depth + c], x, c, actual));
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
    test<uint8_t>(histogram_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(histogram_u16);
#endif
}
