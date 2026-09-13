#include <cstdlib>
#include <string>

#include "affine.h"

#include "halide_benchmark.h"
#include "halide_image_io.h"

#include "test_common.h"

using namespace Halide::Tools;

int main(int argc, char **argv) {
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

    if (argc < 8) {
        fmt::print("Usage: affine_test2 input.png degrees scale_x scale_y shift_x shift_y skew_y\n");
        fmt::print("ex)    affine_test2 input.png 30 1 1 10 30 30\n");
        return 1;
    }

    std::string in_fname = std::string(argv[1]);
    float degrees = atof(argv[2]);
    float scale_x = atof(argv[3]);
    float scale_y = atof(argv[4]);
    float shift_x = atof(argv[5]);
    float shift_y = atof(argv[6]);
    float skew_y  = atof(argv[7]);

    Halide::Runtime::Buffer<uint8_t> input = Halide::Tools::load_image(in_fname.c_str());
    Halide::Runtime::Buffer<uint8_t> output(input.width(), input.height(), input.channels());

    input.set_host_dirty();

    const auto &result = benchmark([&]() {
        affine(input, degrees, scale_x, scale_y,
               shift_x, shift_y, skew_y, output);
        output.device_sync(); });
    fmt::print("Execution time: {} ms\n", double(result) * 1e3);

    output.copy_to_host();

    Halide::Tools::save_image(output, "affine.png");

    return 0;
}

