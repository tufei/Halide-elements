#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "merge3_u8.h"
#include "merge3_u16.h"
#include "merge3_i8.h"
#include "merge3_i16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer1,
                     struct halide_buffer_t *_src_buffer2,
                     struct halide_buffer_t *_src_buffer3,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;
        constexpr unsigned int N = 3; 
        //
        // Run
        //
        constexpr int width{1024};
        constexpr int height{768};
        const std::vector<int> in_extents{width, height};
        const std::vector<int> out_extents{width, height, (int)N};
        Halide::Runtime::Buffer<T> input[N];
        for (int i = 0; i < N; ++i) {
            input[i] = mk_rand_buffer<T>(in_extents);
            input[i].set_host_dirty();
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

        const auto result = benchmark([&]() {
            func(input[0], input[1], input[2], output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int y=0; y<height; ++y) {
            for (int x=0; x<width; ++x) {
                for (int c = 0; c < N; c++) {
                    T actual = output(x, y, c);
                    if (expect[y * width * N + x * N + c] != actual) {
                        throw std::runtime_error(fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                                             x, y, c, expect[y * width * N + x * N + c], x, y, c, actual));
                     }
                }
            }
        }
        delete[] expect;
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
    test<uint8_t>(merge3_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(merge3_u16);
#endif
#ifdef TYPE_i8
    test<int8_t>(merge3_i8);
#endif
#ifdef TYPE_i16
    test<int16_t>(merge3_i16);
#endif
}
