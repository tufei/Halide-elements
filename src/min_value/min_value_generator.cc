#include <cstdint>
#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class MinValue : public Halide::Generator<MinValue<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};
    GeneratorInput<Buffer<uint8_t>> roi{"roi", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 1};

    void generate() {
        dst = min_value<T>(src, roi, width, height, depth);

        schedule(src, {width, height, depth});
        schedule(roi, {width, height, depth});
        schedule(dst, {1});
    }
};

HALIDE_REGISTER_GENERATOR(MinValue<uint8_t>, min_value_u8);
HALIDE_REGISTER_GENERATOR(MinValue<uint16_t>, min_value_u16);
HALIDE_REGISTER_GENERATOR(MinValue<uint32_t>, min_value_u32);
