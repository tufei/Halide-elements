#include <iostream>
#include <limits>
#include "Halide.h"

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class ScaleNN : public Halide::Generator<ScaleNN<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> in_width{"in_width", 1024};
    GeneratorParam<int32_t> in_height{"in_height", 768};

    GeneratorParam<int32_t> out_width{"out_width", 500};
    GeneratorParam<int32_t> out_height{"out_height", 500};

    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = scale_NN<T>(src, in_width, in_height, out_width, out_height, depth);

        schedule(src, {in_width, in_height, depth});
        schedule(dst, {out_width, out_height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(ScaleNN<uint8_t>, scale_NN_u8);
HALIDE_REGISTER_GENERATOR(ScaleNN<uint16_t>, scale_NN_u16);
HALIDE_REGISTER_GENERATOR(ScaleNN<int16_t>, scale_NN_i16);
