#pragma once

#include <ostream>
#include <type_traits>
#include <functional>

#include "lexer.hpp"
#include "language.hpp"


namespace parselib {

using CLIterator = std::vector<parselib::Lexem>::const_iterator;
struct State {
    CLIterator begin;
    CLIterator end;
    CLIterator current;
    parselib::SyntaxTree tree;
    bool accept;

    State();
    State(CLIterator, CLIterator, CLIterator, SyntaxTree, bool=false);
};

bool operator == (const State&, const State&);
bool operator != (const State&, const State&);
std::ostream& operator << (std::ostream& os, const State& state);

bool terminate(const State&);


class IParser {
public:
    IParser() = default;
    IParser(const IParser&) = default;
    IParser(IParser&&) noexcept = default;
    IParser& operator = (const IParser&) = default;
    IParser& operator = (IParser&&) = default;
    virtual ~IParser() = default;

    virtual State operator () (State) const = 0;
    virtual IParser* clone() const = 0;
    virtual bool is_valid() const = 0;
};
template <typename T> concept CParser = std::is_base_of<IParser, T>::value;


class Atom final : public IParser {
    Tag _tag;

public:
    Atom() = default;
    Atom(Tag tag);
    ~Atom() override = default;

    State operator () (State) const override;
    IParser* clone() const override;
    bool is_valid() const override;
};



class Any final : public IParser {
public:
    Any() = default;
    ~Any() override = default;

    State operator() (State) const override;
    IParser* clone() const override;
    bool is_valid() const override;
};



template<CParser Left, CParser Right> class And final : public IParser {
    Left _left;
    Right _right;

public:
    And() = default;

    And(Left left, Right right)
        : IParser()
        , _left(left)
        , _right(right)
    {}

    State operator () (State state) const override {
        if (terminate(state)) return state;

        State l_result = _left(state);
        if (l_result.accept == false) {
            state.accept = false;
            return state;
        }

        State r_result = _right(l_result);
        if (r_result.accept == false) {
            state.accept = false;
            return state;
        }

        return r_result;
    }

    IParser* clone() const override {
        return new And<Left, Right>{ _left, _right };
    }

    bool is_valid() const override {
        return _left.isValid() && _right.isValid();
    }
};


template<CParser Left, CParser Right> class Or final : public IParser {
    Left _left;
    Right _right;

public:
    Or() = default;

    Or(Left left, Right right)
        : IParser()
        , _left(left)
        , _right(right)
    {}

    State operator () (State state) const override {
        if (terminate(state)) return state;

        State result = _left(state);
        if (result.accept == true) {
            return result;
        }

        result = _right(state);
        if (result.accept == true) {
            return result;
        }

        state.accept = false;
        return state;
    }

    IParser* clone() const override {
        return new Or<Left, Right>{ _left, _right };
    }

    bool is_valid() const override {
        return _left.isValid() || _right.isValid();
    }
};


using Action = std::function<void(State&)>;
static Action skip = [](State&){};
class Parser : public IParser {
    IParser* _parser;

    // do nothing by default;
    Action _before = skip;
    Action _on_accept = skip;
    Action _on_fail = skip;

public:
    Parser() : IParser(), _parser(nullptr) {}
    Parser(const Parser&);
    Parser(Parser&&) noexcept;
    const Parser& operator = (const Parser&);
    const Parser& operator = (Parser&&) noexcept;
    ~Parser() override;


    template <CParser This> Parser(const This& parser) : IParser() {
        This* temp = new This;
        *temp = parser;
        _parser = temp;
    }

    State operator () (State state) const override final;
    IParser* clone() const override final;
    bool is_valid() const override final;

    Parser& on_before(Action before) { _before = before; return *this; }
    Parser& on_accept(Action onAccept) { _on_accept = onAccept; return *this; }
    Parser& on_disaccept(Action onFail) { _on_fail = onFail; return *this; }
};



class Forward : public IParser {
    using Impl = std::function<State(const Forward&, const State&)>;

    Impl _parser;

public:
    static Forward Decl(Impl&&);

    Forward() = default;
    Forward(const Forward&) = default;
    Forward(Forward&&) noexcept = default;
    Forward& operator = (const Forward&) = default;
    Forward& operator = (Forward&&) noexcept = default;
    ~Forward() override;

    State operator () (State) const override final;
    IParser* clone() const override final;
    bool is_valid() const override final;

private:
    explicit Forward(Impl&& parser);
};



template <CParser Left, CParser Right>
inline And<Left, Right> operator + (Left left, Right right) {
    return And<Left, Right> { left, right };
}


template <CParser Left, CParser Right>
inline Or<Left, Right> operator | (Left left, Right right) {
    return Or<Left, Right> { left, right };
}


class Driver {
    Parser _parser;
    State _finish;

public:
    Driver() = default;
    Driver(const Parser& parser) : _parser(parser) {}

    bool accept(const Lexems&, AST* = nullptr);
    SyntaxTree parse(const Lexems&, AST* = nullptr);

    const State& finish() const { return _finish; }
    const Parser& parser() const { return _parser; }

private:
    bool is_accept(const State&) const;
};

template<typename Tree, typename ... Args>
inline Action primary_type_builder(Args ... args) {
    return [args...](State& state) {
        CLIterator target = state.current - 1;
        AST* tree = new Tree(target->content, args ...);
        tree->parent(state.tree.cursor());
        state.tree.append(tree);
    };
}


template<typename Tree> inline void before_action(State& state) {
    Tree* candidate = new Tree;
    state.tree.append(candidate);
    candidate->parent(state.tree.cursor());
    state.tree.cursor(candidate);
}


inline void accept_action(State& state) {
    state.tree.cursor(state.tree.cursor()->parent());
}


inline void disaccept_action(State& state) {
    AST* to_delete = state.tree.cursor();
    AST* parent = to_delete->parent();
    state.tree.cursor(parent);
    if (parent) {
        parent->pop(to_delete);
    }
}

}
