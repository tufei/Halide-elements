#include <cstdint>
#include <Halide.h>

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Close : public Halide::Generator<Close<T>> {
public:
    GeneratorInput<Buffer<T>> input{"input", 3};
    GeneratorInput<Buffer<uint8_t>> structure{"structure", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> iteration{"iteration", 2};
    GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
    GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

    GeneratorOutput<Buffer<T>> eroded{"eroded", 3};

    void generate() {
        if (this->auto_schedule) {
            dilated = dilate_pure<T>(input, width, height, depth,
                                     window_width, window_height,
                                     structure, iteration);
            eroded = erode_pure<T>(dilated, width, height, depth,
                                   window_width, window_height,
                                   structure, iteration);
        } else {
            dilated = dilate<T>(input, width, height, depth,
                                window_width, window_height,
                                structure, iteration);
            eroded = erode<T>(dilated, width, height, depth,
                              window_width, window_height,
                              structure, iteration);
        }
    }

    void schedule() {
        if (this->auto_schedule) {
            input.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            structure.set_estimates({{0, 3}, {0, 3}});
            eroded.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(input, {width, height, depth});
            ::schedule(structure, {window_width, window_height});
            ::schedule(dilated, {width, height, depth});
            ::schedule(eroded, {width, height, depth});
        }
    }

private:
    Func dilated{"dilated"};
};

HALIDE_REGISTER_GENERATOR(Close<uint8_t>, close_u8);
HALIDE_REGISTER_GENERATOR(Close<uint16_t>, close_u16);
