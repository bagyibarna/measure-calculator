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

inline bool IsAscii(char c) { return c >= -1 && c < 255; }

inline bool IsWhiteSpace(char c) { return IsAscii(c) && std::isspace(c); }

inline bool IsDigit(char c) { return IsAscii(c) && std::isdigit(c); }

inline bool IsIdentifierStartChar(char c) {
    return !IsAscii(c) ||
           (!IsOperatorChar(c) && !IsReservedChar(c) && !std::isdigit(c) && !std::isspace(c));
}

inline bool IsIdentifierChar(char c) {
    return !IsAscii(c) || (!IsOperatorChar(c) && !IsReservedChar(c) && !std::isspace(c));
}

} // namespace Detail

} // namespace Calc