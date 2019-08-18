#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "threshold_min_u8.h"
#include "threshold_min_u16.h"

#include "test_common.h"

template<typename T>
Halide::Runtime::Buffer<T>& min_ref(Halide::Runtime::Buffer<T>& dst,
                                const Halide::Runtime::Buffer<T>& src,
                                const int32_t width, const int32_t height,
                                const int32_t depth, const T threshold)
{
    for(int c=0; c<depth; c++) {
        for(int y=0; y<height; y++){
            for(int x=0; x<width; x++){
                T val = src(x, y, c);
                dst(x, y, c) = val > threshold ? threshold : val;
            }
        }
    }
    return dst;
}

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     T _threshold,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};

        const T threshold = mk_rand_scalar<T>();
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        func(input, threshold, output);
        auto expect = mk_rand_buffer<T>(extents);
        expect = min_ref(expect, input, width, height, depth, threshold);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T actual = output(x, y, c);
                    if (expect(x, y, c) != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d",
                                                     x, y, c, expect(x, y, c), x, y, c, actual).c_str());
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
    test<uint8_t>(threshold_min_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(threshold_min_u16);
#endif
}
