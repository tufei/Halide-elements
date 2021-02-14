#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class SetScalar : public Halide::Generator<SetScalar<T>> {
public:
    GeneratorInput<T> value{"value"};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        Expr src = value;
        dst = set_scalar(src);
    }

    void schedule() {
        if (this->auto_schedule) {
            value.set_estimate(1);
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(SetScalar<uint8_t>, set_scalar_u8);
HALIDE_REGISTER_GENERATOR(SetScalar<uint16_t>, set_scalar_u16);
