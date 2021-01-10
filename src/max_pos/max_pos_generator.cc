#include <cstdint>
#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class MaxPos : public Halide::Generator<MaxPos<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<uint32_t>> dst{"dst", 1};

    void generate() {
        dst = max_pos<T>(src, width, height, depth, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            dst.set_estimates({{0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {3});
        }
    }
};

HALIDE_REGISTER_GENERATOR(MaxPos<uint8_t>, max_pos_u8);
HALIDE_REGISTER_GENERATOR(MaxPos<uint16_t>, max_pos_u16);
HALIDE_REGISTER_GENERATOR(MaxPos<uint32_t>, max_pos_u32);
HALIDE_REGISTER_GENERATOR(MaxPos<float>, max_pos_f32);
HALIDE_REGISTER_GENERATOR(MaxPos<double>, max_pos_f64);
