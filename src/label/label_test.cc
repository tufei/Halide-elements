#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include "test_common.h"
#include "halide_benchmark.h"

#include "label_u8.h"
#include "label_u16.h"

#include "second_pass.h"

#include <map>
#include <chrono>

using namespace Halide::Tools;

template<typename T>
Halide::Runtime::Buffer<uint32_t>& label_ref(Halide::Runtime::Buffer<uint32_t>& dst,
                                             const Halide::Runtime::Buffer<T>& src,
                                             const int32_t width, const int32_t height)
{
    int dst_pitch = width;
    int x, y;

    for (int i = 0; i < height; ++i) {
      uint32_t label4 = 0;
      for (int j = 0; j < width; ++j) {
        if (src(j, i) == 0) {
          label4 = dst(j, i) = 0;
          continue;
        }
        uint32_t label1 = i > 0 && j > 0 ? dst(j-1, i-1) : 0;
        uint32_t label2 = i > 0 ? dst(j, i-1) : 0;
        uint32_t label3 = i > 0 && j < width - 1 ? dst(j+1, i-1) : 0;

        if (label1 > 0){
            x = (label1-1)%dst_pitch;
            y = (label1-1)/dst_pitch;
            while (label1 != dst(x, y)){
                label1 = dst(x, y);
                x = (label1-1)%dst_pitch;
                y = (label1-1)/dst_pitch;
            }
        }
        if (label2 > 0){
            x = (label2-1)%dst_pitch;
            y = (label2-1)/dst_pitch;
            while (label2 != dst(x, y)){
                label2 = dst(x, y);
                x = (label2-1)%dst_pitch;
                y = (label2-1)/dst_pitch;
            }
        }
        if (label3 > 0){
            x = (label3-1)%dst_pitch;
            y = (label3-1)/dst_pitch;
            while (label3 != dst(x, y)){
                label3 = dst(x, y);
                x = (label3-1)%dst_pitch;
                y = (label3-1)/dst_pitch;
            }
        }
        if (label4 > 0){
            x = (label4-1)%dst_pitch;
            y = (label4-1)/dst_pitch;
            while (label4 != dst(x, y)){
                label4 = dst(x, y);
                x = (label4-1)%dst_pitch;
                y = (label4-1)/dst_pitch;
            }
        }

        if (label1 == 0 && label2 == 0 && label3 == 0 && label4 == 0) {
          label4 = i * dst_pitch + j + 1;
          dst(j, i) = label4;
          continue;
        }

        if (label1 - 1 > label4 - 1 ) (std::swap)(label1, label4);
        if (label2 - 1 > label3 - 1 ) (std::swap)(label2, label3);
        if (label1 - 1 > label2 - 1 ) (std::swap)(label1, label2);

        if (label2 > 0){
            x = (label2-1)%dst_pitch;
            y = (label2-1)/dst_pitch;
            dst(x, y)= label1;

        }
        if (label3 > 0){
            x = (label3-1)%dst_pitch;
            y = (label3-1)/dst_pitch;
            dst(x, y)= label1;
        }
        if (label4 > 0){
            x = (label4-1)%dst_pitch;
            y = (label4-1)/dst_pitch;
            dst(x, y)= label1;
        }

        label4 = dst(j, i) = label1;
      }
    }

    // second pass
    for (int i = 0; i < height; ++i) {
      for (int j = 0; j < width; ++j) {
        uint32_t label = dst(j, i);
        if (label > 0){
            x = (label-1)%dst_pitch;
            y = (label-1)/dst_pitch;

            while (label != dst(x, y)) {
                label = dst(x, y);
                x = (label-1)%dst_pitch;
                y = (label-1)/dst_pitch;
            }
        }

        dst(j, i) = label;
      }
    }

    return dst;
}

//non-Halide process between 2 Halide functions in Halide::Element//////////////
std::map<uint32_t, uint32_t, std::greater<uint32_t>> insertMap(std::map<uint32_t, uint32_t, std::greater<uint32_t>>& myMap,
                                                               uint32_t from, uint32_t into){
    std::pair<std::map<uint32_t, uint32_t>::iterator,bool> ret;
    ret = myMap.insert(std::pair<uint32_t,uint32_t>(from,into));
    if(ret.second == false){
        if(myMap[from] > into){
            myMap = insertMap(myMap, myMap[from], into);
            myMap[from] = into;
        }else if(myMap[from] < into){
            myMap = insertMap(myMap, into, myMap[from]);
        }
    }
    return myMap;
}

Halide::Runtime::Buffer<uint32_t>  mergeSameGroup(Halide::Runtime::Buffer<uint32_t>& srcdst,
                                                  Halide::Runtime::Buffer<uint32_t>& marker,
                                                  int32_t width, int32_t height)
{
    std::map<uint32_t, uint32_t, std::greater<uint32_t>> sameLabel;
    for(int i = 0; i<height; i++){
        for(int j = 0; j<width; j++){
            if(marker(j, i)!= 0){
                uint32_t minLabel = srcdst(j, i);
                uint32_t label1 = j > 0 && i > 0 ? srcdst(j-1, i-1) : 0;
                uint32_t label2 = i > 0 ? srcdst(j, i-1) : 0;
                uint32_t label3 = j < width-1 && i > 0 ? srcdst(j+1, i-1) : 0;
                uint32_t label4 = j > 0 ? srcdst(j-1, i) : 0;

                if(label1 > minLabel)
                    sameLabel = insertMap(sameLabel, label1, minLabel);
                if(label2 > minLabel)
                    sameLabel = insertMap(sameLabel, label2, minLabel);
                if(label3 > minLabel)
                    sameLabel = insertMap(sameLabel, label3, minLabel);
                if(label4 > minLabel)
                    sameLabel = insertMap(sameLabel, label4, minLabel);
            }
        }
    }
    int bufWith = sameLabel.size();
    const std::vector<int32_t> extents{bufWith, 2};

    Halide::Runtime::Buffer<uint32_t> toReturn(extents);
    int r = 0;
    for(auto m: sameLabel){
        toReturn(r, 0) = m.first;
        toReturn(r, 1) = m.second;
        r++;
    }
    return toReturn;
}



