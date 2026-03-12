#pragma once
#include <regex>
#include <string>

namespace betterrx {
    struct Regex {
      public:
        std::string pattern;
        std::regex rx;

        explicit Regex(const std::string& pattern)
            : pattern(pattern), rx(pattern){};
        bool operator==(const Regex& other) const {
            return pattern == other.pattern;
        }
        explicit operator std::regex() const {
            return rx;
        }
    };
} // namespace betterrx
using Regex = betterrx::Regex;

namespace std {
    template <>
    struct hash<betterrx::Regex> {
        std::size_t operator()(const betterrx::Regex& regex) const noexcept {
            return std::hash<std::string>{}(regex.pattern);
        }
    };
} // namespace std