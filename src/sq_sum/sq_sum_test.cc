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

template<typename T, typename D>
int test(int (*func)(struct halide_buffer_t *_src_buffer0,  struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<D>({1, 1, depth});
        D actual_total;
        D expect_total;

        func(input, output);

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
                throw std::runtime_error(format("Error: expect_total = %f, actual_total = %f", expect_total, actual_total).c_str());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    printf("Sucess!\n");
    return 0;
}

int main()
{
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
