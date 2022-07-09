#include "language.hpp"
using namespace parselib;



SyntaxTree::SyntaxTree(AST* root) : AST(), _root(root), _cursor(root) {}


void SyntaxTree::append(AST* tree) {
    if (_cursor) {
        _cursor->append(tree);
    }
}


void SyntaxTree::accept(Visitor* visitor) const {
    if (_root) {
        _root->accept(visitor);
    }
}


void SyntaxTree::pop(AST* subtree) {
    if (_root) {
        _root->pop(subtree);
    }
}
