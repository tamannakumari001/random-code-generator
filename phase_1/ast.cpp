#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;


// Grammar:
//   Program -> Stmt*
//   Stmt    -> name = Expr
//   Stmt    -> print(Expr)
//   Expr    -> Int
//   Expr    -> Var
//   Expr    -> Expr Op Expr
//   Op      -> + | - | *

struct Expr {
    virtual ~Expr() = default;
    virtual string to_python() const = 0;
};

using ExprPtr = unique_ptr<Expr>;

struct IntLiteral : Expr {
    int value;

    IntLiteral(int value) : value(value) {}

    string to_python() const override {
        return to_string(value);
    }
};

struct Variable : Expr {
    string name;

    Variable(string name) : name(move(name)) {}

    string to_python() const override {
        return name;
    }
};

struct BinaryOp : Expr {
    ExprPtr left;
    string op;
    ExprPtr right;

    BinaryOp(ExprPtr left, string op, ExprPtr right)
        : left(move(left)), op(move(op)), right(move(right)) {}

    string to_python() const override {
        return "(" + left->to_python() + " " + op + " " + right->to_python() + ")";
    }
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual string to_python() const = 0;
};

using StmtPtr = unique_ptr<Stmt>;

struct Assign : Stmt {
    string name;
    ExprPtr value;

    Assign(string name, ExprPtr value)
        : name(move(name)), value(move(value)) {}

    string to_python() const override {
        return name + " = " + value->to_python();
    }
};

struct Print : Stmt {
    ExprPtr value;

    Print(ExprPtr value) : value(move(value)) {}

    string to_python() const override {
        return "print(" + value->to_python() + ")";
    }
};

struct Program {
    vector<StmtPtr> statements;

    void add_statement(StmtPtr statement) {
        statements.push_back(move(statement));
    }

    string to_python() const {
        string code;

        for (const StmtPtr& statement : statements) {
            code += statement->to_python();
            code += "\n";
        }

        return code;
    }
};

int main(){
    Program p =  Program();
    
}
