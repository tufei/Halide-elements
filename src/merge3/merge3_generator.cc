#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Merge3 : public Halide::Generator<Merge3<T>> {
public:
    GeneratorInput<Buffer<T>> src0{"src0", 2};
    GeneratorInput<Buffer<T>> src1{"src1", 2};
    GeneratorInput<Buffer<T>> src2{"src2", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = merge3<T>(src0, src1, src2, width, height);

        schedule(src0, {width, height});
        schedule(src1, {width, height});
        schedule(src2, {width, height});
        schedule(dst, {width, height, 3});
    }
};

HALIDE_REGISTER_GENERATOR(Merge3<uint8_t>, merge3_u8);
HALIDE_REGISTER_GENERATOR(Merge3<uint16_t>, merge3_u16);
HALIDE_REGISTER_GENERATOR(Merge3<int8_t>, merge3_i8);
HALIDE_REGISTER_GENERATOR(Merge3<int16_t>, merge3_i16);
