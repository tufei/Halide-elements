#ifndef HALIDE_ELEMENT_ARITHMETIC_H
#define HALIDE_ELEMENT_ARITHMETIC_H

#include "Halide.h"
#include "Util.h"
#include<cstdio>

namespace Halide {
namespace Element {

namespace {

template<typename T, typename D>
Func sq_sum(GeneratorInput<Buffer<T>> &src, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst("sq_sum");

    RDom r(0, width, 0, height);

    dst(x, y, c) = cast<D>(sum(cast<double>(src(r.x, r.y, c)) * cast<double>(src(r.x, r.y, c))));

    return dst;
}

template<typename T, typename D>
Func sum(GeneratorInput<Buffer<T>> &src, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst("sum");

    RDom r(0, width, 0, height);

    dst(x, y, c) = cast<D>(sum(cast<typename SumType<T>::type>(src(r.x, r.y, c))));

    return dst;
}

template<typename T>
Func add(Func src0, Func src1)
{
    using upper_t = typename Upper<T>::type;

    Var x{"x"}, y{"y"};

    Expr srcval0 = cast<upper_t>(src0(x, y)), srcval1 = cast<upper_t>(src1(x, y));
    Expr dstval = min(srcval0 + srcval1, cast<upper_t>(type_of<T>().max()));

    Func dst;
    dst(x, y) = cast<T>(dstval);

    return dst;
}

template<typename T>
Func add_scalar(Func src, Expr val)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Expr dstval = clamp(round(cast<double>(src(x, y, c)) + val), 0, cast<double>(type_of<T>().max()));

    Func dst;
    dst(x, y, c) = cast<T>(dstval);

    return dst;
}

Func calc_and(Func src0, Func src1)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst;
    dst(x, y, c) = src0(x, y, c) & src1(x, y, c);

    return dst;
}

Func and_scalar(Func src0, Expr val)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst;
    dst(x, y, c) = src0(x, y, c) & val;

    return dst;
}

template<typename T>
Func average(GeneratorInput<Buffer<T>> &src, int32_t window_width, int32_t window_height)
{
    using upper_t = typename Upper<T>::type;

    Var x{"x"}, y{"y"}, c{"c"};

    Func clamped = BoundaryConditions::repeat_edge(src);
#if 1
    Expr w_half = window_width / 2;
    Expr h_half = window_height / 2;
#else
    Expr w_half = div_round_to_zero(window_width, 2);
    Expr h_half = div_round_to_zero(window_height, 2);
#endif

    RDom r(-w_half, window_width, -h_half, window_height);

    Func dst;
    dst(x, y, c) = cast<T>(round(cast<float>(sum(cast<upper_t>(clamped(x + r.x, y + r.y, c)))) / cast<float>(window_width * window_height)) + 0.5f);

    return dst;
}

template<typename T>
Func multiply(GeneratorInput<Buffer<T>> &src1, GeneratorInput<Buffer<T>> &src2)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst;
    dst(x, y, c) = src1(x, y, c) * src2(x, y, c);

    return dst;
}

template<typename T>
Func mul_scalar(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<float>> &val)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Expr dstval = min(src0(x, y, c) * val(c), cast<float>(type_of<T>().max()));
    dstval = max(dstval, 0);

    Func dst;
    dst(x, y, c) = cast<T>(round(dstval));

    return dst;
}

template<typename T>
Func div_scalar(GeneratorInput<Buffer<T>> &src, Expr val)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Expr srcval = src(x, y, c);
    Expr dstval = max(min(srcval / val, cast<double>(type_of<T>().max())), 0);

    Func dst;
    dst(x, y, c) = cast<T>(round(dstval));

    return dst;
}

template<typename T>
Func nand(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst;
    dst(x, y, c) = ~(src0(x, y, c) & src1(x, y, c));

    return dst;
}

template<typename T>
Func nor(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1) {
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst;
    dst(x, y, c) = ~(src0(x, y, c) | src1(x, y, c));

    return dst;
}

