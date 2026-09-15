#include <cstdlib>
#include <string>
#include <exception>

#include "average_value_u8_f32.h"
#include "average_value_u16_f32.h"
#include "average_value_u8_f64.h"
#include "average_value_u16_f64.h"

#include "halide_benchmark.h"
#include "test_common.h"

using namespace Halide::Tools;

template <typename S, typename D>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                    struct halide_buffer_t *_roi_buffer,
                    struct halide_buffer_t *_dst_buffer))
{
    try{
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};

        std::vector<int> extents{width, height};
        auto roi = mk_rand_buffer<uint8_t>(extents);

        extents.push_back(depth);
        auto input = mk_rand_buffer<S>(extents);
        auto output = mk_null_buffer<D>({1, depth});

        input.set_host_dirty();
        roi.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, roi, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        //reference
        D expect;
        for (int c = 0; c < depth; c++) {
            double sum = 0;
            int count = 0;
            int els = 0;

            for (int j = 0; j < width; j++) {
                for (int i = 0; i < height; i++) {
                    if (roi(j, i) != 0) {
                        sum += input(j, i, c);
                        count++;
                    }
                }
            }
            expect = static_cast<D>(sum/count);

            D actual = output(0, c);
            if (expect != actual){
                throw std::runtime_error(fmt::format("Error at channel {}: expect = {}, actual = {}",
                                                     c, expect, actual));
            }
        }
    } catch (const std::exception& e){
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}

int main(){
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

    #ifdef TYPE_u8_f32
        test<uint8_t, float>(average_value_u8_f32);
    #endif
    #ifdef TYPE_u16_f32
        test<uint16_t, float>(average_value_u16_f32);
    #endif
    #ifdef TYPE_u8_f64
        test<uint8_t, double>(average_value_u8_f64);
    #endif
    #ifdef TYPE_u16_f64
        test<uint16_t, double>(average_value_u16_f64);
    #endif
}
