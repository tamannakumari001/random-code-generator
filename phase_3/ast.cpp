#include <cctype>
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
};

string type_name(ValueType type) {
    if (type == ValueType::Int) {
        return "int";
    }

    return "bool";
}

struct Symbol {
    string name;
    ValueType type;
};

struct Environment {
    vector<string> names;
    vector<Symbol> declared;

    Environment() : names({"a", "b", "c", "x", "y", "z"}) {}

    void declare_or_update(const string& name, ValueType type) {
        for (Symbol& symbol : declared) {
            if (symbol.name == name) {
                symbol.type = type;
                return;
            }
        }

        declared.push_back(Symbol{name, type});
    }

    vector<string> names_of_type(ValueType type) const {
        vector<string> result;

        for (const Symbol& symbol : declared) {
            if (symbol.type == type) {
                result.push_back(symbol.name);
            }
        }

        return result;
    }
};

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
const T& random_choice(mt19937& rng, const vector<T>& items) {
    if (items.empty()) {
        throw runtime_error("random_choice called with empty vector");
    }

    return items[random_int(rng, 0, static_cast<int>(items.size()) - 1)];
}

struct IntExpr {
    virtual ~IntExpr() = default;
    virtual string to_python() const = 0;
    virtual string repr() const = 0;
};

using IntExprPtr = unique_ptr<IntExpr>;

struct IntLiteral : IntExpr {
    int value;

    IntLiteral(int value) : value(value) {}

    string to_python() const override {
        return to_string(value);
    }

    string repr() const override {
        return "Int(" + to_string(value) + ")";
    }
};

struct IntVariable : IntExpr {
    string name;

    IntVariable(string name) : name(move(name)) {}

    string to_python() const override {
        return name;
    }

    string repr() const override {
        return "IntVar(" + name + ")";
    }
};

struct ArithBinaryOp : IntExpr {
    IntExprPtr left;
    string op;
    IntExprPtr right;

    ArithBinaryOp(IntExprPtr left, string op, IntExprPtr right)
        : left(move(left)), op(move(op)), right(move(right)) {}

    string to_python() const override {
        return "(" + left->to_python() + " " + op + " " + right->to_python() + ")";
    }

    string repr() const override {
        return "Arith(" + op + "," + left->repr() + "," + right->repr() + ")";
    }
};

struct BoolExpr {
    virtual ~BoolExpr() = default;
    virtual string to_python() const = 0;
    virtual string repr() const = 0;
};

using BoolExprPtr = unique_ptr<BoolExpr>;

struct BoolLiteral : BoolExpr {
    bool value;

    BoolLiteral(bool value) : value(value) {}

    string to_python() const override {
        return value ? "True" : "False";
    }

    string repr() const override {
        return value ? "Bool(True)" : "Bool(False)";
    }
};

struct BoolVariable : BoolExpr {
    string name;

    BoolVariable(string name) : name(move(name)) {}

    string to_python() const override {
        return name;
    }

    string repr() const override {
        return "BoolVar(" + name + ")";
    }
};

struct Comparison : BoolExpr {
    IntExprPtr left;
    string op;
    IntExprPtr right;

    Comparison(IntExprPtr left, string op, IntExprPtr right)
        : left(move(left)), op(move(op)), right(move(right)) {}

    string to_python() const override {
        return "(" + left->to_python() + " " + op + " " + right->to_python() + ")";
    }

    string repr() const override {
        return "Compare(" + op + "," + left->repr() + "," + right->repr() + ")";
    }
};

struct BoolNot : BoolExpr {
    BoolExprPtr value;

    BoolNot(BoolExprPtr value) : value(move(value)) {}

    string to_python() const override {
        return "(not " + value->to_python() + ")";
    }

    string repr() const override {
        return "Not(" + value->repr() + ")";
    }
};

struct BoolBinaryOp : BoolExpr {
    BoolExprPtr left;
    string op;
    BoolExprPtr right;

    BoolBinaryOp(BoolExprPtr left, string op, BoolExprPtr right)
        : left(move(left)), op(move(op)), right(move(right)) {}

    string to_python() const override {
        return "(" + left->to_python() + " " + op + " " + right->to_python() + ")";
    }

