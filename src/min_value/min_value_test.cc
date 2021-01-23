#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include "min_value_u8.h"
#include "min_value_u16.h"
#include "min_value_u32.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;

using std::string;
using std::vector;

template <typename T>
T min_value_ref(const Halide::Runtime::Buffer<T>& src, const Halide::Runtime::Buffer<uint8_t>& roi, const int width, const int height, const int depth) {

    T min = std::numeric_limits<T>::has_infinity
        ? std::numeric_limits<T>::infinity()
        : (std::numeric_limits<T>::max)();
    int count = 0;

    for (int c = 0; c < depth; c++) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (roi(j, i, c) != 0) {
                    T val = src(j, i, c);
                    min = val < min ? val : min;
                    count++;
                }
            }
        }
    }
    return (count == 0) ? 0 : min;
}

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_roi_buffer, struct halide_buffer_t *_dst_buffer))
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
        auto roi = mk_rand_buffer<uint8_t>(extents);
        auto output = mk_null_buffer<T>({1});

        const auto &result = benchmark([&]() {
            func(input, roi, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        T expect = min_value_ref<T>(input, roi, width, height, depth);
        T actual = output(0);
        if (expect != actual) {
            throw std::runtime_error(format("Error: expect = %u, actual = %u\n",
                                            static_cast<uint64_t>(expect), static_cast<uint64_t>(actual)));
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
    test<uint8_t>(min_value_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(min_value_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(min_value_u32);
#endif
}
