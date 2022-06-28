#pragma once

#include <cctype>

namespace Calc {

namespace Detail {

inline bool IsOperatorChar(char c) {
    switch (c) {
        case '+':
        case '-':
        case '>':
        case '<':
        case '=':
        case '!':
        case '~':
        case '*':
        case '/':
        case '^':
        case '%':
        case '&':
        case '|':
        case '@':
        case '#': return true;
        default: return false;
    }
}

inline bool IsReservedChar(char c) {
    switch (c) {
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ',':
        case '.':
        case ';': return true;
        default: return false;
    }
}

inline bool IsIdentifierStartChar(char c) {
    return c < 0 ||
           (!IsOperatorChar(c) && !IsReservedChar(c) && !std::isdigit(c) && !std::isspace(c));
}

inline bool IsIdentifierChar(char c) {
    return c < 0 || (!IsOperatorChar(c) && !IsReservedChar(c) && !std::isspace(c));
}

} // namespace Detail

} // namespace Calc