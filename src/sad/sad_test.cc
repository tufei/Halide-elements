#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <climits>

#include "HalideRuntime.h"
#include "HalideBuffer.h"

#include "sad_u8.h"
#include "sad_u16.h"
#include "sad_u32.h"

#include "test_common.h"
#include "halide_benchmark.h"

using namespace std;
using namespace Halide::Tools;

template<typename T>
int test(int (*func)(struct halide_buffer_t *_src_buffer1,
                     struct halide_buffer_t *_src_buffer2,
                     struct halide_buffer_t *_dst_buffer))
{
    try {
        int ret = 0;
        constexpr unsigned int N = 2;
        typedef int64_t S;

        //
        // Run
        //
        constexpr int width = 1024;
        constexpr int height = 768;
        constexpr int depth = 3;
        const std::vector<int> in_extents{width, height, depth};
        const std::vector<int> out_extents{width, height, depth};
        Halide::Runtime::Buffer<T> input[N];
        for (int i = 0; i < N; ++i) {
            input[i] = mk_rand_buffer<T>(in_extents);
            input[i].set_host_dirty();
        }
        auto output = mk_null_buffer<T>(out_extents);

        T *expect = new T[height * width * depth];
        
        // Reference impl.
        // first pass
        for (int c = 0; c < depth; c++) {
            for (int j = 0; j < height; j++) {
              for (int i = 0; i < width; i++) {
                S diff = static_cast<S>(input[0](i,j,c)) -
                  static_cast<S>(input[1](i,j,c));
                expect[(j * width + i) * depth + c] = static_cast<T>(diff < 0 ? -diff : diff);
              }
            }
        }

        const auto result = benchmark([&]() {
            func(input[0], input[1], output);
            output.device_sync(); });
        fmt::print("Execution time: {} ms\n", double(result) * 1e3);

        output.copy_to_host();

//		cout << "input[0]" << endl;
//        for (int y=0; y<height; ++y) {
//			for (int x=0; x<width; ++x) {
//				cout << input[0](x,y) << " ";
//            }
//			cout << endl;
//        }
//		cout << "input[1]" << endl;
//        for (int y=0; y<height; ++y) {
//			for (int x=0; x<width; ++x) {
//				cout << input[1](x,y) << " ";
//            }
//			cout << endl;
//        }
//
//		cout << "halide" << endl;
//        for (int y=0; y<height; ++y) {
//			for (int x=0; x<width; ++x) {
//				T actual = output(x,y);
//				cout << hex << actual << " ";
//            }
//			cout << endl;
//        }
//
//		cout << "ref" << endl;
//        for (int y=0; y<height; ++y) {
//			for (int x=0; x<width; ++x) {
//				cout << hex << expect[y*width+x] << " ";
//            }
//			cout << endl;
//        }

        for (int c=0; c<depth; ++c) {
            for (int y=0; y<height; ++y) {
                for (int x=0; x<width; ++x) {
                    T actual = output(x,y,c);
                    if (expect[(y * width + x) * depth + c] != actual) {
                      throw std::runtime_error(fmt::format("Error: expect({}, {}, {}) = {}, actual({}, {}, {}) = {}, input0={},input1={}", x, y, c, expect[(y * width + x) * depth + c], x, y, c, actual, input[0](x,y,c), input[1](x,y,c)));
                    }
                }
            }
        }
        delete[] expect;
    } catch (const std::exception& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }

    fmt::print("Success!\n");
    return 0;
}

int main()
{
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA
#ifdef TYPE_u8
    test<uint8_t>(sad_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(sad_u16);
#endif
#ifdef TYPE_u32
    test<uint32_t>(sad_u32);
#endif
}
