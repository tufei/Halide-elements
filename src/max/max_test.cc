#include <cstdlib>
#include <string>
#include <exception>
#include <climits>

#include "max_u8.h"
#include "max_u16.h"
#include "max_u32.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer1, struct halide_buffer_t *_src_buffer2, struct halide_buffer_t *_dst_buffer))
{
    try {
        //
        // Run
        //
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        const std::vector<int> extents{width, height, depth};
        auto input0 = mk_rand_buffer<T>(extents);
        auto input1 = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input0.set_host_dirty();
        input1.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input0, input1, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        for (int c = 0; c < depth; ++c) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    T expect = std::max(input0(x, y, c), input1(x, y, c));
                    T actual = output(x, y, c);

                    if (expect != actual) {
                        const auto s = fmt::format(
                            "Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}\n",
                            x, y, c, expect, x, y, c, actual);
                        throw std::runtime_error(s);
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

int main(int argc, char **argv)
{
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

#ifdef TYPE_u8
    test<uint8_t>(max_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(max_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(max_u32);
#endif
}
