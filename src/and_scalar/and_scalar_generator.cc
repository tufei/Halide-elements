#include <iostream>
#include <limits>
#include "Halide.h"

#include "Element.h"

using namespace Halide;

template<typename T>
class AndScalar : public Halide::Generator<AndScalar<T>> {
    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    ImageParam src{type_of<T>(), 2, "src"};
    Param<T> value{"value", 1};

public:
    Func build() {
        Func dst("dst");
        dst = Element::and_scalar<T>(src, value);

        Element::schedule(src, {width, height});
        Element::schedule(dst, {width, height});

        return dst;
    }
};

RegisterGenerator<AndScalar<uint8_t>> and_scalar_u8{"and_scalar_u8"};
RegisterGenerator<AndScalar<uint16_t>> and_scalar_u16{"and_scalar_u16"};
RegisterGenerator<AndScalar<uint32_t>> and_scalar_u32{"and_scalar_u32"};
