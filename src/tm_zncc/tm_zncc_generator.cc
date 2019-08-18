#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class TmZncc : public Halide::Generator<TmZncc<T>> {
public:
    GeneratorInput<Buffer<T>> src0{"src0", 3};
    GeneratorInput<Buffer<T>> src1{"src1", 2};

    GeneratorParam<int32_t> img_width{"img_width", 1024};
    GeneratorParam<int32_t> img_height{"img_height", 768};
    GeneratorParam<int32_t> img_depth{"img_depth", 3};
    GeneratorParam<int32_t> tmp_width{"tmp_width", 16};
    GeneratorParam<int32_t> tmp_height{"tmp_height", 16};

    GeneratorOutput<Buffer<double>> dst{"dst", 3};

    void generate() {
        dst = tm_zncc<T>(src0, src1, img_width, img_height, tmp_width, tmp_height);

        const int32_t res_width = img_width.value() - tmp_width.value() + 1;
        const int32_t res_height = img_height.value() - tmp_height.value() + 1;

        schedule(src0, {img_width, img_height, img_depth});
        schedule(src1, {tmp_width, tmp_height});
        schedule(dst, {res_width, res_height, img_depth});
    }
};

HALIDE_REGISTER_GENERATOR(TmZncc<uint8_t>, tm_zncc_u8);
HALIDE_REGISTER_GENERATOR(TmZncc<uint16_t>, tm_zncc_u16);
HALIDE_REGISTER_GENERATOR(TmZncc<uint32_t>, tm_zncc_u32);
