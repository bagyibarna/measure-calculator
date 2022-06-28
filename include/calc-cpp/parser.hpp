#pragma once

#include "lexer.hpp"
#include "spec.hpp"

#include <optional>

namespace Calc {

struct MeasureData {
    std::pair<std::size_t, std::size_t> source_location;
    std::size_t id;
};

struct MeasuredValue {
    std::optional<MeasureData> measure;
    Data::Value value;
};

namespace Detail {

struct Parser {
    Parser(const Spec& spec, std::string_view total_string)
        : spec(spec),
          lexer{
              .spec = spec,
              .total_string = total_string,
              .unanalyzed = total_string,
              .curr = {.str = "", .data = TokenData::Error{}},
          } {
        Step();
    }

    void Step() {
        if (auto new_error = lexer.Step()) {
            error = *new_error;
        }
    }

    const Spec& spec;
    Lexer lexer;

    std::optional<Error> error;

    template <class T>
    bool Expect() {
        if (error) {
            return false;
        }

        if (std::holds_alternative<T>(lexer.curr.data)) {
            Step();

            return true;
        }

        if (std::holds_alternative<TokenData::Eof>(lexer.curr.data)) {
            error = Error{
                .invalid_range = {lexer.total_string.size(), lexer.total_string.size()},
                .kind = Error::Kind::UnexpectedEof,
            };
            return false;
        }

        const auto last_invalid = lexer.total_string.size() - lexer.unanalyzed.size();
        error = Error{
            .invalid_range = {last_invalid - lexer.curr.str.size(), last_invalid},
            .kind = Error::Kind::UnexpectedToken,
        };
        return false;
    }

    struct AnyMeasure {};
    struct NoMeasure {};

    std::variant<MeasureData, AnyMeasure, NoMeasure>
    ResolveMeasure(const std::optional<MeasuredValue>& left,
                   const std::optional<MeasuredValue>& right) {
        if (left->measure || right->measure) {
            if (left->measure && right->measure) {
                if (left->measure->id != right->measure->id) {
                    return NoMeasure{};
                }

                return *right->measure;
            } else if (left->measure) {
                return *left->measure;
            } else {
                return *right->measure;
            }
        }

        return AnyMeasure{};
    }

    std::optional<MeasuredValue> ParseUnaryOperator(const Data::UnaryOp& op_spec) {
        if (error) {
            return std::nullopt;
        }

        Step();
        auto inner = ParseExpression(op_spec.precedence);
        if (!inner) {
            return std::nullopt;
        }

        return MeasuredValue{
            .measure = op_spec.keepsMeasure ? inner->measure : std::nullopt,
            .value = op_spec.func(inner->value),
        };
    }

    std::optional<MeasuredValue> ParseStandaloneValue() {
        if (error) {
            return std::nullopt;
        }

        std::optional<MeasuredValue> result;
        if (auto* value = std::get_if<TokenData::Value>(&lexer.curr.data)) {
            result = MeasuredValue{.value = *value};
            Step();
            return result;
        }

        if (auto* constant = std::get_if<TokenData::Constant>(&lexer.curr.data)) {
            result = MeasuredValue{.value = (*constant)->second};
            Step();
            return result;
        }

        if (std::holds_alternative<TokenData::OpenParen>(lexer.curr.data)) {
            Step();
            auto inner = ParseExpression();
            if (!inner || !Expect<TokenData::CloseParen>()) {
                return std::nullopt;
            }

            return inner;
        }

        if (auto* unary_op = std::get_if<TokenData::UnaryOp>(&lexer.curr.data)) {
            return ParseUnaryOperator((*unary_op)->second);
        }

        if (auto* ambigous_op = std::get_if<TokenData::AmbigousOp>(&lexer.curr.data)) {
            return ParseUnaryOperator((*ambigous_op)->second.first);
        }

        if (auto* unary_fun = std::get_if<TokenData::UnaryFun>(&lexer.curr.data)) {
            const auto& fun_spec = (*unary_fun)->second;
            Step();
            if (!Expect<TokenData::OpenParen>()) {
                return std::nullopt;
            }

            auto inner = ParseExpression();
            if (!inner) {
                return std::nullopt;
            }

            if (!Expect<TokenData::CloseParen>()) {
                return std::nullopt;
            }

            return MeasuredValue{
                .measure = fun_spec.keepsMeasure ? inner->measure : std::nullopt,
                .value = fun_spec.func(inner->value),
            };
        }

        if (auto* binary_fun = std::get_if<TokenData::BinaryFun>(&lexer.curr.data)) {
            const auto& fun_spec = (*binary_fun)->second;
            Step();
            if (!Expect<TokenData::OpenParen>()) {
                return std::nullopt;
            }

            auto param_start_pos = lexer.total_string.size() - lexer.unanalyzed.size();
            auto left = ParseExpression();
            if (!left) {
                return std::nullopt;
            }

            Expect<TokenData::Comma>();

            auto right = ParseExpression();
            if (!right) {
                return std::nullopt;
            }

            auto param_end_pos = lexer.total_string.size() - lexer.unanalyzed.size();
            if (!Expect<TokenData::CloseParen>()) {
                return std::nullopt;
            }

            std::optional<MeasureData> commonMeasure;
            if (fun_spec.keepsMeasure) {
                auto measure = ResolveMeasure(left, right);
                if (std::holds_alternative<NoMeasure>(measure)) {
                    error = Error{
                        .invalid_range = {param_start_pos, param_end_pos},
                        .kind = Error::Kind::MeasureMismatch,
                    };
                    return std::nullopt;
                }

                if (auto specific = std::get_if<MeasureData>(&measure)) {
                    commonMeasure = *specific;
                }
            }

            return MeasuredValue{
                .measure = commonMeasure,
                .value = fun_spec.func(left->value, right->value),
            };
        }

        const auto last_invalid = lexer.total_string.size() - lexer.unanalyzed.size();
        error = {
            .invalid_range = {last_invalid - lexer.curr.str.size(), last_invalid},
            .kind = Error::Kind::ValueExpected,
        };
        return std::nullopt;
    }

