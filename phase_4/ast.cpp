#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

enum class ValueType {
    Int,
    Bool,
    String,
    IntList,
    BoolList,
    StringList,
};

string type_name(ValueType type) {
    if (type == ValueType::Int) return "int";
    if (type == ValueType::Bool) return "bool";
    if (type == ValueType::String) return "str";
    if (type == ValueType::IntList) return "list[int]";
    if (type == ValueType::BoolList) return "list[bool]";
    return "list[str]";
}

bool is_list_type(ValueType type) {
    return type == ValueType::IntList
        || type == ValueType::BoolList
        || type == ValueType::StringList;
}

ValueType element_type(ValueType type) {
    if (type == ValueType::IntList) return ValueType::Int;
    if (type == ValueType::BoolList) return ValueType::Bool;
    if (type == ValueType::StringList) return ValueType::String;
    throw runtime_error("not a list type");
}

ValueType list_type_for(ValueType type) {
    if (type == ValueType::Int) return ValueType::IntList;
    if (type == ValueType::Bool) return ValueType::BoolList;
    if (type == ValueType::String) return ValueType::StringList;
    throw runtime_error("cannot create nested list type");
}

string indentation(int level) {
    return string(level * 4, ' ');
}

int random_int(mt19937& rng, int low, int high) {
    if (low > high) {
        throw invalid_argument("random_int called with low > high");
    }

    uniform_int_distribution<int> distribution(low, high);
    return distribution(rng);
}

template <typename T>
const T& random_choice(mt19937& rng, const vector<T>& values) {
    if (values.empty()) {
        throw runtime_error("random_choice called with empty vector");
    }

    return values[random_int(rng, 0, static_cast<int>(values.size()) - 1)];
}

struct Symbol {
    string name;
    ValueType type;
};

struct Parameter {
    string name;
    ValueType type;
};

struct FunctionSignature {
    string name;
    vector<Parameter> params;
    ValueType return_type;
};

struct Environment {
    vector<string> variable_pool;
    vector<Symbol> variables;
    vector<FunctionSignature> functions;

    Environment()
        : variable_pool({"Trump","Big_Yahoo", "six_seven", "putin", "Tralalero_Tralala", "Triple_T", "Sahuurrr", "Bombardilo_chan", "Espressona"}) {}

    void declare_or_update(const string& name, ValueType type) {
        for (Symbol& symbol : variables) {
            if (symbol.name == name) {
                symbol.type = type;
                return;
            }
        }

        variables.push_back(Symbol{name, type});
    }

    vector<string> names_of_type(ValueType type) const {
        vector<string> result;

        for (const Symbol& symbol : variables) {
            if (symbol.type == type) {
                result.push_back(symbol.name);
            }
        }

        return result;
    }

    vector<FunctionSignature> functions_returning(ValueType type) const {
        vector<FunctionSignature> result;

        for (const FunctionSignature& function : functions) {
            if (function.return_type == type) {
                result.push_back(function);
            }
        }

        return result;
    }
};

struct Expr {
    ValueType type;

    Expr(ValueType type) : type(type) {}
    virtual ~Expr() = default;
    virtual string to_python() const = 0;
    virtual string repr() const = 0;
};

using ExprPtr = unique_ptr<Expr>;

struct LiteralExpr : Expr {
    string value;

    LiteralExpr(ValueType type, string value)
        : Expr(type), value(move(value)) {}

    string to_python() const override {
        return value;
    }

    string repr() const override {
        return "Literal(" + type_name(type) + "," + value + ")";
    }
};

struct VariableExpr : Expr {
    string name;

    VariableExpr(ValueType type, string name)
        : Expr(type), name(move(name)) {}

    string to_python() const override {
        return name;
    }

    string repr() const override {
        return "Var(" + type_name(type) + "," + name + ")";
    }
};

struct BinaryExpr : Expr {
    ExprPtr left;
    string op;
    ExprPtr right;

    BinaryExpr(ValueType type, ExprPtr left, string op, ExprPtr right)
        : Expr(type), left(move(left)), op(move(op)), right(move(right)) {}

    string to_python() const override {
        return "(" + left->to_python() + " " + op + " " + right->to_python() + ")";
    }

