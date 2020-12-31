#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Bilateral : public Halide::Generator<Bilateral<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorInput<int32_t> window_size{"window_size", 1, 1, 7};
    GeneratorInput<double> sigma_color{"sigma_color", 1};
    GeneratorInput<double> sigma_space{"sigma_space", 1};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() {
        if (this->auto_schedule) {
            dst = bilateral_pure<T>(src, width, height, depth,
                                    window_size, sigma_color, sigma_space);
        } else {
            dst = bilateral<T>(src, width, height, depth,
                               window_size, sigma_color, sigma_space);
        }
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            window_size.set_estimate(5);
            sigma_color.set_estimate(2.0f);
            sigma_space.set_estimate(1.0f);
            dst.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {width, height, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Bilateral<uint8_t>, bilateral_u8);
HALIDE_REGISTER_GENERATOR(Bilateral<uint16_t>, bilateral_u16);
