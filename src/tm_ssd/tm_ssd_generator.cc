#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class TmSsd : public Halide::Generator<TmSsd<T>> {
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
        dst = tm_ssd<T>(src0, src1, img_width, img_height, tmp_width, tmp_height);

        res_width = img_width.value() - tmp_width.value() + 1;
        res_height = img_height.value() - tmp_height.value() + 1;
    }

    void schedule() {
        if (this->auto_schedule) {
            src0.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            src1.set_estimates({{0, 16}, {0, 16}});
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src0, {img_width, img_height, img_depth});
            ::schedule(src1, {tmp_width, tmp_height});
            ::schedule(dst, {res_width, res_height, img_depth});
        }
    }

private:
    int32_t res_width;
    int32_t res_height;
};

HALIDE_REGISTER_GENERATOR(TmSsd<uint8_t>, tm_ssd_u8);
HALIDE_REGISTER_GENERATOR(TmSsd<uint16_t>, tm_ssd_u16);
HALIDE_REGISTER_GENERATOR(TmSsd<uint32_t>, tm_ssd_u32);
