#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;

template<typename T>
class Add : public Halide::Generator<Add<T>> {
public:
    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorInput<Buffer<T>> src0{"src0", 3};
    GeneratorInput<Buffer<T>> src1{"src1", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    Var x, y, c;

    void generate() {
      using upper_t = typename Halide::Element::Upper<T>::type;

      Expr srcval0 = cast<upper_t>(src0(x, y, c));
      Expr srcval1 = cast<upper_t>(src1(x, y, c));
      Expr dstval = min(srcval0 + srcval1, cast<upper_t>(type_of<T>().max()));

      dst(x, y, c) = cast<T>(dstval);

      dst.vectorize(x, 16).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(Add<uint8_t>, add_u8);
HALIDE_REGISTER_GENERATOR(Add<uint16_t>, add_u16);
HALIDE_REGISTER_GENERATOR(Add<uint32_t>, add_u32);
