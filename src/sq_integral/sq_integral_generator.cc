#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T, typename D>
class SqIntegral : public Halide::Generator<SqIntegral<T, D>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<D>> dst{"dst", 3};

    void generate() {
        dst = sq_integral<T, D>(src, width, height, depth);

        schedule(src, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

using Sq_integral_u8_f32 = SqIntegral<uint8_t, float>;
HALIDE_REGISTER_GENERATOR(Sq_integral_u8_f32 , sq_integral_u8_f32);
using Sq_integral_u16_f32 = SqIntegral<uint16_t, float>;
HALIDE_REGISTER_GENERATOR(Sq_integral_u16_f32 , sq_integral_u16_f32);
using Sq_integral_u32_f32 = SqIntegral<uint32_t, float>;
HALIDE_REGISTER_GENERATOR(Sq_integral_u32_f32 , sq_integral_u32_f32);
using Sq_integral_u8_f64 = SqIntegral<uint8_t, double>;
HALIDE_REGISTER_GENERATOR(Sq_integral_u8_f64 , sq_integral_u8_f64);
using Sq_integral_u16_f64 = SqIntegral<uint16_t, double>;
HALIDE_REGISTER_GENERATOR(Sq_integral_u16_f64 , sq_integral_u16_f64);
using Sq_integral_u32_f64 = SqIntegral<uint32_t, double>;
HALIDE_REGISTER_GENERATOR(Sq_integral_u32_f64 , sq_integral_u32_f64);
