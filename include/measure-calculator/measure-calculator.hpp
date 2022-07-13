#pragma once

#include "interpreter.hpp"

#include <string_view>
#include <variant>

namespace Calc {

std::variant<double, Error> Evaluate(const Spec& spec, std::string_view str) {
    Detail::Interpreter parser(spec, str);

    if (auto measuredValue = parser.Parse()) {
        return measuredValue->value;
    }

    return parser.error.value();
}

} // namespace Calc
