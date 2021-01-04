#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <Halide.h>

#include "Schedule.h"
#include "Reduction.h"

namespace Halide {
namespace Element {
namespace{

template<typename T>
Func dilate(Func src, int32_t width, int32_t height, int32_t depth,
            int32_t window_width, int32_t window_height, Func structure,
            int32_t iteration, const bool auto_schedule = false)
{
    Var x{"x"}, y{"y"}, c{"c"};
    RDom r{-(window_width / 2), window_width, -(window_height / 2), window_height};

    Func allzero{"allzero"};
    allzero(x) = cast<bool>(true);
    allzero(x) = allzero(x) &&
                 (structure(r.x + window_width / 2,
                            r.y + window_height / 2) == 0);
    if (!auto_schedule) schedule(allzero, {1});

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0 && !auto_schedule) {
            schedule(dst, {width, height, depth});
        }
        Func clamped =
            BoundaryConditions::repeat_edge(dst,
                                            {{0, cast<int32_t>(width)},
                                             {0, cast<int32_t>(height)},
                                             {0, cast<int32_t>(depth)}});

        Func workbuf{"workbuf" + std::to_string(i)};
        workbuf(x, y, c) =
            select(allzero(0),
                   clamped(x - window_width / 2, y - window_height / 2, c),
                   maximum_unroll(r, select(structure(r.x + window_width / 2,
                                                      r.y + window_height / 2) == 0,
                                            type_of<T>().min(),
                                            clamped(x + r.x, y + r.y, c))));
        dst = workbuf;
    }

    return dst;
}

template<typename T>
Func dilate(GeneratorInput<Buffer<T>> &src,
            int32_t width, int32_t height, int32_t depth,
            int32_t window_width, int32_t window_height,
            Func structure, int32_t iteration,
            const bool auto_schedule = false)
{
    Func s = src;
    return dilate<T>(s, width, height, depth, window_width, window_height,
                     structure, iteration, auto_schedule);
}

template<typename T>
Func dilate_rect(Func src, int32_t width, int32_t height, int32_t depth,
                 int32_t window_width, int32_t window_height,
                 int32_t iteration, const bool auto_schedule = false)
{
    Var x{"x"}, y{"y"}, c{"c"};
    RDom r{-(window_width / 2), window_width,
           -(window_height / 2), window_height};

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0 && !auto_schedule) {
            schedule(dst, {width, height, depth});
        }
        Func clamped =
            BoundaryConditions::repeat_edge(dst,
                                            {{0, cast<int32_t>(width)},
                                             {0, cast<int32_t>(height)},
                                             {0, cast<int32_t>(depth)}});

        Func workbuf{"workbuf" + std::to_string(i)};
        workbuf(x, y, c) = maximum_unroll(r, clamped(x + r.x, y + r.y, c));

        dst = workbuf;
    }

    return dst;
}

template<typename T>
Func dilate_rect(GeneratorInput<Buffer<T>> &src,
                 int32_t width, int32_t height, int32_t depth,
                 int32_t window_width, int32_t window_height,
                 int32_t iteration, const bool auto_schedule = false)
{
    Func s = src;
    return dilate_rect<T>(s, width, height, depth,
                          window_width, window_height,
                          iteration, auto_schedule);
}

template<typename T>
Func dilate_cross(Func src, int32_t width, int32_t height, int32_t depth,
                  int32_t window_width, int32_t window_height,
                  int32_t iteration, const bool auto_schedule = false)
{
    Var x{"x"}, y{"y"}, c{"c"};
    RDom r{-(window_width / 2), window_width,
           -(window_height / 2), window_height};
    r.where(r.x == 0 || r.y == 0);

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0 && !auto_schedule) {
            schedule(dst, {width, height, depth});
        }
        Func clamped =
            BoundaryConditions::repeat_edge(dst,
                                            {{0, cast<int32_t>(width)},
                                             {0, cast<int32_t>(height)},
                                             {0, cast<int32_t>(depth)}});

        Func workbuf{"workbuf" + std::to_string(i)};
        workbuf(x, y, c) = maximum_unroll(r, clamped(x + r.x, y + r.y, c));

        dst = workbuf;
    }

    return dst;
}