    string repr() const override {
        return "Binary(" + type_name(type) + "," + op + "," + left->repr() + "," + right->repr() + ")";
    }
};

struct UnaryExpr : Expr {
    string op;
    ExprPtr value;

    UnaryExpr(ValueType type, string op, ExprPtr value)
        : Expr(type), op(move(op)), value(move(value)) {}

    string to_python() const override {
        return "(" + op + " " + value->to_python() + ")";
    }

    string repr() const override {
        return "Unary(" + type_name(type) + "," + op + "," + value->repr() + ")";
    }
};

struct ListExpr : Expr {
    vector<ExprPtr> items;

    ListExpr(ValueType type, vector<ExprPtr> items)
        : Expr(type), items(move(items)) {}

    string to_python() const override {
        string code = "[";

        for (size_t i = 0; i < items.size(); i++) {
            if (i > 0) {
                code += ", ";
            }

            code += items[i]->to_python();
        }

        code += "]";
        return code;
    }

    string repr() const override {
        string result = "List(" + type_name(type) + ",";

        for (const ExprPtr& item : items) {
            result += item->repr();
            result += ";";
        }

        result += ")";
        return result;
    }
};

struct LenExpr : Expr {
    ExprPtr value;

    LenExpr(ExprPtr value)
        : Expr(ValueType::Int), value(move(value)) {}

    string to_python() const override {
        return "len(" + value->to_python() + ")";
    }

    string repr() const override {
        return "Len(" + value->repr() + ")";
    }
};

struct IndexExpr : Expr {
    ExprPtr value;
    ExprPtr index;

    IndexExpr(ValueType type, ExprPtr value, ExprPtr index)
        : Expr(type), value(move(value)), index(move(index)) {}

    string to_python() const override {
        return value->to_python() + "[" + index->to_python() + "]";
    }

    string repr() const override {
        return "Index(" + type_name(type) + "," + value->repr() + "," + index->repr() + ")";
    }
};

struct FunctionCallExpr : Expr {
    string name;
    vector<ExprPtr> args;

    FunctionCallExpr(ValueType type, string name, vector<ExprPtr> args)
        : Expr(type), name(move(name)), args(move(args)) {}

    string to_python() const override {
        string code = name + "(";

        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) {
                code += ", ";
            }

            code += args[i]->to_python();
        }

        code += ")";
        return code;
    }

    string repr() const override {
        string result = "Call(" + type_name(type) + "," + name + ",";

        for (const ExprPtr& arg : args) {
            result += arg->repr();
            result += ";";
        }

        result += ")";
        return result;
    }
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual string to_python(int indent_level) const = 0;
    virtual string repr() const = 0;
};

using StmtPtr = unique_ptr<Stmt>;

struct Block {
    vector<StmtPtr> statements;

    void add_statement(StmtPtr statement) {
        statements.push_back(move(statement));
    }

    string to_python(int indent_level) const {
        string code;

        for (const StmtPtr& statement : statements) {
            code += statement->to_python(indent_level);
        }

        return code;
    }

    string repr() const {
        string result = "Block(";

        for (const StmtPtr& statement : statements) {
            result += statement->repr();
            result += ";";
        }

        result += ")";
        return result;
    }
};

struct AssignStmt : Stmt {
    string name;
    ExprPtr value;

    AssignStmt(string name, ExprPtr value)
        : name(move(name)), value(move(value)) {}

    string to_python(int indent_level) const override {
        return indentation(indent_level) + name + " = " + value->to_python() + "\n";
    }

    string repr() const override {
        return "Assign(" + name + "," + value->repr() + ")";
    }
};

struct PrintStmt : Stmt {
    ExprPtr value;

    PrintStmt(ExprPtr value) : value(move(value)) {}

    string to_python(int indent_level) const override {
        return indentation(indent_level) + "print(" + value->to_python() + ")\n";
    }

    string repr() const override {
        return "Print(" + value->repr() + ")";
    }
};

struct ReturnStmt : Stmt {
    ExprPtr value;

    ReturnStmt(ExprPtr value) : value(move(value)) {}

    string to_python(int indent_level) const override {
        return indentation(indent_level) + "return " + value->to_python() + "\n";
    }

