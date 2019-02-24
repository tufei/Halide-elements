#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "close_u8.h"
#include "close_u16.h"

#include "test_common.h"


// returns index of result workbuf
template<typename T>
int conv_with_structure(int width, int height, int depth, int window_width, int window_height, int iteration,
              const Halide::Runtime::Buffer<uint8_t>& structure,
              T* workbuf, const T&(*f)(const T&, const T&), T init, bool allzero, int k) {
//    T (workbuf)[width][height][depth] = reinterpret_cast<T (*)[width][height][depth]>(workbuf_ptr);

    int itr;
    for (itr=k; itr<k+iteration; ++itr) {
        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T min = init;
                    for (int j = -(window_height/2); j < -(window_height/2) + window_height; j++) {
                        int yy = y + j >= 0 ? y + j: 0;
                        yy = yy < height ? yy : height - 1;
                        for (int i = -(window_width/2); i < -(window_width/2) + window_width; i++) {
                            if (structure(window_width/2+i, window_height/2+j) ||
                                (allzero && i == -(window_width/2) && j == -(window_height/2))) {
                                int xx = x + i >= 0 ? x + i: 0;
                                xx = xx < width ? xx : width - 1;
                                min = f(min, workbuf[(itr%2)*width*height*depth + xx*height*depth + yy*depth + c]);
                            }
                        }
                    }
                    workbuf[((itr+1)%2)*width*height*depth + x*height*depth + y*depth + c] = min;
                }
            }
        }
    }
    return itr;
}

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_structure_buffer, struct halide_buffer_t *_workbuf__1_buffer))
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
        const std::vector<int32_t> extents{width, height, depth}, extents_structure{window_width, window_height};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);
        auto structure = mk_rand_buffer<uint8_t>(extents_structure);
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

        bool allzero = true;
        for (int y=0; y<window_height; ++y) {
            for (int x=0; x<window_width; ++x) {
                if (structure(x, y)) {
                    allzero = false;
                }
            }
        }
        
        // dilate
        int k = conv_with_structure(width, height, depth, window_width, window_height, iteration, structure,
                                &workbuf[0][0][0][0], static_cast<const T&(*)(const T&, const T&)>(std::max), std::numeric_limits<T>::min(), allzero, 0);

        // erode
        k = conv_with_structure(width, height, depth, window_width, window_height, iteration, structure,
                                    &workbuf[0][0][0][0], static_cast<const T&(*)(const T&, const T&)>(std::min), std::numeric_limits<T>::max(), allzero, k);
        expect = &(workbuf[k%2]);

        func(input, structure, output);

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
    test<uint8_t>(close_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(close_u16);
#endif
}
