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
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
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

        const auto &result = benchmark([&]() {
            func(input, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        // for each x and y
        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    if (expect(x, y, c) != output(x, y, c)) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %f, actual(%d, %d, %d) = %f", x, y, c, expect(x, y, c), x, y, c, output(x, y, c)).c_str());
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