    string repr() const override {
        return "Return(" + value->repr() + ")";
    }
};

struct ExprStmt : Stmt {
    ExprPtr value;

    ExprStmt(ExprPtr value) : value(move(value)) {}

    string to_python(int indent_level) const override {
        return indentation(indent_level) + value->to_python() + "\n";
    }

    string repr() const override {
        return "ExprStmt(" + value->repr() + ")";
    }
};

struct IfStmt : Stmt {
    ExprPtr condition;
    Block then_block;
    Block else_block;

    IfStmt(ExprPtr condition, Block then_block, Block else_block)
        : condition(move(condition)),
          then_block(move(then_block)),
          else_block(move(else_block)) {}

    string to_python(int indent_level) const override {
        string code;
        code += indentation(indent_level) + "if " + condition->to_python() + ":\n";
        code += then_block.to_python(indent_level + 1);
        code += indentation(indent_level) + "else:\n";
        code += else_block.to_python(indent_level + 1);
        return code;
    }

    string repr() const override {
        return "If(" + condition->repr() + "," + then_block.repr() + "," + else_block.repr() + ")";
    }
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    Block body;

    WhileStmt(ExprPtr condition, Block body)
        : condition(move(condition)), body(move(body)) {}

    string to_python(int indent_level) const override {
        string code;
        code += indentation(indent_level) + "while " + condition->to_python() + ":\n";
        code += body.to_python(indent_level + 1);
        return code;
    }

    string repr() const override {
        return "While(" + condition->repr() + "," + body.repr() + ")";
    }
};

struct FunctionDef {
    FunctionSignature signature;
    Block body;

    string to_python(int indent_level) const {
        string code;
        code += indentation(indent_level) + "def " + signature.name + "(";

        for (size_t i = 0; i < signature.params.size(); i++) {
            if (i > 0) {
                code += ", ";
            }

            code += signature.params[i].name + ": " + type_name(signature.params[i].type);
        }

        code += "):\n";
        code += body.to_python(indent_level + 1);
        return code;
    }

    string repr() const {
        string result = "Function(" + signature.name + ",";

        for (const Parameter& param : signature.params) {
            result += param.name + ":" + type_name(param.type) + ",";
        }

        result += "->" + type_name(signature.return_type) + "," + body.repr() + ")";
        return result;
    }
};

struct Program {
    vector<FunctionDef> functions;
    Block body;

    string to_python() const {
        string code;

        for (size_t i = 0; i < functions.size(); i++) {
            code += functions[i].to_python(0);
            code += "\n";
        }

        code += body.to_python(0);
        return code;
    }

    string repr() const {
        string result = "Program(";

        for (const FunctionDef& function : functions) {
            result += function.repr();
            result += ";";
        }

        result += body.repr() + ")";
        return result;
    }
};

string random_variable_name(mt19937& rng, const Environment& env) {
    return random_choice(rng, env.variable_pool);
}

string random_string_literal(mt19937& rng) {
    vector<string> values = {"\"alpha\"", "\"beta\"", "\"mini\"", "\"python\"", "\"phase4\""};
    return random_choice(rng, values);
}

ValueType random_value_type(mt19937& rng) {
    vector<ValueType> types = {
        ValueType::Int,
        ValueType::Bool,
        ValueType::String,
        ValueType::IntList,
        ValueType::BoolList,
        ValueType::StringList,
    };
    return random_choice(rng, types);
}

ValueType random_printable_type(mt19937& rng) {
    vector<ValueType> types = {
        ValueType::Int,
        ValueType::Bool,
        ValueType::String,
    };
    return random_choice(rng, types);
}

string random_declared_name(mt19937& rng, const Environment& env, ValueType type) {
    vector<string> names = env.names_of_type(type);
    return random_choice(rng, names);
}

FunctionSignature random_function_returning(mt19937& rng, const Environment& env, ValueType type) {
    vector<FunctionSignature> functions = env.functions_returning(type);
    return random_choice(rng, functions);
}

vector<string> arith_ops() {
    return {"+", "-", "*"};
}

vector<string> compare_ops() {
    return {"<", ">", "==", "!=", "<=", ">="};
}

