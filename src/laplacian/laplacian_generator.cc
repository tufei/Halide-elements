#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Laplacian : public Halide::Generator<Laplacian<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 8};
    GeneratorParam<int32_t> height{"height", 8};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = laplacian<T>(src, width, height, depth, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 8}, {0, 8}, {0, 3}});
            dst.set_estimates({{0, 8}, {0, 8}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Laplacian<uint8_t>, laplacian_u8);
HALIDE_REGISTER_GENERATOR(Laplacian<uint16_t>, laplacian_u16);
