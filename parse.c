#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

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

Expr *parseRelational(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseAdditive(&tokens, token_end);

    for (; tokens < token_end;) {
        Token maybe_relational = *tokens;

        if (maybe_relational.kind == '>') {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens, token_end);
            result = binaryExpr(result, numberexp, '>');
        }
        if (maybe_relational.kind == aa('>', '=')) {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens, token_end);
            result = binaryExpr(result, numberexp, aa('>', '='));
        }
        if (maybe_relational.kind == '<') {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens, token_end);
            // swap children of operator node
            result = binaryExpr(numberexp, result, '>');
        }
        if (maybe_relational.kind == aa('<', '=')) {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens, token_end);
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

Expr *parseEquality(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseRelational(&tokens, token_end);

    while (tokens < token_end) {
        Token maybe_relational = *tokens;
        if (maybe_relational.kind == aa('=', '=')) {
            tokens++;
            Expr *numberexp = parseRelational(&tokens, token_end);
            result = binaryExpr(result, numberexp, aa('=', '='));
        } else if (maybe_relational.kind == aa('!', '=')) {
            tokens++;
            Expr *numberexp = parseRelational(&tokens, token_end);
            result = binaryExpr(result, numberexp, aa('!', '='));
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

Expr *parsePrimary(Token **ptrptr, Token *token_end) {
    Token *maybe_number = *ptrptr;
    if (maybe_number >= token_end) {
        fprintf(stderr, "Expected: number, but got EOF");
        exit(1);
    }
    if (maybe_number->kind == aaa('n', 'u', 'm')) {
        *ptrptr += 1;
        return numberexpr(maybe_number->value);
    } else if (maybe_number->kind == aaaa('i', 'd', 'n', 't')) {
        *ptrptr += 1;
        Token *maybe_leftparenthesis = *ptrptr;
        if (maybe_leftparenthesis->kind == '(') {
            *ptrptr += 1;
            Expr **arguments = calloc(6, sizeof(Expr*));

            if ((*ptrptr)->kind == ')') {
                *ptrptr += 1;
                Expr *callexp = calloc(1, sizeof(Expr));
                callexp->name = maybe_number->identifier_name;
                callexp->expr_kind = EK_Call;
                callexp->func_args = arguments;
                callexp->func_arg_len = 0;
                return callexp;
            }

            Expr *expr = parseExpr(ptrptr, token_end);
            if ((*ptrptr)->kind != ')') {
                fprintf(stderr, "Expected: right parenthesis. Token Kind:%d", (*ptrptr)->kind);
                exit(1);
            }
            *ptrptr += 1;
            arguments[0] = expr;

            Expr *callexp = calloc(1, sizeof(Expr));
            callexp->name = maybe_number->identifier_name;
            callexp->expr_kind = EK_Call;
            callexp->func_args = arguments;
            callexp->func_arg_len = 1;
            return callexp;
        } else {
            return identifierexpr(maybe_number->identifier_name);
        }
    } else {
        Token *maybe_leftparenthesis = maybe_number;
        if (maybe_leftparenthesis->kind == '(') {
            *ptrptr += 1;
            Expr *expr = parseExpr(ptrptr, token_end);
            Token *maybe_rightparenthesis = *ptrptr;
            if (maybe_rightparenthesis->kind != ')') {
                fprintf(stderr, "Expected: right parenthesis. Token Kind:%d", maybe_rightparenthesis->kind);
                exit(1);
            }
            *ptrptr += 1;
            return expr;
        }
        fprintf(stderr, "Expected: number. Token Kind:%d", maybe_number->kind);
        exit(1);
    }
}

Expr *parseUnary(Token **ptrptr, Token *token_end) {
    Token *maybe_unary = *ptrptr;
    if (maybe_unary >= token_end) {
        fprintf(stderr, "Expected: number, but got EOF");
        exit(1);
    }
    if (maybe_unary->kind == '+') {
        *ptrptr += 1;
        return parsePrimary(ptrptr, token_end);
    }
    if (maybe_unary->kind == '-') {
        *ptrptr += 1;
        return binaryExpr(numberexpr(0), parsePrimary(ptrptr, token_end), '-');
    }
    return parsePrimary(ptrptr, token_end);
}

Expr *parseExpr(Token **ptrptr, Token *token_end) {
    return parseAssign(ptrptr, token_end);
}

Expr *parseAssign(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseEquality(&tokens, token_end);
    if (tokens->kind == '=') {
        tokens++;
        Expr *newresult = parseAssign(&tokens, token_end);
        *ptrptr = tokens;
        return binaryExpr(result, newresult, '=');
    }
    *ptrptr = tokens;
    return result;
}

Expr *parseOptionalExprAndToken(Token **ptrptr, Token *token_end, TokenKind target) {
    Token *tokens = *ptrptr;
    if (tokens->kind == target) {
        tokens++;
        *ptrptr = tokens;
        return NULL;
    }
    Expr *expr = parseExpr(&tokens, token_end);
    if (tokens->kind == target) {
        tokens++;
        *ptrptr = tokens;
        return expr;
    }
    fprintf(stderr, "expected TokenKind#%d after optional expression but did not find one", target);
    exit(-1);
}

Stmt *parseFor(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    tokens++;

    if (tokens->kind == '(') {
        tokens++;
    } else {
        fprintf(stderr, "expected left parenthesis got %d\n", tokens->kind);
        exit(1);
    }
    Expr *exprs[3] = {NULL, NULL, NULL};
    exprs[0] = parseOptionalExprAndToken(&tokens, token_end, ';');
    exprs[1] = parseOptionalExprAndToken(&tokens, token_end, ';');
    exprs[2] = parseOptionalExprAndToken(&tokens, token_end, ')');

    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = aaa('f', 'o', 'r');
    stmt->expr = exprs[0];
    stmt->expr1 = exprs[1];
    stmt->expr2 = exprs[2];

    Stmt *statement = parseStmt(&tokens, token_end);
    stmt->second_child = statement;

    *ptrptr = tokens;

    return stmt;
}

Stmt *parseStmt(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
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
            Stmt *statement = parseStmt(&tokens, token_end);
            Stmt *newstmt = calloc(1, sizeof(Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = aaaa('n', 'e', 'x', 't');
            newstmt->second_child = statement;
            result = newstmt;
        }
        tokens++;
        *ptrptr = tokens;
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
        Stmt *stmt = parseFor(&tokens, token_end);
        *ptrptr = tokens;
        return stmt;
    }
    Expr *expr = parseExpr(&tokens, token_end);

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
            Stmt *statement = parseStmt(&tokens, token_end);
            stmt->second_child = statement;
        }
        Token maybe_else = *(tokens);
        if (maybe_else.kind == aaaa('e', 'l', 's', 'e')) {
            tokens++;
            Stmt *statement1 = parseStmt(&tokens, token_end);
            stmt->third_child = statement1;
        }
    }
    if (is_while) {
        stmt->stmt_kind = aaaa('w', 'h', 'i', 'l');
        Stmt *statement = parseStmt(&tokens, token_end);
        stmt->second_child = statement;
    }
    if (is_return) {
        stmt->stmt_kind = aaa('r', 'e', 't');
    }
    *ptrptr = tokens;
    return stmt;
}

Stmt *parseProgram(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Stmt *result = parseStmt(&tokens, token_end);

    for (; tokens < token_end;) {
        Stmt *statement = parseStmt(&tokens, token_end);
        Stmt *newexp = calloc(1, sizeof(Stmt));
        newexp->first_child = result;
        newexp->stmt_kind = aaaa('n', 'e', 'x', 't');
        newexp->second_child = statement;
        result = newexp;
    }
    *ptrptr = tokens;
    return result;
}

Expr *parseMultiplicative(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseUnary(&tokens, token_end);

    while (tokens < token_end) {
        if (tokens->kind == aaa('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (tokens->kind == '*') {
            tokens++;
            Expr *numberexp = parseUnary(&tokens, token_end);
            result = binaryExpr(result, numberexp, '*');
        } else if (tokens->kind == '/') {
            tokens++;
            Expr *numberexp = parseUnary(&tokens, token_end);
            result = binaryExpr(result, numberexp, '/');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

Expr *parseAdditive(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseMultiplicative(&tokens, token_end);

    while (tokens < token_end) {
        if (tokens->kind == aaa('n', 'u', 'm')) {
            fprintf(stderr, "Expected operator got Number");
            exit(1);
        } else if (tokens->kind == '-') {
            tokens++;
            Expr *numberexp = parseMultiplicative(&tokens, token_end);
            result = binaryExpr(result, numberexp, '-');
        } else if (tokens->kind == '+') {
            tokens++;
            Expr *numberexp = parseMultiplicative(&tokens, token_end);
            result = binaryExpr(result, numberexp, '+');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}