vector<string> string_compare_ops() {
    return {"==", "!="};
}

vector<string> bool_ops() {
    return {"and", "or"};
}

ExprPtr generate_expression(mt19937& rng, const Environment& env, ValueType type, int depth);

ExprPtr generate_list_literal(mt19937& rng, const Environment& env, ValueType list_type, int depth) {
    vector<ExprPtr> items;
    ValueType item_type = element_type(list_type);
    int count = random_int(rng, 1, 3);

    for (int i = 0; i < count; i++) {
        items.push_back(generate_expression(rng, env, item_type, depth - 1));
    }

    return make_unique<ListExpr>(list_type, move(items));
}

ExprPtr generate_function_call(mt19937& rng, const Environment& env, ValueType return_type, int depth) {
    FunctionSignature signature = random_function_returning(rng, env, return_type);
    vector<ExprPtr> args;

    for (const Parameter& param : signature.params) {
        args.push_back(generate_expression(rng, env, param.type, depth - 1));
    }

    return make_unique<FunctionCallExpr>(return_type, signature.name, move(args));
}

ExprPtr generate_literal(mt19937& rng, const Environment& env, ValueType type, int depth) {
    if (type == ValueType::Int) {
        return make_unique<LiteralExpr>(ValueType::Int, to_string(random_int(rng, 0, 99)));
    }

    if (type == ValueType::Bool) {
        return make_unique<LiteralExpr>(ValueType::Bool, random_int(rng, 0, 1) == 1 ? "True" : "False");
    }

    if (type == ValueType::String) {
        return make_unique<LiteralExpr>(ValueType::String, random_string_literal(rng));
    }

    return generate_list_literal(rng, env, type, depth);
}

ExprPtr generate_variable_or_literal(mt19937& rng, const Environment& env, ValueType type, int depth) {
    vector<string> names = env.names_of_type(type);

    if (!names.empty() && random_int(rng, 0, 1) == 1) {
        return make_unique<VariableExpr>(type, random_declared_name(rng, env, type));
    }

    return generate_literal(rng, env, type, depth);
}

ExprPtr generate_expression(mt19937& rng, const Environment& env, ValueType type, int depth) {
    vector<string> names = env.names_of_type(type);
    vector<FunctionSignature> functions = env.functions_returning(type);

    if (depth <= 0) {
        return generate_variable_or_literal(rng, env, type, depth);
    }

    vector<int> choices = {0};

    if (!names.empty()) choices.push_back(1);
    if (!functions.empty()) choices.push_back(2);

    if (type == ValueType::Int) {
        choices.push_back(3);
        choices.push_back(4);
        choices.push_back(5);
    } else if (type == ValueType::Bool) {
        choices.push_back(6);
        choices.push_back(7);
        choices.push_back(8);
        choices.push_back(9);
    } else if (type == ValueType::String) {
        choices.push_back(10);
        choices.push_back(11);
    } else if (is_list_type(type)) {
        choices.push_back(12);
    }

    int choice = random_choice(rng, choices);

    if (choice == 0) {
        return generate_literal(rng, env, type, depth);
    }

    if (choice == 1) {
        return make_unique<VariableExpr>(type, random_declared_name(rng, env, type));
    }

    if (choice == 2) {
        return generate_function_call(rng, env, type, depth);
    }

    if (choice == 3) {
        return make_unique<BinaryExpr>(
            ValueType::Int,
            generate_expression(rng, env, ValueType::Int, depth - 1),
            random_choice(rng, arith_ops()),
            generate_expression(rng, env, ValueType::Int, depth - 1)
        );
    }

    if (choice == 4) {
        ValueType list_type = random_choice(rng, vector<ValueType>{ValueType::IntList, ValueType::BoolList, ValueType::StringList});
        return make_unique<LenExpr>(generate_expression(rng, env, list_type, depth - 1));
    }

    if (choice == 5) {
        return make_unique<IndexExpr>(
            ValueType::Int,
            generate_expression(rng, env, ValueType::IntList, depth - 1),
            make_unique<LiteralExpr>(ValueType::Int, "0")
        );
    }

    if (choice == 6) {
        return make_unique<BinaryExpr>(
            ValueType::Bool,
            generate_expression(rng, env, ValueType::Int, depth - 1),
            random_choice(rng, compare_ops()),
            generate_expression(rng, env, ValueType::Int, depth - 1)
        );
    }

    if (choice == 7) {
        return make_unique<BinaryExpr>(
            ValueType::Bool,
            generate_expression(rng, env, ValueType::String, depth - 1),
            random_choice(rng, string_compare_ops()),
            generate_expression(rng, env, ValueType::String, depth - 1)
        );
    }

    if (choice == 8) {
        return make_unique<UnaryExpr>(
            ValueType::Bool,
            "not",
            generate_expression(rng, env, ValueType::Bool, depth - 1)
        );
    }

    if (choice == 9) {
        return make_unique<BinaryExpr>(
            ValueType::Bool,
            generate_expression(rng, env, ValueType::Bool, depth - 1),
            random_choice(rng, bool_ops()),
            generate_expression(rng, env, ValueType::Bool, depth - 1)
        );
    }

    if (choice == 10) {
        return make_unique<BinaryExpr>(
            ValueType::String,
            generate_expression(rng, env, ValueType::String, depth - 1),
            "+",
            generate_expression(rng, env, ValueType::String, depth - 1)
        );
    }

    if (choice == 11) {
        return make_unique<IndexExpr>(
            ValueType::String,
            generate_expression(rng, env, ValueType::StringList, depth - 1),
            make_unique<LiteralExpr>(ValueType::Int, "0")
        );
    }

    return generate_list_literal(rng, env, type, depth);
}