template<typename T>
Func dilate_cross(GeneratorInput<Buffer<T>> &src,
                  int32_t width, int32_t height, int32_t depth,
                  int32_t window_width, int32_t window_height,
                  int32_t iteration, const bool auto_schedule = false)
{
    Func s = src;
    return dilate_cross<T>(s, width, height, depth,
                           window_width, window_height,
                           iteration, auto_schedule);
}

Func conv_rect(Func src, std::function<Expr(RDom, Expr)> f, int32_t width, int32_t height, int32_t iteration, int32_t window_width, int32_t window_height) {
    Var x{"x"}, y{"y"};
    RDom r{-(window_width / 2), window_width, -(window_height / 2), window_height};

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0) {
            schedule(dst, {width, height});
        }
        Func clamped = BoundaryConditions::repeat_edge(dst, {{0, cast<int32_t>(width)}, {0, cast<int32_t>(height)}});

        Func workbuf{"workbuf" + std::to_string(i)};
        workbuf(x, y) = f(r, clamped(x + r.x, y + r.y));

        dst = workbuf;
    }

    return dst;
}


Func conv_cross(Func src, std::function<Expr(RDom, Expr)> f, int32_t width, int32_t height, int32_t iteration, int32_t window_width, int32_t window_height) {
    Var x{"x"}, y{"y"};
    RDom r{-(window_width / 2), window_width, -(window_height / 2), window_height};
    r.where(r.x == 0 || r.y == 0);

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0) {
            schedule(dst, {width, height});
        }
        Func clamped = BoundaryConditions::repeat_edge(dst, {{0, cast<int32_t>(width)}, {0, cast<int32_t>(height)}});

        Func workbuf("workbuf");
        workbuf(x, y) = f(r, clamped(x + r.x, y + r.y));

        dst = workbuf;
    }

    return dst;
}


Func conv_with_structure(Func src, std::function<Expr(RDom, Expr)> f, Expr init, Func structure,
                         int32_t width, int32_t height, int32_t window_width, int32_t window_height, int32_t iteration) {
    Var x{"x"}, y{"y"};
    RDom r{-(window_width / 2), window_width, -(window_height / 2), window_height};

    Func allzero{"allzero"};
    allzero(x) = cast<bool>(true);
    allzero(x) = allzero(x) && (structure(r.x + window_width / 2, r.y + window_height / 2) == 0);
    schedule(allzero, {1});

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0) {
            schedule(dst, {width, height});
        }
        Func clamped = BoundaryConditions::repeat_edge(dst, {{0, cast<int32_t>(width)}, {0, cast<int32_t>(height)}});

        Func workbuf("workbuf");
        workbuf(x, y)= select(allzero(0),
                              clamped(x - window_width / 2, y - window_height / 2),
                              f(r, select(structure(r.x + window_width / 2, r.y + window_height / 2) == 0,
                                          init,
                                          clamped(x + r.x, y + r.y))));
        dst = workbuf;
    }

    return dst;
}

