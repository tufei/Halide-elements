#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "close_cross_u8.h"
#include "close_cross_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

// returns index of result workbuf
template<typename T>
int conv_cross(int width, int height, int depth, int window_width, int window_height, int iteration,
              T* workbuf, const T&(*f)(const T&, const T&), T init, int k) {
    //T (*workbuf)[width][height][depth] = reinterpret_cast<T (*)[width][height][depth]>(workbuf_ptr);

    int itr;
    for (itr=k; itr<k+iteration; ++itr) {
      for (int c=0; c<depth; ++c) {
        for (int y=0; y<height; ++y) {
            for (int x=0; x<width; ++x) {
                T minx = init, miny = init;
                for (int j = -(window_height/2); j < -(window_height/2) + window_height; j++) {
                    int yy = y + j >= 0 ? y + j: 0;
                    yy = yy < height ? yy : height - 1;
                    miny = f(miny, workbuf[(itr%2)*width*height*depth + x*height*depth + yy*depth + c]);
                }
                for (int i = -(window_width/2); i < -(window_width/2) + window_width; i++) {
                    int xx = x + i >= 0 ? x + i: 0;
                    xx = xx < width ? xx : width - 1;
                    minx = f(minx, workbuf[(itr%2)*width*height*depth + xx*height*depth + y*depth + c]);
                }
                workbuf[((itr+1)%2)*width*height*depth + x*height*depth + y*depth + c] = f(minx, miny);
            }
        }
      }
    }
    return itr;
}

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_workbuf__1_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const int window_width = 3;
        const int window_height = 3;
        const int iteration = 2;
        const std::vector<int32_t> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);
        //T (*expect)[width][height][depth], workbuf[2][width][height][depth];
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
        
        // dilate
        int k = conv_cross(width, height, depth, window_width, window_height, iteration,
                                &workbuf[0][0][0][0], static_cast<const T&(*)(const T&, const T&)>(std::max), std::numeric_limits<T>::min(), k);

        // erode
        k = conv_cross(width, height, depth, window_width, window_height, iteration,
                                    &workbuf[0][0][0][0], static_cast<const T&(*)(const T&, const T&)>(std::min), std::numeric_limits<T>::max(), 0);
        expect = &(workbuf[k%2]);

        const auto &result = benchmark([&]() {
            func(input, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T actual = output(x, y, c);
                    if ((*expect)[x][y][c] != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d", x, y, c, (*expect)[x][y][c], x, y, c, actual).c_str());
                    }
                }
            }
          }
        delete[] tmp;
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
    test<uint8_t>(close_cross_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(close_cross_u16);
#endif
}
