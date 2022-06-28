#pragma once

#include "data.hpp"

#include <variant>

namespace Calc {

namespace Detail {

namespace TokenData {

using UnaryOp = Data::Container<Data::UnaryOp>::const_iterator;
using BinaryOp = Data::Container<Data::BinaryOp>::const_iterator;
using AmbigousOp = Data::Container<std::pair<Data::UnaryOp, Data::BinaryOp>>::const_iterator;

using UnaryFun = Data::Container<Data::UnaryFun>::const_iterator;
using BinaryFun = Data::Container<Data::BinaryFun>::const_iterator;

using Value = Data::Value;
using Constant = Data::Container<Data::Value>::const_iterator;

using Measure = Data::Container<Data::Measure>::const_iterator;

struct OpenParen {};
struct CloseParen {};
struct Comma {};
struct Error {};
struct Eof {};

using Any = std::variant<UnaryOp, BinaryOp, AmbigousOp, Measure, UnaryFun, BinaryFun, Constant,
                         Value, OpenParen, CloseParen, Comma, Error, Eof>;

} // namespace TokenData

struct Token {
    std::string_view str;
    TokenData::Any data;
};

} // namespace Detail

} // namespace Calc