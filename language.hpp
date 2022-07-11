#pragma once

#include <memory>
#include <typeinfo>


namespace parselib {

class Visitor;

class AST {
    AST* _parent = nullptr;

public:
    AST() = default;
    virtual ~AST() = default;

    virtual void append(AST*) = 0;
    virtual void pop(AST*) = 0;
    virtual void accept(Visitor*) const = 0;

    AST* parent() const { return _parent; }
    void parent(AST* parent) { _parent = parent; }
};

using uAST = std::unique_ptr<AST>;
template <typename T> concept IAST = std::is_base_of<AST, T>::value;
#define NO_POP    void pop(parselib::AST*) override {}
#define NO_APPEND void append(parselib::AST*) override {}



class SyntaxTree : public AST {
    AST* _root = nullptr;
    AST* _cursor = nullptr;

public:
    SyntaxTree(AST* root=nullptr);
    ~SyntaxTree() override = default;

    void append(AST*) override;
    void pop(AST*) override;
    void accept(Visitor*) const override;

    AST* cursor() const { return _cursor; }
    void cursor(AST* cursor) { _cursor = cursor; }
};

}
