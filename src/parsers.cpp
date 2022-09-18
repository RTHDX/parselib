#include <iostream>
#include <cassert>
#include <iterator>

#include "constants.hpp"
#include "parsers.hpp"


namespace parselib {

State::State()
    : begin(std::cbegin(constants::empty<Lexems>()))
    , end(std::end(constants::empty<Lexems>()))
    , current(std::end(constants::empty<Lexems>()))
    , tree(constants::empty<SyntaxTree>())
    , accept(constants::empty<bool>())
{}


State::State(CLIterator begin,
             CLIterator end,
             CLIterator current,
             SyntaxTree tree,
             bool accept)
    : begin(begin)
    , end(end)
    , current(current)
    , tree(tree)
    , accept(accept)
{}


std::ostream& parselib::operator << (std::ostream& os, const State& state) {
    os << "Accept - " << std::boolalpha
       << state.accept << " [Accepted substring: ";
    for (auto current = state.begin; current < state.current; ++current) {
        os << current->content << ";";
    }
    os << "], [Raw substring: ";
    for (auto current = state.current; current < state.end; ++current) {
        os << current->content << ";";
    }
    return os << "]";
}


bool parselib::terminate(const State& state) {
    /* exit when execution reaches the end */
    return state.current == state.end;
}


Atom::Atom(Tag tag) : IParser(), _tag(tag) {}


State Atom::operator () (State state) const {
    state.accept = state.current->tag == _tag;
    state.current += state.accept;
    return state;
}


IParser* Atom::clone() const { return new Atom(_tag); }
bool Atom::is_valid() const { return bool(_tag); }


State Any::operator () (State state) const {
    state.accept = true;
    state.current += 1;
    return state;
}

IParser* Any::clone() const { return new Any; }
bool Any::is_valid() const { return true; }



Parser::~Parser() {
    delete _parser;
}


Parser::Parser(const Parser& old) {
    if (this != &old) {
        _parser = old._parser != nullptr ? old._parser->clone() : nullptr;
        _before = old._before;
        _on_accept = old._on_accept;
        _on_fail = old._on_fail;
    }
}


const Parser& Parser::operator = (const Parser& old) {
    if (this == &old) return *this;

    delete _parser;
    _parser = old._parser == nullptr ? nullptr : old._parser->clone();
    _before = old._before;
    _on_accept = old._on_accept;
    _on_fail = old._on_fail;
    return *this;
}


Parser::Parser(Parser&& old) noexcept {
    if (this != &old) {
        _parser = old._parser;
        old._parser = nullptr;
        _before = std::move(old._before);
        _on_accept = std::move(old._on_accept);
        _on_fail = std::move(old._on_fail);
    }
}


const Parser& Parser::operator = (Parser&& old) noexcept {
    if (this == &old) return *this;

    delete _parser;
    _parser = old._parser;
    old._parser = nullptr;
    _before = std::move(old._before);
    _on_accept = std::move(old._on_accept);
    _on_fail = std::move(old._on_fail);
    return *this;
}


State Parser::operator()(State state) const {
    assert(is_valid() && "using of unassigned parser");

    if (_before) { _before(state); }
    State result = _parser->operator()(state);
    if (result.accept) {
        if (_on_accept) { _on_accept(result); }
    } else {
        if (_on_fail) { _on_fail(result); }
    }
    return result;
}


IParser* Parser::clone() const {
    Parser* temp = nullptr;
    if (is_valid()) {
        temp = new Parser;
        temp->_parser = _parser->clone();
    }
    return temp;
}


bool Parser::is_valid() const {
    return _parser != nullptr && _parser->is_valid();
}



Forward Forward::Decl(Impl&& impl) {
    return Forward(move(impl));
}


Forward::Forward(Impl&& parser) : IParser(), _parser(move(parser)) {}


Forward::~Forward() {}


State Forward::operator ()(State state) const {
    assert(is_valid() && "using of invalid parser");
    State result = _parser(*this, state);
    return result;
}


IParser* Forward::clone() const {
    Forward* newFwd = new Forward;
    newFwd->_parser = _parser;
    return newFwd;
}


bool Forward::is_valid() const {
    return bool(_parser);
}



bool Driver::accept(const Lexems& input, AST* tree) {
    if (distance(cbegin(input), cend(input)) == 0) return false;

    State start {cbegin(input), cend(input), cbegin(input), SyntaxTree{tree}};
    _finish = _parser(start);
    return is_accept(start);
}


SyntaxTree Driver::parse(const Lexems& input, AST* tree) {
    if (input.size() == 0) return SyntaxTree(nullptr);

    State start{cbegin(input), cend(input), cbegin(input), SyntaxTree(tree)};
    _finish = _parser(start);
    return is_accept(start) ? _finish.tree : SyntaxTree(nullptr);
}


bool Driver::is_accept(const State& start) const {
    return _finish.accept && _finish.current == start.end;
}

}
