#include <cstdint>

#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class DivScalar : public Halide::Generator<DivScalar<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};
    GeneratorInput<double> value{"value", 2.0};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = div_scalar<T>(src, value);

        schedule(src, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(DivScalar<uint8_t>, div_scalar_u8);
HALIDE_REGISTER_GENERATOR(DivScalar<uint16_t>, div_scalar_u16);
HALIDE_REGISTER_GENERATOR(DivScalar<uint32_t>, div_scalar_u32);
