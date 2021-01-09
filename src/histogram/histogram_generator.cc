#include <climits>
#include <cstdint>
#include "Halide.h"
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class Histogram : public Halide::Generator<Histogram<T>> {
public:
    GeneratorInput<Buffer<T>> src{"src", 3};

    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};
    GeneratorParam<int32_t> hist_width{"hist_width",
                                       std::numeric_limits<T>::max() + 1};

    GeneratorOutput<Buffer<uint32_t>> dst{"dst", 2};

    void generate() {
        dst = histogram<T>(src, width, height, depth, hist_width,
                           this->auto_schedule);
    }

    void schedule() {
        if (this->auto_schedule) {
            src.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
            dst.set_estimates({{0, std::numeric_limits<T>::max()}, {0, 3}});
        } else {
            ::schedule(src, {width, height, depth});
            ::schedule(dst, {hist_width, depth});
        }
    }
};

HALIDE_REGISTER_GENERATOR(Histogram<uint8_t>, histogram_u8);
HALIDE_REGISTER_GENERATOR(Histogram<uint16_t>, histogram_u16);
