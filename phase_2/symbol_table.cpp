#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;




struct Symbol {
    string name;
    string type;
};

struct Environment{
    vector<string> names;
    vector<int> present;
    vector<Symbol> declared;

    Environment()
        : names({"a", "b", "c", "x", "y", "z"}),
          present(names.size(), 0) {}
};

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

int random_int(mt19937& rng, int low, int high) {
    if (low > high) {
        throw invalid_argument("random_int called with low > high");
    }

    uniform_int_distribution<int> distribution(low, high);
    return distribution(rng);
}

string random_variable_name(mt19937& rng, Environment& env) {
    return env.names[random_int(rng, 0, static_cast<int>(env.names.size()) - 1)];
}

vector<string> declared_names_of_type(Environment& env, const string& type) {
    vector<string> matching_names;

    for (const Symbol& symbol : env.declared) {
        if (symbol.type == type) {
            matching_names.push_back(symbol.name);
        }
    }

    return matching_names;
}

string random_declared_variable_name(mt19937& rng, Environment& env, const string& type){
    vector<string> matching_names = declared_names_of_type(env, type);

    if (matching_names.empty()) {
        throw runtime_error("cannot choose a declared " + type + " variable before any exists");
    }

    return matching_names[random_int(rng, 0, static_cast<int>(matching_names.size()) - 1)];
}

string random_operator(mt19937& rng) {
    vector<string> operators = {"+", "-", "*"};
    return operators[random_int(rng, 0, static_cast<int>(operators.size()) - 1)];
}

ExprPtr generate_expression(mt19937& rng, Environment& env, int depth) {
    if (depth <= 0) {
        int choice = random_int(rng, 0, 1);

        if (choice == 0 || declared_names_of_type(env, "int").empty()) {
            return make_unique<IntLiteral>(random_int(rng, 0, 99));
        }

        return make_unique<Variable>(random_declared_variable_name(rng, env, "int"));
    }

    int choice = declared_names_of_type(env, "int").empty() ? random_int(rng, 0, 1) : random_int(rng, 0, 2);

    if (choice == 0) {
        return make_unique<IntLiteral>(random_int(rng, 0, 99));
    }

    if (choice == 1 && !declared_names_of_type(env, "int").empty()) {
        return make_unique<Variable>(random_declared_variable_name(rng, env, "int"));
    }

    return make_unique<BinaryOp>(
        generate_expression(rng, env, depth - 1),
        random_operator(rng),
        generate_expression(rng, env, depth - 1)
    );
}

StmtPtr generate_statement(mt19937& rng, Environment& env, int expression_depth) {
    int choice = random_int(rng, 0, 1);

    if (choice == 0) {
        int n = random_int(rng, 0, static_cast<int>(env.names.size()) - 1);
        string name = env.names[n];
        ExprPtr value = generate_expression(rng, env, expression_depth);

        if (env.present[n] == 0){
            env.present[n] = 1;
            env.declared.push_back(Symbol{name, "int"});
        }
        
        return make_unique<Assign>(
            name,
            move(value)
        );
    }

    return make_unique<Print>(generate_expression(rng, env, expression_depth));
}

Program generate_program(mt19937& rng, int statement_count, int expression_depth) {
    Program program;
    Environment env;

    for (int i = 0; i < statement_count; i++) {
        program.add_statement(generate_statement(rng, env, expression_depth));
    }

    return program;
}

int main() {
    int seed = 50;
    int statement_count = 8;
    int expression_depth = 3;

    mt19937 rng(seed);
    Program program = generate_program(rng, statement_count, expression_depth);

    cout << "# seed: " << seed << "\n";
    cout << program.to_python();

    return 0;
}
