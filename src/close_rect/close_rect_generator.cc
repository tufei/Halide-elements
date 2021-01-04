#include <cstdint>
#include <Halide.h>

#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

template<typename T>
class CloseRect : public Halide::Generator<CloseRect<T>> {
public:
  GeneratorInput<Buffer<T>> input{"input", 3};

  GeneratorParam<int32_t> width{"width", 1024};
  GeneratorParam<int32_t> height{"height", 768};
  GeneratorParam<int32_t> depth{"depth", 3};
  GeneratorParam<int32_t> iteration{"iteration", 2};
  GeneratorParam<int32_t> window_width{"window_width", 3, 3, 17};
  GeneratorParam<int32_t> window_height{"window_height", 3, 3, 17};

  GeneratorOutput<Buffer<T>> erode_rected{"erode_rected", 3};

  void generate() {
    // Run dilate
    dilate_rected =
        dilate_rect<T>(input, width, height, depth,
                       window_width, window_height,
                       iteration, this->auto_schedule);

    // Run erode
    erode_rected =
        erode_rect<T>(dilate_rected, width, height, depth,
                      window_width, window_height,
                      iteration, this->auto_schedule);
  }

  void schedule() {
      if (this->auto_schedule) {
          input.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
          erode_rected.set_estimates({{0, 1024}, {0, 768}, {0, 3}});
      } else {
        ::schedule(input, {width, height, depth});
        ::schedule(dilate_rected, {width, height, depth});
        ::schedule(erode_rected, {width, height, depth});
      }
  }

private:
    Func dilate_rected{"dilate_rected"};
};

HALIDE_REGISTER_GENERATOR(CloseRect<uint8_t>, close_rect_u8);
HALIDE_REGISTER_GENERATOR(CloseRect<uint16_t>, close_rect_u16);
