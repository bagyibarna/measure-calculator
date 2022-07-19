#include "measure-calculator/defaults.hpp"
#include "measure-calculator/measure-calculator.hpp"

#include <iostream>

using namespace Calc;

int main(int argc, char** argv) {
    std::string to_eval = "";
    for (int i = 1; i < argc; ++i) {
        to_eval.append(argv[i]);
    }

    auto spec = std::get<Spec>(SpecBuilder{
        .unaryOps = Defaults::kNegateUnaryOp,
        .binaryOps = Defaults::kArithmeticBinaryOps,
        .unaryFuns = SpecUnion(Defaults::kBasicUnaryFuns, Defaults::kExponentialUnaryFuns,
                               Defaults::kTrigonometricUnaryFuns),
        .binaryFuns = Defaults::kBasicBinaryFuns,
        .constants = Defaults::kBasicConstants,
        .measures = {Defaults::kLinearMeasure},
    }
                                   .Build());

    auto res = Evaluate(spec, to_eval);
    std::cout << "\"" << to_eval << "\"=";
    if (auto d = std::get_if<double>(&res)) {
        std::cout << *d << std::endl;
    } else {
        std::cout << std::get<Error>(res);
    }
}
