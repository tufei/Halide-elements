#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "test_common.h"
#include "halide_benchmark.h"

#include "or_u8.h"
#include "or_u16.h"
#include "or_u32.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src0_buffer,
                        struct halide_buffer_t *_src1_buffer,
                        struct halide_buffer_t *_dst_buffer))
{
    try {
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
        auto input0 = mk_rand_buffer<T>(extents);
        auto input1 = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        const auto &result = benchmark([&]() {
            func(input0, input1, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        //for each x and y
        for (int c=0; c<depth; ++c) {
            for (int j=0; j<height; ++j) {
                for (int i=0; i<width; ++i) {
                    T expect = (input0(i, j, c) | input1(i, j, c));
                    T actual = output(i, j, c);
                    if (expect != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d",
                                                    i, j, c, expect, i, j, c, actual));
                    }
                }
            }
        }
    } catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        return 1;
    }

    printf("Success!\n");
    return 0;
}

int main()
{
#ifdef TYPE_u8
    test<uint8_t>(or_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(or_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(or_u32);
#endif
}
