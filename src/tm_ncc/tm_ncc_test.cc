#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"
#include "halide_benchmark.h"

#include "tm_ncc_u8.h"
#include "tm_ncc_u16.h"
#include "tm_ncc_u32.h"

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
        const int img_width = 1024;
        const int img_height = 768;
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

        const auto &result = benchmark([&]() {
            func(input0, input1, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        for (int c=0; c<img_depth; ++c) {
            for (int y=0; y<res_height; ++y) {
                for (int x=0; x<res_width; ++x) {
                    double sum1 = 0.0;
                    double sum2 = 0.0;
                    double sum3 = 0.0;
                    for (int tmp_y=0; tmp_y<tmp_height; ++tmp_y) {
                        for (int tmp_x=0; tmp_x<tmp_width; ++tmp_x) {
                            sum1 += static_cast<double>(input0(x+tmp_x, y+tmp_y, c)) * static_cast<double>(input1(tmp_x, tmp_y));
                            sum2 += static_cast<double>(input0(x+tmp_x, y+tmp_y, c)) * static_cast<double>(input0(x+tmp_x, y+tmp_y, c));
                            sum3 += static_cast<double>(input1(tmp_x, tmp_y)) * static_cast<double>(input1(tmp_x, tmp_y));
                        }
                    }
                    double expect = sum1 / sqrt(sum2 * sum3);

                    double actual = output(x, y, c);
                    if (expect != actual) {
                        throw std::runtime_error(format("Error0: expect(%d, %d, %d) = %f, actual(%d, %d, %d) = %f", x, y, c, expect, x, y, c, actual).c_str());
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    printf("Success!\n");
    return 0;
}

int main()
{
#ifdef TYPE_u8
    test<uint8_t>(tm_ncc_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(tm_ncc_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(tm_ncc_u32);
#endif
}
