#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <climits>

#include "halide_benchmark.h"

#include "warp_perspective_bicubic_u8.h"
#include "warp_perspective_bicubic_u16.h"

#include "test_common.h"

using namespace Halide::Tools;

void getCubicKernel(float n, std::array<float, 4>& w)
{
    static constexpr float a = -0.75f;

    w[0] = ((a * (n + 1.0f) - 5.0f * a) * (n + 1.0f) + 8.0f * a) *
           (n + 1.0f) -4.0f * a;
    w[1] = ((a + 2.0f) * n - (a + 3.0f)) * n * n + 1.0f;
    w[2] = ((a + 2.0f) * (1.0f - n) - (a + 3.0f)) *
           (1.0f - n) * (1.0f - n) + 1.0f;
    w[3] = 1.0f - w[2] - w[1] - w[0];
}

template<typename T>
T interpolateBC(const Halide::Runtime::Buffer<T>& data,
                const int width, const int height, float x, float y,
                const int channel, T border_value, const int border_type)
{
    if (x != x) x = 0;
    if (y != y) y = 0;
    x -= 0.5f;
    y -= 0.5f;

    int xf = static_cast<int>(x - 1.0f);
    int yf = static_cast<int>(y - 1.0f);

    xf = xf - (xf > x - 1);
    yf = yf - (yf > y - 1);

    std::array<std::array<float, 4>, 4> d;
    if (xf >= 0 && yf >= 0 && xf < width - 3 && yf < height - 3) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                d[i][j] = data(xf + j, yf + i, channel);
            }
        }
    } else {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (xf >= -j && yf >= -i && xf < width - j && yf < height - i) {
                    d[i][j] = data(xf + j, yf + i, channel);
                } else if (border_type == 1) {
                    int xfj = std::clamp(xf + j, 0, width - 1);
                    int yfi = std::clamp(yf + i, 0, height - 1);
                    d[i][j] = data(xfj, yfi, channel);
                } else {
                    assert(border_type == 0);
                    d[i][j] = border_value;
                }
            }
        }
    }

    std::array<float, 4> w{};
    const float dx = std::clamp(x - xf - 1.f, 0.f, 1.f);
    getCubicKernel(dx, w);

    std::array<float, 4> col{};
    for (int i = 0; i < 4; i++) {
        col[i] = (d[i][0] * w[0] + d[i][1] * w[1]) +
                 (d[i][2] * w[2] + d[i][3] * w[3]);
    }

    const float dy = std::clamp(y - yf - 1.f, 0.f, 1.f);
    getCubicKernel(dy, w);
    float value = (col[0] * w[0] + col[1] * w[1]) +
                  (col[2] * w[2] + col[3] * w[3]) + 0.5f;

    T min = (std::numeric_limits<T>::min)();
    T max = (std::numeric_limits<T>::max)();
    return static_cast<T>(std::clamp<float>(value, min, max));
}


template<typename T>
Halide::Runtime::Buffer<T>& BC_ref(Halide::Runtime::Buffer<T>& dst,
                                const Halide::Runtime::Buffer<T>& src,
                                const int32_t width, const int32_t height,
                                const int32_t depth,
                                const T border_value, const int32_t border_type,
                                const Halide::Runtime::Buffer<double>& transform)
{
    /* avoid overflow from X-1 to X+2 */
    float imin = static_cast<float>((std::numeric_limits<int>::min)() + 1);
    float imax = static_cast<float>((std::numeric_limits<int>::max)() - 2);

    for(int c = 0; c < depth; ++c){
        for(int i = 0; i < height; ++i){
            float org_y = static_cast<float>(i) + 0.5f;
            float src_xw0 = static_cast<float>(transform(2)) +
                            static_cast<float>(transform(1)) * org_y;
            float src_yw0 = static_cast<float>(transform(5)) +
                            static_cast<float>(transform(4)) * org_y;
            float src_w0 = static_cast<float>(transform(8)) +
                           static_cast<float>(transform(7)) * org_y;
            for(int j = 0; j < width; ++j){
                float org_x = static_cast<float>(j) + 0.5f;
                float inv_w = 1.0f / (src_w0 + static_cast<float>(transform(6)) * org_x);
                float src_x = (src_xw0 + static_cast<float>(transform(0)) * org_x) * inv_w;
                float src_y = (src_yw0 + static_cast<float>(transform(3)) * org_x) * inv_w;

                src_x = std::max(imin, std::min(imax, src_x));
                src_y = std::max(imin, std::min(imax, src_y));

                dst(j, i, c) = interpolateBC(src, width, height, src_x, src_y, c, border_value, border_type);
            }
        }
    }
    return dst;
}


template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer,
                     T _border_value, struct halide_buffer_t *_transform,
                     struct halide_buffer_t *_dst_buffer))

{
    try {
        const int width = 1024;
        const int height = 768;
        const int depth = 3;
        const std::vector<int32_t> extents{width, height, depth};
        const std::vector<int32_t> tableSize{9};
        const T border_value = mk_rand_scalar<T>();
        const int32_t border_type = 1; // 0 or 1
        auto transform = mk_rand_buffer<double>(tableSize);
        auto input = mk_rand_buffer<T>(extents);
        auto output = mk_null_buffer<T>(extents);

        input.set_host_dirty();

        const auto &result = benchmark([&]() {
            func(input, border_value, transform, output);
            output.device_sync(); });

        std::cout << "Execution time: " << double(result) * 1e3 << "ms\n";

        auto expect = mk_null_buffer<T>(extents);
        expect = BC_ref(expect, input, width, height, depth, border_value, border_type, transform);

        output.copy_to_host();

        for (int c=0; c<depth; ++c) {
            //for each x and y
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    if (expect(x, y, c) != output(x, y, c)) {
                        throw std::runtime_error(format("Error: expect(%d, %d, %d) = %d, actual(%d, %d, %d) = %d",
                                                        x, y, c, expect(x, y, c), x, y, c, output(x, y, c)).c_str());
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    printf("Success!\n");
    return 0;
}

int main(int argc, char **argv)
{
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA

#ifdef TYPE_u8
    test<uint8_t>(warp_perspective_bicubic_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(warp_perspective_bicubic_u16);
#endif

    return 0;
}

