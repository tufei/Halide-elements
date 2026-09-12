---
color: "#87D981"
description: Update source files in a subdirectory for building and testing with CUDA
effort: high
mode: subagent
model: ollama/qwen3.8:27b-128k
temperature: 1.0
thinking:
  budgetTokens: 131072
  type: enabled
tools:
  bash: true
  edit: true
  glob: true
  grep: true
  list: true
  lsp: true
  question: true
  read: true
  task: true
  todoread: true
  todowrite: true
  webfetch: true
  websearch: true
---

# Instructions

You receive a subdirectory, which should contain a Makefile, generator source files, and test source files. You need to modify the Makefile and test source files for building and testing with CUDA. Leave the generator source files unchanged.

Edit source files only. Ignore any caller instructions that ask you to build and test.

## CUDA-related changes

### Makefile

Add the following lines to the Makefile, delete the lines if they already exist:

```makefile
AUTO_SCHEDULE:=true
AUTO_SCHEDULER:=Anderson2021
TARGET:=host-cuda
```

### Test source file

Add the following lines to the main function of the test source file:
```cpp
#ifdef USE_CUDA
    fmt::print("Checking CUDA...\n");
    if (check_cuda_device()) return 0;
#endif //~USE_CUDA
```

If existing source files do not follow the patterns below, modify them to match the patterns. For example, replace all std::cout, std::cerr, and printf with fmt::print.

```cpp
template<typename T>
int test(int (*func)(Buffer<T>*, Buffer<T>*, Buffer<T>*)) {
    constexpr int width{1024};
    constexpr int height{768};
    constexpr int depth{3};
    const std::vector<int> extents{width, height, depth};

    auto input0 = mk_rand_buffer<T>(extents);
    auto input1 = mk_rand_buffer<T>(extents);
    auto output = mk_null_buffer<T>(extents);

    input0.set_host_dirty();
    input1.set_host_dirty();

    const auto result = benchmark([&]() {
        func(input0, input1, output);
        output.device_sync(); });
    fmt::print("Execution time: {} ms\n", double(result) * 1e3);

    output.copy_to_host();

    // Verify element-wise correctness
    for (int c = 0; c < depth; ++c) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                upper_t f = static_cast<upper_t>(input0(x, y, c)) +
                            static_cast<upper_t>(input1(x, y, c));
                f = std::min(f, static_cast<upper_t>(std::numeric_limits<T>::max()));
                T expect = f;
                T actual = output(x, y, c);

                if (expect != actual) {
                    throw std::runtime_error(
                        fmt::format("Error at ({}, {}, {}): expect={}, actual={}",
                                    x, y, c, expect, actual));
                }
            }
        }
    }

    fmt::print("Success!\n");
    return 0;
}
```

