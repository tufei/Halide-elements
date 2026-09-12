# Agents Guide for Halide-Elements

This repository contains elemental code snippets implementation using Halide C++ library.

## Build System

**Build tool**: Make (via common.mk template + per-module Makefiles)

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `HALIDE_ROOT` | `/usr/local/` | Path to Halide root directory |
| `HALIDE_BUILD` | `${HALIDE_ROOT}build` | Path to Halide build directory |
| `AUTO_SCHEDULE` | `false` | Enable autoscheduler |
| `TARGET` | `host` | Target platform (host, cuda, etc.) |

## Commands Reference

### Build a Single Module

```bash
export HALIDE_ROOT=~/.local/git/Halide
export HALIDE_BUILD=/usr/local
cd src/<Module>
make
```

### Run Test for Single Module

```bash
export HALIDE_ROOT=~/.local/git/Halide
export HALIDE_BUILD=/usr/local
cd src/<Module>
make && ./<Module>_test
```

### Run All Tests

```bash
export HALIDE_ROOT=~/.local/git/Halide
export HALIDE_BUILD=/usr/local
./testall.sh [-t target]
```

**Target options:**
- `host` — Build and run regular host tests (default)
- `csim` — Run CSIM tests for FPGA simulation
- `device` — Build device code for deployment
- `clean` — Clean all build artifacts across modules

### Clean Build Artifacts

```bash
cd src/<Module>
make clean
rm -f *.log
```

### Clean All Modules

```bash
./testall.sh clean
```

## Code Style Guidelines

### Import Conventions

Include order in `.cc` files:
1. C headers: `#include <stdint.h>`, `<limits.h>`, `<algorithm>`, `<numeric>`
2. Standard templates: `#include <iostream>`, `<string>`, `<vector>`, `<exception>`, `<cstdlib>`
3. Halide headers: `#include "Halide.h"`, `#include "Element.h"`
4. Local project headers: `#include "test_common.h"`, `#include "halide_benchmark.h"`

```cpp
#include <cstdlib>
#include <iostream>
#include <climits>
#include <limits>
#include <exception>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdint>
#include "Halide.h"
#include "Element.h"
#include "test_common.h"
```

Use `using namespace Halide;` at file top level. Avoid using wildcards inside function scope.

### Formatting & Indentation

- Use spaces for indentation (Makefile is an exception)
- Single trailing newline at end of file
- Spaces before braces `{ ... }`
- No unnecessary comments in generated code

### Type Conventions

- Use exact-width types from `cstdint`:
  - `uint8_t`, `uint16_t`, `uint32_t` for integer operations
  - Avoid generic `int`, `long` unless necessary
- For Halide type traits:

```cpp
typedef upper_t = typename Halide::Element::Upper<T>::type;
```

### Naming Conventions

- **Files**: `<module>_generator.cc` and `<module>_test.cc` pattern
- **Constants**: UPPER_SNAKE_CASE for constexpr (`WIDTH`, `HEIGHT`, `DEPTH`)
- **Variables**: Descriptive names (`src0`, `output`, `x_outer`, `x_inner`, `extents`)
- **Halide functions**: Readable Var names (`x`, `y`, `c`, `dst`)

### Error Handling

```cpp
catch (const std::exception& e) {
    fmt::print(stderr, "{}\n", e.what());
    return 1;
}
```

- Throw `std::runtime_error` with descriptive formatted messages
- Use `fmt::format()` for error string construction
- Catch `const std::exception&` for proper const-correctness

### Halide Generator Pattern

```cpp
template<typename T>
class Add : public Halide::Generator<Add<T>> {
public:
    GeneratorParam<int32_t> width{"width", 1024};
    GeneratorParam<int32_t> height{"height", 768};
    GeneratorParam<int32_t> depth{"depth", 3};

    GeneratorInput<Buffer<T>> src0{"src0", 3};
    GeneratorInput<Buffer<T>> src1{"src1", 3};
    GeneratorOutput<Buffer<T>> dst{"dst", 3};

    void generate() override;

    void schedule() override {
        if (this->using_autoscheduler()) {
            // Use auto-scheduler with estimates
            src0.set_estimates({{0, WIDTH}, {0, HEIGHT}, {0, DEPTH}});
            dst.set_estimates({{0, WIDTH}, {0, HEIGHT}, {0, DEPTH}});
        } else {
            // Manual scheduling
            Var x_outer, x_inner;
            dst.split(x, x_outer, x_inner,
                      natural_vector_size(dst.type()))
               .vectorize(x_inner)
               .parallel(y);
        }
    }
};

HALIDE_REGISTER_GENERATOR(Add<uint8_t>, add_u8);
HALIDE_REGISTER_GENERATOR(Add<uint16_t>, add_u16);
HALIDE_REGISTER_GENERATOR(Add<uint32_t>, add_u32);
```

### Test Pattern

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

### File Structure per Module

```
src/<Module>/
├── <Module>_generator.cc  # Halide Generator definition
└── <Module>_test.cc       # Test code with benchmarks and verification
```

## Include Directory

The `include/` directory provides shared headers:
- `Element.h`, `Element2.h` — Element type utilities
- `run_common.h` — Runtime common utilities
- `test_common.h` — Testing infrastructure (buffer creation, helpers)
- `timing.h` — Benchmark timing utilities

## Test Verification

All tests should verify:
1. Correctness: element-wise output matching expected computation
2. Performance: benchmark execution time reporting via `halide_benchmark.h`
3. Edge cases: boundary conditions, overflow protection with type limits

## Module Examples

Common module types:
- **Elementary operations**: add, sub, mul, div, abs, neg, etc.
- **Boolean ops**: and, or, xor, nand, nor, equal, etc.
- **Scalar ops**: _scalar variants for unary/binary operations
- **Image filters**: gaussian, sobel, laplacian, bilateral, etc.
- **Transforms**: affine, warp_* (perspective, bicubic, etc.)
- **Convolution**: convolution, arbitrary_bits variants
- **Histograms**: histogram, histogram2d, integral
- **Color transforms**: scale_* interpolation methods (bilinear, bicubic, NN)
