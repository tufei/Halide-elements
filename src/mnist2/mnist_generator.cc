#include <fstream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <Halide.h>
#include <Element2.h>

using namespace Halide;
using namespace Halide::Element;
using namespace Halide::Element::Cnn;

typedef float ElemT;

class MNIST : public Halide::Generator<MNIST> {
    GeneratorInput<Buffer<float>> in{"in", 4};

    GeneratorParam<int32_t> batch_size{"batch_size", 128};

    GeneratorOutput<Buffer<float>> out{"out", 2};

public:
    void generate()
    {
        // const std::string param_name = "data/LeNet.bin";
        const std::string param_name = "data/LeNet_binarize.bin";

        Type type = Float(32);

        input_shape[3] = batch_size;

        net.add_layer("Conv", "conv1", type, 5, 20, 1, 0, true);
        net.add_layer("BatchNorm", "bn1", type);
        net.add_layer("Relu", "relu1", type);
        net.add_layer("Pool", "pool1", type, 2, 2);
        net.add_layer("BatchNorm", "bn2", type);
        net.add_layer("Scale", "scale2", type);
        net.add_layer("BinActive", "active2", type);
        net.add_layer("BinConv", "conv2", type, 5, 50, 1, 0, true);
        net.add_layer("Relu", "relu2", type);
        net.add_layer("Pool", "pool2", type, 2, 2);
        net.add_layer("BatchNorm", "bn3", type);
        net.add_layer("Scale", "scale3", type);
        net.add_layer("BinActive", "active3", type);
        net.add_layer("BinLinear", "ip3", type, 500);
        net.add_layer("Relu", "relu3", type);
        net.add_layer("Linear", "ip4", type, 10);
        net.add_layer("Softmax", "prob", type);

        net.setup(in, input_shape, auto_schedule);
        net.load(param_name);
        net.print_info();

        Var c{"c"}, n{"n"};
        out(c, n) = net.output()(c, n);
    }

    void schedule() {
        if (auto_schedule) {
            in.set_estimates({{0, 1}, {0, 28}, {0, 28}, {0, batch_size}});
            out.set_estimates({{0, 10}, {0, batch_size}});
        } else {
            net.auto_schedule();
        }
    }

private:
    std::vector<int32_t> input_shape{1, 28, 28, 0};

    Net net{"LeNet-5"};
};

HALIDE_REGISTER_GENERATOR(MNIST, mnist)