Block generate_block(mt19937& rng, Environment env, int statement_count, int expression_depth, int control_depth, bool allow_return, ValueType return_type);

StmtPtr generate_assignment(mt19937& rng, Environment& env, int expression_depth) {
    ValueType type = random_value_type(rng);
    string name = random_variable_name(rng, env);
    ExprPtr value = generate_expression(rng, env, type, expression_depth);
    env.declare_or_update(name, type);
    return make_unique<AssignStmt>(name, move(value));
}

StmtPtr generate_print(mt19937& rng, const Environment& env, int expression_depth) {
    ValueType type = random_printable_type(rng);
    return make_unique<PrintStmt>(generate_expression(rng, env, type, expression_depth));
}

StmtPtr generate_function_expr_stmt(mt19937& rng, const Environment& env, int expression_depth) {
    if (env.functions.empty()) {
        return generate_print(rng, env, expression_depth);
    }

    FunctionSignature signature = random_choice(rng, env.functions);
    vector<ExprPtr> args;

    for (const Parameter& param : signature.params) {
        args.push_back(generate_expression(rng, env, param.type, expression_depth - 1));
    }

    return make_unique<ExprStmt>(
        make_unique<FunctionCallExpr>(signature.return_type, signature.name, move(args))
    );
}

StmtPtr generate_statement(mt19937& rng, Environment& env, int expression_depth, int control_depth, bool allow_return, ValueType return_type) {
    vector<int> choices = {0, 1, 2};

    if (control_depth > 0) {
        choices.push_back(3);
        choices.push_back(4);
    }

    if (allow_return) {
        choices.push_back(5);
    }

    int choice = random_choice(rng, choices);

    if (choice == 0) {
        return generate_assignment(rng, env, expression_depth);
    }

    if (choice == 1) {
        return generate_print(rng, env, expression_depth);
    }

    if (choice == 2) {
        return generate_function_expr_stmt(rng, env, expression_depth);
    }

    if (choice == 3) {
        Environment then_env = env;
        Environment else_env = env;
        Block then_block = generate_block(rng, then_env, random_int(rng, 1, 3), expression_depth, control_depth - 1, false, return_type);
        Block else_block = generate_block(rng, else_env, random_int(rng, 1, 3), expression_depth, control_depth - 1, false, return_type);

        return make_unique<IfStmt>(
            generate_expression(rng, env, ValueType::Bool, expression_depth),
            move(then_block),
            move(else_block)
        );
    }

    if (choice == 4) {
        Environment body_env = env;
        Block body = generate_block(rng, body_env, random_int(rng, 1, 3), expression_depth, control_depth - 1, false, return_type);

        return make_unique<WhileStmt>(
            generate_expression(rng, env, ValueType::Bool, expression_depth),
            move(body)
        );
    }

    return make_unique<ReturnStmt>(generate_expression(rng, env, return_type, expression_depth));
}

