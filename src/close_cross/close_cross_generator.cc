#include <cstdint>
#include <Halide.h>

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class CloseCross : public Halide::Generator<CloseCross<T>> {
public:
  GeneratorInput<Buffer<T>> input{"input", 3};

  GeneratorParam<int32_t> width{"width", 1024};
  GeneratorParam<int32_t> height{"height", 768};
  GeneratorParam<int32_t> depth{"depth", 3};
  GeneratorParam<int32_t> iteration{"iteration", 2};
  GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
  GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

  GeneratorOutput<Buffer<T>> erode_crossed{"erode_crossed", 3};

  void generate() {
    Func dilate_crossed{"dilate_crossed"};

    // Run dilate
    dilate_crossed = dilate_cross<T>(input, width, height, depth, window_width, window_height, iteration);

    // Run erode
    erode_crossed = erode_cross<T>(dilate_crossed, width, height, depth, window_width, window_height, iteration);

    schedule(input, {width, height, depth});
    schedule(dilate_crossed, {width, height, depth});
    schedule(erode_crossed, {width, height, depth});
  }
};

HALIDE_REGISTER_GENERATOR(CloseCross<uint8_t>, close_cross_u8);
HALIDE_REGISTER_GENERATOR(CloseCross<uint16_t>, close_cross_u16);
