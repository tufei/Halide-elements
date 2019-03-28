#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Median : public Halide::Generator<Median<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> window_width{"window_width", 3};
    GeneratorParam<int32_t> window_height{"window_height", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = median<T>(src, width, height, depth, window_width, window_height);

        schedule(src, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(Median<uint8_t>, median_u8);
HALIDE_REGISTER_GENERATOR(Median<uint16_t>, median_u16);
