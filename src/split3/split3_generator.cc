#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Split3 : public Halide::Generator<Split3<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};

    GeneratorOutput<Buffer<>> dst{"dst", {type_of<T>(), type_of<T>(), type_of<T>()}, 2};

    void generate() {
        dst = split3<T>(src, width, height);

        schedule(src, {width, height, 3});
        //schedule(dst, {width, height});
    }
};

HALIDE_REGISTER_GENERATOR(Split3<uint8_t>, split3_u8);
HALIDE_REGISTER_GENERATOR(Split3<uint16_t>, split3_u16);
HALIDE_REGISTER_GENERATOR(Split3<int8_t>, split3_i8);
HALIDE_REGISTER_GENERATOR(Split3<int16_t>, split3_i16);
