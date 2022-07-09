#include <iostream>

#include "exceptions.hpp"
#include "constants.hpp"
#include "lexer.hpp"

using namespace parselib;



Rule::Rule()
    : pattern(::constants::empty<std::string>())
    , regex(::constants::empty<std::string>())
    , tag(::constants::empty<uint32_t>())
    , ignorable(true)
{}


Rule::Rule(const std::string& pattern, uint32_t tag, bool ignorable) :
    pattern(pattern), regex(pattern), tag(tag), ignorable(ignorable) {}

Rule::Rule(const char* pattern, uint32_t tag, bool ignorable) :
    pattern(pattern), regex(pattern), tag(tag), ignorable(ignorable) {}

MatchObject Rule::match(const std::string& input, const uint64_t pos) const {
    CSIterator begin = input.cbegin(), end = input.cend();
    return pos >= static_cast<uint64_t>(std::distance(begin, end)) ?
        MatchObject() : match(begin + pos, end);
}


MatchObject Rule::match(CSIterator begin, CSIterator end) const {
    MatchObject matchObject;
    const bool result = std::regex_search(begin, end, matchObject, regex);
    return result && matchObject.position() == 0 ? matchObject : MatchObject();
}


bool Rule::isValid() const {
    return !(pattern == ::constants::empty<std::string>() && ignorable);
}



Lexem::Lexem()
    : content(::constants::empty<std::string>())
    , start(::constants::empty<uint64_t>())
    , length(::constants::empty<uint64_t>())
    , end(::constants::empty<uint64_t>())
    , tag(::constants::empty<Tag>())
{}


Lexem::Lexem(const std::string& value, uint64_t begin, Tag tag)
    : content(value)
    , start(begin)
    , length(value.length())
    , end(start + length)
    , tag(tag)
{}


bool Lexem::empty() const {
    return content == ::constants::empty<std::string>() &&
           start == 0 && length == 0 && end == 0;
}



Lexer::Lexer(const Rules& rules) : _rules(rules), _position(0) {}


std::vector<Lexem> Lexer::tokenize(const std::string& input) {
    const uint64_t length = input.length();
    std::vector<Lexem> out;
    while (_position < length) {
        Lexem lexem = findLexem(input);
        if (!lexem.empty()) {
            out.push_back(lexem);
        }
    }
    _position = 0;
    return out;
}


Lexem Lexer::findLexem(const std::string& input) {
    for (const Rule& rule: _rules) {
        MatchObject result = rule.match(input, _position);
        if (!result.empty()) {
            std::string content = result.str();
            const uint64_t start = _position;
            _position += content.length();
            return rule.ignorable ? Lexem() : Lexem(content, start, rule.tag);
        }
    }
    // TODO: make message more verbose
    throw error::lexical::UnexpectedLexem("UnexpectedLexem");
}


std::ostream& parselib::operator << (std::ostream& os, const Lexem& lexem) {
    return os << "[Lexem content: " << "'" << lexem.content << "'"
              << "(" << lexem.start << " - " << lexem.end << ")]";
}


std::ostream& parselib::operator << (std::ostream& os, const Lexems& lexems) {
    const size_t length = lexems.size();

    os << "{";
    for (size_t index = 0; index < length - 1; ++index) {
        os << lexems[index] << ", ";
    }
    return length == 0 ? os << "}" : os << lexems[length - 1] << "}";
}
