#include <cstdint>

#include <Halide.h>
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

class Convolution : public Halide::Generator<Convolution> {
public:
    static constexpr uint32_t NB = 20;
    static constexpr uint32_t FB = 10;
    static constexpr uint32_t UB = 32;

    GeneratorInput<Buffer<uint8_t>> in{"in", 3};
#if defined(HALIDE_FOR_FPGA)
    GeneratorInput<Buffer<Int(NB)>> kernel{"kernel", 2};
#else
    GeneratorInput<Buffer<int32_t>> kernel{"kernel", 2};
#endif
    GeneratorInput<int32_t> kernel_size{"kernel_size", 3, 1, 5};

    GeneratorParam<int32_t> width{"width", 512};
    GeneratorParam<int32_t> height{"height", 512};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> unroll_factor{"unroll_factor", 2};

    GeneratorOutput<Buffer<uint8_t>> out{"out", 3};

    void generate() {
        out = convolution<uint8_t, int32_t, NB, FB>(in, width, height, depth,
                                                    kernel, kernel_size,
                                                    unroll_factor,
                                                    auto_schedule);
    }

    void schedule() {
        if (auto_schedule) {
            in.set_estimates({{0, 512}, {0, 512}, {0, 3}});
            kernel.set_estimates({{0, 3}, {0, 3}});
            kernel_size.set_estimate(3);
            out.set_estimates({{0, 512}, {0, 512}, {0, 3}});
        } else {
            ::schedule(in, {width, height, depth});
            ::schedule(kernel, {5, 5});
            ::schedule(out, {width, height, depth});

            if (unroll_factor) {
                out.unroll(out.args()[0], unroll_factor);
            }
        }
    }
};

HALIDE_REGISTER_GENERATOR(Convolution, convolution_arbitrary_bits)
