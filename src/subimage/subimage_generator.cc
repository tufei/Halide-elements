#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Subimage : public Halide::Generator<Subimage<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};
    GeneratorInput<uint32_t> origin_x{"origin_x", 1};
    GeneratorInput<uint32_t> origin_y{"origin_y", 1};

    GeneratorParam<int32_t> in_width{"in_width", 1024};
    GeneratorParam<int32_t> in_height{"in_height", 768};
    GeneratorParam<int32_t> in_depth{"in_depth", 3};

    GeneratorParam<int32_t> out_width{"out_width", 500};
    GeneratorParam<int32_t> out_height{"out_height", 500};
    GeneratorParam<int32_t> out_depth{"out_depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = subimage<T>(src, origin_x, origin_y);
        schedule(src, {in_width, in_height, in_depth});
        schedule(dst, {out_width, out_height, in_depth});
    }
};

HALIDE_REGISTER_GENERATOR(Subimage<uint8_t>, subimage_u8);
HALIDE_REGISTER_GENERATOR(Subimage<uint16_t>, subimage_u16);
HALIDE_REGISTER_GENERATOR(Subimage<uint32_t>, subimage_u32);
