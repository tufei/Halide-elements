#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Label : public Halide::Generator<Label<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 2};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};

#if 1
    GeneratorOutput<Buffer<uint32_t>> dst0{"dst0", 2};
    GeneratorOutput<Buffer<uint32_t>> dst1{"dst1", 2};
#else
    GeneratorOutput<Buffer<>> dst{"dst", {UInt(32), UInt(32)}, 2};
#endif

    void generate() {
#if 1
        Var x{"x"}, y{"y"};

        dst0(x, y) =
            label_firstpass<T>(src, width, height,
                               this->auto_schedule)(x, y)[0];
        dst1(x, y) =
            label_firstpass<T>(src, width, height,
                               this->auto_schedule)(x, y)[1];
#else
        dst = label_firstpass<T>(src, width, height, this->auto_schedule);
#endif
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}});
            dst0.set_estimates({{0, 1024}, {0, 768}});
            dst1.set_estimates({{0, 1024}, {0, 768}});
        } else {
            ::schedule(src, {width, height});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Label<uint8_t>, label_u8);
HALIDE_REGISTER_GENERATOR(Label<uint16_t>, label_u16);
