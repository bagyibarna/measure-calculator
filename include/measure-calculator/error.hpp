#pragma once

#include <compare>
#include <ostream>
#include <utility>

namespace Calc {

struct Error {
    enum class Kind {
        UnclosedParen,
        ConstantTooLarge,
        ConstantTooSmall,

        UnknownIdentifier,
        UnknownOperator,
        UnknownChar,

        UnexpectedEof,
        UnexpectedToken,

        ValueExpected,

        NotANumber,
        InfiniteValue,

        MeasureMismatch,
    };

    Kind kind;

    std::pair<std::size_t, std::size_t> invalid_range;
    std::pair<std::size_t, std::size_t> secondary_invalid_range = {0, 0};

    std::strong_ordering operator<=>(const Error& other) const = default;
};

inline std::ostream& operator<<(std::ostream& os, const Error& error) {
    switch (error.kind) {
        case Error::Kind::UnclosedParen: os << "UnclosedParen"; break;
        case Error::Kind::ConstantTooLarge: os << "ConstantTooLarge"; break;
        case Error::Kind::ConstantTooSmall: os << "ConstantTooSmall"; break;
        case Error::Kind::UnknownIdentifier: os << "UnknownIdentifier"; break;
        case Error::Kind::UnknownOperator: os << "UnknownOperator"; break;
        case Error::Kind::UnknownChar: os << "UnknownChar"; break;
        case Error::Kind::UnexpectedEof: os << "UnexpectedEof"; break;
        case Error::Kind::UnexpectedToken: os << "UnexpectedToken"; break;
        case Error::Kind::ValueExpected: os << "ValueExpected"; break;
        case Error::Kind::MeasureMismatch: os << "MeasureMismatch"; break;
        case Error::Kind::NotANumber: os << "MeasureMismatch"; break;
    }

    os << "{" << error.invalid_range.first << ", " << error.invalid_range.second << "}";
    os << " {" << error.secondary_invalid_range.first << ", "
       << error.secondary_invalid_range.second << "}";

    return os;
}

} // namespace Calc
