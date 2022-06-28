#pragma once

#include <functional>
#include <string_view>
#include <utility>

namespace Calc {

namespace Data {

struct UnaryOp {
    std::function<double(double)> func;

    bool keepsMeasure = true;
    std::size_t precedence;
};

struct BinaryOp {
    std::function<double(double, double)> func;

    bool left_associative = true;
    bool keepsMeasure = true;
    std::size_t precedence;
};

using AmbigousOp = std::pair<UnaryOp, BinaryOp>;

template <class T>
struct Fun {
    std::function<T> func;

    bool keepsMeasure = true;
};

using UnaryFun = Fun<double(double)>;
using BinaryFun = Fun<double(double, double)>;

using Value = double;

struct Measure {
    std::size_t id = 0;
    double multiplier;
};

template <class T>
using Container = std::unordered_map<std::string_view, T>;

} // namespace Data

template <class T>
Data::Container<T> operator+(Data::Container<T> a, const Data::Container<T>& b) {
    a.insert(b.begin(), b.end());
    return std::move(a);
}

} // namespace Calc