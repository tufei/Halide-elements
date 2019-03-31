#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "mul_scalar_u8.h"
#include "mul_scalar_u16.h"
#include "mul_scalar_u32.h"

#include "test_common.h"

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_value, struct halide_buffer_t *_dst_buffer))
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
        auto value = mk_rand_buffer<float>({depth});
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        func(input, value, output);

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T expect = input(x, y, c);
                    T actual = output(x, y, c);
                    float f = expect * value(c);
                    f = std::min(static_cast<float>(std::numeric_limits<T>::max()), f);
                    f = std::max(0.0f, f);
                    expect = round_to_nearest_even<T>(f);
                    if (expect != actual) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d", x, y, c, expect, x, y, c, actual).c_str());
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
    test<uint8_t>(mul_scalar_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(mul_scalar_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(mul_scalar_u32);
#endif
}
