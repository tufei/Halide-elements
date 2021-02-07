#include <cmath>
#include <cstdint>

#include <Halide.h>
#include "Element.h"

using namespace Halide;
using namespace Halide::Element;

class POC : public Generator<POC> {
public:
    GeneratorInput<Buffer<float>> input1{"intpu1", 2};
    GeneratorInput<Buffer<float>> input2{"intpu2", 2};

    GeneratorParam<int32_t> n_{"n", 16};
    GeneratorParam<int32_t> batch_size_{"batch_size", 16};

    Var c{"c"}, i{"i"}, k{"k"}, h{"h"};
    Var x{"x"}, y{"y"};

    GeneratorOutput<Buffer<float>> func_poc{"func_poc", 2};

    ComplexExpr fft2(ComplexExpr a, bool auto_schedule) {
        const int32_t n = static_cast<int32_t>(n_);
        const int32_t batch_size = static_cast<int32_t>(batch_size_);

        Func cmpl_in("cmp_in");
        cmpl_in(c, x, y) = select(c == 0, a.x, a.y);

        Func fftx("fftx");
        fftx = fft(cmpl_in, n, batch_size, auto_schedule);

        Func tr("tr");
        tr(c, x, y) = fftx(c, y, x);

        Func ffty("ffty");
        ffty = fft(tr, n, batch_size, auto_schedule);

        ComplexExpr ret;
        ret = {ffty(0, x, y), ffty(1, x, y)};

        return ret;
    }

    ComplexExpr ifft2(ComplexExpr a, bool auto_schedule) {
        const int32_t n = static_cast<int32_t>(n_);

        ComplexExpr fft_a_conj, ifft_a;
        fft_a_conj = fft2(conj(a), auto_schedule);
        ifft_a     = conj(fft_a_conj) / (n * n);

        return ifft_a;
    }

    void generate() {
        n = static_cast<int32_t>(n_);

        Expr w = sqrt((x / (float)n - 0.5f) * (x / (float)n - 0.5f) +
                      (y / (float)n - 0.5f) * (y / (float)n - 0.5f));
        Expr hann_w_tmp = 0.5f - 0.5f * cos(2.0f * (float)M_PI * ((w + 0.5f)));
        Expr hann_w = select(w <= 0.5f, hann_w_tmp, 0.0f);

        ComplexExpr cmpl_hann_w1 = {hann_w * input1(x, y), 0};
        ComplexExpr cmpl_hann_w2 = {hann_w * input2(x, y), 0};

        ComplexExpr cmpl_fft1 = fft2(cmpl_hann_w1, auto_schedule);
        ComplexExpr cmpl_fft2 = fft2(cmpl_hann_w2, auto_schedule);

        ComplexExpr r = cmpl_fft2 * conj(cmpl_fft1);
        ComplexExpr r_normal = norm(r);

        ComplexExpr ifft_r_n = ifft2(r_normal, auto_schedule);

        func_poc(x, y) = ifft_r_n.x;
    }

    void schedule() {
        if (auto_schedule) {
            input1.set_estimates({{0, n}, {0, n}});
            input2.set_estimates({{0, n}, {0, n}});
            func_poc.set_estimates({{0, n}, {0, n}});
        } else {
            ::schedule(input1, {n, n});
            ::schedule(input2, {n, n});
            ::schedule(func_poc, {n, n});
        }
    }

private:
    int32_t n;
};

HALIDE_REGISTER_GENERATOR(POC, poc);
