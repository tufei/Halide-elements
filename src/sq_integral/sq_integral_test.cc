#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "sq_integral_u8_f32.h"
#include "sq_integral_u16_f32.h"
#include "sq_integral_u32_f32.h"
#include "sq_integral_u8_f64.h"
#include "sq_integral_u16_f64.h"
#include "sq_integral_u32_f64.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T, typename D>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_dst_buffer))
{
    try {
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<D>(extents);
        auto expect = mk_null_buffer<D>(extents);

        // ref expect
        for (int c = 0; c < depth; c++) {
            for (int i = 0; i < height; i++) {
                D sum = 0;
                for (int j = 0; j < width; ++j) {
                    sum += static_cast<D>(input(j, i, c)) * static_cast<D>(input(j, i, c));
                    if(i > 0){
                        expect(j, i, c) = sum + expect(j, i-1, c);
                    }else{
                        expect(j, i, c) = sum;
                    }
                }
            }
        }

        input.set_host_dirty();
        expect.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        // for each x and y
        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    if (expect(x, y, c) != output(x, y, c)) {
                        throw std::runtime_error(fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                                             x, y, c, expect(x, y, c), x, y, c, output(x, y, c)));
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

#ifdef TYPE_u8_f32
    test<uint8_t, float>(sq_integral_u8_f32);
#endif
#ifdef TYPE_u16_f32
    test<uint16_t, float>(sq_integral_u16_f32);
#endif
#ifdef TYPE_u32_f32
    test<uint32_t, float>(sq_integral_u32_f32);
#endif
#ifdef TYPE_u8_f64
    test<uint8_t, double>(sq_integral_u8_f64);
#endif
#ifdef TYPE_u16_f64
    test<uint16_t, double>(sq_integral_u16_f64);
#endif
#ifdef TYPE_u32_f64
    test<uint32_t, double>(sq_integral_u32_f64);
#endif
}
