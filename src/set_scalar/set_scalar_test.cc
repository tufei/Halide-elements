#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "set_scalar_u8.h"
#include "set_scalar_u16.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

template<typename T>
int test(int (*func)(T _value, struct halide_buffer_t *_dst_buffer)) {
    try {
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
        const T value = mk_rand_scalar<T>(); //input scalar
        auto output = mk_null_buffer<T>(extents);

        const auto &result = benchmark([&]() {
            func(value, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        //for each x and y
        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    if (value != output(x, y, c)) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d",
                                                        x, y, c, value, x, y, c, output(x, y, c)).c_str());
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

int main(int argc, char **argv) {
#ifdef TYPE_u8
    test<uint8_t>(set_scalar_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(set_scalar_u16);
#endif

}
