#include "calc-cpp/calc-cpp.hpp"
#include "calc-cpp/defaults.hpp"

#include <iostream>

using namespace Calc;

int main(int argc, char** argv ) {
    std::string to_eval= "";
    for (int i = 1; i < argc; ++i) {
        to_eval.append(argv[i]);
    }

    auto spec = std::get<Spec>(SpecBuilder {
        .unary_ops = Defaults::kNegateUnaryOp,
        .binary_ops = Defaults::kArithmeticBinaryOps,
        .unary_funs = Defaults::kBasicUnaryFuns + Defaults::kExponentialUnaryFuns +
                    Defaults::kTrigonometricUnaryFuns,
        .binary_funs = Defaults::kBasicBinaryFuns,
        .constants = Defaults::kBasicConstants,
        .measures = {Defaults::kLinearMeasure},
    }.Build());

    auto res = Evaluate(spec, to_eval);
    if (auto d = std::get_if<double>(&res)) {
        std::cout << *d << std::endl;
    } else {
        std::cout << std::get<Error>(res);
    }
}