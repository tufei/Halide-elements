#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class WarpMapBicubic : public Halide::Generator<WarpMapBicubic<T>> {
public:
    GeneratorInput<Buffer<T>> src0{"src0", 3};
    GeneratorInput<Buffer<float>> src1{"src1", 2};
    GeneratorInput<Buffer<float>> src2{"src2", 2};
    GeneratorInput<T> border_value{"border_value", 1};

    GeneratorParam<int32_t> border_type{"border_type", 0}; //0 or 1
    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = warp_map_bicubic<T>(src0, src1, src2,
                                  border_type, border_value,
                                  width, height, depth);
    }

    void schedule() {
        if (this->auto_schedule) {
            src0.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            src1.set_estimates({{0, 1024}, {0, 768}});
            src2.set_estimates({{0, 1024}, {0, 768}});
            border_value.set_estimate(0);
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src0, {width, height, depth});
            ::schedule(src1, {width, height});
            ::schedule(src2, {width, height});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(WarpMapBicubic<uint8_t>, warp_map_bicubic_u8);
HALIDE_REGISTER_GENERATOR(WarpMapBicubic<uint16_t>, warp_map_bicubic_u16);