    string repr() const override {
        return "BoolOp(" + op + "," + left->repr() + "," + right->repr() + ")";
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

struct AssignInt : Stmt {
    string name;
    IntExprPtr value;

    AssignInt(string name, IntExprPtr value)
        : name(move(name)), value(move(value)) {}

    string to_python(int indent_level) const override {
        return indentation(indent_level) + name + " = " + value->to_python() + "\n";
    }

    string repr() const override {
        return "AssignInt(" + name + "," + value->repr() + ")";
    }
};

struct AssignBool : Stmt {
    string name;
    BoolExprPtr value;

    AssignBool(string name, BoolExprPtr value)
        : name(move(name)), value(move(value)) {}

    string to_python(int indent_level) const override {
        return indentation(indent_level) + name + " = " + value->to_python() + "\n";
    }

    string repr() const override {
        return "AssignBool(" + name + "," + value->repr() + ")";
    }
};

struct PrintInt : Stmt {
    IntExprPtr value;

    PrintInt(IntExprPtr value) : value(move(value)) {}

    string to_python(int indent_level) const override {
        return indentation(indent_level) + "print(" + value->to_python() + ")\n";
    }

    string repr() const override {
        return "PrintInt(" + value->repr() + ")";
    }
};

struct IfStmt : Stmt {
    BoolExprPtr condition;
    Block then_block;
    Block else_block;

    IfStmt(BoolExprPtr condition, Block then_block, Block else_block)
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
    BoolExprPtr condition;
    Block body;

    WhileStmt(BoolExprPtr condition, Block body)
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

struct Program {
    Block body;

    string to_python() const {
        return body.to_python(0);
    }

    string repr() const {
        return "Program(" + body.repr() + ")";
    }
};

string random_variable_name(mt19937& rng, const Environment& env) {
    return random_choice(rng, env.names);
}

string random_declared_name(mt19937& rng, const Environment& env, ValueType type) {
    vector<string> names = env.names_of_type(type);
    return random_choice(rng, names);
}

string random_arith_operator(mt19937& rng) {
    vector<string> operators = {"+", "-", "*"};
    return random_choice(rng, operators);
}

string random_compare_operator(mt19937& rng) {
    vector<string> operators = {"<", ">", "==", "!=", "<=", ">="};
    return random_choice(rng, operators);
}

string random_bool_operator(mt19937& rng) {
    vector<string> operators = {"and", "or"};
    return random_choice(rng, operators);
}

IntExprPtr generate_int_expression(mt19937& rng, const Environment& env, int depth) {
    vector<string> int_names = env.names_of_type(ValueType::Int);

    if (depth <= 0) {
        if (!int_names.empty() && random_int(rng, 0, 1) == 1) {
            return make_unique<IntVariable>(random_declared_name(rng, env, ValueType::Int));
        }

        return make_unique<IntLiteral>(random_int(rng, 0, 99));
    }

    int max_choice = int_names.empty() ? 1 : 2;
    int choice = random_int(rng, 0, max_choice);

    if (choice == 0) {
        return make_unique<IntLiteral>(random_int(rng, 0, 99));
    }

    if (choice == 1 && !int_names.empty()) {
        return make_unique<IntVariable>(random_declared_name(rng, env, ValueType::Int));
    }

    return make_unique<ArithBinaryOp>(
        generate_int_expression(rng, env, depth - 1),
        random_arith_operator(rng),
        generate_int_expression(rng, env, depth - 1)
    );
}

BoolExprPtr generate_bool_expression(mt19937& rng, const Environment& env, int depth) {
    if (depth <= 0) {
        return make_unique<BoolLiteral>(random_int(rng, 0, 1) == 1);
    }

    int choice = random_int(rng, 0, 3);

    if (choice == 0) {
        return make_unique<BoolLiteral>(random_int(rng, 0, 1) == 1);
    }

    if (choice == 1) {
        return make_unique<Comparison>(
            generate_int_expression(rng, env, depth - 1),
            random_compare_operator(rng),
            generate_int_expression(rng, env, depth - 1)
        );
    }

    if (choice == 2) {
        return make_unique<BoolNot>(generate_bool_expression(rng, env, depth - 1));
    }

    return make_unique<BoolBinaryOp>(
        generate_bool_expression(rng, env, depth - 1),
        random_bool_operator(rng),
        generate_bool_expression(rng, env, depth - 1)
    );
}

Block generate_block(mt19937& rng, Environment env, int statement_count, int expression_depth, int control_depth);

StmtPtr generate_statement(mt19937& rng, Environment& env, int expression_depth, int control_depth) {
    int max_choice = control_depth > 0 ? 4 : 2;
    int choice = random_int(rng, 0, max_choice);

    if (choice == 0) {
        string name = random_variable_name(rng, env);
        IntExprPtr value = generate_int_expression(rng, env, expression_depth);
        env.declare_or_update(name, ValueType::Int);
        return make_unique<AssignInt>(name, move(value));
    }

    if (choice == 1) {
        string name = random_variable_name(rng, env);
        BoolExprPtr value = generate_bool_expression(rng, env, expression_depth);
        env.declare_or_update(name, ValueType::Bool);
        return make_unique<AssignBool>(name, move(value));
    }

    if (choice == 2) {
        return make_unique<PrintInt>(generate_int_expression(rng, env, expression_depth));
    }

    if (choice == 3) {
        Environment then_env = env;
        Environment else_env = env;
        Block then_block = generate_block(rng, then_env, random_int(rng, 1, 3), expression_depth, control_depth - 1);
        Block else_block = generate_block(rng, else_env, random_int(rng, 1, 3), expression_depth, control_depth - 1);

        return make_unique<IfStmt>(
            generate_bool_expression(rng, env, expression_depth),
            move(then_block),
            move(else_block)
        );
    }

    Environment body_env = env;
    Block body = generate_block(rng, body_env, random_int(rng, 1, 3), expression_depth, control_depth - 1);

    return make_unique<WhileStmt>(
        make_unique<BoolLiteral>(false),
        move(body)
    );
}

Block generate_block(mt19937& rng, Environment env, int statement_count, int expression_depth, int control_depth) {
    Block block;

    for (int i = 0; i < statement_count; i++) {
        block.add_statement(generate_statement(rng, env, expression_depth, control_depth));
    }

    return block;
}

Program generate_program(mt19937& rng, int statement_count, int expression_depth, int control_depth) {
    Environment env;
    Program program;
    program.body = generate_block(rng, env, statement_count, expression_depth, control_depth);
    return program;
}

enum class TokenType {
    Name,
    Int,
    Newline,
    Indent,
    Dedent,
    End,
    KwIf,
    KwElse,
    KwWhile,
    KwPrint,
    KwTrue,
    KwFalse,
    KwNot,
    KwAnd,
    KwOr,
    Equals,
    Plus,
    Minus,
    Star,
    LParen,
    RParen,
    Colon,
    Less,
    Greater,
    EqualEqual,
    BangEqual,
    LessEqual,
    GreaterEqual,
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
};

Token make_token(TokenType type, string lexeme, int line) {
    return Token{type, move(lexeme), line};
}

bool is_name_start(char c) {
    return isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool is_name_char(char c) {
    return isalnum(static_cast<unsigned char>(c)) || c == '_';
}

TokenType keyword_or_name(const string& text) {
    if (text == "if") return TokenType::KwIf;
    if (text == "else") return TokenType::KwElse;
    if (text == "while") return TokenType::KwWhile;
    if (text == "print") return TokenType::KwPrint;
    if (text == "True") return TokenType::KwTrue;
    if (text == "False") return TokenType::KwFalse;
    if (text == "not") return TokenType::KwNot;
    if (text == "and") return TokenType::KwAnd;
    if (text == "or") return TokenType::KwOr;
    return TokenType::Name;
}

vector<Token> lex(const string& source) {
    vector<Token> tokens;
    vector<int> indent_stack = {0};
    bool at_line_start = true;
    int line = 1;
    size_t i = 0;

    while (i < source.size()) {
        if (at_line_start) {
            int spaces = 0;

            while (i < source.size() && source[i] == ' ') {
                spaces++;
                i++;
            }

            if (i < source.size() && source[i] == '\n') {
                tokens.push_back(make_token(TokenType::Newline, "\\n", line));
                i++;
                line++;
                continue;
            }

            int current_indent = indent_stack.back();

            if (spaces > current_indent) {
                indent_stack.push_back(spaces);
                tokens.push_back(make_token(TokenType::Indent, "INDENT", line));
            } else {
                while (spaces < indent_stack.back()) {
                    indent_stack.pop_back();
                    tokens.push_back(make_token(TokenType::Dedent, "DEDENT", line));
                }

                if (spaces != indent_stack.back()) {
                    throw runtime_error("inconsistent indentation on line " + to_string(line));
                }
            }

            at_line_start = false;
        }

        char c = source[i];

        if (c == ' ' || c == '\t') {
            i++;
            continue;
        }

        if (c == '\n') {
            tokens.push_back(make_token(TokenType::Newline, "\\n", line));
            i++;
            line++;
            at_line_start = true;
            continue;
        }

        if (isdigit(static_cast<unsigned char>(c))) {
            size_t start = i;

            while (i < source.size() && isdigit(static_cast<unsigned char>(source[i]))) {
                i++;
            }

            tokens.push_back(make_token(TokenType::Int, source.substr(start, i - start), line));
            continue;
        }

        if (is_name_start(c)) {
            size_t start = i;

            while (i < source.size() && is_name_char(source[i])) {
                i++;
            }

            string text = source.substr(start, i - start);
            tokens.push_back(make_token(keyword_or_name(text), text, line));
            continue;
        }

        if (i + 1 < source.size()) {
            string two = source.substr(i, 2);

            if (two == "==") {
                tokens.push_back(make_token(TokenType::EqualEqual, two, line));
                i += 2;
                continue;
            }

            if (two == "!=") {
                tokens.push_back(make_token(TokenType::BangEqual, two, line));
                i += 2;
                continue;
            }

            if (two == "<=") {
                tokens.push_back(make_token(TokenType::LessEqual, two, line));
                i += 2;
                continue;
            }

            if (two == ">=") {
                tokens.push_back(make_token(TokenType::GreaterEqual, two, line));
                i += 2;
                continue;
            }
        }

        if (c == '=') tokens.push_back(make_token(TokenType::Equals, "=", line));
        else if (c == '+') tokens.push_back(make_token(TokenType::Plus, "+", line));
        else if (c == '-') tokens.push_back(make_token(TokenType::Minus, "-", line));
        else if (c == '*') tokens.push_back(make_token(TokenType::Star, "*", line));
        else if (c == '(') tokens.push_back(make_token(TokenType::LParen, "(", line));
        else if (c == ')') tokens.push_back(make_token(TokenType::RParen, ")", line));
        else if (c == ':') tokens.push_back(make_token(TokenType::Colon, ":", line));
        else if (c == '<') tokens.push_back(make_token(TokenType::Less, "<", line));
        else if (c == '>') tokens.push_back(make_token(TokenType::Greater, ">", line));
        else throw runtime_error("unexpected character on line " + to_string(line) + ": " + string(1, c));

        i++;
    }

    if (!tokens.empty() && tokens.back().type != TokenType::Newline) {
        tokens.push_back(make_token(TokenType::Newline, "\\n", line));
    }

    while (indent_stack.size() > 1) {
        indent_stack.pop_back();
        tokens.push_back(make_token(TokenType::Dedent, "DEDENT", line));
    }

    tokens.push_back(make_token(TokenType::End, "EOF", line));
    return tokens;
}

bool is_compare_token(TokenType type) {
    return type == TokenType::Less
        || type == TokenType::Greater
        || type == TokenType::EqualEqual
        || type == TokenType::BangEqual
        || type == TokenType::LessEqual
        || type == TokenType::GreaterEqual;
}

bool is_arith_token(TokenType type) {
    return type == TokenType::Plus
        || type == TokenType::Minus
        || type == TokenType::Star;
}

bool is_bool_operator(TokenType type) {
    return type == TokenType::KwAnd || type == TokenType::KwOr;
}

class Parser {
public:
    Parser(vector<Token> tokens) : tokens(move(tokens)) {}

    Program parse_program() {
        Program program;

        while (!check(TokenType::End)) {
            skip_newlines();

            if (check(TokenType::End)) {
                break;
            }

            program.body.add_statement(parse_statement());
            consume_optional_newline();
        }

        expect(TokenType::End, "expected EOF");
        return program;
    }

private:
    const vector<Token> tokens;
    size_t current = 0;

    const Token& peek() const {
        return tokens[current];
    }

    const Token& previous() const {
        return tokens[current - 1];
    }

    bool check(TokenType type) const {
        return peek().type == type;
    }

    bool match(TokenType type) {
        if (!check(type)) {
            return false;
        }

        current++;
        return true;
    }

    const Token& expect(TokenType type, const string& message) {
        if (!check(type)) {
            throw runtime_error(message + " on line " + to_string(peek().line));
        }

        current++;
        return previous();
    }

    void skip_newlines() {
        while (match(TokenType::Newline)) {}
    }

    void consume_optional_newline() {
        match(TokenType::Newline);
    }

    StmtPtr parse_statement() {
        if (match(TokenType::KwIf)) {
            return parse_if_statement();
        }

        if (match(TokenType::KwWhile)) {
            return parse_while_statement();
        }

        if (match(TokenType::KwPrint)) {
            return parse_print_statement();
        }

        if (check(TokenType::Name)) {
            return parse_assignment();
        }

        throw runtime_error("expected statement on line " + to_string(peek().line));
    }

    StmtPtr parse_if_statement() {
        BoolExprPtr condition = parse_bool_expression();
        expect(TokenType::Colon, "expected ':' after if condition");
        Block then_block = parse_block();
        expect(TokenType::KwElse, "expected else block");
        expect(TokenType::Colon, "expected ':' after else");
        Block else_block = parse_block();
        return make_unique<IfStmt>(move(condition), move(then_block), move(else_block));
    }

    StmtPtr parse_while_statement() {
        BoolExprPtr condition = parse_bool_expression();
        expect(TokenType::Colon, "expected ':' after while condition");
        Block body = parse_block();
        return make_unique<WhileStmt>(move(condition), move(body));
    }

    StmtPtr parse_print_statement() {
        expect(TokenType::LParen, "expected '(' after print");
        IntExprPtr value = parse_int_expression();
        expect(TokenType::RParen, "expected ')' after print argument");
        return make_unique<PrintInt>(move(value));
    }

    StmtPtr parse_assignment() {
        string name = expect(TokenType::Name, "expected assignment target").lexeme;
        expect(TokenType::Equals, "expected '=' in assignment");

        if (next_value_is_bool()) {
            return make_unique<AssignBool>(name, parse_bool_expression());
        }

        return make_unique<AssignInt>(name, parse_int_expression());
    }

    Block parse_block() {
        expect(TokenType::Newline, "expected newline before block");
        expect(TokenType::Indent, "expected indented block");

        Block block;

        while (!check(TokenType::Dedent) && !check(TokenType::End)) {
            skip_newlines();

            if (check(TokenType::Dedent)) {
                break;
            }

            block.add_statement(parse_statement());
            consume_optional_newline();
        }

        expect(TokenType::Dedent, "expected end of indented block");
        return block;
    }

    bool next_value_is_bool() const {
        int depth = 0;

        for (size_t i = current; i < tokens.size(); i++) {
            TokenType type = tokens[i].type;

            if (type == TokenType::Newline || type == TokenType::Dedent || type == TokenType::End) {
                return false;
            }

            if (type == TokenType::LParen) depth++;
            else if (type == TokenType::RParen) depth--;

            if (type == TokenType::KwTrue
                || type == TokenType::KwFalse
                || type == TokenType::KwNot
                || type == TokenType::KwAnd
                || type == TokenType::KwOr
                || is_compare_token(type)) {
                return true;
            }
        }

        return false;
    }

    string consume_arith_operator() {
        if (match(TokenType::Plus)) return "+";
        if (match(TokenType::Minus)) return "-";
        if (match(TokenType::Star)) return "*";
        throw runtime_error("expected arithmetic operator on line " + to_string(peek().line));
    }

    string consume_compare_operator() {
        if (match(TokenType::Less)) return "<";
        if (match(TokenType::Greater)) return ">";
        if (match(TokenType::EqualEqual)) return "==";
        if (match(TokenType::BangEqual)) return "!=";
        if (match(TokenType::LessEqual)) return "<=";
        if (match(TokenType::GreaterEqual)) return ">=";
        throw runtime_error("expected comparison operator on line " + to_string(peek().line));
    }

    string consume_bool_operator() {
        if (match(TokenType::KwAnd)) return "and";
        if (match(TokenType::KwOr)) return "or";
        throw runtime_error("expected boolean operator on line " + to_string(peek().line));
    }

    IntExprPtr parse_int_expression() {
        if (match(TokenType::Int)) {
            return make_unique<IntLiteral>(stoi(previous().lexeme));
        }

        if (match(TokenType::Name)) {
            return make_unique<IntVariable>(previous().lexeme);
        }

        if (match(TokenType::LParen)) {
            IntExprPtr left = parse_int_expression();
            string op = consume_arith_operator();
            IntExprPtr right = parse_int_expression();
            expect(TokenType::RParen, "expected ')' after arithmetic expression");
            return make_unique<ArithBinaryOp>(move(left), op, move(right));
        }

        throw runtime_error("expected integer expression on line " + to_string(peek().line));
    }

    string parenthesized_bool_shape() const {
        if (!check(TokenType::LParen)) {
            return "";
        }

        int depth = 0;

        for (size_t i = current + 1; i < tokens.size(); i++) {
            TokenType type = tokens[i].type;

            if (type == TokenType::LParen) {
                depth++;
                continue;
            }

            if (type == TokenType::RParen) {
                if (depth == 0) {
                    return "";
                }

                depth--;
                continue;
            }

            if (depth == 0 && is_bool_operator(type)) {
                return "boolop";
            }

            if (depth == 0 && is_compare_token(type)) {
                return "comparison";
            }
        }

        return "";
    }

    BoolExprPtr parse_bool_expression() {
        if (match(TokenType::KwTrue)) {
            return make_unique<BoolLiteral>(true);
        }

        if (match(TokenType::KwFalse)) {
            return make_unique<BoolLiteral>(false);
        }

        if (match(TokenType::Name)) {
            return make_unique<BoolVariable>(previous().lexeme);
        }

        if (match(TokenType::KwNot)) {
            return make_unique<BoolNot>(parse_bool_expression());
        }

        if (check(TokenType::LParen)) {
            string shape = parenthesized_bool_shape();
            expect(TokenType::LParen, "expected '('");

            if (shape == "comparison") {
                IntExprPtr left = parse_int_expression();
                string op = consume_compare_operator();
                IntExprPtr right = parse_int_expression();
                expect(TokenType::RParen, "expected ')' after comparison");
                return make_unique<Comparison>(move(left), op, move(right));
            }

            if (shape == "boolop") {
                BoolExprPtr left = parse_bool_expression();
                string op = consume_bool_operator();
                BoolExprPtr right = parse_bool_expression();
                expect(TokenType::RParen, "expected ')' after boolean expression");
                return make_unique<BoolBinaryOp>(move(left), op, move(right));
            }

            if (match(TokenType::KwNot)) {
                BoolExprPtr value = parse_bool_expression();
                expect(TokenType::RParen, "expected ')' after not expression");
                return make_unique<BoolNot>(move(value));
            }

            BoolExprPtr value = parse_bool_expression();
            expect(TokenType::RParen, "expected ')' after parenthesized boolean expression");
            return value;
        }

        throw runtime_error("expected boolean expression on line " + to_string(peek().line));
    }
};

struct Options {
    int seed = 73;
    int statement_count = 8;
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
        } else if (arg == "--statements" && i + 1 < argc) {
            options.statement_count = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--expr-depth" && i + 1 < argc) {
            options.expression_depth = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--control-depth" && i + 1 < argc) {
            options.control_depth = parse_int_arg(arg, argv[++i]);
        } else if (arg == "--help") {
            cout << "Usage: ./phase3_ast [--seed N] [--statements N] [--expr-depth N] [--control-depth N]\n";
            exit(0);
        } else {
            throw runtime_error("unknown or incomplete argument: " + arg);
        }
    }

    if (options.statement_count < 1) {
        throw runtime_error("--statements must be at least 1");
    }

    if (options.expression_depth < 0) {
        throw runtime_error("--expr-depth must be at least 0");
    }

    if (options.control_depth < 0) {
        throw runtime_error("--control-depth must be at least 0");
    }

    return options;
}

int main(int argc, char* argv[]) {
    Options options = parse_options(argc, argv);

    mt19937 rng(options.seed);
    Program generated = generate_program(
        rng,
        options.statement_count,
        options.expression_depth,
        options.control_depth
    );
    string source = generated.to_python();

    vector<Token> tokens = lex(source);
    Parser parser(move(tokens));
    Program parsed = parser.parse_program();

    if (generated.repr() != parsed.repr()) {
        cerr << "[roundtrip: bad]\n";
        cerr << "generated: " << generated.repr() << "\n";
        cerr << "parsed:    " << parsed.repr() << "\n";
        return 1;
    }

    cerr << "[seed: " << options.seed << "]\n";
    cerr << "[roundtrip: ok]\n";
    cout << source;

    return 0;
}
