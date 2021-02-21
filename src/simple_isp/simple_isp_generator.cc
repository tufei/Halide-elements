#include <Halide.h>
#include <Element.h>

using namespace Halide;
using namespace Halide::Element;

class SimpleISP : public Halide::Generator<SimpleISP> {
public:
    GeneratorInput<Buffer<uint16_t>> in{"in", 2};

    GeneratorInput<uint16_t> optical_black_value{"optical_black_value", 0};
    GeneratorInput<float> gamma_value{"gamma_value", 1.0f};
    GeneratorInput<float> saturation_value{"saturation_value", 1.0f};

    GeneratorParam<int32_t> width{"width", 3280};
    GeneratorParam<int32_t> height{"height", 2486};

    GeneratorOutput<Buffer<uint8_t>> out{"out", 3};

    void generate()
    {
        w = static_cast<int32_t>(width);
        h = static_cast<int32_t>(height);

        Var x, y;
        Func f0("optical_black_clamp");
        f0(x, y) = optical_black_clamp(in, optical_black_value, w, h)(x, y);

        Func f0_ = BoundaryConditions::mirror_interior(f0, {{0, w}, {0, h}});

        Func f1("color_interpolation_raw2rgb");
        f1(c, x, y) = color_interpolation_raw2rgb<frac_bits>(f0_)(c, x, y);

        f2(c, x, y) = gamma_correction<frac_bits>(f1, gamma_value)(c,x,y);

        Func f3("color_interpolation_rgb2hsv");
        f3(c, x, y) = color_interpolation_rgb2hsv<frac_bits>(f2)(c, x, y);

        f4(c, x, y) = saturation_adjustment<frac_bits>(f3, saturation_value)(c, x, y);

        Func f5("color_interpolation_hsv2rgb");
        f5(c, x, y) = color_interpolation_hsv2rgb<frac_bits>(f4)(c, x, y);

        out(c, x, y) = select(c == 3, 0, denormalize<frac_bits>(f5)(c, x, y));

    }

    void schedule() {
        if (auto_schedule) {
            in.set_estimates({{0, 3280}, {0, 2486}});
            optical_black_value.set_estimate(0);
            gamma_value.set_estimate(1.f);
            saturation_value.set_estimate(1.f);
            out.set_estimates({{0, 4}, {0, 3280}, {0, 2486}});
        } else {
            ::schedule(in, {w, h});
            ::schedule(f2, {3, w, h}).unroll(c);
            ::schedule(f4, {3, w, h}).unroll(c);
            //::schedule(out, {4, w, h}).unroll(c);
        }
    }

private:
    constexpr static uint32_t frac_bits = 10;

    int32_t w;
    int32_t h;

    Var c{"c"};
    Func f2{"gamma_correction"};
    Func f4{"saturation_adjustment"};
};

HALIDE_REGISTER_GENERATOR(SimpleISP, simple_isp)
