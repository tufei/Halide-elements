#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Merge4 : public Halide::Generator<Merge4<T>> {
public:
    GeneratorInput<Buffer<T>> src0{"src0", 2};
    GeneratorInput<Buffer<T>> src1{"src1", 2};
    GeneratorInput<Buffer<T>> src2{"src2", 2};
    GeneratorInput<Buffer<T>> src3{"src3", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = merge4<T>(src0, src1, src2, src3,
                        width, height, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src0.set_estimates({{0, 1024}, {0, 768}});
            src1.set_estimates({{0, 1024}, {0, 768}});
            src2.set_estimates({{0, 1024}, {0, 768}});
            src3.set_estimates({{0, 1024}, {0, 768}});
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 4}});
        } else {
            ::schedule(src0, {width, height});
            ::schedule(src1, {width, height});
            ::schedule(src2, {width, height});
            ::schedule(src3, {width, height});
            ::schedule(dst, {width, height, 4});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Merge4<uint8_t>, merge4_u8);
HALIDE_REGISTER_GENERATOR(Merge4<uint16_t>, merge4_u16);
HALIDE_REGISTER_GENERATOR(Merge4<int8_t>, merge4_i8);
HALIDE_REGISTER_GENERATOR(Merge4<int16_t>, merge4_i16);
