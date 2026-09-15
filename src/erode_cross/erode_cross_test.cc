#include <cstdlib>
#include <string>
#include <exception>
#include <climits>

#include "erode_cross_u8.h"
#include "erode_cross_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_workbuf__1_buffer))
{
    try {
        //
        // Run
        //
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        constexpr int window_width{3};
        constexpr int window_height{3};
        constexpr int iteration{2};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);
        input.set_host_dirty();
        T (*expect)[width][height][depth];
        T *tmp = new T[2 * width * height * depth];
        T (*workbuf)[width][height][depth] = reinterpret_cast<T (*)[width][height][depth]>(tmp);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    workbuf[0][x][y][c] = input(x, y, c);
                }
            }
        }

        int k;
        for (k=0; k<iteration; ++k) {
            for (int c=0; c<depth; ++c) {
                for (int y=0; y<height; ++y) {
                    for (int x=0; x<width; ++x) {
                        T minx = std::numeric_limits<T>::max(), miny = std::numeric_limits<T>::max();
                        for (int j = -(window_height/2); j < -(window_height/2) + window_height; j++) {
                            int yy = std::min(std::max(0, y + j), height - 1);
                            if (miny > workbuf[k%2][x][yy][c]) {
                                miny = workbuf[k%2][x][yy][c];
                            }
                        }
                        for (int i = -(window_width/2); i < -(window_width/2) + window_width; i++) {
                            int xx = std::min(std::max(0, x + i), width - 1);
                            if (minx > workbuf[k%2][xx][y][c]) {
                                minx = workbuf[k%2][xx][y][c];
                            }
                        }
                        workbuf[(k+1)%2][x][y][c] = minx < miny ? minx : miny;
                    }
                }
            }
        }
        expect = &(workbuf[k%2]);

        const auto result = benchmark([&]() {
            func(input, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T actual = output(x, y, c);
                    if ((*expect)[x][y][c] != actual) {
                        throw std::runtime_error(
                            fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                        x, y, c, (*expect)[x][y][c], x, y, c, actual));
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
    test<uint8_t>(erode_cross_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(erode_cross_u16);
#endif
}
