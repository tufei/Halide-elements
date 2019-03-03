#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include "copy_u8.h"
#include "copy_u16.h"
#include "test_common.h"

using std::string;
using std::vector;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        func(input, output);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T expect = input(x, y, c);
                    T actual = output(x, y, c);
                    if (expect != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d",
                                                        x, y, c, static_cast<uint64_t>(expect),
                                                        x, y, c, static_cast<uint64_t>(actual)).c_str());
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
    test<uint8_t>(copy_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(copy_u16);
#endif
}