    std::optional<MeasuredValue> ParseValueWithMeasure() {
        if (error) {
            return std::nullopt;
        }

        auto standalone_value = ParseStandaloneValue();
        if (!standalone_value) {
            return std::nullopt;
        }

        if (auto* measure = std::get_if<TokenData::Measure>(&lexer.curr.data)) {
            const auto measure_end = lexer.total_string.size() - lexer.unanalyzed.size();
            const auto measure_start = measure_end - lexer.curr.str.size();
            const auto& measure_data = (*measure)->second;
            if (standalone_value->measure) {
                if (standalone_value->measure->id != measure_data.id) {
                    error = Error{
                        .invalid_range = {measure_end - lexer.curr.str.size(), measure_end},
                        .secondary_invalid_range = {standalone_value->measure->source_location},
                        .kind = Error::Kind::MeasureMismatch,
                    };
                    return std::nullopt;
                }
            } else {
                Step();
                standalone_value->measure =
                    MeasureData{
                        .source_location = {measure_start, measure_end},
                        .id = measure_data.id,
                    },
                standalone_value->value *= measure_data.multiplier;
            }
        }

        return standalone_value;
    }

    std::optional<MeasuredValue> ParseExpression(std::size_t parent_precedence = 0) {
        if (error) {
            return std::nullopt;
        }

        auto expr_start_pos = lexer.total_string.size() - lexer.unanalyzed.size();
        auto root_value = ParseValueWithMeasure();
        if (!root_value) {
            return std::nullopt;
        }

        while (std::holds_alternative<TokenData::BinaryOp>(lexer.curr.data) ||
               std::holds_alternative<TokenData::AmbigousOp>(lexer.curr.data)) {
            const auto& op_spec =
                std::holds_alternative<TokenData::BinaryOp>(lexer.curr.data)
                    ? std::get<TokenData::BinaryOp>(lexer.curr.data)->second
                    : std::get<TokenData::AmbigousOp>(lexer.curr.data)->second.second;

            if (op_spec.precedence < parent_precedence) {
                break;
            }

            const auto right_prec =
                op_spec.left_associative ? op_spec.precedence + 1 : op_spec.precedence;

            Step();
            auto right_expr = ParseExpression(right_prec);
            if (!right_expr) {
                return std::nullopt;
            }

            std::optional<MeasureData> commonMeasure;
            auto measure = ResolveMeasure(root_value, right_expr);
            if (std::holds_alternative<NoMeasure>(measure)) {
                auto curr_pos = lexer.total_string.size() - lexer.unanalyzed.size();
                error = Error{
                    .invalid_range = {expr_start_pos, curr_pos},
                    .kind = Error::Kind::MeasureMismatch,
                };
                return std::nullopt;
            }

            if (auto specific = std::get_if<MeasureData>(&measure)) {
                commonMeasure = *specific;
            }

            root_value = MeasuredValue{
                .measure = commonMeasure,
                .value = op_spec.func(root_value->value, right_expr->value),
            };
        }

        return root_value;
    }
};

} // namespace Detail

} // namespace Calc