template<typename T>
Func min(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst;
    dst(x, y, c) = min(src0(x, y, c), src1(x, y, c));

    return dst;
}

template<typename T>
Func min_pos(GeneratorInput<Buffer<T>> &src, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"};
    RDom r{0, width, 0, height, 0, depth, "r"};

    Func res{"res"};
    res(x) = argmin(r, src(r.x, r.y, r.z));
    schedule(res, {1});

    Var d{"d"};
    Func dst{"dst"};
    dst(d) = cast<uint32_t>(0);
    dst(0) = cast<uint32_t>(res(0)[0]);
    dst(1) = cast<uint32_t>(res(0)[1]);
    dst(2) = cast<uint32_t>(res(0)[2]);

    return dst;
}

template<typename T>
Func min_value(GeneratorInput<Buffer<T>> &src, GeneratorInput<Buffer<uint8_t>> &roi, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"};
    Func count{"count"}, dst;
    RDom r{0, width, 0, height, 0, depth, "r"};
    r.where(roi(r.x, r.y, r.z) != 0);

    count(x) = sum(select(roi(r.x, r.y, r.z) == 0, 0, 1));
    schedule(count, {1});

    dst(x) = cast<T>(select(count(x) == 0, 0, minimum(src(r.x, r.y, r.z))));

    return dst;
}

template<typename T>
Func max(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst;
    dst(x, y, c) = max(src0(x, y, c), src1(x, y, c));

    return dst;
}

template<typename T>
Func max_pos(GeneratorInput<Buffer<T>> &src, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"};
    RDom r{0, width, 0, height, 0, depth, "r"};

    Func res{"res"};
    res(x) = argmax(r, src(r.x, r.y, r.z));
    schedule(res, {1});

    Var d{"d"};
    Func dst;
    dst(d) = cast<uint32_t>(0);
    dst(0) = cast<uint32_t>(res(0)[0]);
    dst(1) = cast<uint32_t>(res(0)[1]);
    dst(2) = cast<uint32_t>(res(0)[2]);

    return dst;
}

template<typename T>
Func max_value(GeneratorInput<Buffer<T>> &src, GeneratorInput<Buffer<uint8_t>> &roi, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"};
    Func count{"count"}, dst;
    RDom r{0, width, 0, height, 0, depth, "r"};
    r.where(roi(r.x, r.y, r.z) != 0);

    count(x) = sum(select(roi(r.x, r.y, r.z) == 0, 0, 1));
    schedule(count, {1});

    dst(x) = cast<T>(select(count(x) == 0, 0, maximum(src(r.x, r.y, r.z))));

    return dst;
}

template<typename T>
Func equal(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst{"dst"};
    dst(x, y, c) = cast<T>(select(src0(x, y, c) == src1(x, y, c), type_of<T>().max(), 0));

    return dst;
}

template<typename T>
Func cmpgt(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst{"dst"};
    dst(x, y, c) = cast<T>(select(src0(x, y, c) > src1(x, y, c),
                                  type_of<T>().max(), 0));

    return dst;
}

template<typename T>
Func cmpge(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1)
{
    Var x{"x"}, y{"y"}, c{"c"};

    Func dst{"dst"};
    dst(x, y, c) = cast<T>(select(src0(x, y, c) >= src1(x, y, c),
                                  type_of<T>().max(), 0));;

    return dst;
}

template<typename T, typename D>
Func integral(GeneratorInput<Buffer<T>> &in, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"}, y{"y"}, c{"c"};
    Func dst{"dst"}, integral{"integral"};
    integral(x, y, c) = cast<uint64_t>(in(x, y, c));

    RDom r1{1, width - 1, 0, height, "r1"};
    integral(r1.x, r1.y, c) += integral(r1.x - 1, r1.y, c);

    RDom r2{0, width, 1, height - 1, "r2"};
    integral(r2.x, r2.y, c) += integral(r2.x, r2.y - 1, c);
    schedule(integral, {width, height, depth});

    dst(x, y, c) = cast<D>(integral(x, y, c));

    return dst;
}