template<typename T>
Func erode(Func src, int32_t width, int32_t height, int32_t depth,
           int32_t window_width, int32_t window_height,
           Func structure, int32_t iteration,
           const bool auto_schedule = false)
{
    Var x{"x"}, y{"y"}, c{"c"};
    RDom r{-(window_width / 2), window_width,
           -(window_height / 2), window_height};

    Func allzero{"allzero"};
    allzero(x) = cast<bool>(true);
    allzero(x) = allzero(x) &&
                 (structure(r.x + window_width / 2,
                            r.y + window_height / 2) == 0);
    if (!auto_schedule) schedule(allzero, {1});

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0 && !auto_schedule) {
            schedule(dst, {width, height, depth});
        }
        Func clamped =
            BoundaryConditions::repeat_edge(dst,
                                            {{0, cast<int32_t>(width)},
                                             {0, cast<int32_t>(height)},
                                             {0, cast<int32_t>(depth)}});

        Func workbuf("workbuf" + std::to_string(i));
        workbuf(x, y, c) =
            select(allzero(0),
                   clamped(x - window_width / 2, y - window_height / 2, c),
                   minimum_unroll(r, select(structure(r.x + window_width / 2,
                                                      r.y + window_height / 2) == 0,
                                            type_of<T>().max(),
                                            clamped(x + r.x, y + r.y, c))));

        dst = workbuf;
    }

    return dst;
}

template<typename T>
Func erode(GeneratorInput<Buffer<T>> &src,
           int32_t width, int32_t height, int32_t depth,
           int32_t window_width, int32_t window_height,
           Func structure, int32_t iteration,
           const bool auto_schedule = false)
{
    Func s = src;
    return erode<T>(s, width, height, depth, window_width, window_height,
                    structure, iteration, auto_schedule);
}

template<typename T>
Func erode_cross(Func src, int32_t width, int32_t height, int32_t depth,
                 int32_t window_width, int32_t window_height,
                 int32_t iteration, const bool auto_schedule = false)
{
    Var x{"x"}, y{"y"}, c{"c"};
    RDom r{-(window_width / 2), window_width,
           -(window_height / 2), window_height};
    r.where(r.x == 0 || r.y == 0);

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0 && !auto_schedule) {
            schedule(dst, {width, height, depth});
        }
        Func clamped =
            BoundaryConditions::repeat_edge(dst,
                                            {{0, cast<int32_t>(width)},
                                             {0, cast<int32_t>(height)},
                                             {0, cast<int32_t>(depth)}});

        Func workbuf{"workbuf" + std::to_string(i)};
        workbuf(x, y, c) = minimum_unroll(r, clamped(x + r.x, y + r.y, c));

        dst = workbuf;
    }

    return dst;
}

template<typename T>
Func erode_cross(GeneratorInput<Buffer<T>> &src,
                 int32_t width, int32_t height, int32_t depth,
                 int32_t window_width, int32_t window_height,
                 int32_t iteration, const bool auto_schedule = false)
{
    Func s = src;
    return erode_cross<T>(s, width, height, depth,
                          window_width, window_height,
                          iteration, auto_schedule);
}

template<typename T>
Func erode_rect(Func src, int32_t width, int32_t height, int32_t depth,
                int32_t window_width, int32_t window_height,
                int32_t iteration, const bool auto_schedule = false)
{
    Var x{"x"}, y{"y"}, c{"c"};
    RDom r{-(window_width / 2), window_width,
           -(window_height / 2), window_height};

    Func dst = src;

    for (int32_t i = 0; i < iteration; i++) {
        if (i != 0 && !auto_schedule) {
            schedule(dst, {width, height, depth});
        }
        Func clamped =
            BoundaryConditions::repeat_edge(dst,
                                            {{0, cast<int32_t>(width)},
                                             {0, cast<int32_t>(height)},
                                             {0, cast<int32_t>(depth)}});

        Func workbuf{"workbuf" + std::to_string(i)};
        workbuf(x, y, c) = minimum_unroll(r, clamped(x + r.x, y + r.y, c));

        dst = workbuf;
    }

    return dst;
}

template<typename T>
Func erode_rect(GeneratorInput<Buffer<T>> &src,
                int32_t width, int32_t height, int32_t depth,
                int32_t window_width, int32_t window_height,
                int32_t iteration, const bool auto_schedule = false)
{
    Func s = src;
    return erode_rect<T>(s, width, height, depth,
                         window_width, window_height,
                         iteration, auto_schedule);
}

} // anonymous
} // Element
} // Halide
