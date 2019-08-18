#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class ThresholdBin : public Halide::Generator<ThresholdBin<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorInput<T> threshold{"threshold"};
    GeneratorInput<T> value{"value"};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = threshold_binary<T>(src, threshold, value);

        schedule(src, {width, height, depth});
        schedule(dst, {width, height, depth});
    }
};

HALIDE_REGISTER_GENERATOR(ThresholdBin<uint8_t>, threshold_bin_u8);
HALIDE_REGISTER_GENERATOR(ThresholdBin<uint16_t>, threshold_bin_u16);
