#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"
extern Token **PTR_PTR;
extern Token *token_end;
extern Token *tokens;

Expr *numberexpr(int value) {
    Expr *numberexp = calloc(1, sizeof(Expr));
    numberexp->value = value;
    numberexp->expr_kind = EK_Number;
    return numberexp;
}

Expr *identifierexpr(char *name) {
    Expr *numberexp = calloc(1, sizeof(Expr));
    numberexp->name = name;
    numberexp->expr_kind = EK_Identifier;
    return numberexp;
}

Expr *binaryExpr(Expr *first_child, Expr *second_child, BinaryOperation binaryop) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = EK_Operator;
    newexp->binary_op = binaryop;
    newexp->second_child = second_child;
    return newexp;
}

Expr *parseRelational(Token **ptrptr) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseAdditive(&tokens);

    for (; tokens < token_end;) {
        Token maybe_relational = *tokens;

        if (maybe_relational.kind == '>') {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens);
            result = binaryExpr(result, numberexp, '>');
        }
        if (maybe_relational.kind == aa('>', '=')) {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens);
            result = binaryExpr(result, numberexp, aa('>', '='));
        }
        if (maybe_relational.kind == '<') {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens);
            // swap children of operator node
            result = binaryExpr(numberexp, result, '>');
        }
        if (maybe_relational.kind == aa('<', '=')) {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens);
            // swap children of operator node
            result = binaryExpr(numberexp, result, aa('>', '='));

        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

Expr *parseEquality(Token **ptrptr) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseRelational(&tokens);

    while (tokens < token_end) {
        Token maybe_relational = *tokens;
        if (maybe_relational.kind == aa('=', '=')) {
            tokens++;
            Expr *numberexp = parseRelational(&tokens);
            result = binaryExpr(result, numberexp, aa('=', '='));
        } else if (maybe_relational.kind == aa('!', '=')) {
            tokens++;
            Expr *numberexp = parseRelational(&tokens);
            result = binaryExpr(result, numberexp, aa('!', '='));
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

Expr *parsePrimary(Token **ptrptr) {
    Token *tokens = *ptrptr;
    if (tokens >= token_end) {
        fprintf(stderr, "Expected: number, but got EOF");
        exit(1);
    }
    if (tokens->kind == aaa('n', 'u', 'm')) {
        int value = tokens->value;
        tokens += 1;
        *ptrptr = tokens;
        return numberexpr(value);
    } else if (tokens->kind == aaaa('i', 'd', 'n', 't')) {
        char *name = tokens->identifier_name;
        *ptrptr += 1;
        Token *maybe_leftparenthesis = *ptrptr;
        if (maybe_leftparenthesis->kind == '(') {
            *ptrptr += 1;
            Expr **arguments = calloc(6, sizeof(Expr *));

            if ((*ptrptr)->kind == ')') {
                *ptrptr += 1;
                Expr *callexp = calloc(1, sizeof(Expr));
                callexp->name = name;
                callexp->expr_kind = EK_Call;
                callexp->func_args = arguments;
                callexp->func_arg_len = 0;
                return callexp;
            }

            int i = 0;
            for (; i < 6; i++) {
                Expr *expr = parseExpr(ptrptr);
                if ((*ptrptr)->kind == ',') {
                    *ptrptr += 1;
                    arguments[i] = expr;
                } else if ((*ptrptr)->kind == ')') {
                    *ptrptr += 1;
                    arguments[i] = expr;
                    break;
                } else {
                    fprintf(stderr, "Expected: comma or right paren. Token Kind:%d", (*ptrptr)->kind);
                    exit(1);
                }
            }

            Expr *callexp = calloc(1, sizeof(Expr));
            callexp->name = tokens->identifier_name;
            callexp->expr_kind = EK_Call;
            callexp->func_args = arguments;
            callexp->func_arg_len = i + 1;
            return callexp;
        } else {
            return identifierexpr(name);
        }
    } else {
        Token *maybe_leftparenthesis = tokens;
        if (maybe_leftparenthesis->kind == '(') {
            *ptrptr += 1;
            Expr *expr = parseExpr(ptrptr);
            Token *maybe_rightparenthesis = *ptrptr;
            if (maybe_rightparenthesis->kind != ')') {
                fprintf(stderr, "Expected: right parenthesis. Token Kind:%d", maybe_rightparenthesis->kind);
                exit(1);
            }
            *ptrptr += 1;
            return expr;
        }
        fprintf(stderr, "Expected: number. Token Kind:%d", tokens->kind);
        exit(1);
    }
}

Expr *parseUnary(Token **ptrptr) {
    Token *tokens = *ptrptr;
    if (tokens >= token_end) {
        fprintf(stderr, "Expected: number, but got EOF");
        exit(1);
    }
    if (tokens->kind == '+') {
        tokens += 1;
        Expr *expr = parsePrimary(&tokens);
        *ptrptr = tokens;
        return expr;
    }
    if (tokens->kind == '-') {
        tokens += 1;
        Expr *expr = binaryExpr(numberexpr(0), parsePrimary(&tokens), '-');
        *ptrptr = tokens;
        return expr;
    }
    Expr *expr = parsePrimary(&tokens);
    *ptrptr = tokens;
    return expr;
}

Expr *parseExpr(Token **ptrptr) {
    return parseAssign(ptrptr);
}

Expr *parseAssign(Token **ptrptr) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseEquality(&tokens);
    if (tokens->kind == '=') {
        tokens++;
        Expr *newresult = parseAssign(&tokens);
        *ptrptr = tokens;
        return binaryExpr(result, newresult, '=');
    }
    *ptrptr = tokens;
    return result;
}

Expr *parseOptionalExprAndToken(TokenKind target) {
    if (tokens->kind == target) {
        tokens++;
        return NULL;
    }
    Expr *expr = parseExpr(&tokens);
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
    if (token_end == tokens) {
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
        Stmt *stmt = parseFor(&tokens);
        return stmt;
    }
    Expr *expr = parseExpr(&tokens);

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

Expr *parseMultiplicative(Token **ptrptr) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseUnary(&tokens);

    while (tokens < token_end) {
        if (tokens->kind == aaa('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (tokens->kind == '*') {
            tokens++;
            Expr *numberexp = parseUnary(&tokens);
            result = binaryExpr(result, numberexp, '*');
        } else if (tokens->kind == '/') {
            tokens++;
            Expr *numberexp = parseUnary(&tokens);
            result = binaryExpr(result, numberexp, '/');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

Expr *parseAdditive(Token **ptrptr) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseMultiplicative(&tokens);

    while (tokens < token_end) {
        if (tokens->kind == aaa('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (tokens->kind == '-') {
            tokens++;
            Expr *numberexp = parseMultiplicative(&tokens);
            result = binaryExpr(result, numberexp, '-');
        } else if (tokens->kind == '+') {
            tokens++;
            Expr *numberexp = parseMultiplicative(&tokens);
            result = binaryExpr(result, numberexp, '+');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}
