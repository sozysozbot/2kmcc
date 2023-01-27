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

int consume(TokenKind kind) {
    if (tokens->kind == kind) {
        tokens += 1;
        return 1;
    }
    return 0;
}

Expr *binaryExpr(Expr *first_child, Expr *second_child, BinaryOperation binaryop) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = EK_Operator;
    newexp->binary_op = binaryop;
    newexp->second_child = second_child;
    return newexp;
}

Expr *parseRelational() {
    if (tokens_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseAdditive();

    while (tokens < tokens_end) {
        if (consume('>')) {
            result = binaryExpr(result, parseAdditive(), '>');
        } else if (consume(enum2('>', '='))) {
            result = binaryExpr(result, parseAdditive(), enum2('>', '='));
        } else if (consume('<')) {
            result = binaryExpr(parseAdditive(), result, '>');  // children & operator swapped
        } else if (consume(enum2('<', '='))) {
            result = binaryExpr(parseAdditive(), result, enum2('>', '=')); // children & operator swapped
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseEquality() {
    if (tokens_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseRelational();

    while (tokens < tokens_end) {
        if (consume(enum2('=', '='))) {
            result = binaryExpr(result, parseRelational(), enum2('=', '='));
        } else if (consume(enum2('!', '='))) {
            result = binaryExpr(result, parseRelational(), enum2('!', '='));
        } else {
            return result;
        }
    }
    return result;
}

Expr *parsePrimary() {
    if (tokens >= tokens_end) {
        fprintf(stderr, "Expected: number, but got EOF");
        exit(1);
    }
    if (tokens->kind == enum3('n', 'u', 'm')) {
        int value = tokens->value;
        tokens += 1;
        return numberexpr(value);
    } else if (tokens->kind == enum4('i', 'd', 'n', 't')) {
        char *name = tokens->identifier_name;
        tokens += 1;
        if (consume('(')) {
            Expr **arguments = calloc(6, sizeof(Expr *));

            if (consume(')')) {
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
                if (consume(',')) {
                    arguments[i] = expr;
                } else if (consume(')')) {
                    arguments[i] = expr;
                    break;
                } else {
                    fprintf(stderr, "Expected: comma or right paren. Token Kind:%d", tokens->kind);
                    exit(1);
                }
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
    } else {
        if (consume('(')) {
            Expr *expr = parseExpr();
            if (tokens->kind != ')') {
                fprintf(stderr, "Expected: right parenthesis. Token Kind:%d", tokens->kind);
                exit(1);
            }
            tokens += 1;
            return expr;
        }
        fprintf(stderr, "Expected: number. Token Kind:%d", tokens->kind);
        exit(1);
    }
}

Expr *parseUnary() {
    if (tokens >= tokens_end) {
        fprintf(stderr, "Expected: number, but got EOF");
        exit(1);
    }
    if (consume('+')) {
        return parsePrimary();
    }
    if (consume('-')) {
        return binaryExpr(numberexpr(0), parsePrimary(), '-');
    }
    return parsePrimary();
}

Expr *parseExpr() {
    return parseAssign();
}

Expr *parseAssign() {
    if (tokens_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseEquality();
    if (consume('=')) {
        return binaryExpr(result, parseAssign(), '=');
    }
    return result;
}

Expr *parseOptionalExprAndToken(TokenKind target) {
    if (consume(target)) {
        return NULL;
    }
    Expr *expr = parseExpr();
    if (consume(target)) {
        return expr;
    }
    fprintf(stderr, "expected TokenKind#%d after optional expression but did not find one", target);
    exit(-1);
}

Stmt *parseFor() {
    tokens++;
    if (consume('(')) {
    } else {
        fprintf(stderr, "expected left parenthesis got %d\n", tokens->kind);
        exit(1);
    }
    Expr *exprs[3] = {NULL, NULL, NULL};
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
    if (tokens_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }

    if (consume('{')) {
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
        if (consume('(')) {
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    }
    if (tokens->kind == enum4('w', 'h', 'i', 'l')) {
        tokens++;
        is_while = 1;
        if (consume('(')) {
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    }
    if (tokens->kind == enum3('f', 'o', 'r')) {
        Stmt *stmt = parseFor();
        return stmt;
    }
    Expr *expr = parseExpr();

    if (is_if || is_while) {
        if (consume(')')) {
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    } else {
        if (consume(';')) {
        } else {
            fprintf(stderr, "no semicolon after expr. kind=%d\n", tokens->kind);
            exit(1);
        }
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
    if (consume('{')) {
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
    } else {
        fprintf(stderr, "no { after a parenthesis defining a function. kind=%d\n", tokens->kind);
        exit(1);
    }
}

Stmt *parseProgram() {
    if (tokens->kind == enum4('i', 'd', 'n', 't')) {
        tokens++;
    }
    if (consume('(')) {
    }
    if (consume(')')) {
    }
    return parseFunctionContent();
}

Expr *parseMultiplicative() {
    if (tokens_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseUnary();

    while (tokens < tokens_end) {
        if (tokens->kind == enum3('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (consume('*')) {
            result = binaryExpr(result, parseUnary(), '*');
        } else if (consume('/')) {
            result = binaryExpr(result, parseUnary(), '/');
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseAdditive() {
    if (tokens_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseMultiplicative();

    while (tokens < tokens_end) {
        if (tokens->kind == enum3('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator, got Number");
            exit(1);
        } else if (consume('-')) {
            result = binaryExpr(result, parseMultiplicative(), '-');
        } else if (consume('+')) {
            result = binaryExpr(result, parseMultiplicative(), '+');
        } else {
            return result;
        }
    }
    return result;
}
