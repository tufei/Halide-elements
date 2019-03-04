#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "dilate_u8.h"
#include "dilate_u16.h"

#include "test_common.h"

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

        int k;
        for (k=0; k<iteration; ++k) {
            for (int c=0; c<depth; ++c) {
                for (int y=0; y<height; ++y) {
                    for (int x=0; x<width; ++x) {
                        T max = std::numeric_limits<T>::min();
                        for (int j = -(window_height/2); j < -(window_height/2) + window_height; j++) {
                            int yy = std::min(std::max(0, y + j), height - 1);
                            for (int i = -(window_width/2); i < -(window_width/2) + window_width; i++) {
                                if (structure(window_width/2+i, window_height/2+j) ||
                                    (allzero && i == -(window_width/2) && j == -(window_height/2))) {
                                    int xx = std::min(std::max(0, x + i), width - 1);
                                    if (max < workbuf[k%2][xx][yy][c]) {
                                        max = workbuf[k%2][xx][yy][c];
                                    }
                                }
                            }
                        }
                        workbuf[(k+1)%2][x][y][c] = max;
                    }
                }
            }
        }
        expect = &(workbuf[k%2]);

        func(input, structure, output);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T actual = output(x, y, c);
                    if ((*expect)[x][y][c] != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d", x, y, c, (*expect)[x][y], x, y, c, actual).c_str());
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
    test<uint8_t>(dilate_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(dilate_u16);
#endif
}
