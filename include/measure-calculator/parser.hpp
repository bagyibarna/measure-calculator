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
    double value;
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

    void OnError(Error newError) {
        if (error) {
            return;
        }

        error = newError;
    }

    void ErrorCurrentToken(Error::Kind kind) {
        const auto current_end = lexer.total_string.size() - lexer.unanalyzed.size();
        const auto current_start = current_end - lexer.curr.str.size();
        OnError({kind, {current_start, current_end}});
    }

    template <class T>
    bool Expect() {
        if (std::holds_alternative<T>(lexer.curr.data)) {
            Step();

            return true;
        }

        if (std::holds_alternative<TokenData::Eof>(lexer.curr.data)) {
            OnError({
                .kind = Error::Kind::UnexpectedEof,
                .invalid_range = {lexer.total_string.size(), lexer.total_string.size()},
            });
            return false;
        }

        ErrorCurrentToken(Error::Kind::UnexpectedToken);
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

    std::optional<MeasuredValue> ParseUnaryOperator(const UnaryOp& op_spec) {
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
        std::optional<MeasuredValue> result;
        if (auto* value = std::get_if<TokenData::Value>(&lexer.curr.data)) {
            result = MeasuredValue{.value = *value};
            Step();
            return result;
        }

        if (auto* constant = std::get_if<TokenData::Constant>(&lexer.curr.data)) {
            result = MeasuredValue{.value = **constant};
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

        if (auto* op = std::get_if<TokenData::Operator>(&lexer.curr.data)) {
            if ((*op)->unary) {
                return ParseUnaryOperator(*(*op)->unary);
            }
        }

        if (auto* unary_fun = std::get_if<TokenData::UnaryFun>(&lexer.curr.data)) {
            const auto& fun_spec = **unary_fun;
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
            const auto& fun_spec = **binary_fun;
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
                    OnError({
                        .kind = Error::Kind::MeasureMismatch,
                        .invalid_range = {param_start_pos, param_end_pos},
                    });
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

        ErrorCurrentToken(Error::Kind::ValueExpected);
        return std::nullopt;
    }

    std::optional<MeasuredValue> ParseValueWithMeasure() {
        auto standalone_value = ParseStandaloneValue();
        if (!standalone_value) {
            return std::nullopt;
        }

        if (auto* measure = std::get_if<TokenData::Measure>(&lexer.curr.data)) {
            const auto measure_end = lexer.total_string.size() - lexer.unanalyzed.size();
            const auto measure_start = measure_end - lexer.curr.str.size();
            const auto& measure_data = **measure;
            if (standalone_value->measure) {
                if (standalone_value->measure->id != measure_data.id) {
                    OnError({
                        .kind = Error::Kind::MeasureMismatch,
                        .invalid_range = {measure_end - lexer.curr.str.size(), measure_end},
                        .secondary_invalid_range = {standalone_value->measure->source_location},
                    });

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
        auto expr_start_pos = lexer.total_string.size() - lexer.unanalyzed.size();
        auto root_value = ParseValueWithMeasure();
        if (!root_value) {
            return std::nullopt;
        }

        while (std::holds_alternative<TokenData::Operator>(lexer.curr.data)) {
            const auto& binary = std::get<TokenData::Operator>(lexer.curr.data)->binary;
            if (!binary) {
                break;
            }

            auto binary_end = lexer.total_string.size() - lexer.unanalyzed.size();
            auto binary_start = binary_end - lexer.curr.str.size();
            if (binary->precedence < parent_precedence) {
                break;
            }

            const auto right_prec =
            binary->left_associative ? binary->precedence + 1 : binary->precedence;

            Step();
            std::optional<MeasuredValue> right;

            if (spec.usePostfixShorthand && std::holds_alternative<TokenData::Eof>(lexer.curr.data)) {
                right = root_value;
            } else {
                right = ParseExpression(right_prec);
            }

            if (!right) {
                return std::nullopt;
            }

            std::optional<MeasureData> commonMeasure;
            auto measure = ResolveMeasure(root_value, right);
            if (std::holds_alternative<NoMeasure>(measure)) {
                auto curr_pos = lexer.total_string.size() - lexer.unanalyzed.size();
                OnError({
                    .kind = Error::Kind::MeasureMismatch,
                    .invalid_range = {expr_start_pos, curr_pos},
                });
                return std::nullopt;
            }

            if (auto specific = std::get_if<MeasureData>(&measure)) {
                commonMeasure = *specific;
            }

            auto result = binary->func(root_value->value, right->value);
            bool isNan = std::isnan(result);
            bool isInf = std::isinf(result);
            if (isNan || isInf) {
                OnError({
                    .kind = isNan ? Error::Kind::NotANumber : Error::Kind::InfiniteValue,
                    .invalid_range = {binary_start, binary_end}
                });
                return std::nullopt;
            }

            root_value = MeasuredValue{
                .measure = commonMeasure,
                .value = result,
            };
        }

        return root_value;
    }

    std::optional<MeasuredValue> Parse() {
        auto result = ParseExpression();
        if (!result) {
            return std::nullopt;
        }

        if (std::holds_alternative<TokenData::Eof>(lexer.curr.data)) {
            return result;
        }

        ErrorCurrentToken(Error::Kind::UnexpectedToken);
        return std::nullopt;
    }
};

} // namespace Detail

} // namespace Calc
