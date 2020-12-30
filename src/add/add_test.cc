#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"
#include "halide_benchmark.h"

#include "add_u8.h"
#include "add_u16.h"
#include "add_u32.h"

#include "test_common.h"

using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer0, struct halide_buffer_t *_src_buffer1, struct halide_buffer_t *_dst_buffer))
{
    try {
        using upper_t = typename Halide::Element::Upper<T>::type;

        int ret = 0;

        //
        // Run
        //
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
        auto input0 = mk_rand_buffer<T>(extents);
        auto input1 = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        const auto &result = benchmark([&]() {
            func(input0, input1, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        for (int c=0; c<depth; ++c) {
          for (int y=0; y<height; ++y) {
              for (int x=0; x<width; ++x) {
                  upper_t f = static_cast<upper_t>(input0(x, y, c)) + static_cast<upper_t>(input1(x, y, c));
                  f = std::min(f, static_cast<upper_t>(std::numeric_limits<T>::max()));
                  T expect = f;
                  T actual = output(x, y, c);

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
    test<uint8_t>(add_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(add_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(add_u32);
#endif
}
