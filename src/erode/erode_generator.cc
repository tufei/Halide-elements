#include <iostream>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Erode : public Halide::Generator<Erode<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};
    GeneratorInput<Buffer<uint8_t>> structure{"structure", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> iteration{"iteration", 2};

    GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
    GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = erode<T>(src, width, height, depth, window_width, window_height, structure, iteration);

        schedule(src, {width, height, depth});
        schedule(structure, {window_width, window_height});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(Erode<uint8_t>, erode_u8);
HALIDE_REGISTER_GENERATOR(Erode<uint16_t>, erode_u16);