Block generate_block(mt19937& rng, Environment env, int statement_count, int expression_depth, int control_depth, bool allow_return, ValueType return_type) {
    Block block;

    for (int i = 0; i < statement_count; i++) {
        block.add_statement(generate_statement(rng, env, expression_depth, control_depth, false, return_type));
    }

    if (allow_return) {
        block.add_statement(make_unique<ReturnStmt>(generate_expression(rng, env, return_type, expression_depth)));
    }

    return block;
}

vector<Parameter> generate_params(mt19937& rng) {
    vector<Parameter> params;
    vector<string> names = {"p", "q", "r"};
    int count = random_int(rng, 0, 2);

    for (int i = 0; i < count; i++) {
        params.push_back(Parameter{names[i], random_value_type(rng)});
    }

    return params;
}

FunctionDef generate_function(mt19937& rng, Environment global_env, const FunctionSignature& signature, int expression_depth, int control_depth) {
    Environment local_env = global_env;

    for (const Parameter& param : signature.params) {
        local_env.declare_or_update(param.name, param.type);
    }

    Block body = generate_block(
        rng,
        local_env,
        random_int(rng, 1, 3),
        expression_depth,
        control_depth,
        true,
        signature.return_type
    );

    return FunctionDef{signature, move(body)};
}

Program generate_program(mt19937& rng, int function_count, int statement_count, int expression_depth, int control_depth) {
    Program program;
    vector<FunctionSignature> signatures;

    for (int i = 0; i < function_count; i++) {
        FunctionSignature signature;
        signature.name = "f" + to_string(i);
        signature.params = generate_params(rng);
        signature.return_type = random_value_type(rng);
        signatures.push_back(signature);
    }

    Environment function_env;

    for (const FunctionSignature& signature : signatures) {
        program.functions.push_back(generate_function(rng, function_env, signature, expression_depth, control_depth));
        function_env.functions.push_back(signature);
    }

    Environment top_level_env;
    top_level_env.functions = signatures;

    program.body = generate_block(
        rng,
        top_level_env,
        statement_count,
        expression_depth,
        control_depth,
        false,
        ValueType::Int
    );

    return program;
}

struct Options {
    int seed = 91;
    int functions = 2;
    int statements = 10;
    int expression_depth = 3;
    int control_depth = 2;
};

int parse_int_arg(const string& flag, const string& value) {
    try {
        return stoi(value);
    } catch (const exception&) {
        throw runtime_error("invalid integer for " + flag + ": " + value);
    }
}

Options parse_options(int argc, char* argv[]) {
    Options options;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "--seed" && i + 1 < argc) {
            options.seed = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--functions" && i + 1 < argc) {
            options.functions = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--statements" && i + 1 < argc) {
            options.statements = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--expr-depth" && i + 1 < argc) {
            options.expression_depth = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--control-depth" && i + 1 < argc) {
            options.control_depth = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--help") {
            cout << "Usage: ./phase4_ast [--seed N] [--functions N] [--statements N] [--expr-depth N] [--control-depth N]\n";
            exit(0);
        } else {
            throw runtime_error("unknown or incomplete argument: " + arg);
        }
    }

    if (options.functions < 0) throw runtime_error("--functions must be at least 0");
    if (options.statements < 1) throw runtime_error("--statements must be at least 1");
    if (options.expression_depth < 0) throw runtime_error("--expr-depth must be at least 0");
    if (options.control_depth < 0) throw runtime_error("--control-depth must be at least 0");

    return options;
}

int main(int argc, char* argv[]) {
    Options options = parse_options(argc, argv);
    mt19937 rng(options.seed);

    Program program = generate_program(
        rng,
        options.functions,
        options.statements,
        options.expression_depth,
        options.control_depth
    );

    cerr << "[seed: " << options.seed << "]\n";
    cerr << "[functions: " << options.functions << "]\n";
    cerr << "[statements: " << options.statements << "]\n";

    cout << program.to_python();
    return 0;
}
