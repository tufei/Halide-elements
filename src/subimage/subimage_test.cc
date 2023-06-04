#include <cstdlib>
#include <iostream>

#include "halide_benchmark.h"

#include "subimage_u8.h"
#include "subimage_u16.h"
#include "subimage_u32.h"

#include "test_common.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     uint32_t _origin_x, uint32_t _origin_y,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        constexpr int in_width{1024};
        constexpr int in_height{768};
        constexpr int in_depth{3};

        constexpr int out_width{500};
        constexpr int out_height{500};
        constexpr int out_depth{3};

        constexpr auto xLimit = in_width - out_width;
        constexpr auto yLimit = in_height - out_height;
        const int origin_x = mk_rand_scalar<uint32_t>() % xLimit;
        const int origin_y = mk_rand_scalar<uint32_t>() % yLimit;

        const std::vector<int> in_extents{in_width, in_height, in_depth};
        const std::vector<int> out_extents{out_width, out_height, out_depth};
        auto input = mk_rand_buffer<T>(in_extents);
        auto output = mk_null_buffer<T>(out_extents);
        auto expect = mk_null_buffer<T>(out_extents);

        input.set_host_dirty();

        for(int c = 0; c < out_depth; c++) {
            for(int i = 0; i < out_height; i++) {
                for (int j = 0; j < out_width; j++) {
                    expect(j, i, c) = input(origin_x + j, origin_y + i, c);
                }
            }
        }

        const auto &result = benchmark([&]() {
            func(input, origin_x, origin_y, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        //for each x, y, and c
        for (int c = 0; c < out_depth; ++c) {
            for (int i = 0; i < out_height; ++i) {
                for (int j = 0; j < out_width; ++j) {
                    if (expect(j, i, c) != output(j, i, c)) {
                        const auto s =
                            fmt::format("Error: expect({}, {}, {}) = {}, "
                                        "actual({}, {}, {}) = {}",
                                        j, i, c, expect(j, i, c),
                                        j, i, c, output(j, i, c));
                        throw std::runtime_error(s);
                    }
                }
            }
        }

    } catch (const std::exception& e){
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
    test<uint8_t>(subimage_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(subimage_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(subimage_u32);
#endif
}
