#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class WarpMapBilinear : public Halide::Generator<WarpMapBilinear<T>> {
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
        dst = warp_map_bilinear<T>(src0, src1, src2, border_type, border_value, width, height, depth);
        schedule(src0, {width, height, depth});
        schedule(src1, {width, height});
        schedule(src2, {width, height});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(WarpMapBilinear<uint8_t>, warp_map_bilinear_u8);
HALIDE_REGISTER_GENERATOR(WarpMapBilinear<uint16_t>, warp_map_bilinear_u16);
