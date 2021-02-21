#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

class SGM : public Halide::Generator<SGM> {
public:
    GeneratorInput<Buffer<uint8_t>> in_l{"in_l", 2};
    GeneratorInput<Buffer<uint8_t>> in_r{"in_r", 2};

    GeneratorParam<int32_t> disp{"disp", 16};
    GeneratorParam<int32_t> width{"width", 641};
    GeneratorParam<int32_t> height{"height", 555};

    GeneratorOutput<Buffer<uint8_t>> dst{"dst", 2};

    void generate()
    {
        dst = semi_global_matching(in_l, in_r, width, height,
                                   disp, auto_schedule);
    }

    void schedule() {
        if (auto_schedule) {
            in_l.set_estimates({{0, 641}, {0, 555}});
            in_r.set_estimates({{0, 641}, {0, 555}});
            dst.set_estimates({{0, 641}, {0, 555}});
        } else {
            ::schedule(in_l, {width, height});
            ::schedule(in_r, {width, height});
            ::schedule(dst, {width, height});
        }
    }
};

HALIDE_REGISTER_GENERATOR(SGM, sgm)
