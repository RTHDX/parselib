#pragma once

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
    virtual bool isEmpty() const = 0;
    virtual AST* clone() const { return nullptr; }

    AST* parent() const { return _parent; }
    void parent(AST* parent) { _parent = parent; }
};



class SyntaxTree : public AST {
    AST* _root = nullptr;
    AST* _cursor = nullptr;

public:
    SyntaxTree(AST* root=nullptr);
    ~SyntaxTree() override = default;

    void append(AST*) override;
    void pop(AST*) override;
    void accept(Visitor*) const override;
    bool isEmpty() const override { return _root == nullptr; }

    AST* cursor() const { return _cursor; }
    void cursor(AST* cursor) { _cursor = cursor; }
};

}
