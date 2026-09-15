#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "split4_u8.h"
#include "split4_u16.h"
#include "split4_i8.h"
#include "split4_i16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     struct halide_buffer_t *_dst_buffer0,
                     struct halide_buffer_t *_dst_buffer1,
                     struct halide_buffer_t *_dst_buffer2,
                     struct halide_buffer_t *_dst_buffer3))
{
    try {
        constexpr int N{4};
        constexpr int width{1024};
        constexpr int height{768};
        const std::vector<int> in_extents{width, height, N};
        const std::vector<int> out_extents{width, height};
        auto input = mk_null_buffer<T>(in_extents);
        Halide::Runtime::Buffer<T> output[N];
        for (int i = 0; i < N; ++i) {
            output[i] = mk_rand_buffer<T>(out_extents);
        }
        T *tmp = new T[width * height * N];
        T (*expect)[height][N] = reinterpret_cast<T (*)[height][N]>(tmp);

        //ref
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                for (int c = 0; c < N; c++) {
                    expect[x][y][c] = input(x, y, c);
                }
            }
        }

        input.set_host_dirty();
        for (int i = 0; i < N; ++i) {
            output[i].set_host_dirty();
        }

        const auto result = benchmark([&]() {
            func(input, output[0], output[1], output[2], output[3]);
            output[0].device_sync();
            output[1].device_sync();
            output[2].device_sync();
            output[3].device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        for (int i = 0; i < N; ++i) {
            output[i].copy_to_host();
        }

        for (int y=0; y<height; ++y) {
            for (int x=0; x<width; ++x) {
                for (int c = 0; c < N; c++) {
                    T actual = output[c](x, y);
                    if (expect[x][y][c] != actual) {
                        throw std::runtime_error(fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                                             x, y, c, expect[x][y][c], x, y, c, actual));
                    }
                }
            }
        }
        delete[] tmp;
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
    test<uint8_t>(split4_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(split4_u16);
#endif
#ifdef TYPE_i8
    test<int8_t>(split4_i8);
#endif
#ifdef TYPE_i16
    test<int16_t>(split4_i16);
#endif
}
