#include <cstdint>

#include "Halide.h"
#include "Element.h"
#include "ImageProcessing.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Prewitt : public Generator<Prewitt<T>> {
public:
    GeneratorInput<Buffer<T>> input{"input", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> output{"output", 3};

    void generate() {
        output = prewitt<T>(input, width, height, depth);

        schedule(input, {width, height, depth});
        schedule(output, {width, height, depth});
    }

};

HALIDE_REGISTER_GENERATOR(Prewitt<uint8_t>, prewitt_u8);
HALIDE_REGISTER_GENERATOR(Prewitt<uint16_t>, prewitt_u16);
