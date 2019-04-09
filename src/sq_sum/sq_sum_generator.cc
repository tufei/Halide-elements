#include <cstdint>
#include "Halide.h"

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T, typename D>
class Sq_sum : public Halide::Generator<Sq_sum<T, D>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<D>> dst{"dst", 3};

    void generate() {
        dst = sq_sum<T, D>(src, width, height, depth);

        schedule(src, {width, height, depth});
        schedule(dst, {1, 1, depth});
    }
};

using Sq_sum_u8_f32 = Sq_sum<uint8_t, float>;
HALIDE_REGISTER_GENERATOR(Sq_sum_u8_f32 , sq_sum_u8_f32);
using Sq_sum_u16_f32 = Sq_sum<uint16_t, float>;
HALIDE_REGISTER_GENERATOR(Sq_sum_u16_f32 , sq_sum_u16_f32);
using Sq_sum_u32_f32 = Sq_sum<uint32_t, float>;
HALIDE_REGISTER_GENERATOR(Sq_sum_u32_f32 , sq_sum_u32_f32);
using Sq_sum_u8_f64 = Sq_sum<uint8_t, double>;
HALIDE_REGISTER_GENERATOR(Sq_sum_u8_f64 , sq_sum_u8_f64);
using Sq_sum_u16_f64 = Sq_sum<uint16_t, double>;
HALIDE_REGISTER_GENERATOR(Sq_sum_u16_f64 , sq_sum_u16_f64);
using Sq_sum_u32_f64 = Sq_sum<uint32_t, double>;
HALIDE_REGISTER_GENERATOR(Sq_sum_u32_f64 , sq_sum_u32_f64);
