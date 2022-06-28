#pragma once

#include "parser.hpp"

#include <string_view>
#include <variant>

namespace Calc {

std::variant<double, Error> Evaluate(const Spec& spec, std::string_view str) {
    Detail::Parser parser(spec, str);

    if (auto measuredValue = parser.ParseExpression()) {
        return measuredValue->value;
    }

    return parser.error.value();
}

} // namespace Calc