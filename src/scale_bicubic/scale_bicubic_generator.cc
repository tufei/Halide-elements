#include <iostream>
#include <limits>
#include "Halide.h"

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class ScaleBicubic : public Halide::Generator<ScaleBicubic<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> in_width{"in_width", 1024};
    GeneratorParam<int32_t> in_height{"in_height", 768};

    GeneratorParam<int32_t> out_width{"out_width", 500};
    GeneratorParam<int32_t> out_height{"out_height", 500};

    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = scale_bicubic<T>(src, in_width, in_height,out_width, out_height,
                               depth, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            dst.set_estimates({{0, 500}, {0, 500}, {0, 3}});
        } else {
            ::schedule(src, {in_width, in_height, depth});
            ::schedule(dst, {out_width, out_height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(ScaleBicubic<uint8_t>, scale_bicubic_u8);
HALIDE_REGISTER_GENERATOR(ScaleBicubic<uint16_t>, scale_bicubic_u16);
HALIDE_REGISTER_GENERATOR(ScaleBicubic<int16_t>, scale_bicubic_i16);
