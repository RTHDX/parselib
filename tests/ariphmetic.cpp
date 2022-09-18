#include <gtest/gtest.h>

#include "parsers.hpp"
#include "lexer.hpp"
#include "language.hpp"

/*
 * num = d+
 * add = '+' | '-'
 * mul = '*' | '/'
 * open = '('
 * close = ')'
 * lhs_add = num
 * rhs_add = add + num
 * add_stmt = lhs_add + rhs_add
 * lhs_mul = num
 * rhs_mul = mul + (num | add_stmt)
 * mul_stmt = lhs_mul + rhs_mul
 * stmt = add_stmt | mul_stmt | (open + stmt + close)
 */

struct Tag {
    enum Type {
        NUM = 0,
        ADD,
        SUB,
        MUL,
        DIV,
        OPEN,
        CLOSE,

        SPACE = 254,
        UNDEF = 255
    };
};

std::vector<parselib::Rule> rules() {
    return std::vector<parselib::Rule> {
        parselib::Rule{R"(\d+)", Tag::NUM},
        parselib::Rule{R"(\+)", Tag::ADD},
        parselib::Rule{R"(\-)", Tag::SUB},
        parselib::Rule{R"(\*)", Tag::MUL},
        parselib::Rule{R"(/)", Tag::DIV},
        parselib::Rule{R"(\()", Tag::OPEN},
        parselib::Rule{R"(\))", Tag::CLOSE},
        parselib::Rule{"\\s+", Tag::SPACE, true}
    };
}

TEST(Lexer, test) {
    parselib::Lexer lexer(rules());
    auto result = lexer.tokenize("()");
    ASSERT_EQ(result.size(), 2);
    ASSERT_TRUE(result[0].tag == Tag::OPEN && result[1].tag == Tag::CLOSE);

    result = lexer.tokenize("34 + 4");
    ASSERT_EQ(result.size(), 3);
    ASSERT_TRUE(result[0].tag == Tag::NUM &&
                result[1].tag == Tag::ADD &&
                result[2].tag == Tag::NUM);
}

class NumAST : public parselib::AST {
public:
    NumAST(const std::string& num)
        : _val(std::atoi(num.c_str()))
    {}

    void append(AST*) override {}
    void pop(AST*) override {}
    void accept(parselib::Visitor*) const override;

    int num() const { _val; }

private:
    int _val;
};

class OpAST : public parselib::AST {
public:
    OpAST(const std::string& op)
        : _op(op)
    {}

    Tag::Type op_code() const {
        if (_op == "+") {
            return Tag::ADD;
        }
        if (_op == "-") {
            return Tag::SUB;
        }
        if (_op == "*") {
            return Tag::MUL;
        }
        if (_op == "/") {
            return Tag::DIV;
        }
        return Tag::UNDEF;
    }

    void append(AST*) override {}
    void pop(AST*) override {}
    void accept(parselib::Visitor*) const override;

private:
    std::string _op;
};


