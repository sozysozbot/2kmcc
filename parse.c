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
        if (maybe_relational.kind == '>' * 256 + '=') {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens, token_end);
            result = binaryExpr(result, numberexp, '>' * 256 + '=');
        }
        if (maybe_relational.kind == '<') {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens, token_end);
            // swap children of operator node
            result = binaryExpr(numberexp, result, '>');
        }
        if (maybe_relational.kind == '<' * 256 + '=') {
            tokens++;
            Expr *numberexp = parseAdditive(&tokens, token_end);
            // swap children of operator node
            result = binaryExpr(numberexp, result, '>' * 256 + '=');

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

    for (; tokens < token_end;) {
        Token maybe_relational = *tokens;
        switch (maybe_relational.kind) {
            case '=' * 256 + '=': {
                tokens++;
                Expr *numberexp = parseRelational(&tokens, token_end);
                result = binaryExpr(result, numberexp, '=' * 256 + '=');
                break;
            }
            case '!' * 256 + '=': {
                tokens++;
                Expr *numberexp = parseRelational(&tokens, token_end);
                result = binaryExpr(result, numberexp, '!' * 256 + '=');
                break;
            }
            default:
                *ptrptr = tokens;
                return result;
                break;
        }
    }
    *ptrptr = tokens;
    return result;
}
// primary    = num | ident | "(" expr ")"
Expr *parsePrimary(Token **ptrptr, Token *token_end) {
    Token *maybe_number = *ptrptr;
    if (maybe_number >= token_end) {
        fprintf(stderr, "Expected: number, but got EOF");
        exit(1);
    }
    if (maybe_number->kind == ('n' * 256 + 'u') * 256 + 'm') {
        *ptrptr += 1;
        return numberexpr(maybe_number->value);
    } else if (maybe_number->kind == (('i' * 256 + 'd') * 256 + 'n') * 256 + 't') {
        *ptrptr += 1;
        return identifierexpr(maybe_number->identifier_name);
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
// unary   = ("+" | "-")? primary
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

// expr       = equality
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

    Stmt *stmt = malloc(sizeof(Stmt));
    stmt->stmt_kind = SK_For;
    stmt->first_child = NULL;
    stmt->second_child = NULL;
    stmt->third_child = NULL;
    stmt->expr = exprs[0];
    stmt->expr1 = exprs[1];
    stmt->expr2 = exprs[2];

    Stmt *statement = parseStmt(&tokens, token_end);
    stmt->second_child = statement;

    *ptrptr = tokens;

    return stmt;
}
// statement= expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "while" "(" expr ")" stmt ?
//        | for?
Stmt *parseStmt(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    int is_return = 0;
    int is_if = 0;
    int is_while = 0;

    if (tokens->kind == ('r' * 256 + 'e') * 256 + 't') {
        tokens++;
        is_return = 1;
    }
    if (tokens->kind == 'i' * 256 + 'f') {
        tokens++;
        is_if = 1;
        if (tokens->kind == '(') {
            tokens++;
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    }
    if (tokens->kind == (('w' * 256 + 'h') * 256 + 'i') * 256 + 'l') {
        tokens++;
        is_while = 1;
        if (tokens->kind == '(') {
            tokens++;
        } else {
            fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
            exit(1);
        }
    }
    if (tokens->kind == ('f' * 256 + 'o') * 256 + 'r') {
        Stmt *stmt = parseFor(&tokens, token_end);
        *ptrptr = tokens;
        return stmt;
    }
    Expr *expr = parseExpr(&tokens, token_end);
    {
        Token maybe_operator = *tokens;
        if (is_if || is_while) {
            if (maybe_operator.kind == ')') {
                tokens++;
            } else {
                fprintf(stderr, "expected right parenthesis got %d\n", maybe_operator.kind);
                exit(1);
            }
        } else {
            if (maybe_operator.kind == ';') {
                tokens++;
            } else {
                fprintf(stderr, "no semicolon after expr. kind=%d\n", maybe_operator.kind);
                exit(1);
            }
        }
    }
    Stmt *stmt = malloc(sizeof(Stmt));
    stmt->stmt_kind = SK_Expr;
    stmt->first_child = NULL;
    stmt->second_child = NULL;
    stmt->third_child = NULL;
    stmt->expr = expr;
    if (is_if) {
        stmt->stmt_kind = SK_If;
        {
            Stmt *statement = parseStmt(&tokens, token_end);
            stmt->second_child = statement;
        }
        Token maybe_else = *(tokens);
        if (maybe_else.kind == (('e' * 256 + 'l') * 256 + 's') * 256 + 'e') {
            tokens++;
            Stmt *statement1 = parseStmt(&tokens, token_end);
            stmt->third_child = statement1;
        }
    }
    if (is_while) {
        stmt->stmt_kind = SK_While;
        Stmt *statement = parseStmt(&tokens, token_end);
        stmt->second_child = statement;
    }
    if (is_return) {
        stmt->stmt_kind = SK_Return;
    }
    *ptrptr = tokens;
    return stmt;
}
// program = (expr ";")*
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
        newexp->stmt_kind = SK_AndThen;
        newexp->second_child = statement;
        result = newexp;
    }
    *ptrptr = tokens;
    return result;
}

// mul = unary ("*" unary | "/" unary)*
Expr *parseMultiplicative(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseUnary(&tokens, token_end);

    for (; tokens < token_end;) {
        Token maybe_operator = *tokens;
        switch (maybe_operator.kind) {
            case ('n' * 256 + 'u') * 256 + 'm': {
                fprintf(stderr, "Expected operator got Number");
                exit(1);
            }
            case '*': {
                tokens++;
                Expr *numberexp = parseUnary(&tokens, token_end);
                result = binaryExpr(result, numberexp, '*');
                // ptr++;
                break;
            }
            case '/': {
                tokens++;
                Expr *numberexp = parseUnary(&tokens, token_end);
                result = binaryExpr(result, numberexp, '/');
                // ptr++;
                break;
            }
            default:
                *ptrptr = tokens;
                return result;
                break;
        }
    }
    *ptrptr = tokens;
    return result;
}

// additive = mul ("+" mul | "-" mul)*
Expr *parseAdditive(Token **ptrptr, Token *token_end) {
    Token *tokens = *ptrptr;
    if (token_end == tokens) {
        fprintf(stderr, "No token found");
        exit(1);
    }
    Expr *result = parseMultiplicative(&tokens, token_end);

    for (; tokens < token_end;) {
        Token maybe_operator = *tokens;
        switch (maybe_operator.kind) {
            case ('n' * 256 + 'u') * 256 + 'm': {
                fprintf(stderr, "Expected operator got Number");
                exit(1);
            }
            case '-': {
                tokens++;
                Expr *numberexp = parseMultiplicative(&tokens, token_end);
                result = binaryExpr(result, numberexp, '-');
                // ptr++;
                break;
            }
            case '+': {
                tokens++;
                Expr *numberexp = parseMultiplicative(&tokens, token_end);
                result = binaryExpr(result, numberexp, '+');
                // ptr++;
                break;
            }
            default:
                *ptrptr = tokens;
                return result;
                break;
        }
    }
    *ptrptr = tokens;
    return result;
}
