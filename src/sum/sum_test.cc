#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"
#include "Element/Util.h"

#include "sum_u8_f32.h"
#include "sum_u16_f32.h"
#include "sum_u32_f32.h"
#include "sum_u8_f64.h"
#include "sum_u16_f64.h"
#include "sum_u32_f64.h"
#include "sum_f32_f32.h"
#include "sum_f64_f64.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

using Halide::Element::SumType;

template<typename D, typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer0,  struct halide_buffer_t *_dst_buffer))
{
    try {
        //
        // Run
        //
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
            typename SumType<T>::type sum(0);
            actual_total = output(0, 0, c);

            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    sum += static_cast<typename SumType<T>::type>(input(x, y, c));
                }
            }
            expect_total = static_cast<D>(sum);

            if (expect_total != actual_total) {
                throw std::runtime_error(
                    fmt::format("Error: channel {}, expect_total = {}, actual_total = {}",
                                c, expect_total, actual_total));
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
    test<float, uint8_t>(sum_u8_f32);
#endif
#ifdef TYPE_u16_f32
    test<float, uint16_t>(sum_u16_f32);
#endif
#ifdef TYPE_u32_f32
    test<float, uint32_t>(sum_u32_f32);
#endif
#ifdef TYPE_u8_f64
    test<double, uint8_t>(sum_u8_f64);
#endif
#ifdef TYPE_u16_f64
    test<double, uint16_t>(sum_u16_f64);
#endif
#ifdef TYPE_u32_f64
    test<double, uint32_t>(sum_u32_f64);
#endif
#ifdef TYPE_f32_f32
    test<float, float>(sum_f32_f32);
#endif
#ifdef TYPE_f64_f64
    test<double, double>(sum_f64_f64);
#endif
}
