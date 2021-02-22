#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Sobel : public Generator<Sobel<T>> {
public:
    GeneratorInput<Buffer<T>> input{"input", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> output{"output", 3};

    void generate() {
        output = sobel<T>(input, width, height, depth, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            input.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            output.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(input, {width, height, depth});
            ::schedule(output, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Sobel<uint8_t>, sobel_u8);
HALIDE_REGISTER_GENERATOR(Sobel<uint16_t>, sobel_u16);

