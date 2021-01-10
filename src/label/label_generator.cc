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

    GeneratorOutput<Buffer<>> dst{"dst", {UInt(32), UInt(32)}, 2};

    void generate() {
        dst = label_firstpass<T>(src, width, height, this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}});
            for (auto &buffer : static_cast<Func>(dst).output_buffers()) {
                buffer.set_estimates({{0, 1024}, {0, 768}});
            }
        } else {
            ::schedule(src, {width, height});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Label<uint8_t>, label_u8);
HALIDE_REGISTER_GENERATOR(Label<uint16_t>, label_u16);
