#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class AndScalar : public Halide::Generator<AndScalar<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorInput<T> value{"value", 1};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = and_scalar(src, value);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            value.set_estimate(255);
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(AndScalar<uint8_t>, and_scalar_u8);
HALIDE_REGISTER_GENERATOR(AndScalar<uint16_t>, and_scalar_u16);
HALIDE_REGISTER_GENERATOR(AndScalar<uint32_t>, and_scalar_u32);
