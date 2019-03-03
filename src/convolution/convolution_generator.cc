#include <cstdint>

#include <Halide.h>
#include <Element.h>
#include "ImageProcessing.h"

using namespace Halide;
using namespace Halide::Element;

class Convolution : public Halide::Generator<Convolution> {
public:
    GeneratorInput<Buffer<uint8_t>> in{"in", 3};
    GeneratorInput<Buffer<int16_t>> kernel{"kernel", 2};
    GeneratorInput<int32_t> kernel_size{"kernel_size", 3, 1, 5};

    GeneratorParam<int32_t> width{"width", 512};
    GeneratorParam<int32_t> height{"height", 512};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> unroll_factor{"unroll_factor", 2};

    GeneratorOutput<Buffer<uint8_t>> out{"out", 3};

    void generate() {
        out = convolution<uint8_t, int16_t, 16, 10>(in, width, height, depth, kernel, kernel_size, unroll_factor);

        schedule(in, {width, height, depth});
        schedule(kernel, {5, 5});
        schedule(out, {width, height, depth});

        if (unroll_factor) {
            out.unroll(out.args()[0], unroll_factor);
        }
    }
};

HALIDE_REGISTER_GENERATOR(Convolution, convolution)
