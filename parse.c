#include <stdio.h>
#include <stdlib.h>

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

void consume_otherwise_panic(int kind) {
    if (!maybe_consume(kind)) {
        fprintf(stderr, "expected TokenKind `%x`, got TokenKind `%x`\n", kind, tokens->kind);
        exit(1);
    }
}

void expect_otherwise_panic(int kind) {
    if (tokens->kind != kind) {
        fprintf(stderr, "expected TokenKind `%x`, got TokenKind `%x`\n", kind, tokens->kind);
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
    if (tokens->kind == enum3('n', 'u', 'm')) {
        int value = tokens->value;
        tokens += 1;
        return numberexpr(value);
    } else if (tokens->kind == enum4('i', 'd', 'n', 't')) {
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
        if (tokens->kind == enum3('n', 'u', 'm')) {
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
        if (tokens->kind == enum3('n', 'u', 'm')) {
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

Stmt *parseFor() {
    tokens++;
    consume_otherwise_panic('(');
    Expr *exprs[3] = {0, 0, 0};
    exprs[0] = parseOptionalExprAndToken(';');
    exprs[1] = parseOptionalExprAndToken(';');
    exprs[2] = parseOptionalExprAndToken(')');

    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = enum3('f', 'o', 'r');
    stmt->expr = exprs[0];
    stmt->expr1 = exprs[1];
    stmt->expr2 = exprs[2];

    Stmt *loop_body = parseStmt();
    stmt->second_child = loop_body;
    return stmt;
}

Stmt *parseStmt() {
    if (maybe_consume('{')) {
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

    int is_return = 0;
    int is_if = 0;
    int is_while = 0;

    if (tokens->kind == enum3('r', 'e', 't')) {
        tokens++;
        is_return = 1;
    }
    if (tokens->kind == enum2('i', 'f')) {
        tokens++;
        is_if = 1;
        consume_otherwise_panic('(');
    }
    if (tokens->kind == enum4('w', 'h', 'i', 'l')) {
        tokens++;
        is_while = 1;
        consume_otherwise_panic('(');
    }
    if (tokens->kind == enum3('f', 'o', 'r')) {
        Stmt *stmt = parseFor();
        return stmt;
    }
    Expr *expr = parseExpr();

    if (is_if || is_while) {
        consume_otherwise_panic(')');
    } else {
        consume_otherwise_panic(';');
    }

    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
    stmt->expr = expr;
    if (is_if) {
        stmt->stmt_kind = enum2('i', 'f');
        {
            Stmt *statement = parseStmt();
            stmt->second_child = statement;
        }
        if (tokens->kind == enum4('e', 'l', 's', 'e')) {
            tokens++;
            Stmt *statement1 = parseStmt();
            stmt->third_child = statement1;
        }
    }
    if (is_while) {
        stmt->stmt_kind = enum4('w', 'h', 'i', 'l');
        Stmt *statement = parseStmt();
        stmt->second_child = statement;
    }
    if (is_return) {
        stmt->stmt_kind = enum3('r', 'e', 't');
    }
    return stmt;
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
    if (tokens->kind == enum4('i', 'd', 'n', 't')) {
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
            expect_otherwise_panic(enum4('i', 'd', 'n', 't'));
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
    } else {
        fprintf(stderr, "toplevel but not function\n");
        exit(1);
    }
}

void parseProgram() {
    int i = 0;
    while (tokens < tokens_end) {
       all_funcdefs[i] = parseFunction();
       i++;
    }
}
