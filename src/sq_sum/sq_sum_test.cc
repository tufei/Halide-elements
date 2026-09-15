#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"
#include "Element/Util.h"

#include "sq_sum_u8_f32.h"
#include "sq_sum_u16_f32.h"
#include "sq_sum_u32_f32.h"
#include "sq_sum_u8_f64.h"
#include "sq_sum_u16_f64.h"
#include "sq_sum_u32_f64.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T, typename D>
int test(int (*func)(struct halide_buffer_t *_src_buffer0,  struct halide_buffer_t *_dst_buffer))
{
    try {
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<D>({1, 1, depth});
        D actual_total;
        D expect_total;

        input.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int c=0; c<depth; ++c) {
            double sum = 0.0;
            actual_total = output(0, 0, c);

            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    sum += (double) input(x, y, c) * (double) input(x, y, c);
                }
            }
            expect_total = static_cast<D>(sum);

            if (expect_total != actual_total) {
                throw std::runtime_error(fmt::format("Error: expect_total = {}, actual_total = {}",
                                                     expect_total, actual_total));
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

#ifdef TYPE_u8_f32
    test<uint8_t, float>(sq_sum_u8_f32);
#endif
#ifdef TYPE_u16_f32
    test<uint16_t, float>(sq_sum_u16_f32);
#endif
#ifdef TYPE_u32_f32
    test<uint32_t, float>(sq_sum_u32_f32);
#endif
#ifdef TYPE_u8_f64
    test<uint8_t, double>(sq_sum_u8_f64);
#endif
#ifdef TYPE_u16_f64
    test<uint16_t, double>(sq_sum_u16_f64);
#endif
#ifdef TYPE_u32_f64
    test<uint32_t, double>(sq_sum_u32_f64);
#endif
}
