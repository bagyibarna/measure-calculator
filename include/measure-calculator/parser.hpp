#pragma once

#include "lexer.hpp"
#include "spec.hpp"

#include <optional>

namespace Calc {

struct MeasureData {
    std::pair<std::size_t, std::size_t> sourceLocation;
    std::size_t id;
};

struct MeasuredValue {
    std::optional<MeasureData> measure;
    double value;
};

namespace Detail {

struct Parser {
    Parser(const Spec& spec, std::string_view totalString)
        : spec(spec),
          lexer{
              .spec = spec,
              .totalString = totalString,
              .unanalyzed = totalString,
              .curr = {.str = "", .data = TokenData::Error{}},
          } {
        Step();
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
        const auto currentEnd = lexer.totalString.size() - lexer.unanalyzed.size();
        const auto currentStart = currentEnd - lexer.curr.str.size();
        OnError({kind, {currentStart, currentEnd}});
    }

    void Step() {
        if (auto newError = lexer.Step()) {
            OnError(*newError);
        }
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
                .invalidRange = {lexer.totalString.size(), lexer.totalString.size()},
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

    std::optional<MeasuredValue> ParseUnaryOperator(const UnaryOp& opSpec) {
        Step();
        auto inner = ParseExpression(opSpec.precedence);
        if (!inner) {
            return std::nullopt;
        }

        return MeasuredValue{
            .measure = opSpec.keepsMeasure ? inner->measure : std::nullopt,
            .value = opSpec.func(inner->value),
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

        if (auto* unaryFun = std::get_if<TokenData::UnaryFun>(&lexer.curr.data)) {
            const auto& funSpec = **unaryFun;
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
                .measure = funSpec.keepsMeasure ? inner->measure : std::nullopt,
                .value = funSpec.func(inner->value),
            };
        }

        if (auto* binaryFun = std::get_if<TokenData::BinaryFun>(&lexer.curr.data)) {
            const auto& funSpec = **binaryFun;
            Step();
            if (!Expect<TokenData::OpenParen>()) {
                return std::nullopt;
            }

            auto left = ParseExpression();
            if (!left) {
                return std::nullopt;
            }

            Expect<TokenData::Comma>();

            auto right = ParseExpression();
            if (!right) {
                return std::nullopt;
            }

            if (!Expect<TokenData::CloseParen>()) {
                return std::nullopt;
            }

            std::optional<MeasureData> commonMeasure;
            if (funSpec.keepsMeasure) {
                auto measure = ResolveMeasure(left, right);
                if (std::holds_alternative<NoMeasure>(measure)) {
                    OnError({
                        .kind = Error::Kind::MeasureMismatch,
                        .invalidRange = right->measure->sourceLocation,
                        .secondaryInvalidRange = left->measure->sourceLocation,
                    });
                    return std::nullopt;
                }

                if (auto specific = std::get_if<MeasureData>(&measure)) {
                    commonMeasure = *specific;
                }
            }

            return MeasuredValue{
                .measure = commonMeasure,
                .value = funSpec.func(left->value, right->value),
            };
        }

        ErrorCurrentToken(Error::Kind::ValueExpected);
        return std::nullopt;
    }

    std::optional<MeasuredValue> ParseValueWithMeasure() {
        auto standaloneValue = ParseStandaloneValue();
        if (!standaloneValue) {
            return std::nullopt;
        }

        if (auto* measure = std::get_if<TokenData::Measure>(&lexer.curr.data)) {
            const auto measure_end = lexer.totalString.size() - lexer.unanalyzed.size();
            const auto measure_start = measure_end - lexer.curr.str.size();
            const auto& measure_data = **measure;
            if (standaloneValue->measure) {
                if (standaloneValue->measure->id != measure_data.id) {
                    OnError({
                        .kind = Error::Kind::MeasureMismatch,
                        .invalidRange = {measure_end - lexer.curr.str.size(), measure_end},
                        .secondaryInvalidRange = {standaloneValue->measure->sourceLocation},
                    });

                    return std::nullopt;
                }
            } else {
                Step();
                standaloneValue->measure =
                    MeasureData{
                        .sourceLocation = {measure_start, measure_end},
                        .id = measure_data.id,
                    },
                standaloneValue->value *= measure_data.multiplier;
            }
        }

        return standaloneValue;
    }

    std::optional<MeasuredValue> ParseExpression(std::size_t parentPrecedence = 0) {
        auto rootValue = ParseValueWithMeasure();
        if (!rootValue) {
            return std::nullopt;
        }

        while (std::holds_alternative<TokenData::Operator>(lexer.curr.data)) {
            const auto& binary = std::get<TokenData::Operator>(lexer.curr.data)->binary;
            if (!binary) {
                break;
            }

            auto binaryEnd = lexer.totalString.size() - lexer.unanalyzed.size();
            auto binaryStart = binaryEnd - lexer.curr.str.size();
            if (binary->precedence < parentPrecedence) {
                break;
            }

            const auto rightPrec =
                binary->leftAssociative ? binary->precedence + 1 : binary->precedence;

            Step();
            std::optional<MeasuredValue> right;

            if (spec.usePostfixShorthand &&
                std::holds_alternative<TokenData::Eof>(lexer.curr.data)) {
                right = rootValue;
            } else {
                right = ParseExpression(rightPrec);
            }

            if (!right) {
                return std::nullopt;
            }

            std::optional<MeasureData> commonMeasure;
            auto measure = ResolveMeasure(rootValue, right);
            if (std::holds_alternative<NoMeasure>(measure)) {
                OnError({
                    .kind = Error::Kind::MeasureMismatch,
                    .invalidRange = right->measure->sourceLocation,
                    .secondaryInvalidRange = rootValue->measure->sourceLocation,
                });
                return std::nullopt;
            }

            if (auto specific = std::get_if<MeasureData>(&measure)) {
                commonMeasure = *specific;
            }

            auto result = binary->func(rootValue->value, right->value);
            bool isNan = std::isnan(result);
            bool isInf = std::isinf(result);
            if (isNan || isInf) {
                OnError({.kind = isNan ? Error::Kind::NotANumber : Error::Kind::InfiniteValue,
                         .invalidRange = {binaryStart, binaryEnd}});
                return std::nullopt;
            }

            rootValue = MeasuredValue{
                .measure = commonMeasure,
                .value = result,
            };
        }

        return rootValue;
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
