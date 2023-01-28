#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"
extern Token *tokens_end;
extern Token *tokens;

Expr *numberexpr(int value) {
    Expr *numberexp = calloc(1, sizeof(Expr));
    numberexp->value = value;
    numberexp->expr_kind = EK_Number;
    return numberexp;
}

Expr *binaryExpr(Expr *first_child, Expr *second_child, BinaryOperation binaryop) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = EK_BinaryOperator;
    newexp->op = binaryop;
    newexp->second_child = second_child;
    return newexp;
}

Expr *unaryExpr(Expr *first_child, UnaryOperation unaryop) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = EK_UnaryOperator;
    newexp->op = unaryop;
    return newexp;
}

int maybe_consume(TokenKind kind) {
    if (tokens->kind == kind) {
        tokens += 1;
        return 1;
    }
    return 0;
}

char *decode_kind(int kind) {
    void *r = &kind;
    char *q = r;
    char *ans = calloc(5, sizeof(char));
    strncpy(ans, q, 4);
    return ans;
}

void consume_otherwise_panic(int kind) {
    if (!maybe_consume(kind)) {
        fprintf(stderr, "expected TokenKind `%s`, got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens->kind));
        exit(1);
    }
}

void expect_otherwise_panic(int kind) {
    if (tokens->kind != kind) {
        fprintf(stderr, "expected TokenKind `%s`, got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens->kind));
        exit(1);
    }
}

void panic_if_eof() {
    if (tokens >= tokens_end) {
        fprintf(stderr, "EOF encountered");
        exit(1);
    }
}

Expr *parsePrimary() {
    panic_if_eof();
    if (tokens->kind == enum3('N', 'U', 'M')) {
        int value = tokens->value;
        tokens += 1;
        return numberexpr(value);
    } else if (tokens->kind == enum4('I', 'D', 'N', 'T')) {
        char *name = tokens->identifier_name;
        tokens += 1;
        if (maybe_consume('(')) {
            Expr **arguments = calloc(6, sizeof(Expr *));
            if (maybe_consume(')')) {
                Expr *callexp = calloc(1, sizeof(Expr));
                callexp->name = name;
                callexp->expr_kind = EK_Call;
                callexp->func_args = arguments;
                callexp->func_arg_len = 0;
                return callexp;
            }

            int i = 0;
            for (; i < 6; i++) {
                Expr *expr = parseExpr();
                if (maybe_consume(')')) {
                    arguments[i] = expr;
                    break;
                }
                consume_otherwise_panic(',');
                arguments[i] = expr;
            }
            Expr *callexp = calloc(1, sizeof(Expr));
            callexp->name = name;
            callexp->expr_kind = EK_Call;
            callexp->func_args = arguments;
            callexp->func_arg_len = i + 1;
            return callexp;
        } else {
            Expr *numberexp = calloc(1, sizeof(Expr));
            numberexp->name = name;
            numberexp->expr_kind = EK_Identifier;
            return numberexp;
        }
    }

    consume_otherwise_panic('(');
    Expr *expr = parseExpr();
    consume_otherwise_panic(')');
    return expr;
}

Expr *parseUnary() {
    panic_if_eof();
    if (maybe_consume('+')) {
        return parsePrimary();
    }
    if (maybe_consume('-')) {
        return binaryExpr(numberexpr(0), parsePrimary(), '-');
    }
    if (maybe_consume('*')) {
        return unaryExpr(parsePrimary(), '*');
    }
    if (maybe_consume('&')) {
        return unaryExpr(parsePrimary(), '&');
    }
    return parsePrimary();
}

Expr *parseMultiplicative() {
    panic_if_eof();
    Expr *result = parseUnary();
    while (tokens < tokens_end) {
        if (tokens->kind == enum3('N', 'U', 'M')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (maybe_consume('*')) {
            result = binaryExpr(result, parseUnary(), '*');
        } else if (maybe_consume('/')) {
            result = binaryExpr(result, parseUnary(), '/');
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseAdditive() {
    panic_if_eof();
    Expr *result = parseMultiplicative();
    while (tokens < tokens_end) {
        if (tokens->kind == enum3('N', 'U', 'M')) {
            fprintf(stderr, "Expected operator, got Number");
            exit(1);
        } else if (maybe_consume('-')) {
            result = binaryExpr(result, parseMultiplicative(), '-');
        } else if (maybe_consume('+')) {
            result = binaryExpr(result, parseMultiplicative(), '+');
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseRelational() {
    panic_if_eof();
    Expr *result = parseAdditive();
    while (tokens < tokens_end) {
        if (maybe_consume('>')) {
            result = binaryExpr(result, parseAdditive(), '>');
        } else if (maybe_consume(enum2('>', '='))) {
            result = binaryExpr(result, parseAdditive(), enum2('>', '='));
        } else if (maybe_consume('<')) {
            result = binaryExpr(parseAdditive(), result, '>');  // children & operator swapped
        } else if (maybe_consume(enum2('<', '='))) {
            result = binaryExpr(parseAdditive(), result, enum2('>', '='));  // children & operator swapped
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseEquality() {
    panic_if_eof();
    Expr *result = parseRelational();
    while (tokens < tokens_end) {
        if (maybe_consume(enum2('=', '='))) {
            result = binaryExpr(result, parseRelational(), enum2('=', '='));
        } else if (maybe_consume(enum2('!', '='))) {
            result = binaryExpr(result, parseRelational(), enum2('!', '='));
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseAssign() {
    panic_if_eof();
    Expr *result = parseEquality();
    if (maybe_consume('=')) {
        return binaryExpr(result, parseAssign(), '=');
    }
    return result;
}

Expr *parseExpr() {
    return parseAssign();
}

Expr *parseOptionalExprAndToken(TokenKind target) {
    if (maybe_consume(target)) {
        return 0;
    }
    Expr *expr = parseExpr();
    consume_otherwise_panic(target);
    return expr;
}

Stmt *parseStmt() {
    if (maybe_consume('{')) {
        Stmt *result = calloc(1, sizeof(Stmt));
        result->stmt_kind = enum4('e', 'x', 'p', 'r');
        result->expr = numberexpr(1);
        while (tokens->kind != '}') {
            Stmt *newstmt = calloc(1, sizeof(Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
            newstmt->second_child = parseStmt();
            result = newstmt;
        }
        tokens++;
        return result;
    }
    if (maybe_consume(enum3('R', 'E', 'T'))) {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        stmt->stmt_kind = enum3('R', 'E', 'T');
        stmt->expr = parseExpr();
        consume_otherwise_panic(';');
        return stmt;
    }
    if (maybe_consume(enum2('i', 'f'))) {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        consume_otherwise_panic('(');
        stmt->expr = parseExpr();
        consume_otherwise_panic(')');
        stmt->stmt_kind = enum2('i', 'f');
        stmt->second_child = parseStmt();  // then-block
        if (maybe_consume(enum4('e', 'l', 's', 'e'))) {
            stmt->third_child = parseStmt();  // else-block
        }
        return stmt;
    }
    if (maybe_consume(enum4('W', 'H', 'I', 'L'))) {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        consume_otherwise_panic('(');
        stmt->expr = parseExpr();
        consume_otherwise_panic(')');
        stmt->stmt_kind = enum4('W', 'H', 'I', 'L');
        Stmt *statement = parseStmt();
        stmt->second_child = statement;
        return stmt;
    }
    if (maybe_consume(enum3('f', 'o', 'r'))) {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        stmt->stmt_kind = enum3('f', 'o', 'r');
        consume_otherwise_panic('(');
        stmt->expr = parseOptionalExprAndToken(';');
        stmt->expr1 = parseOptionalExprAndToken(';');
        stmt->expr2 = parseOptionalExprAndToken(')');
        stmt->second_child = parseStmt();
        return stmt;
    } else {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
        stmt->expr = parseExpr();
        consume_otherwise_panic(';');
        return stmt;
    }
}

Stmt *parseFunctionContent() {
    consume_otherwise_panic('{');
    Stmt *result = calloc(1, sizeof(Stmt));
    result->stmt_kind = enum4('e', 'x', 'p', 'r');
    result->expr = numberexpr(1);
    while (tokens->kind != '}') {
        Stmt *statement = parseStmt();
        Stmt *newstmt = calloc(1, sizeof(Stmt));
        newstmt->first_child = result;
        newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
        newstmt->second_child = statement;
        result = newstmt;
    }
    tokens++;
    return result;
}

FuncDef *parseFunction() {
    consume_otherwise_panic(enum3('i', 'n', 't'));
    expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
    char *name = tokens->identifier_name;
    tokens++;
    char **params = calloc(6, sizeof(char *));
    consume_otherwise_panic('(');
    if (maybe_consume(')')) {
        Stmt *content = parseFunctionContent();
        FuncDef *funcdef = calloc(1, sizeof(FuncDef));
        funcdef->content = content;
        funcdef->name = name;
        funcdef->param_len = 0;
        funcdef->params = params;
        return funcdef;
    }

    int i = 0;
    for (; i < 6; i++) {
        consume_otherwise_panic(enum3('i', 'n', 't'));
        expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
        char *name = tokens->identifier_name;
        tokens++;
        if (maybe_consume(')')) {
            params[i] = name;
            break;
        }
        consume_otherwise_panic(',');
        params[i] = name;
    }

    Stmt *content = parseFunctionContent();
    FuncDef *funcdef = calloc(1, sizeof(FuncDef));
    funcdef->content = content;
    funcdef->name = name;
    funcdef->param_len = i + 1;
    funcdef->params = params;
    return funcdef;
}

void parseProgram() {
    int i = 0;
    while (tokens < tokens_end) {
        all_funcdefs[i] = parseFunction();
        i++;
    }
}
