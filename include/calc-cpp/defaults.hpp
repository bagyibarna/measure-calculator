#pragma once

namespace Calc {

namespace Defaults {

using Unary = double (*)(double);
using Binary = double (*)(double, double);

const Data::Container<Data::UnaryOp> kNegateUnaryOp{
    {"-", {.func = std::negate<double>{}, .precedence = 12}},
};

const Data::Container<Data::BinaryOp> kArithmeticBinaryOps{
    {"*", {.func = std::multiplies<double>{}, .precedence = 8}},
    {"/", {.func = std::divides<double>{}, .precedence = 8}},
    {"+", {.func = std::plus<double>{}, .precedence = 4}},
    {"-", {.func = std::minus<double>{}, .precedence = 4}},
};

const Data::Container<Data::UnaryFun> kBasicUnaryFuns{
    {"abs", Data::UnaryFun{.func = Unary(std::abs)}},

    {"ceil", Data::UnaryFun{.func = Unary(std::ceil)}},
    {"floor", Data::UnaryFun{.func = Unary(std::floor)}},
    {"round", Data::UnaryFun{.func = Unary(std::round)}},
};

const Data::Container<Data::UnaryFun> kExponentialUnaryFuns{
    {"exp", Data::UnaryFun{.func = Unary(std::exp)}},
    {"exp2", Data::UnaryFun{.func = Unary(std::exp2)}},
    {"sqrt", Data::UnaryFun{.func = Unary(std::sqrt)}},

    {"ln", Data::UnaryFun{.func = Unary(std::log)}},
    {"log2", Data::UnaryFun{.func = Unary(std::log2)}},
    {"log10", Data::UnaryFun{.func = Unary(std::log10)}},
};

const Data::Container<Data::UnaryFun> kTrigonometricUnaryFuns{
    {"sin", Data::UnaryFun{.func = Unary(std::sin), .keepsMeasure = false}},
    {"cos", Data::UnaryFun{.func = Unary(std::cos), .keepsMeasure = false}},
    {"tan", Data::UnaryFun{.func = Unary(std::tan), .keepsMeasure = false}},

    {"asin", Data::UnaryFun{.func = Unary(std::asin), .keepsMeasure = false}},
    {"acos", Data::UnaryFun{.func = Unary(std::acos), .keepsMeasure = false}},
    {"atan", Data::UnaryFun{.func = Unary(std::atan), .keepsMeasure = false}},

    {"sinh", Data::UnaryFun{.func = Unary(std::sinh), .keepsMeasure = false}},
    {"cosh", Data::UnaryFun{.func = Unary(std::cosh), .keepsMeasure = false}},
    {"tanh", Data::UnaryFun{.func = Unary(std::tanh), .keepsMeasure = false}},

    {"asinh", Data::UnaryFun{.func = Unary(std::asinh), .keepsMeasure = false}},
    {"acosh", Data::UnaryFun{.func = Unary(std::acosh), .keepsMeasure = false}},
    {"atanh", Data::UnaryFun{.func = Unary(std::atanh), .keepsMeasure = false}},
};

const Data::Container<Data::BinaryFun> kBasicBinaryFuns{
    {"min", Data::BinaryFun{.func = Binary(std::fmin)}},
    {"max", Data::BinaryFun{.func = Binary(std::fmax)}},

    {"pow", Data::BinaryFun{.func = Binary(std::pow)}},
};

const Data::Container<Data::Value> kBasicConstants{
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