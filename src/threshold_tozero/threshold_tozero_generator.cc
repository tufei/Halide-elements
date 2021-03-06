#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class ThresholdTozero : public Halide::Generator<ThresholdTozero<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorInput<T> threshold{"threshold"};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        dst = threshold_tozero<T>(src, threshold);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            threshold.set_estimate(128);
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(ThresholdTozero<uint8_t>, threshold_tozero_u8);
HALIDE_REGISTER_GENERATOR(ThresholdTozero<uint16_t>, threshold_tozero_u16);
