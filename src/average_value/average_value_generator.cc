#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename S, typename D>
class Average_value : public Halide::Generator<Average_value<S, D>> {
public:
    GeneratorInput<Buffer<S>> src{"src", 3};
    GeneratorInput<Buffer<uint8_t>> roi{"roi", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<D>> dst{"dst", 2};

    void generate() {
        dst = average_value<S, D>(src, roi, width, height, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            roi.set_estimates({{0, 1024}, {0, 768}});
            dst.set_estimates({{0, 1}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(roi, {width, height});
            ::schedule(dst, {1, depth});
        }
    }
};

using Average_value_u8_f32 = Average_value<uint8_t, float>;
HALIDE_REGISTER_GENERATOR(Average_value_u8_f32, average_value_u8_f32);
using Average_value_u16_f32 = Average_value<uint16_t, float>;
HALIDE_REGISTER_GENERATOR(Average_value_u16_f32, average_value_u16_f32);

using Average_value_u8_f64 = Average_value<uint8_t, double>;
HALIDE_REGISTER_GENERATOR(Average_value_u8_f64, average_value_u8_f64);
using Average_value_u16_f64 = Average_value<uint16_t, double>;
HALIDE_REGISTER_GENERATOR(Average_value_u16_f64, average_value_u16_f64);