template<typename T>
int test(int (*first_pass)(struct halide_buffer_t *_src_buffer,
                           struct halide_buffer_t *_dst0_buffer,
                           struct halide_buffer_t *_dst1_buffer))
{
    try {
        bool printbuff = false;
        bool printtime = false;

        constexpr int width{1024};
        constexpr int height{768};
        const std::vector<int32_t> extents{width, height};
        auto input = mk_rand_buffer<T>(extents);
        Halide::Runtime::Buffer<uint32_t> pass1[2];
        pass1[0] = mk_null_buffer<uint32_t>(extents);
        pass1[1] = mk_null_buffer<uint32_t>(extents);
        auto output = mk_null_buffer<uint32_t>(extents);

        //more likely to have 0 and separate objects in input image
        for (int j=0; j<width; ++j) {
            for (int i=0; i<height; ++i) {
                input(j,i) = input(j,i)%10;
                if(input(j,i) < 6){
                    input(j,i) = 0;
                }
            }
        }
        input.set_host_dirty();

        auto expect = mk_null_buffer<uint32_t>(extents);
        expect = label_ref(expect, input, width, height);

        auto halide1_s = std::chrono::high_resolution_clock::now();
        const auto result0 = benchmark([&]() {
            first_pass(input, pass1[0], pass1[1]);
            pass1[0].device_sync();
            pass1[1].device_sync(); });
        fmt::print("Execution time 1st pass: {} ms\n", double(result0) * 1e3);
        pass1[0].copy_to_host();
        pass1[1].copy_to_host();
        auto halide1_e = std::chrono::high_resolution_clock::now();

        auto non_halide_s = std::chrono::high_resolution_clock::now();
        Halide::Runtime::Buffer<uint32_t> buf = mergeSameGroup(pass1[0], pass1[1], width, height);
        auto non_halide_e = std::chrono::high_resolution_clock::now();

        int32_t bufWidth = buf.width();

        auto halide2_s = std::chrono::high_resolution_clock::now();
        const auto result1 = benchmark([&]() {
            second_pass(pass1[0], buf, bufWidth, output);
            output.device_sync(); });
        fmt::print("Execution time 2nd pass: {} ms\n", double(result1) * 1e3);
        auto halide2_e = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> dth1 = halide1_e - halide1_s;
        std::chrono::duration<double> dtn = non_halide_e - non_halide_s;
        std::chrono::duration<double> dth2 = halide2_e - halide2_s;

        if(printtime==true){
            fmt::print("\nsize of bffer is {}:::for each, Halide part1:{}s, non-Halide part:{}s, Halide part2:{}s\n",
                       bufWidth, dth1.count(), dtn.count(), dth2.count());
        }
        if(printbuff == true){
                fmt::print("\ninput\n");
                for (int i=0; i<height; ++i) {
                    for (int j=0; j<width; ++j) {
                                if(input(j,i)==0)
                                fmt::print("    .");
                                else
                                fmt::print("{:5d}", input(j, i));
                            }
                            fmt::print("\n");
                        }

                fmt::print("\npass1[0]\n");
                for (int i=0; i<height; ++i) {
                    for (int j=0; j<width; ++j) {
                                if(pass1[0](j,i)==0)
                                fmt::print("    .");
                                else
                                fmt::print("{:5d}", pass1[0](j, i));
                            }
                            fmt::print("\n");
                        }
            fmt::print("\npass1[1]\n");
                    for (int i=0; i<height; ++i) {
                        for (int j=0; j<width; ++j) {
                            if(pass1[1](j,i)==0)
                            fmt::print("    .");
                            else
                            fmt::print("{:5d}", pass1[1](j, i));
                        }
                        fmt::print("\n");
                    }

                fmt::print("\noutput\n");
                        for (int i=0; i<height; ++i) {
                            for (int j=0; j<width; ++j) {
                                if(output(j,i)==0)
                                fmt::print("    .");
                                else
                                fmt::print("{:5d}", output(j, i));
                            }
                            fmt::print("\n");
                        }
        }

        // for each x and y
        for (int i=0; i<height; ++i) {
          for (int j=0; j<width; ++j) {
              if (abs(expect(j, i) - output(j, i)) > 0) {
                  throw std::runtime_error(
                      fmt::format("Error: expect({}, {}) = {}, actual({}, {}) = {}",
                                  j, i, expect(j, i), j, i, output(j, i)));
              }

          }
        }
    } catch (const std::exception& e){
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
    test<uint8_t>(label_u8);
#endif
#ifdef TYPE_u16
    test<uint16_t>(label_u16);
#endif
}
