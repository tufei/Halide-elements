#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T, typename D>
class Integral : public Halide::Generator<Integral<T, D>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<D>> dst{"dst", 3};

    void generate() {
        dst = integral<T, D>(src, width, height, depth, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {width, height, depth});
        }
    }
};

using Integral_u8_f32 = Integral<uint8_t, float>;
HALIDE_REGISTER_GENERATOR(Integral_u8_f32 , integral_u8_f32);
using Integral_u16_f32 = Integral<uint16_t, float>;
HALIDE_REGISTER_GENERATOR(Integral_u16_f32 , integral_u16_f32);
using Integral_u32_f32 = Integral<uint32_t, float>;
HALIDE_REGISTER_GENERATOR(Integral_u32_f32 , integral_u32_f32);
using Integral_u8_f64 = Integral<uint8_t, double>;
HALIDE_REGISTER_GENERATOR(Integral_u8_f64 , integral_u8_f64);
using Integral_u16_f64 = Integral<uint16_t, double>;
HALIDE_REGISTER_GENERATOR(Integral_u16_f64 , integral_u16_f64);
using Integral_u32_f64 = Integral<uint32_t, double>;
HALIDE_REGISTER_GENERATOR(Integral_u32_f64 , integral_u32_f64);
