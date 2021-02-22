#include <cstdint>
#include "Halide.h"

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

class SinCos : public Halide::Generator<SinCos> {
public:
    GeneratorInput<Buffer<float>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<float>> dst{"dst", 3};

    void generate() {
        Var x, y, c;
        FixedN<30, 28, true> e =
            select((x + y) % 2 == 0, sin(to_fixed<30, 25>(src(x, y, c))),
                   cos(to_fixed<30, 25>(src(x, y, c))));
        dst(x, y, c) = from_fixed<float>(e);

    }

    void schedule() {
        if (auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(SinCos, sin_cos);
