#pragma once

#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <optional>

namespace Calc {

struct UnaryOp {
    std::function<double(double)> func;

    bool keepsMeasure = true;
    std::size_t precedence;
};

struct BinaryOp {
    std::function<double(double, double)> func;

    bool leftAssociative = true;
    bool keepsMeasure = true;
    std::size_t precedence;
};

struct Operator {
    std::optional<UnaryOp> unary;
    std::optional<BinaryOp> binary;
};

template <class T>
struct Fun {
    std::function<T> func;

    bool keepsMeasure = true;
};

using UnaryFun = Fun<double(double)>;
using BinaryFun = Fun<double(double, double)>;

using Constant = double;

struct Measure {
    std::size_t id = 0;
    double multiplier;
};

using Identifier = std::variant<UnaryFun, BinaryFun, Constant, Measure>;

} // namespace Calc
