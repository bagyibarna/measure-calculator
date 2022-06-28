#pragma once

#include "char-classification.hpp"
#include "data.hpp"

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

    Data::Container<Data::UnaryOp> unary_ops;
    Data::Container<Data::BinaryOp> binary_ops;
    Data::Container<Data::AmbigousOp> ambigous_ops;

    std::vector<std::size_t> op_lengths;

    Data::Container<Data::UnaryFun> unary_funs;
    Data::Container<Data::BinaryFun> binary_funs;
    Data::Container<Data::Value> constants;
    Data::Container<Data::Measure> measures;

    std::vector<std::size_t> ident_lengths;

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

        std::unordered_set<std::string_view> deduplicator;
        std::set<std::size_t> size_set;

        for (auto& [name, spec] : unary_ops) {
            if (!ValidOp(name)) {
                return Error::InvalidOperatorName;
            }

            if (!deduplicator.insert(name).second) {
                return Error::DuplicateOperator;
            }

            size_set.insert(name.size());
            if (auto found = binary_ops.find(name); found != binary_ops.end()) {
                result.ambigous_ops.emplace(
                    name, std::make_pair(std::move(spec), std::move(found->second)));
                binary_ops.erase(found);
            } else {
                result.unary_ops.emplace(name, std::move(spec));
            }
        }

        deduplicator.clear();
        result.binary_ops = std::move(binary_ops);
        for (const auto& [name, _] : result.binary_ops) {
            if (!ValidOp(name)) {
                return Error::InvalidOperatorName;
            }

            if (!deduplicator.insert(name).second) {
                return Error::DuplicateOperator;
            }
            size_set.insert(name.size());
        }
        result.op_lengths = {size_set.begin(), size_set.end()};

        deduplicator.clear();
        size_set.clear();
        for (auto&& [name, units] : measures) {
            result.measure_names.push_back(name);
            for (auto& [unit_name, multiplier] : units) {
                if (!ValidIdentifier(unit_name)) {
                    return Error::InvalidOperatorName;
                }

                if (!deduplicator.insert(unit_name).second) {
                    return Error::DuplicateIdentifier;
                }

                result.measures.emplace(unit_name, Data::Measure{
                                                       .id = result.measure_names.size(),
                                                       .multiplier = multiplier,
                                                   });

                size_set.insert(unit_name.size());
            }
        }

        result.unary_funs = std::move(unary_funs);
        for (const auto& [name, _] : result.unary_funs) {
            if (!ValidIdentifier(name)) {
                return Error::InvalidOperatorName;
            }

            if (!deduplicator.insert(name).second) {
                return Error::DuplicateIdentifier;
            }

            size_set.insert(name.size());
        }
        result.binary_funs = std::move(binary_funs);
        for (const auto& [name, _] : result.binary_funs) {
            if (!ValidIdentifier(name)) {
                return Error::InvalidOperatorName;
            }

            if (!deduplicator.insert(name).second) {
                return Error::DuplicateIdentifier;
            }

            size_set.insert(name.size());
        }
        result.constants = std::move(constants);
        for (const auto& [name, _] : result.constants) {
            if (!ValidIdentifier(name)) {
                return Error::InvalidOperatorName;
            }

            if (!deduplicator.insert(name).second) {
                return Error::DuplicateIdentifier;
            }

            size_set.insert(name.size());
        }
        result.ident_lengths = {size_set.begin(), size_set.end()};

        return result;
    }
};

} // namespace Calc