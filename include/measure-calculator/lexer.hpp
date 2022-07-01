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

    std::string_view totalString;
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
            const auto firstInvalid = totalString.size() - unanalyzed.size();
            return Error{
                .kind = result == HUGE_VAL ? Error::Kind::ConstantTooLarge
                                           : Error::Kind::ConstantTooSmall,
                .invalidRange = {firstInvalid, end - unanalyzed.data()},
            };
        }

        curr = Token{
            .str = std::string_view(unanalyzed.data(), end - unanalyzed.data()),
            .data = TokenData::Value{result},
        };
        unanalyzed.remove_prefix(end - unanalyzed.data());

        return std::nullopt;
    }

    template <class Container>
    std::variant<Error, const typename Container::mapped_type*>
    TokenizeFromSpec(Container& lookupSource, bool (*take_while)(char), Error::Kind kind) {
        const auto begin = unanalyzed.begin();
        const auto end = std::find_if(unanalyzed.begin(), unanalyzed.end(),
                                      [take_while](char c) { return !take_while(c); });

        for (auto size = end - begin; size > 0; --size) {
            std::string_view atom(&*begin, size);
            if (auto found = lookupSource.find(atom); found != lookupSource.end()) {
                curr.str = atom;
                unanalyzed.remove_prefix(size);
                return &found->second;
            }
        }

        const auto startIndex = totalString.size() - unanalyzed.size();
        curr.data = TokenData::Error{};
        return Error{
            .kind = kind,
            .invalidRange = {startIndex, startIndex + (end - begin)},
        };
    }

    std::nullopt_t TokenizeSingleChar(TokenData::Any data) {
        curr = {
            .str = unanalyzed.substr(0, 1),
            .data = data,
        };
        unanalyzed.remove_prefix(1);

        return std::nullopt;
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
            auto result =
                TokenizeFromSpec(spec.opSpecs, IsOperatorChar, Error::Kind::UnknownOperator);
            if (auto* error = std::get_if<Error>(&result)) {
                return *error;
            }
            curr.data = std::get<const Operator*>(result);
            return std::nullopt;
        }

        if (IsIdentifierStartChar(unanalyzed.front())) {
            auto result = TokenizeFromSpec(spec.identifierSpecs, IsIdentifierChar,
                                           Error::Kind::UnknownIdentifier);
            if (auto* error = std::get_if<Error>(&result)) {
                return *error;
            }
            std::visit([this](const auto& data) { curr.data = &data; },
                       *std::get<const Identifier*>(result));
            return std::nullopt;
        }

        // unknown character
        curr.data = TokenData::Error{};
        const auto firstInvalid = totalString.size() - unanalyzed.size();
        return Error{
            .kind = Error::Kind::UnknownChar,
            .invalidRange = {firstInvalid, firstInvalid + 1},
        };
    }
};

} // namespace Detail

} // namespace Calc
