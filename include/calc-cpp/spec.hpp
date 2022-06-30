#pragma once

#include "char-classification.hpp"
#include "data.hpp"
#include "token.hpp"

#include <algorithm>
#include <functional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace Calc {

// TODO: move into Spec
struct MeasureSpec {
    std::string_view name;
    std::vector<std::pair<std::string_view, double>> units;
};

namespace Detail {

struct Lexer;
struct Parser;

} // namespace Detail

struct Spec {
    Spec() = default;
    Spec(Spec&&) = default;
    Spec(const Spec&) = delete;

    Spec& operator=(Spec&&) = default;

  private:
    friend struct SpecBuilder;
    friend struct Detail::Lexer;
    friend struct Detail::Parser;

    Data::Container<Data::Operator> op_specs;

    Data::Container<Data::Identifier> identifier_specs;

    std::vector<std::string_view> measure_names;
};

struct SpecBuilder {
    Data::Container<Data::UnaryOp> unary_ops;
    Data::Container<Data::BinaryOp> binary_ops;

    Data::Container<Data::UnaryFun> unary_funs;
    Data::Container<Data::BinaryFun> binary_funs;
    Data::Container<Data::Value> constants;

    std::vector<MeasureSpec> measures;

    enum class Error {
        InvalidOperatorName,
        InvalidIdentifierName,
        DuplicateOperator,
        DuplicateIdentifier
    };

    static bool ValidOp(std::string_view name) {
        return !name.empty() && std::all_of(name.begin(), name.end(), Detail::IsOperatorChar);
    }

    static bool ValidIdentifier(std::string_view name) {
        return !name.empty() && Detail::IsIdentifierStartChar(name.front()) &&
               std::all_of(std::next(name.begin()), name.end(), Detail::IsIdentifierChar);
    }

    // TODO: remove duplication
    std::variant<Spec, Error> Build() && {
        Spec result;

        for (auto& [name, spec] : unary_ops) {
            if (!ValidOp(name)) {
                return Error::InvalidOperatorName;
            }

            auto& op = result.op_specs[name];
            if (op.unary) {
                return Error::DuplicateOperator;
            }
            op.unary = std::move(spec);
        }

        for (auto& [name, spec] : binary_ops) {
            if (!ValidOp(name)) {
                return Error::InvalidOperatorName;
            }

            auto& op = result.op_specs[name];
            if (op.binary) {
                return Error::DuplicateOperator;
            }
            op.binary = std::move(spec);
        }

        for (auto&& [name, units] : measures) {
            result.measure_names.push_back(name);
            for (auto& [unit_name, multiplier] : units) {
                if (!ValidIdentifier(unit_name)) {
                    return Error::InvalidOperatorName;
                }

                auto inserted = result.identifier_specs.emplace(unit_name, Data::Measure{
                                                       .id = result.measure_names.size(),
                                                       .multiplier = multiplier,
                                                   }).second;
                if (!inserted) {
                    return Error::DuplicateIdentifier;
                }
            }
        }

        auto add_identifiers = [&result](auto& source) {
            for (auto& [name, spec] : source) {
                if (!ValidIdentifier(name)) {
                    return Error::InvalidOperatorName;
                }

                if (!result.identifier_specs.emplace(name, std::move(spec)).second) {
                    return Error::DuplicateIdentifier;
                }
            }
        };

        add_identifiers(unary_funs);
        add_identifiers(binary_funs);
        add_identifiers(constants);

        return result;
    }
};

} // namespace Calc
