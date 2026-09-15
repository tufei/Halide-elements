#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"
#include "halide_benchmark.h"

#include "tm_zncc_u8.h"
#include "tm_zncc_u16.h"
#include "tm_zncc_u32.h"

#include "test_common.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src0_buffer, struct halide_buffer_t *_src1_buffer, struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        const int img_width = 256;
        const int img_height = 192;
        const int img_depth = 3;
        const int tmp_width = 16;
        const int tmp_height = 16;
        const int res_width = img_width - tmp_width + 1;
        const int res_height = img_height - tmp_height + 1;
        const std::vector<int32_t> img_extents{img_width, img_height, img_depth};
        const std::vector<int32_t> tmp_extents{tmp_width, tmp_height};
        const std::vector<int32_t> res_extents{res_width, res_height, img_depth};
        auto input0 = mk_rand_buffer<T>(img_extents);
        auto input1 = mk_rand_buffer<T>(tmp_extents);
        auto output = mk_null_buffer<double>(res_extents);

        if (typeid(T) == typeid(uint32_t)) {
            for (int c=0; c<img_depth; ++c) {
                for (int y=0; y<img_height; ++y) {
                    for (int x=0; x<img_width; ++x) {
                        input0(x, y, c) = static_cast<T>(input0(x, y, c) / 10000000);
                    }
                }
            }
            for (int y=0; y<tmp_height; ++y) {
                for (int x=0; x<tmp_width; ++x) {
                    input1(x, y) = static_cast<T>(input1(x, y) / 10000000);
                }
            }
        }

        input0.set_host_dirty();
        input1.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input0, input1, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        const double tmp_size = static_cast<double>(tmp_width * tmp_height);
        double avr1 = 0.0;
        for (int tmp_y=0; tmp_y<tmp_height; ++tmp_y) {
            for (int tmp_x=0; tmp_x<tmp_width; ++tmp_x) {
                avr1 += static_cast<double>(input1(tmp_x, tmp_y));
            }
        }
        avr1 = avr1 / tmp_size;
        double sum3 = 0.0;
        for (int tmp_y=0; tmp_y<tmp_height; ++tmp_y) {
            for (int tmp_x=0; tmp_x<tmp_width; ++tmp_x) {
                sum3 += static_cast<double>(input1(tmp_x, tmp_y)-avr1) * static_cast<double>(input1(tmp_x, tmp_y)-avr1);
            }
        }
        for (int c=0; c<img_depth; ++c) {
            for (int y=0; y<res_height; ++y) {
                for (int x=0; x<res_width; ++x) {
                    double avr0 = 0.0;
                    for (int tmp_y=0; tmp_y<tmp_height; ++tmp_y) {
                        for (int tmp_x=0; tmp_x<tmp_width; ++tmp_x) {
                            avr0 += static_cast<double>(input0(x+tmp_x, y+tmp_y, c));
                        }
                    }
                    avr0 = avr0 / tmp_size;

                    double sum1 = 0.0;
                    double sum2 = 0.0;
                    for (int tmp_y=0; tmp_y<tmp_height; ++tmp_y) {
                        for (int tmp_x=0; tmp_x<tmp_width; ++tmp_x) {
                            sum1 += static_cast<double>(input0(x+tmp_x, y+tmp_y, c)-avr0) * static_cast<double>(input1(tmp_x, tmp_y)-avr1);
                            sum2 += static_cast<double>(input0(x+tmp_x, y+tmp_y, c)-avr0) * static_cast<double>(input0(x+tmp_x, y+tmp_y, c)-avr0);
                        }
                    }
                    double expect = sum1 / sqrt(sum2 * sum3);

                    double actual = output(x, y, c);
                    if (expect != actual) {
                        throw std::runtime_error(
                            fmt::format("Error0: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                       x, y, c, expect, x, y, c, actual));
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
    if (test<uint8_t>(tm_zncc_u8)) return 1;
#endif
#ifdef TYPE_u16
    if (test<uint16_t>(tm_zncc_u16)) return 1;
#endif
#ifdef TYPE_u32
    if (test<uint32_t>(tm_zncc_u32)) return 1;
#endif
    return 0;
}
