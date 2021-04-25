#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class WarpAffineNN : public Halide::Generator<WarpAffineNN<T>> {
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
        dst = warp_affine_NN<T>(src, border_type, border_value,
                                transform, width, height, depth);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            border_value.set_estimate(0);
            transform.set_estimates({{0, 6}});
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(transform, {6});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(WarpAffineNN<uint8_t>, warp_affine_NN_u8);
HALIDE_REGISTER_GENERATOR(WarpAffineNN<uint16_t>, warp_affine_NN_u16);
