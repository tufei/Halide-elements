#include <cstdint>

#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class MulScalar : public Halide::Generator<MulScalar<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};
    GeneratorInput<Buffer<float>> value{"value", 1};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = mul_scalar<T>(src, value);

        schedule(src, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(MulScalar<uint8_t>, mul_scalar_u8);
HALIDE_REGISTER_GENERATOR(MulScalar<uint16_t>, mul_scalar_u16);
HALIDE_REGISTER_GENERATOR(MulScalar<uint32_t>, mul_scalar_u32);
