#include <cstdint>
#include <Halide.h>

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class DilateCross : public Halide::Generator<DilateCross<T>> {
public:
    GeneratorInput<Buffer<T>> input{"input", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> iteration{"iteration", 2};
    GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
    GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

    GeneratorOutput<Buffer<T>> output{"output", 3};

    void generate() {
        output = dilate_cross<T>(input, width, height, depth, window_width, window_height, iteration);

        schedule(input, {width, height, depth});
        schedule(output, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(DilateCross<uint8_t>, dilate_cross_u8);
HALIDE_REGISTER_GENERATOR(DilateCross<uint16_t>, dilate_cross_u16);
