#ifndef HALIDE_ELEMENT_ALGORITHM_H
#define HALIDE_ELEMENT_ALGORITHM_H

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#include <Halide.h>
#include "Schedule.h"
#include "Complex.h"

namespace Halide {
namespace Element {
namespace {

Expr bit_reverse(Expr i, const int n)
{
    // Reverse the low log2(n) bits of i using arithmetic ops only.
    // NOTE: a shift-based formulation (<<, >>, &, |) defeats Halide's
    // bounds analysis in this Halide version: shifted indices stay
    // unbounded through clamp-based wrappers such as
    // BoundaryConditions::repeat_edge, so lowering fails with
    // "accessed over an unbounded domain" even under
    // unsafe_promise_clamped. %, /, *, + keep bounds precise.
    int bits = 0;
    for (int t = n; t > 1; t >>= 1) {
        ++bits;
    }
    Expr ri = 0;
    for (int j = 0; j < bits; ++j) {
        const int pw_j = 1 << j;
        const int pw_b = 1 << (bits - 1 - j);
        ri = ri + ((i / pw_j) % 2) * pw_b;
    }
    return ri;
}

Func fft(Func in, const int32_t n, const int32_t batch_size,
         const bool auto_schedule = false)
{
    Var c{"c"}, i{"i"}, k{"k"};

    Func weight("weight");
    Expr theta = static_cast<float>(-2.0 * M_PI) * cast<float>(i) /
                 static_cast<float>(n);
    weight(c, i) = select(c == 0, cos(theta), sin(theta));

    Func stage("source");
    stage(c, i, k) = in(c, i, k);

    for (int j = 0; j < log2(n); ++j) {
        stage = BoundaryConditions::repeat_edge(stage,
                                                {{0, 2},
                                                 {0, n},
                                                 {0, batch_size}});

        Func next_stage("stage" + std::to_string(j));

        const int m = (n >> (j + 1));

        Expr cond = (i % (n >> j)) < m;

        Expr o = select(cond, i + m, i - m);

        ComplexExpr vi = {stage(0, i, k), stage(1, i, k)};
        ComplexExpr vo = {stage(0, o, k), stage(1, o, k)};

        // Case 1
        ComplexExpr v1 = vi + vo;

        // Case 2
        Expr wi = (i % m) * (1<<j);
        ComplexExpr w = {weight(0, wi), weight(1, wi)};
        ComplexExpr v2 = (vo - vi) * w;
        next_stage(c, i, k) = select(cond,
                                     select(c == 0, v1.x, v1.y),
                                     select(c == 0, v2.x, v2.y));

        if (!auto_schedule)
            schedule(next_stage, {2, n, batch_size}).unroll(c);

        stage = next_stage;
    }

    // Make bit-reversal 32-bit integer index
    Expr ri = unsafe_promise_clamped(bit_reverse(i, n), 0, n - 1);
    stage = BoundaryConditions::repeat_edge(stage,
                                            {{0, 2},
                                             {0, n},
                                             {0, batch_size}});

    Func out("out");
    out(c, i, k) = stage(c, ri, k);

    if (!auto_schedule) schedule(weight, {2, n/2});

    return out;
}

Func fft(GeneratorInput<Buffer<float>> &in, const int32_t n,
         const int32_t batch_size, const bool auto_schedule = false)
{
    Func src = in;
    return fft(src, n, batch_size, auto_schedule);
}

template<typename T>
Func copy(GeneratorInput<Buffer<T>> &src)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst{"dst"};
    dst(x, y, c) = src(x, y, c);

    return dst;
}

} // anonymous
} // Element
} // Halide

#endif
