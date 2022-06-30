#pragma once

#include "data.hpp"

#include <variant>

namespace Calc {

namespace Detail {

namespace TokenData {

using Operator = const Data::Operator*;

using UnaryFun = const Data::UnaryFun*;
using BinaryFun = const Data::BinaryFun*;

using Value = Data::Value;

using Constant = const Data::Value*;

using Measure = const Data::Measure*;

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
