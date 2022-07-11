#pragma once

#include <ostream>
#include <type_traits>
#include <functional>
#include <concepts>

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
    virtual bool is_valid() const = 0;
    virtual IParser* clone() const = 0;
};
template <typename T> concept CParser = std::derived_from<T, IParser>;
template <typename T> concept CParser_ref =
    std::is_reference<T>::value &&
    std::derived_from<typename std::remove_reference<T>::type, IParser>;


class Atom final : public IParser {
    Tag _tag;

public:
    Atom() = default;
    Atom(Tag tag);
    ~Atom() override = default;

    State operator () (State) const override;
    bool is_valid() const override;
    IParser* clone() const override { return new Atom(_tag); }
};



class Any final : public IParser {
public:
    Any() = default;
    ~Any() override = default;

    State operator() (State) const override;
    bool is_valid() const override;

    IParser* clone() const override { return new Any(); }
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

    bool is_valid() const override {
        return _left.is_valid() && _right.is_valid();
    }

    IParser* clone() const override { return new And(_left, _right); }
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

    bool is_valid() const override {
         return _left.is_valid() || _right.is_valid();
    }

    IParser* clone() const override { return new Or(_left, _right); }
};


using Action = std::function<void(State&)>;
static Action skip = [](State&){};
class Parser : public IParser {
    IParser* _parser = nullptr;

    // do nothing by default;
    Action _before = skip;
    Action _on_accept = skip;
    Action _on_fail = skip;

public:
    Parser() = default;
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

    State operator () (State state) const override;
    bool is_valid() const override;
    IParser* clone() const override;

    Parser& on_before(Action before) { _before = before; return *this; }
    Parser& on_accept(Action accept) { _on_accept = accept; return *this; }
    Parser& on_disaccept(Action fail) { _on_fail = fail; return *this; }
};


template<CParser Impl> class OneOrMore final : public IParser {
    Impl _parser;

public:
    OneOrMore() = default;
    OneOrMore(const Impl& parser) : IParser()
        , _parser(parser) {}

    State operator () (State state) const override {
        State result = _parser(state);
        while (!terminate(result) && result.accept) {
            const State temp = _parser(result);
            if (temp.accept == false) {
                break;
            }
            result = temp;
        }
        return result;
    }

    IParser* clone() const override { return new OneOrMore{ _parser }; }
    bool is_valid() const override { return _parser.is_valid(); }
};


class Forward final : public IParser {
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

    State operator () (State) const override;
    bool is_valid() const override;
    IParser* clone() const override;

private:
    explicit Forward(Impl&& parser);
};


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

template<IAST Tree, typename ... Args>
inline Action primary_type_builder(Args ... args) {
    return [args...](State& state) {
        CLIterator target = state.current - 1;
        AST* tree = new Tree(target->content, args ...);
        tree->parent(state.tree.cursor());
        state.tree.append(tree);
    };
}

template<IAST Tree> inline void before_action(State& state) {
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

template <typename T> concept ASTProducer = requires (T t, Action a) {
    { t.on_before(a) }    -> CParser_ref;
    { t.on_accept(a) }    -> CParser_ref;
    { t.on_disaccept(a) } -> CParser_ref;
};

template <CParser Left, CParser Right>
inline And<Left, Right> operator + (Left left, Right right) {
    return And<Left, Right> { left, right };
}

template <CParser Left, CParser Right>
inline Or<Left, Right> operator | (Left left, Right right) {
    return Or<Left, Right> { left, right };
}

template <CParser Impl> OneOrMore<Impl> one_or_more(Impl impl) {
    return OneOrMore<Impl>(impl);
}

template <IAST Tree, ASTProducer P>
inline P& bind(P& p) {
    return p.on_before(before_action<Tree>)
            .on_accept(accept_action)
            .on_disaccept(disaccept_action);
}

}
