#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>


struct Expr {
    virtual ~Expr() = default;
    virtual std::string to_python() const = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

struct IntLiteral final : Expr {
    explicit IntLiteral(int value) : value(value) {}

    std::string to_python() const override {
        return std::to_string(value);
    }

    int value;
};

struct Variable final : Expr {
    explicit Variable(std::string name) : name(std::move(name)) {}

    std::string to_python() const override {
        return name;
    }

    std::string name;
};

struct BinaryOp final : Expr {
    BinaryOp(ExprPtr left, std::string op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    std::string to_python() const override {
        return "(" + left->to_python() + " " + op + " " + right->to_python() + ")";
    }

    ExprPtr left;
    std::string op;
    ExprPtr right;
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual std::string to_python() const = 0;
};