template<typename T>
Func histogram(GeneratorInput<Buffer<T>> &src, int32_t width, int32_t height, int32_t depth, int32_t hist_width)
{
    uint32_t hist_size = static_cast<uint32_t>(std::numeric_limits<T>::max()) + 1;
    int32_t bin_size = (hist_size + hist_width - 1) / hist_width;

    Var x{"x"}, c{"c"};
    RDom r{0, width, 0, height};

    Func dst;
    dst(x, c) = cast<uint32_t>(0);

    Expr idx = cast<int32_t>(src(r.x, r.y, c) / bin_size);
    dst(idx, c) += cast<uint32_t>(1);

    return dst;
}

template<typename T>
Func histogram2d(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1, int32_t width, int32_t height, int32_t hist_width)
{
    Var x{"x"}, y{"y"}, c{"c"};
    RDom r{0, width, 0, height};

    Func dst;
    dst(x, y, c) = cast<uint32_t>(0);
    Expr idx0 = cast<int32_t>(src0(r.x, r.y, c) * cast<uint64_t>(hist_width) / (cast<uint64_t>(type_of<T>().max()) + 1));
    Expr idx1 = cast<int32_t>(src1(r.x, r.y, c) * cast<uint64_t>(hist_width) / (cast<uint64_t>(type_of<T>().max()) + 1));
    dst(idx0, idx1, c) += cast<uint32_t>(1);

    return dst;
}

template<typename T>
Func sub(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1)
{
    Var x{"x"}, y{"y"}, c{"c"};
    Func dst;
    dst(x, y, c) = cast<T>(select(src0(x, y, c) > src1(x, y, c), src0(x, y, c) - src1(x, y, c), 0));

    return dst;
}

template<typename T>
Func filter_xor(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1) {
    Var x{"x"}, y{"y"}, c{"c"};
    Func dst;
    dst(x, y, c) = src0(x, y, c) ^ src1(x, y, c);
    return dst;
}

template<typename T, typename D>
Func sq_integral(GeneratorInput<Buffer<T>> &src, int32_t width, int32_t height, int32_t depth)
{
    Var x{"x"}, y{"y"}, c{"c"};
    Func dst{"dst"};

    dst(x, y, c) = cast<D>(src(x, y, c)) * cast<D>(src(x, y, c));

    RDom h{1, width-1, 0, height, "h"};
    dst(h.x, h.y, c) += dst(h.x-1, h.y, c);

    RDom v{0, width, 1, height-1, "v"};
    dst(v.x, v.y, c) += dst(v.x, v.y-1, c);

    return dst;
}


template<typename T>
Func sub_scalar(GeneratorInput<Buffer<T>> &src, Expr val)
{
    Var x{"x"}, y{"y"}, c{"c"};
    Func dst;

    dst(x, y, c) = cast<T>(clamp(round(cast<double>(src(x, y, c)) - val), 0, cast<double>(type_of<T>().max())));

    return dst;
}

template<typename S, typename T>
Func average_value(GeneratorInput<Buffer<S>> &src,
                   GeneratorInput<Buffer<uint8_t>> &roi,
                   int32_t width, int32_t height,
                   const bool auto_schedule = false)
{
    Var x{"x"}, c{"c"};
    Func count{"count"}, dst{"dst"};
    RDom r{0, width, 0, height, "r"};
    r.where(roi(r.x, r.y) != 0);

    count(x) = sum(select(roi(r.x, r.y) == 0, 0, 1));
    if (!auto_schedule) schedule(count, {1});
    dst(x, c) = cast<T>(select(count(x)==0, 0,
                               sum(cast<double>(src(r.x, r.y, c)))/count(x)));

    return dst;
}

template<typename T>
Func filter_or(GeneratorInput<Buffer<T>> &src0, GeneratorInput<Buffer<T>> &src1) {
    Var x{"x"}, y{"y"}, c{"c"};
    Func dst;
    dst(x, y, c) = src0(x, y, c) | src1(x, y, c);

    return dst;
}



} // anonymous

} // element
} // Halide

#endif
