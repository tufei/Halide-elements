#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class WarpPerspectiveBicubic : public Halide::Generator<WarpPerspectiveBicubic<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};
    GeneratorInput<T> border_value{"border_value", 1};
    GeneratorInput<Buffer<double>> transform{"transform", 1};

    GeneratorParam<int32_t> border_type{"border_type", 1}; //0 or 1
    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = warp_perspective_bicubic<T>(src, border_type, border_value,
                                          transform, width, height, depth);
        schedule(src, {width, height, depth});
        schedule(transform, {9});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(WarpPerspectiveBicubic<uint8_t>,
                          warp_perspective_bicubic_u8);
HALIDE_REGISTER_GENERATOR(WarpPerspectiveBicubic<uint16_t>,
                          warp_perspective_bicubic_u16);
