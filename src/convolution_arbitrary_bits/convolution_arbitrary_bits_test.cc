#include <cstdlib>
#include <iostream>
#include <exception>

#include "convolution_arbitrary_bits.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

#define BORDER_INTERPOLATE(x, l) (x < 0 ? 0 : (x >= l ? l - 1 : x))

int main(int argc, char **argv) {
    try {

        const int width = 512;
        const int height = 512;
        const int depth = 3;
        Buffer<uint8_t> input = mk_const_buffer<uint8_t>({width, height, depth}, 1);

        using fixed20_t = int32_t;
        constexpr uint32_t frac_bits = 10;
        constexpr uint32_t base_bits = 20;
        const fixed20_t kv = static_cast<fixed20_t>(round(1.0 / 9.0 * (1 << frac_bits)));
        fixed20_t kernel_data[5][5] = {
            { kv, kv, kv, 0, 0},
            { kv, kv, kv, 0, 0},
            { kv, kv, kv, 0, 0},
            { 0,  0,  0,  0, 0},
            { 0,  0,  0,  0, 0}
        };

        Buffer<fixed20_t> kernel(reinterpret_cast<fixed20_t*>(kernel_data), 5, 5);

        Buffer<uint8_t> output(width, height, depth);

        const auto &result = benchmark([&]() {
            convolution_arbitrary_bits(input, kernel, 3, output); });
        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    fixed20_t s = 0;
                    for (int ry=-1; ry<=1; ry++) {
                        for (int rx=-1; rx<=1; rx++) {
                            int cx = BORDER_INTERPOLATE(x + rx, width);
                            int cy = BORDER_INTERPOLATE(y + ry, height);
                            // Simulate add & mult of Fixed<base_bits, frac_bits>
                            s += ((kernel(rx + 1, ry + 1) * (input(cx, cy, c) << frac_bits)) >> frac_bits);
                            // Bitmask for simulating overflow
                            s = s & ((1 << base_bits) - 1);
                            // Sign extension
                            s = s << (base_bits - frac_bits) >> (base_bits - frac_bits);
                        }
                    }
                    uint8_t ev = s >> frac_bits;
                    uint8_t av = output(x, y, c);
                    if (ev != av) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d", x, y, c, ev, x, y, c, av).c_str());
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
