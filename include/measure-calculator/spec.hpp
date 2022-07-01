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

    std::unordered_map<std::string_view, Operator> op_specs;

    std::unordered_map<std::string_view, Identifier> identifier_specs;

    std::vector<std::string_view> measure_names;

    bool usePostfixShorthand;
};

template<class T>
using SpecFor = std::vector<std::pair<std::string_view, T>>;

struct SpecBuilder {
    SpecFor<UnaryOp> unary_ops;
    SpecFor<BinaryOp> binary_ops;

    SpecFor<UnaryFun> unary_funs;
    SpecFor<BinaryFun> binary_funs;
    SpecFor<Constant> constants;

    std::vector<MeasureSpec> measures;

    bool usePostfixShorthand = false;

    enum class Error {
        InvalidOperatorName,
        InvalidIdentifierName,
        DuplicateOperator,
        DuplicateIdentifier,
        ZeroMultiplier,
        NegativeMultiplier
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

            if (!result.op_specs.emplace(name, Operator{.unary = std::move(spec)}).second) {
                return Error::DuplicateOperator;
            }
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

                if (multiplier < 0.) {
                    return Error::NegativeMultiplier;
                }

                if (multiplier < std::numeric_limits<double>::epsilon()) {
                    return Error::ZeroMultiplier;
                }

                auto inserted = result.identifier_specs.emplace(unit_name, Measure{
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

        result.usePostfixShorthand = usePostfixShorthand;

        return result;
    }
};

} // namespace Calc
