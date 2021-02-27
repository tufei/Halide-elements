#include <cstdint>
#include "Halide.h"

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T, typename D>
class Sum : public Halide::Generator<Sum<T, D>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<D>> dst{"dst", 3};

    void generate() {
        dst = sum<T, D>(src, width, height, depth);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            dst.set_estimates({{0, 1}, {0, 1}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {1, 1, depth});
        }
    }
};

using Sum_u8_f32 = Sum<uint8_t, float>;
HALIDE_REGISTER_GENERATOR(Sum_u8_f32, sum_u8_f32);
using Sum_u16_f32 = Sum<uint16_t, float>;
HALIDE_REGISTER_GENERATOR(Sum_u16_f32, sum_u16_f32);
using Sum_u32_f32 = Sum<uint32_t, float>;
HALIDE_REGISTER_GENERATOR(Sum_u32_f32, sum_u32_f32);
using Sum_u8_f64 = Sum<uint8_t, double>;
HALIDE_REGISTER_GENERATOR(Sum_u8_f64, sum_u8_f64);
using Sum_u16_f64 = Sum<uint16_t, double>;
HALIDE_REGISTER_GENERATOR(Sum_u16_f64, sum_u16_f64);
using Sum_u32_f64 = Sum<uint32_t, double>;
HALIDE_REGISTER_GENERATOR(Sum_u32_f64, sum_u32_f64);
using Sum_f32_f32 = Sum<float, float>;
HALIDE_REGISTER_GENERATOR(Sum_f32_f32, sum_f32_f32);
using Sum_f64_f64 = Sum<double, double>;
HALIDE_REGISTER_GENERATOR(Sum_f64_f64, sum_f64_f64);
