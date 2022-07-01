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

    std::unordered_map<std::string_view, Operator> opSpecs;

    std::unordered_map<std::string_view, Identifier> identifierSpecs;

    std::vector<std::string_view> measureNames;

    bool usePostfixShorthand;
};

template <class T>
using SpecFor = std::vector<std::pair<std::string_view, T>>;

struct SpecBuilder {
    SpecFor<UnaryOp> unaryOps;
    SpecFor<BinaryOp> binaryOps;

    SpecFor<UnaryFun> unaryFuns;
    SpecFor<BinaryFun> binaryFuns;
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

        for (auto& [name, spec] : unaryOps) {
            if (!ValidOp(name)) {
                return Error::InvalidOperatorName;
            }

            if (!result.opSpecs.emplace(name, Operator{.unary = std::move(spec)}).second) {
                return Error::DuplicateOperator;
            }
        }

        for (auto& [name, spec] : binaryOps) {
            if (!ValidOp(name)) {
                return Error::InvalidOperatorName;
            }

            auto& op = result.opSpecs[name];
            if (op.binary) {
                return Error::DuplicateOperator;
            }
            op.binary = std::move(spec);
        }

        for (auto&& [name, units] : measures) {
            result.measureNames.push_back(name);
            for (auto& [unitName, multiplier] : units) {
                if (!ValidIdentifier(unitName)) {
                    return Error::InvalidIdentifierName;
                }

                if (multiplier < 0.) {
                    return Error::NegativeMultiplier;
                }

                if (multiplier < std::numeric_limits<double>::epsilon()) {
                    return Error::ZeroMultiplier;
                }

                auto inserted = result.identifierSpecs
                                    .emplace(unitName,
                                             Measure{
                                                 .id = result.measureNames.size(),
                                                 .multiplier = multiplier,
                                             })
                                    .second;
                if (!inserted) {
                    return Error::DuplicateIdentifier;
                }
            }
        }

        auto addIdentifiers = [&result](auto& source) -> std::optional<Error> {
            for (auto& [name, spec] : source) {
                if (!ValidIdentifier(name)) {
                    return Error::InvalidIdentifierName;
                }

                if (!result.identifierSpecs.emplace(name, std::move(spec)).second) {
                    return Error::DuplicateIdentifier;
                }
            }

            return std::nullopt;
        };

        if (auto err = addIdentifiers(unaryFuns)) {
            return *err;
        }
        if (auto err = addIdentifiers(binaryFuns)) {
            return *err;
        }
        if (auto err = addIdentifiers(constants)) {
            return *err;
        }

        result.usePostfixShorthand = usePostfixShorthand;

        return result;
    }
};

template <class FirstContainer, class... Containers>
auto SpecUnion(FirstContainer firstContainer, Containers... containers) {
    const auto unionOne = [&](auto& container) {
        firstContainer.insert(firstContainer.end(), std::move_iterator(container.begin()),
                              std::move_iterator(container.end()));
        return true;
    };

    (unionOne(containers) && ... && true);

    return firstContainer;
}

} // namespace Calc
