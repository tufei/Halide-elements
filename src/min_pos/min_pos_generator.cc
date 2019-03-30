#include <cstdint>

#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class MinPos : public Halide::Generator<MinPos<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<uint32_t>> dst{"dst", 1};

    void generate() {
        dst = min_pos<T>(src, width, height, depth);

        schedule(src, {width, height, depth});
        schedule(dst, {3});
    }
};

HALIDE_REGISTER_GENERATOR(MinPos<uint8_t>, min_pos_u8);
HALIDE_REGISTER_GENERATOR(MinPos<uint16_t>, min_pos_u16);
HALIDE_REGISTER_GENERATOR(MinPos<uint32_t>, min_pos_u32);
HALIDE_REGISTER_GENERATOR(MinPos<float>, min_pos_f32);
HALIDE_REGISTER_GENERATOR(MinPos<double>, min_pos_f64);
