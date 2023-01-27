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

    for (; tokens < tokens_end;) {
        Token maybe_relational = *tokens;

        if (maybe_relational.kind == '>') {
            tokens++;
            Expr *numberexp = parseAdditive();
            result = binaryExpr(result, numberexp, '>');
        }
        if (maybe_relational.kind == aa('>', '=')) {
            tokens++;
            Expr *numberexp = parseAdditive();
            result = binaryExpr(result, numberexp, aa('>', '='));
        }
        if (maybe_relational.kind == '<') {
            tokens++;
            Expr *numberexp = parseAdditive();
            // swap children of operator node
            result = binaryExpr(numberexp, result, '>');
        }
        if (maybe_relational.kind == aa('<', '=')) {
            tokens++;
            Expr *numberexp = parseAdditive();
            // swap children of operator node
            result = binaryExpr(numberexp, result, aa('>', '='));

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
        Token maybe_relational = *tokens;
        if (maybe_relational.kind == aa('=', '=')) {
            tokens++;
            Expr *numberexp = parseRelational();
            result = binaryExpr(result, numberexp, aa('=', '='));
        } else if (maybe_relational.kind == aa('!', '=')) {
            tokens++;
            Expr *numberexp = parseRelational();
            result = binaryExpr(result, numberexp, aa('!', '='));
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
    if (tokens->kind == aaa('n', 'u', 'm')) {
        int value = tokens->value;
        tokens += 1;
        return numberexpr(value);
    } else if (tokens->kind == aaaa('i', 'd', 'n', 't')) {
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
        Expr *expr = parsePrimary();
        return expr;
    }
    if (consume('-')) {
        Expr *expr = binaryExpr(numberexpr(0), parsePrimary(), '-');
        return expr;
    }
    Expr *expr = parsePrimary();
    return expr;
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
    if (tokens->kind == '=') {
        tokens++;
        Expr *newresult = parseAssign();
        return binaryExpr(result, newresult, '=');
    }
    return result;
}

Expr *parseOptionalExprAndToken(TokenKind target) {
    if (tokens->kind == target) {
        tokens++;
        return NULL;
    }
    Expr *expr = parseExpr();
    if (tokens->kind == target) {
        tokens++;
        return expr;
    }
    fprintf(stderr, "expected TokenKind#%d after optional expression but did not find one", target);
    exit(-1);
}

Stmt *parseFor() {
    tokens++;
    if (tokens->kind == '(') {
        tokens++;
    } else {
        fprintf(stderr, "expected left parenthesis got %d\n", tokens->kind);
        exit(1);
    }
    Expr *exprs[3] = {NULL, NULL, NULL};
    exprs[0] = parseOptionalExprAndToken(';');
    exprs[1] = parseOptionalExprAndToken(';');
    exprs[2] = parseOptionalExprAndToken(')');

    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = aaa('f', 'o', 'r');
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

    if (tokens->kind == '{') {
        tokens++;
        Stmt *result = calloc(1, sizeof(Stmt));
        result->stmt_kind = aaaa('e', 'x', 'p', 'r');
        result->expr = numberexpr(1);
        while (tokens->kind != '}') {
            Stmt *statement = parseStmt();
            Stmt *newstmt = calloc(1, sizeof(Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = aaaa('n', 'e', 'x', 't');
            newstmt->second_child = statement;
            result = newstmt;
        }
        tokens++;
        return result;
    }

    int is_return = 0;
    int is_if = 0;
    int is_while = 0;

    if (tokens->kind == aaa('r', 'e', 't')) {
        tokens++;
        is_return = 1;
    }
    if (tokens->kind == aa('i', 'f')) {
        tokens++;
        is_if = 1;
        if (tokens->kind == '(') {
            tokens++;
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    }
    if (tokens->kind == aaaa('w', 'h', 'i', 'l')) {
        tokens++;
        is_while = 1;
        if (tokens->kind == '(') {
            tokens++;
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    }
    if (tokens->kind == aaa('f', 'o', 'r')) {
        Stmt *stmt = parseFor();
        return stmt;
    }
    Expr *expr = parseExpr();

    if (is_if || is_while) {
        if (tokens->kind == ')') {
            tokens++;
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    } else {
        if (tokens->kind == ';') {
            tokens++;
        } else {
            fprintf(stderr, "no semicolon after expr. kind=%d\n", tokens->kind);
            exit(1);
        }
    }

    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = aaaa('e', 'x', 'p', 'r');
    stmt->expr = expr;
    if (is_if) {
        stmt->stmt_kind = aa('i', 'f');
        {
            Stmt *statement = parseStmt();
            stmt->second_child = statement;
        }
        if (tokens->kind == aaaa('e', 'l', 's', 'e')) {
            tokens++;
            Stmt *statement1 = parseStmt();
            stmt->third_child = statement1;
        }
    }
    if (is_while) {
        stmt->stmt_kind = aaaa('w', 'h', 'i', 'l');
        Stmt *statement = parseStmt();
        stmt->second_child = statement;
    }
    if (is_return) {
        stmt->stmt_kind = aaa('r', 'e', 't');
    }
    return stmt;
}

Stmt *parseFunctionContent() {
    if (tokens->kind == '{') {
        tokens++;
        Stmt *result = calloc(1, sizeof(Stmt));
        result->stmt_kind = aaaa('e', 'x', 'p', 'r');
        result->expr = numberexpr(1);
        while (tokens->kind != '}') {
            Stmt *statement = parseStmt();
            Stmt *newstmt = calloc(1, sizeof(Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = aaaa('n', 'e', 'x', 't');
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
    if (tokens->kind == aaaa('i', 'd', 'n', 't')) {
        tokens++;
    }
    if (tokens->kind == '(') {
        tokens++;
    }
    if (tokens->kind == ')') {
        tokens++;
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
        if (tokens->kind == aaa('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (tokens->kind == '*') {
            tokens++;
            Expr *numberexp = parseUnary();
            result = binaryExpr(result, numberexp, '*');
        } else if (tokens->kind == '/') {
            tokens++;
            Expr *numberexp = parseUnary();
            result = binaryExpr(result, numberexp, '/');
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
        if (tokens->kind == aaa('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (tokens->kind == '-') {
            tokens++;
            Expr *numberexp = parseMultiplicative();
            result = binaryExpr(result, numberexp, '-');
        } else if (tokens->kind == '+') {
            tokens++;
            Expr *numberexp = parseMultiplicative();
            result = binaryExpr(result, numberexp, '+');
        } else {
            return result;
        }
    }
    return result;
}
