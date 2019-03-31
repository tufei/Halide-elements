#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class OR : public Halide::Generator<OR<T>> {
public:
    GeneratorInput<Buffer<T>> src0{"src0", 3};
    GeneratorInput<Buffer<T>> src1{"src1", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = filter_or<T>(src0, src1);
        schedule(src0, {width, height, depth});
        schedule(src1, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(OR<uint8_t>, or_u8);
HALIDE_REGISTER_GENERATOR(OR<uint16_t>, or_u16);
HALIDE_REGISTER_GENERATOR(OR<uint32_t>, or_u32);
