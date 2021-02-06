#include <cstdint>

#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Multiply : public Halide::Generator<Multiply<T>> {
public:
    GeneratorInput<Buffer<T>> src0{"src0", 3};
    GeneratorInput<Buffer<T>> src1{"src1", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = multiply<T>(src0, src1);
    }

    void schedule() {
        if (this->auto_schedule) {
            src0.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            src1.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src0, {width, height, depth});
            ::schedule(src1, {width, height, depth});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Multiply<uint8_t>, multiply_u8);
HALIDE_REGISTER_GENERATOR(Multiply<uint16_t>, multiply_u16);
HALIDE_REGISTER_GENERATOR(Multiply<uint32_t>, multiply_u32);
