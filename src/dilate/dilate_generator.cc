#include <cstdint>
#include <Halide.h>

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Dilate : public Halide::Generator<Dilate<T>> {
public:
    GeneratorInput<Buffer<T>> input{"input", 3};
    GeneratorInput<Buffer<uint8_t>> structure{"structure", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> iteration{"iteration", 2};
    GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
    GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

    GeneratorOutput<Buffer<T>> output{"output", 3};

    void generate() {
        output = dilate<T>(input, width, height, depth,
                           window_width, window_height,
                           structure, iteration, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            input.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            structure.set_estimates({{0, 3}, {0, 3}});
            output.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(input, {width, height, depth});
            ::schedule(structure, {window_width, window_height});
            ::schedule(output, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Dilate<uint8_t>, dilate_u8);
HALIDE_REGISTER_GENERATOR(Dilate<uint16_t>, dilate_u16);
