#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

class FFT : public Halide::Generator<FFT> {
public:
    GeneratorInput<Buffer<float>> src{"src", 3};

    GeneratorParam<int32_t> n_{"n", 256};
    GeneratorParam<int32_t> batch_size_{"batch_size", 4};

    GeneratorOutput<Buffer<float>> dst{"dst", 3};

    void generate() {
        Var c{"cc"}, i{"ii"}, k{"kk"};

        const int32_t n = static_cast<uint32_t>(n_);
        const int32_t batch_size = static_cast<uint32_t>(batch_size_);

        dst(c, i, k) = fft(src, n, batch_size)(c, i, k);

        schedule(src, {2, n, batch_size});
        //schedule(dst, {2, n, batch_size}).unroll(c);
    }
};

HALIDE_REGISTER_GENERATOR(FFT, fft);
