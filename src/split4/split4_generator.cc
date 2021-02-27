#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Split4 : public Halide::Generator<Split4<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};

    GeneratorOutput<Buffer<>> dst{"dst", {type_of<T>(), type_of<T>(),
                                          type_of<T>(), type_of<T>()}, 2};

    void generate() {
        dst = split4<T>(src, width, height);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 4}});
            for (auto &buffer : static_cast<Func>(dst).output_buffers()) {
                buffer.set_estimates({{0, 1024}, {0, 768}});
            }
        } else {
            ::schedule(src, {width, height, 4});
            //::schedule(dst, {width, height});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Split4<uint8_t>, split4_u8);
HALIDE_REGISTER_GENERATOR(Split4<uint16_t>, split4_u16);
HALIDE_REGISTER_GENERATOR(Split4<int8_t>, split4_i8);
HALIDE_REGISTER_GENERATOR(Split4<int16_t>, split4_i16);
