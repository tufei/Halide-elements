#include <cstdint>
#include "Halide.h"
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Copy : public Halide::Generator<Copy<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = copy(src);

        schedule(src, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(Copy<uint8_t>, copy_u8);
HALIDE_REGISTER_GENERATOR(Copy<uint16_t>, copy_u16);
