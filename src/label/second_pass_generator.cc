#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

class SecondPass : public Halide::Generator<SecondPass> {
public:
    GeneratorInput<Buffer<uint32_t>> src{"src", 2};
    GeneratorInput<Buffer<uint32_t>> buf{"buf", 2};
    GeneratorInput<int32_t> bufW{"bufW", 1};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};

    GeneratorOutput<Buffer<uint32_t>> dst{"dst", 2};

    void generate() {
        dst = label_secondpass(src, buf, width, height, bufW);

        schedule(src, {width, height});
        schedule(dst, {width, height});
    }
};

HALIDE_REGISTER_GENERATOR(SecondPass, second_pass);
