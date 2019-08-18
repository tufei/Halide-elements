#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class ThresholdTozeroInv : public Halide::Generator<ThresholdTozeroInv<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorInput<T> threshold{"threshold"};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = threshold_tozero_inv<T>(src, threshold);

        schedule(src, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(ThresholdTozeroInv<uint8_t>, threshold_tozero_inv_u8);
HALIDE_REGISTER_GENERATOR(ThresholdTozeroInv<uint16_t>, threshold_tozero_inv_u16);
