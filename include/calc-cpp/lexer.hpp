#pragma once

#include "char-classification.hpp"
#include "error.hpp"
#include "spec.hpp"
#include "token.hpp"

#include <optional>
#include <string_view>

namespace Calc {

namespace Detail {

struct Lexer {
    const Spec& spec;

    std::string_view total_string;
    std::string_view unanalyzed;

    Token curr;

    void EatWhitespace() {
        while (!unanalyzed.empty() && std::isspace(unanalyzed.front())) {
            unanalyzed.remove_prefix(1);
        }
    }

    std::optional<Error> TokenizeValue() {
        char* end;
        auto result = std::strtod(unanalyzed.data(), &end);
        if (result == HUGE_VAL || result == -HUGE_VAL) {
            const auto first_invalid = total_string.size() - unanalyzed.size();
            return Error{
                .invalid_range = {first_invalid, end - unanalyzed.data()},
                .kind = result == HUGE_VAL ? Error::Kind::ConstantTooLarge
                                           : Error::Kind::ConstantTooSmall,
            };
        }

        curr = Token{
            .str = std::string_view(unanalyzed.data(), end),
            .data = TokenData::Value{result},
        };
        unanalyzed.remove_prefix(end - unanalyzed.data());

        return std::nullopt;
    }

    template <class... Containers>
    bool TokenizeLargestMatchingTokenFrom(const std::vector<std::size_t>& lengths,
                                          const Containers&... containers) {
        std::string_view last_str;
        std::optional<TokenData::Any> last_data;

        for (auto len : lengths) {
            if (len > unanalyzed.size()) {
                break;
            }

            std::string_view new_trial(unanalyzed.data(), len);
            auto check_container = [&](const auto& container) {
                if (auto found = container.find(new_trial); found != container.end()) {
                    last_data.emplace(found);
                    last_str = new_trial;
                    return true;
                } else {
                    return false;
                }
            };

            (check_container(containers) || ...);
        }

        if (!last_data) {
            return false;
        }

        curr = {
            .str = last_str,
            .data = *last_data,
        };

        unanalyzed.remove_prefix(last_str.size());

        return true;
    }

    std::nullopt_t TokenizeSingleChar(TokenData::Any data) {
        curr = {
            .str = unanalyzed.substr(0, 1),
            .data = data,
        };
        unanalyzed.remove_prefix(1);

        return std::nullopt;
    }

    Error ErrorFromUnanalyzed(bool (*take_while)(char), Error::Kind kind) {
        const auto last_invalid_it = std::find_if(unanalyzed.begin(), unanalyzed.end(),
                                                  [take_while](char c) { return !take_while(c); });

        const auto first_invalid = total_string.size() - unanalyzed.size();
        const auto invalid_len = last_invalid_it - unanalyzed.begin();

        curr.data = TokenData::Error{};
        return Error{
            .invalid_range = {first_invalid, first_invalid + invalid_len},
            .kind = kind,
        };
    }

    std::optional<Error> Step() {
        EatWhitespace();

        if (unanalyzed.empty()) {
            curr.data = TokenData::Eof{};
            curr.str = "";
            return std::nullopt;
        }

        switch (unanalyzed.front()) {
            case '(': return TokenizeSingleChar(TokenData::OpenParen{});
            case ')': return TokenizeSingleChar(TokenData::CloseParen{});
            case ',': return TokenizeSingleChar(TokenData::Comma{});
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': return TokenizeValue();
            default: break;
        }

        if (IsOperatorChar(unanalyzed.front())) {
            bool success = TokenizeLargestMatchingTokenFrom(spec.op_lengths, spec.unary_ops,
                                                            spec.binary_ops, spec.ambigous_ops);
            if (!success) {
                curr.data = TokenData::Error{};
                return ErrorFromUnanalyzed(IsOperatorChar, Error::Kind::UnknownOperator);
            }

            return std::nullopt;
        }

        if (IsIdentifierStartChar(unanalyzed.front())) {
            bool success =
                TokenizeLargestMatchingTokenFrom(spec.ident_lengths, spec.unary_funs,
                                                 spec.binary_funs, spec.constants, spec.measures);
            if (!success) {
                curr.data = TokenData::Error{};
                return ErrorFromUnanalyzed(IsIdentifierChar, Error::Kind::UnknownIdentifier);
            }

            return std::nullopt;
        }

        // unknown character
        curr.data = TokenData::Error{};
        const auto first_invalid = total_string.size() - unanalyzed.size();
        return Error{
            .invalid_range = {first_invalid, first_invalid + 1},
            .kind = Error::Kind::UnknownChar,
        };
    }
};

} // namespace Detail

} // namespace Calc