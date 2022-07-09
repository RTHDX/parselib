#pragma once

#include <vector>
#include <regex>
#include <string>
#include <ostream>
#include <cstdint>

namespace parselib {

using CSIterator = std::string::const_iterator;
using CAllocator = std::allocator<std::sub_match<CSIterator>>;
using MatchObject = std::match_results<CSIterator, CAllocator>;

struct Rule {
    std::string pattern;
    std::regex regex;
    uint32_t tag;
    bool ignorable;

    explicit Rule();
    explicit Rule(const std::string&, uint32_t, bool ignorable=false);
    explicit Rule(const char*, uint32_t, bool ignorable=false);

    MatchObject match(CSIterator, CSIterator) const;
    MatchObject match(const std::string& input, const uint64_t) const;
    bool isValid() const;
};
using Rules = std::vector<Rule>;


using Tag = unsigned int;
struct Lexem {
    std::string content;
    uint64_t start;
    uint64_t length;
    uint64_t end;
    Tag tag;

    explicit Lexem();
    explicit Lexem(const std::string&, uint64_t=0, Tag=0);
    bool empty() const;
};
using Lexems = std::vector<Lexem>;

std::ostream& operator << (std::ostream& os, const Lexem& lexem);
std::ostream& operator << (std::ostream& os, const Lexems& lexems);



class Lexer {
    const Rules _rules;
    uint64_t _position;

public:
    Lexer(const Rules&);

    Lexems tokenize(const std::string& input) noexcept(false);

private:
    Lexem findLexem(const std::string&) noexcept(false);
};

}
