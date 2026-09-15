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
        constexpr int width = 1024;
        constexpr int height = 768;
        constexpr int depth = 3;
        const std::vector<int> extents{width, height, depth};
        const T value = mk_rand_scalar<T>(); //input scalar
        auto output = mk_null_buffer<T>(extents);

        const auto result = benchmark([&]() {
            func(value, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        //for each x and y
        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    if (value != output(x, y, c)) {
                        throw std::runtime_error(fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}",
                                                        x, y, c, value, x, y, c, output(x, y, c)));
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}

int main(int argc, char **argv) {
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA
#ifdef TYPE_u8
    test<uint8_t>(set_scalar_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(set_scalar_u16);
#endif

}
