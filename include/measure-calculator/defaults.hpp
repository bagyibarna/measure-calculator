#pragma once

#include "spec.hpp"

namespace Calc {

namespace Defaults {

using Unary = double (*)(double);
using Binary = double (*)(double, double);

const SpecFor<UnaryOp> kNegateUnaryOp{
    {"-", {.func = std::negate<double>{}, .precedence = 12}},
};

const SpecFor<BinaryOp> kArithmeticBinaryOps{
    {"*", {.func = std::multiplies<double>{}, .precedence = 8}},
    {"/", {.func = std::divides<double>{}, .precedence = 8}},
    {"+", {.func = std::plus<double>{}, .precedence = 4}},
    {"-", {.func = std::minus<double>{}, .precedence = 4}},
};

const SpecFor<UnaryFun> kBasicUnaryFuns{
    {"abs", UnaryFun{.func = Unary(std::abs)}},

    {"ceil", UnaryFun{.func = Unary(std::ceil)}},
    {"floor", UnaryFun{.func = Unary(std::floor)}},
    {"round", UnaryFun{.func = Unary(std::round)}},
};

const SpecFor<UnaryFun> kExponentialUnaryFuns{
    {"exp", UnaryFun{.func = Unary(std::exp)}},
    {"exp2", UnaryFun{.func = Unary(std::exp2)}},
    {"sqrt", UnaryFun{.func = Unary(std::sqrt)}},

    {"ln", UnaryFun{.func = Unary(std::log)}},
    {"log2", UnaryFun{.func = Unary(std::log2)}},
    {"log10", UnaryFun{.func = Unary(std::log10)}},
};

const SpecFor<UnaryFun> kTrigonometricUnaryFuns{
    {"sin", UnaryFun{.func = Unary(std::sin), .keepsMeasure = false}},
    {"cos", UnaryFun{.func = Unary(std::cos), .keepsMeasure = false}},
    {"tan", UnaryFun{.func = Unary(std::tan), .keepsMeasure = false}},

    {"asin", UnaryFun{.func = Unary(std::asin), .keepsMeasure = false}},
    {"acos", UnaryFun{.func = Unary(std::acos), .keepsMeasure = false}},
    {"atan", UnaryFun{.func = Unary(std::atan), .keepsMeasure = false}},

    {"sinh", UnaryFun{.func = Unary(std::sinh), .keepsMeasure = false}},
    {"cosh", UnaryFun{.func = Unary(std::cosh), .keepsMeasure = false}},
    {"tanh", UnaryFun{.func = Unary(std::tanh), .keepsMeasure = false}},

    {"asinh", UnaryFun{.func = Unary(std::asinh), .keepsMeasure = false}},
    {"acosh", UnaryFun{.func = Unary(std::acosh), .keepsMeasure = false}},
    {"atanh", UnaryFun{.func = Unary(std::atanh), .keepsMeasure = false}},
};

const SpecFor<BinaryFun> kBasicBinaryFuns{
    {"min", BinaryFun{.func = Binary(std::fmin)}},
    {"max", BinaryFun{.func = Binary(std::fmax)}},

    {"pow", BinaryFun{.func = Binary(std::pow)}},
};

const SpecFor<Constant> kBasicConstants{
    {"pi", 3.14159265358979323846},
    {"e", 2.71828182845904523536},
};

const MeasureSpec kLinearMeasure{"length",
                                 {
                                     {"mm", 1e-3},
                                     {"cm", 1e-2},
                                     {"dm", 1e-1},
                                     {"m", 1.},
                                     {"km", 1e3},
                                     {"ft", 0.3048},
                                     {"in", 0.0254},
                                 }};

} // namespace Defaults

} // namespace Calc
