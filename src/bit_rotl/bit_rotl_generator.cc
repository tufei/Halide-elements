#include <cstdint>
#include <limits>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T, typename I>
class BitRotl : public Halide::Generator<BitRotl<T, I>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorInput<size_t> rotate_left_bits{"rotate_left_bits"};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
      Expr shift = cast<uint32_t>(rotate_left_bits) % (int)(sizeof(I) * 8);
      Expr as_int = reinterpret(type_of<I>(), src(x, y, c));
      Expr bits = cast<uint32_t>((int)(sizeof(I) * 8));
      Expr rotated = (as_int << shift) | (as_int >> (bits - shift));
      rotated = select(shift == 0, as_int, rotated);
      dst(x, y, c) = reinterpret(type_of<T>(), rotated);
    }

    void schedule() {
        if (this->using_autoscheduler()) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            rotate_left_bits.set_estimate(1);
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            Var x_outer, x_inner;
            dst.split(x, x_outer, x_inner,
                      Halide::Internal::GeneratorBase::natural_vector_size(dst.type()))
               .vectorize(x_inner)
               .parallel(y);
        }
    }

private:
    Var x{"x"}, y{"y"}, c{"c"};
};

using BitRotl_f32_u32 = BitRotl<float, uint32_t>;
HALIDE_REGISTER_GENERATOR(BitRotl_f32_u32, bit_rotl_f32_u32);
using BitRotl_f64_u64 = BitRotl<double, uint64_t>;
HALIDE_REGISTER_GENERATOR(BitRotl_f64_u64, bit_rotl_f64_u64);
