#include <cstdint>

#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Open : public Halide::Generator<Open<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};
    GeneratorInput<Buffer<uint8_t>> structure{"structure", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> iteration{"iteration", 2};
    GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
    GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

    GeneratorOutput<Buffer<T>> dilated{"dilated", 3};

    void generate() {
        eroded = erode<T>(src, width, height, depth,
                          window_width, window_height,
                          structure, iteration, this->auto_schedule);
        dilated = dilate<T>(eroded, width, height, depth,
                            window_width, window_height,
                            structure, iteration, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            structure.set_estimates({{3, 15}, {3, 15}});
            dilated.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(structure, {window_width, window_height});
            ::schedule(eroded, {width, height, depth});
            ::schedule(dilated, {width, height, depth});
        }
    }

private:
    Func eroded{"eroded"};
};

HALIDE_REGISTER_GENERATOR(Open<uint8_t>, open_u8);
HALIDE_REGISTER_GENERATOR(Open<uint16_t>, open_u16);
