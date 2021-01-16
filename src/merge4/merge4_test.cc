#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "merge4_u8.h"
#include "merge4_u16.h"
#include "merge4_i8.h"
#include "merge4_i16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer0,
                     struct halide_buffer_t *_src_buffer1,
                     struct halide_buffer_t *_src_buffer2,
                     struct halide_buffer_t *_src_buffer3,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;
        constexpr unsigned int N = 4; 
        //
        // Run
        //
        const int width = 1024;
        const int height = 768;
        const std::vector<int32_t> in_extents{width, height};
        const std::vector<int32_t> out_extents{width, height, 4};
        Halide::Runtime::Buffer<T> input[N];
        for (int i = 0; i < N; ++i) {
            input[i] = mk_rand_buffer<T>(in_extents);
        }
        auto output = mk_null_buffer<T>(out_extents);

        T *expect = new T[N * height * width];

        // Reference impl.
        // first pass
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                for (int c = 0; c < N; c++) {
                    expect[y * width * N + x * N + c] = input[c](x, y);
                }
            }
        }

        const auto &result = benchmark([&]() {
            func(input[0], input[1], input[2], input[3], output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        for (int y=0; y<height; ++y) {
            for (int x=0; x<width; ++x) {
                for (int c = 0; c < N; c++) {
                    T actual = output(x, y, c);
                    if (expect[y * width * N + x * N + c] != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %u, actual(%d, %d, %d) = %u", x, y, c, expect[y * width * N + x * N + c], x, y, c, actual).c_str());
                     }
                }
            }
        }
        delete[] expect;
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
    test<uint8_t>(merge4_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(merge4_u16);
#endif
#ifdef TYPE_i8
    test<int8_t>(merge4_i8);
#endif
#ifdef TYPE_i16
    test<int16_t>(merge4_i16);
#endif
}
