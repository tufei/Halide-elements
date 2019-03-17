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

    GeneratorOutput<Buffer<>> dst{"dst", {Int(32), Int(32)}, 2};

    void generate() {
        dst = label_firstpass<T>(src, width, height);

        schedule(src, {width, height});
        //schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(Label<uint8_t>, label_u8);
HALIDE_REGISTER_GENERATOR(Label<uint16_t>, label_u16);
