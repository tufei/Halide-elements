#include <iostream>

#include "halide_benchmark.h"

#include "test_common.h"
#include "run_common.h"

#include "sgm.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    try {
        Buffer<uint8_t> in_l = load_pgm("data/left.pgm");
        Buffer<uint8_t> in_r = load_pgm("data/right.pgm");

        in_l.set_host_dirty();
        in_r.set_host_dirty();

        const int width = in_l.extent(0);
        const int height = in_l.extent(1);

        Buffer<uint8_t> out(width, height);

        const auto &result = benchmark([&]() {
            sgm(in_l, in_r, out);
            out.device_sync(); });

        fmt::print("Execution time: {}ms\n", double(result) * 1e3);

        out.copy_to_host();

        Buffer<uint8_t> disp = load_pgm("data/disp.pgm");

        save_pgm("out_test.pgm", out.data(), width, height);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t ev = disp(x, y);
                uint8_t av = out(x, y);
                if (ev != av) {
                    const auto s =
                        fmt::format("Error: expect({}, {}) = {}, "
                                    "actual({}, {}) = {}\n",
                                    x, y, ev, x, y, av);
                    throw std::runtime_error(s);
                }
            }
        }

    } catch (const std::exception& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}
