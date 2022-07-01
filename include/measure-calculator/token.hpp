#pragma once

#include "data.hpp"

#include <variant>

namespace Calc {

namespace Detail {

namespace TokenData {

using Operator = const Operator*;

using UnaryFun = const UnaryFun*;
using BinaryFun = const BinaryFun*;

using Value = double;

using Constant = const Constant*;

using Measure = const Measure*;

struct OpenParen {};
struct CloseParen {};
struct Comma {};
struct Error {};
struct Eof {};

using Any = std::variant<Operator, Measure, UnaryFun, BinaryFun, Constant,
                         Value, OpenParen, CloseParen, Comma, Error, Eof>;

} // namespace TokenData

struct Token {
    std::string_view str;
    TokenData::Any data;
};

} // namespace Detail

} // namespace Calc
