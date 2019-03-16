#include <climits>
#include <cmath>
#include <cstdint>

#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Histogram2D : public Halide::Generator<Histogram2D<T>> {
public:
    GeneratorInput<Buffer<T>> src0{"src0", 3};
    GeneratorInput<Buffer<T>> src1{"src1", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> hist_width{"hist_width", 256, 1, static_cast<int32_t>(sqrt(std::numeric_limits<int32_t>::max()))};

    GeneratorOutput<Buffer<uint32_t>> dst{"dst", 3};

    void generate() {
        dst = histogram2d<T>(src0, src1, width, height, hist_width);

        schedule(src0, {width, height, depth});
        schedule(src1, {width, height, depth});
        schedule(dst, {hist_width, hist_width, depth});
    }
};

HALIDE_REGISTER_GENERATOR(Histogram2D<uint8_t>, histogram2d_u8);
HALIDE_REGISTER_GENERATOR(Histogram2D<uint16_t>, histogram2d_u16);
