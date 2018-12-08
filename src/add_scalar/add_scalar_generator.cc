#include <cstdint>
#include <limits>
#include "Halide.h"
#include "Element.h"

using namespace Halide;

template<typename T>
class AddScalar : public Halide::Generator<AddScalar<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorInput<double> value{"value"};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    Var x, y, c;

    void generate() {
      Expr dstval = clamp(round(cast<double>(src(x, y, c)) + value), 0,
                          cast<double>(type_of<T>().max()));

      dst(x, y, c) = cast<T>(dstval);

      dst.vectorize(x, 16).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(AddScalar<uint8_t>, add_scalar_u8);
HALIDE_REGISTER_GENERATOR(AddScalar<uint16_t>, add_scalar_u16);
HALIDE_REGISTER_GENERATOR(AddScalar<uint32_t>, add_scalar_u32);
