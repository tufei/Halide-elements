#include <iostream>
#include <limits>
#include "Halide.h"

#include "Element.h"

using namespace Halide;

template<typename T>
class Average : public Halide::Generator<Average<T>> {
    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> window_width{"window_width", 3};
    GeneratorParam<int32_t> window_height{"window_height", 3};
    ImageParam src{type_of<T>(), 2, "src"};

public:
    Func build() {
        Func dst("dst");
        dst = Element::average<T>(src, window_width, window_height);

        Element::schedule(src, {width, height});
        Element::schedule(dst, {width, height});

        return dst;
    }
};

RegisterGenerator<Average<uint8_t>> average_u8{"average_u8"};
RegisterGenerator<Average<uint16_t>> average_u16{"average_u16"};
