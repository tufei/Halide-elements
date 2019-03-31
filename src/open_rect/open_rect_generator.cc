#include <cstdint>

#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class OpenRect : public Halide::Generator<OpenRect<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> iteration{"iteration", 2};
    GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
    GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

    GeneratorOutput<Buffer<T>> dilated{"dilated", 3};

    void generate() {
        Func eroded{"eroded"};

        eroded = erode_rect<T>(src, width, height, depth, window_width, window_height, iteration);
        dilated = dilate_rect<T>(eroded, width, height, depth, window_width, window_height, iteration);

        schedule(src, {width, height, depth});
        schedule(eroded, {width, height, depth});
        schedule(dilated, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(OpenRect<uint8_t>, open_rect_u8);
HALIDE_REGISTER_GENERATOR(OpenRect<uint16_t>, open_rect_u16);
