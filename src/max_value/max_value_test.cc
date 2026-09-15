#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include "max_value_u8.h"
#include "max_value_u16.h"
#include "max_value_u32.h"
#include "test_common.h"
#include "halide_benchmark.h"

using std::string;
using std::vector;

using namespace Halide::Tools;

template <typename T, bool has_infinity>
struct initial {};

template <typename T>
struct initial<T, true> {
  static T value(void) { return -std::numeric_limits<T>::infinity(); }
};

template <typename T>
struct initial<T, false> {
  static T value(void) { return (std::numeric_limits<T>::min)(); }
};

template <typename T>
T max_value_ref(const Halide::Runtime::Buffer<T>& src, const Halide::Runtime::Buffer<uint8_t>& roi, const int width, const int height, const int depth) {

    T max = initial<T, std::numeric_limits<T>::has_infinity>::value();
    int count = 0;

    for (int c = 0; c < depth; c++) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (roi(j, i, c) != 0) {
                    T val = src(j, i, c);
                    max = val > max ? val : max;
                    count++;
                }
            }
        }
    }
    return (count == 0) ? 0 : max;
}

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer, struct halide_buffer_t *_roi_buffer, struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;

        //
        // Run
        //
        constexpr int width{1024};
        constexpr int height{768};
        constexpr int depth{3};
        const std::vector<int> extents{width, height, depth};
        auto input = mk_rand_buffer<T>(extents);
        auto roi = mk_rand_buffer<uint8_t>(extents);
        auto output = mk_null_buffer<T>({1});

        input.set_host_dirty();
        roi.set_host_dirty();

        const auto result = benchmark([&]() {
            func(input, roi, output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

        T expect = max_value_ref<T>(input, roi, width, height, depth);
        T actual = output(0);
        if (expect != actual) {
            throw std::runtime_error(fmt::format("Error: expect = {}, actual = {}\n",
                                                 static_cast<uint64_t>(expect), static_cast<uint64_t>(actual)));
        }
    } catch (const std::exception& e) {
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
    test<uint8_t>(max_value_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(max_value_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(max_value_u32);
#endif
}
