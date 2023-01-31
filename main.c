#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int Kind;

typedef struct Type {
    Kind ty;
    struct Type *ptr_to;
} Type;

typedef struct Expr {
    Kind op;
    Kind expr_kind;
    int value;
    struct Expr *first_child;
    struct Expr *second_child;
    struct Expr **func_args;
    int func_arg_len;
    char *func_or_ident_name;
    Type *typ;
} Expr;

typedef struct NameAndType {
    char *name;
    Type *type;
} NameAndType;

typedef struct FuncDef {
    struct Stmt *content;
    char *name;
    char **params;
    int param_len;
    NameAndType *lvar_table_start;
    NameAndType *lvar_table_end;
    Type *return_type;
} FuncDef;

typedef struct Stmt {
    Kind stmt_kind;
    struct Expr *expr;
    struct Expr *expr1;
    struct Expr *expr2;
    struct Stmt *first_child;
    struct Stmt *second_child;
    struct Stmt *third_child;
} Stmt;

typedef struct LVar {
    struct LVar *next;
    char *name;
    int offset_from_rbp;
} LVar;

typedef struct Token {
    Kind kind;
    int value;
    char *identifier_name;
} Token;

int enum2(int a, int b) {
    return a + b * 256;
}

int enum3(int a, int b, int c) {
    return enum2(a, enum2(b, c));
}

int enum4(int a, int b, int c, int d) {
    return enum2(a, enum3(b, c, d));
}

/*** ^ LIB | v PARSE ***/
Token all_tokens[1000];
Token *tokens_end;
Token *tokens;
int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}
int tokenize(char *str) {
    int token_index = 0;
    for (int i = 0; str[i];) {
        char c = str[i];
        char *ptr = str + i;
        if (strncmp(ptr, "return", 6) == 0 && !is_alnum(ptr[6])) {
            Token token = {enum3('R', 'E', 'T'), 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i += 6;
            continue;
        }
        if (strncmp(ptr, "if", 2) == 0 && !is_alnum(ptr[2])) {
            Token token = {enum2('i', 'f'), 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i += 2;
            continue;
        }
        if (strncmp(ptr, "while", 5) == 0 && !is_alnum(ptr[5])) {
            Token token = {enum4('W', 'H', 'I', 'L'), 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i += 5;
            continue;
        }
        if (strncmp(ptr, "else", 4) == 0 && !is_alnum(ptr[4])) {
            Token token = {enum4('e', 'l', 's', 'e'), 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i += 4;
            continue;
        }
        if (strncmp(ptr, "for", 3) == 0 && !is_alnum(ptr[3])) {
            Token token = {enum3('f', 'o', 'r'), 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i += 3;
            continue;
        }
        if (strncmp(ptr, "int", 3) == 0 && !is_alnum(ptr[3])) {
            Token token = {enum3('i', 'n', 't'), 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i += 3;
            continue;
        }
        if (c == '+') {
            Token token = {'+', 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',') {
            all_tokens[token_index].kind = c;
            token_index++;
            i++;
        } else if (c == '-') {
            Token token = {'-', 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '*') {
            Token token = {'*', 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '&') {
            Token token = {'&', 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '/') {
            Token token = {'/', 0, 0};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '>') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'>', 0, 0};
                all_tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('>', '='), 0, 0};
                all_tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '<') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'<', 0, 0};
                all_tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('<', '='), 0, 0};
                all_tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '=') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'=', 0, 0};
                all_tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('=', '='), 0, 0};
                all_tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '!') {
            i++;
            char c = str[i];
            if (c != '=') {
                fprintf(stderr, "%s: unknown token !%c(%d)\n", __FUNCTION__, c, c);
                return -1;
            }
            i++;
            Token token = {enum2('!', '='), 0, 0};
            all_tokens[token_index] = token;
            token_index++;
        } else if ('0' <= c && c <= '9') {
            char *str_ = &str[i];
            int parsed_num;
            int parsed_length = 0;
            for (parsed_num = 0; '0' <= *str_ && *str_ <= '9'; str_++) {
                parsed_num = parsed_num * 10 + (*str_ - '0');
                parsed_length++;
            }
            i += parsed_length;
            Token token = {enum3('N', 'U', 'M'), parsed_num, 0};
            all_tokens[token_index] = token;
            token_index++;
        } else if (c == ' ') {
            i++;
        } else if (('a' <= c && c <= 'z') || c == '_') {
            char *start = &str[i];
            for (i++; is_alnum(str[i]); i++) {
            }
            int length = &str[i] - start;
            char *name = calloc(length + 1, sizeof(char));
            memcpy(name, start, length);
            Token token = {enum4('I', 'D', 'N', 'T'), 0, 0};
            token.identifier_name = name;
            all_tokens[token_index] = token;
            token_index++;
        } else {
            fprintf(stderr, "%s: unknown character %c(%d)\n", __FUNCTION__, c, c);
            return -1;
        }
    }
    return token_index;
}

/*** ^ TOKENIZE | v PARSE ***/

Type *type_int() {
    Type *t = calloc(1, sizeof(Type));
    t->ty = enum3('i', 'n', 't');
    return t;
}

Type *ptr_of(Type *t) {
    Type *new_t = calloc(1, sizeof(Type));
    new_t->ty = '*';
    new_t->ptr_to = t;
    return new_t;
}

Type *deref(Type *t) {
    if (t->ty == '*') {
        return t->ptr_to;
    } else {
        fprintf(stderr, "Cannot deref a non-pointer type\n");
        exit(1);
    }
}

int size(Type *t) {
    if (t->ty == '*') {
        return 8;
    } else if (t->ty == enum3('i', 'n', 't')) {
        return 4;
    } else {
        fprintf(stderr, "unknown size\n");
        exit(1);
    }
}

Expr *numberexpr(int value) {
    Expr *numberexp = calloc(1, sizeof(Expr));
    numberexp->value = value;
    numberexp->expr_kind = enum3('N', 'U', 'M');
    numberexp->typ = type_int();
    return numberexp;
}

Expr *binaryExpr(Expr *first_child, Expr *second_child, Kind binaryop, Type *typ) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('2', 'A', 'R', 'Y');
    newexp->op = binaryop;
    newexp->second_child = second_child;
    newexp->typ = typ;
    return newexp;
}

Expr *unaryExpr(Expr *first_child, Kind unaryop, Type *typ) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('1', 'A', 'R', 'Y');
    newexp->op = unaryop;
    newexp->typ = typ;
    return newexp;
}

int maybe_consume(Kind kind) {
    if (tokens->kind == kind) {
        tokens += 1;
        return 1;
    }
    return 0;
}

char *decode_kind(Kind kind) {
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

char *expect_identifier_and_get_name() {
    expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
    char *name = tokens->identifier_name;
    tokens++;
    return name;
}

void panic_if_eof() {
    if (tokens >= tokens_end) {
        fprintf(stderr, "EOF encountered");
        exit(1);
    }
}

Expr *parseExpr(void);

NameAndType *lvars_start;
NameAndType *lvars;
NameAndType *all_funcdecls_start[100];
NameAndType **all_funcdecls;
FuncDef *all_funcdefs[100];

Type *lookup_ident_type(char *name) {
    for (int i = 0; lvars_start[i].name; i++) {
        if (strcmp(lvars_start[i].name, name) == 0) {
            return lvars_start[i].type;
        }
    }
    fprintf(stderr, "cannot find an identifier named `%s`; cannot determine the type\n", name);
    exit(1);
}

Type *lookup_func_type(char *name) {
    for (int i = 0; all_funcdecls_start[i]; i++) {
        if (strcmp(all_funcdecls_start[i]->name, name) == 0) {
            return all_funcdecls_start[i]->type;
        }
    }
    fprintf(stderr, "cannot find a function named `%s`; cannot determine the return type. Implicitly assumes that it return an int\n", name);
    return type_int();
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
                callexp->func_or_ident_name = name;
                callexp->expr_kind = enum4('C', 'A', 'L', 'L');
                callexp->func_args = arguments;
                callexp->func_arg_len = 0;
                callexp->typ = lookup_func_type(name);
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
            callexp->func_or_ident_name = name;
            callexp->expr_kind = enum4('C', 'A', 'L', 'L');
            callexp->func_args = arguments;
            callexp->func_arg_len = i + 1;
            callexp->typ = lookup_func_type(name);
            return callexp;
        } else {
            Expr *ident_exp = calloc(1, sizeof(Expr));
            ident_exp->func_or_ident_name = name;
            ident_exp->expr_kind = enum4('I', 'D', 'N', 'T');
            ident_exp->typ = lookup_ident_type(name);
            return ident_exp;
        }
    }

    consume_otherwise_panic('(');
    Expr *expr = parseExpr();
    consume_otherwise_panic(')');
    return expr;
}

Expr *assert_int(Expr *e) {
    if (e->typ->ty != enum3('i', 'n', 't')) {
        fprintf(stderr, "int is expected, but not an int\n");
        exit(1);
    }
    return e;
}

void assert_same_type(Type *t1, Type *t2) {
    if (t1->ty == '*' && t2->ty == '*') {
        assert_same_type(t1->ptr_to, t2->ptr_to);
    } else if (t1->ty != t2->ty) {
        fprintf(stderr, "two different types detected\n");
        exit(1);
    }
}

Expr *parseUnary() {
    panic_if_eof();
    if (maybe_consume('+')) {
        return assert_int(parsePrimary());
    } else if (maybe_consume('-')) {
        return binaryExpr(numberexpr(0), assert_int(parsePrimary()), '-', type_int());
    } else if (maybe_consume('*')) {
        Expr *expr = parsePrimary();
        return unaryExpr(expr, '*', deref(expr->typ));
    } else if (maybe_consume('&')) {
        Expr *expr = parsePrimary();
        return unaryExpr(expr, '&', ptr_of(expr->typ));
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
            result = binaryExpr(assert_int(result), assert_int(parseUnary()), '*', type_int());
        } else if (maybe_consume('/')) {
            result = binaryExpr(assert_int(result), assert_int(parseUnary()), '/', type_int());
        } else {
            return result;
        }
    }
    return result;
}

Expr *add(Expr *lhs, Expr *rhs) {
    if (lhs->typ->ty == enum3('i', 'n', 't')) {
        if (rhs->typ->ty == enum3('i', 'n', 't')) {
            return binaryExpr(lhs, rhs, '+', type_int());
        } else if (rhs->typ->ty == '*') {
            return add(rhs, lhs);
        } else {
            fprintf(stderr, "unknown type in addition\n");
            exit(1);
        }
    } else if (lhs->typ->ty == '*') {
        if (rhs->typ->ty == enum3('i', 'n', 't')) {
            return binaryExpr(lhs, binaryExpr(numberexpr(size(deref(lhs->typ))), rhs, '*', type_int()), '+', lhs->typ);
        } else {
            fprintf(stderr, "cannot add\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "unknown type\n");
        exit(1);
    }
}

Expr *subtract(Expr *lhs, Expr *rhs) {
    if (lhs->typ->ty == enum3('i', 'n', 't')) {
        if (rhs->typ->ty == enum3('i', 'n', 't')) {
            return binaryExpr(lhs, rhs, '-', type_int());
        } else if (rhs->typ->ty == '*') {
            fprintf(stderr, "cannot subtract\n");
            exit(1);
        }
    } else if (lhs->typ->ty == '*') {
        if (rhs->typ->ty == enum3('i', 'n', 't')) {
            return binaryExpr(lhs, binaryExpr(numberexpr(size(deref(lhs->typ))), rhs, '*', type_int()), '-', lhs->typ);
        } else if (rhs->typ->ty == '*') {
            assert_same_type(lhs->typ, rhs->typ);
            return binaryExpr(binaryExpr(lhs, rhs, '-', type_int()), numberexpr(size(deref(lhs->typ))), '/', type_int());
        } else {
            fprintf(stderr, "cannot add\n");
            exit(1);
        }
    }
    fprintf(stderr, "unknown type\n");
    exit(1);
}

Expr *parseAdditive() {
    panic_if_eof();
    Expr *result = parseMultiplicative();
    while (tokens < tokens_end) {
        if (tokens->kind == enum3('N', 'U', 'M')) {
            fprintf(stderr, "Expected operator, got Number");
            exit(1);
        } else if (maybe_consume('-')) {
            result = subtract(result, parseMultiplicative());
        } else if (maybe_consume('+')) {
            result = add(result, parseMultiplicative());
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
            result = binaryExpr(result, parseAdditive(), '>', type_int());
        } else if (maybe_consume(enum2('>', '='))) {
            result = binaryExpr(result, parseAdditive(), enum2('>', '='), type_int());
        } else if (maybe_consume('<')) {
            result = binaryExpr(parseAdditive(), result, '>', type_int());  // children & operator swapped
        } else if (maybe_consume(enum2('<', '='))) {
            result = binaryExpr(parseAdditive(), result, enum2('>', '='), type_int());  // children & operator swapped
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
            result = binaryExpr(result, parseRelational(), enum2('=', '='), type_int());
        } else if (maybe_consume(enum2('!', '='))) {
            result = binaryExpr(result, parseRelational(), enum2('!', '='), type_int());
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
        Expr *rhs = parseAssign();
        assert_same_type(result->typ, rhs->typ);
        return binaryExpr(result, rhs, '=', result->typ);
    }
    return result;
}

Expr *parseExpr() {
    return parseAssign();
}

Expr *parseOptionalExprAndToken(Kind target) {
    if (maybe_consume(target)) {
        return 0;
    }
    Expr *expr = parseExpr();
    consume_otherwise_panic(target);
    return expr;
}

Type *consume_type_otherwise_panic() {
    consume_otherwise_panic(enum3('i', 'n', 't'));

    Type *t = calloc(1, sizeof(Type));
    t->ty = enum3('i', 'n', 't');

    while (maybe_consume('*')) {
        t = ptr_of(t);
    }
    return t;
}

Stmt *parseStmt() {
    if (maybe_consume('{')) {
        Stmt *result = calloc(1, sizeof(Stmt));
        result->stmt_kind = enum4('e', 'x', 'p', 'r');
        result->expr = numberexpr(42);
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
    }
    if (maybe_consume(enum3('i', 'n', 't'))) {
        tokens--;
        Type *t = consume_type_otherwise_panic();
        if (lvars == lvars_start + 100) {
            fprintf(stderr, "too many local variables");
            exit(1);
        }
        char *name = expect_identifier_and_get_name();
        lvars->name = name;
        lvars->type = t;
        lvars++;
        consume_otherwise_panic(';');
        Stmt *stmt = calloc(1, sizeof(Stmt));
        stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
        stmt->expr = numberexpr(42);
        return stmt;
    }
    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
    stmt->expr = parseExpr();
    consume_otherwise_panic(';');
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
    Type *return_type = consume_type_otherwise_panic();
    char *name = expect_identifier_and_get_name();
    char **params = calloc(6, sizeof(char *));
    consume_otherwise_panic('(');
    if (maybe_consume(')')) {
        lvars = lvars_start = calloc(100, sizeof(NameAndType));
        Stmt *content = parseFunctionContent();
        FuncDef *funcdef = calloc(1, sizeof(FuncDef));
        funcdef->content = content;
        funcdef->name = name;
        funcdef->return_type = return_type;
        funcdef->param_len = 0;
        funcdef->params = params;
        funcdef->lvar_table_start = lvars_start;
        funcdef->lvar_table_end = lvars;
        NameAndType *decl = calloc(1, sizeof(NameAndType));
        decl->type = return_type;
        decl->name = name;
        *all_funcdecls = decl;
        all_funcdecls++;
        return funcdef;
    }

    lvars = lvars_start = calloc(100, sizeof(char *));
    int i = 0;
    for (; i < 6; i++) {
        Type *t = consume_type_otherwise_panic();
        char *name = expect_identifier_and_get_name();
        if (maybe_consume(')')) {
            params[i] = name;
            lvars->name = name;
            lvars->type = t;
            lvars++;
            break;
        }
        consume_otherwise_panic(',');
        params[i] = name;
        lvars->name = name;
        lvars->type = t;
        lvars++;
    }

    Stmt *content = parseFunctionContent();
    FuncDef *funcdef = calloc(1, sizeof(FuncDef));
    funcdef->content = content;
    funcdef->name = name;
    funcdef->return_type = return_type;
    funcdef->param_len = i + 1;
    funcdef->params = params;
    funcdef->lvar_table_start = lvars_start;
    funcdef->lvar_table_end = lvars;
    NameAndType *decl = calloc(1, sizeof(NameAndType));
    decl->type = return_type;
    decl->name = name;
    *all_funcdecls = decl;
    all_funcdecls++;
    return funcdef;
}

void parseProgram() {
    int i = 0;
    while (tokens < tokens_end) {
        all_funcdefs[i] = parseFunction();
        i++;
    }
}

/*** ^ PARSE | v CODEGEN ***/

int labelCounter = 0;

LVar *locals;

LVar *findLVar(char *name) {
    LVar *local = locals;
    if (!local) {
        return 0;
    }
    while (local) {
        if (!strcmp(name, local->name)) {
            return local;
        }
        local = local->next;
    }
    return 0;
}

LVar *lastLVar() {
    LVar *local = locals;
    if (!local) {
        return 0;
    }
    while (1) {
        if (!local->next) {
            return local;
        }
        local = local->next;
    }
}

LVar *insertLVar(char *name) {
    LVar *newlocal = calloc(1, sizeof(LVar));
    LVar *last = lastLVar();
    newlocal->name = name;
    if (!last) {
        newlocal->offset_from_rbp = 8;
    } else {
        newlocal->offset_from_rbp = last->offset_from_rbp + 8;  // offset+last size
    }
    newlocal->next = 0;

    if (!last) {
        locals = newlocal;
    } else {
        last->next = newlocal;
    }
    return newlocal;
}

void EvaluateExprIntoRax(Expr *expr);

void EvaluateLValueAddressIntoRax(Expr *expr) {
    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        if (!findLVar(expr->func_or_ident_name)) {
            fprintf(stderr, "undefined variable %s\n", expr->func_or_ident_name);
            exit(1);
        }
        LVar *local = findLVar(expr->func_or_ident_name);
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", local->offset_from_rbp);
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y') && expr->op == '*') {
        EvaluateExprIntoRax(expr->first_child);
    } else {
        fprintf(stderr, "not lvalue");
        exit(1);
    }
}

void CodegenStmt(Stmt *stmt) {
    if (stmt->stmt_kind == enum4('e', 'x', 'p', 'r')) {
        EvaluateExprIntoRax(stmt->expr);
    } else if (stmt->stmt_kind == enum4('n', 'e', 'x', 't')) {
        CodegenStmt(stmt->first_child);
        CodegenStmt(stmt->second_child);
    } else if (stmt->stmt_kind == enum3('R', 'E', 'T')) {
        EvaluateExprIntoRax(stmt->expr);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    } else if (stmt->stmt_kind == enum2('i', 'f')) {
        int label = ++labelCounter;

        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp .Lend%d\n", label);
        printf(".Lelse%d:\n", label);
        if (stmt->third_child != 0) {
            CodegenStmt(stmt->third_child);
        }
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == enum4('W', 'H', 'I', 'L')) {
        int label = ++labelCounter;

        printf(".Lbegin%d:\n", label);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == enum3('f', 'o', 'r')) {
        int label = ++labelCounter;

        if (stmt->expr) {
            EvaluateExprIntoRax(stmt->expr);
        }
        printf(".Lbegin%d:\n", label);
        if (stmt->expr1) {
            EvaluateExprIntoRax(stmt->expr1);
        } else {
            printf("  mov rax, 1\n");
        }
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        CodegenStmt(stmt->second_child);
        if (stmt->expr2) {
            EvaluateExprIntoRax(stmt->expr2);
        }
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    }
}

const char *nth_arg_reg(int n) {
    return "rdi\0rsi\0rdx\0rcx\0r8 \0r9" + 4 * n;
}

void CodegenFunc(FuncDef *funcdef) {
    printf(".globl %s\n", funcdef->name);
    printf("%s:\n", funcdef->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");
    for (int i = 0; i < funcdef->param_len; i++) {
        char *param_name = funcdef->params[i];
        insertLVar(param_name);
        LVar *local = findLVar(param_name);
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", local->offset_from_rbp);
        printf("  mov [rax], %s\n", nth_arg_reg(i));
    }
    for (NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++) {
        insertLVar(ptr->name);
    }
    CodegenStmt(funcdef->content);
}

void EvaluateExprIntoRax(Expr *expr) {
    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        EvaluateLValueAddressIntoRax(expr);
        printf("  mov rax,[rax]\n");
        return;
    } else if (expr->expr_kind == enum4('C', 'A', 'L', 'L')) {
        for (int i = 0; i < expr->func_arg_len; i++) {
            EvaluateExprIntoRax(expr->func_args[i]);
            printf("    push rax\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--) {
            printf("    pop %s\n", nth_arg_reg(i));
        }
        printf(" call %s\n", expr->func_or_ident_name);
        return;
    } else if (expr->expr_kind == enum3('N', 'U', 'M')) {
        printf("  mov rax, %d\n", expr->value);
        return;
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y')) {
        if (expr->op == '*') {
            EvaluateExprIntoRax(expr->first_child);
            printf("  mov rax, [rax]\n");
        } else if (expr->op == '&') {
            EvaluateLValueAddressIntoRax(expr->first_child);
        } else {
            fprintf(stderr, "Invalid unaryop kind:%d", expr->op);
            exit(1);
        }
    } else if (expr->expr_kind == enum4('2', 'A', 'R', 'Y')) {
        if (expr->op == '=') {
            EvaluateLValueAddressIntoRax(expr->first_child);
            printf("    push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\n");
            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
        } else {
            EvaluateExprIntoRax(expr->first_child);
            printf("    push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\n");
            printf("    pop rdi\n");
            printf("    pop rax\n");

            if (expr->op == '+') {
                printf("    add rax,rdi\n");
            } else if (expr->op == '-') {
                printf("    sub rax,rdi\n");

            } else if (expr->op == '*') {
                printf("    imul rax,rdi\n");
            } else if (expr->op == '/') {
                printf("  cqo\n");
                printf("  idiv rdi\n");
            } else if (expr->op == enum2('=', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  sete al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op == enum2('!', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  setne al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op == '>') {
                printf("  cmp rax, rdi\n");
                printf("  setg al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op == enum2('>', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  setge al\n");
                printf("  movzb rax, al\n");
            } else {
                fprintf(stderr, "Invalid binaryop kind: %s\n", decode_kind(expr->op));
                exit(1);
            }
        }
    } else {
        fprintf(stderr, "Invalid expr kind: %s\n", decode_kind(expr->expr_kind));
        exit(1);
    }
}

/*** ^ CODEGEN | v MAIN ***/

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect cmd line arg\n");
        return 1;
    }
    char *p = argv[1];
    int tokens_length = tokenize(p);
    if (tokens_length == 0) {
        fprintf(stderr, "No token found");
        return 1;
    }
    tokens = all_tokens;
    tokens_end = all_tokens + tokens_length;

    all_funcdecls = all_funcdecls_start;
    parseProgram();
    printf(".intel_syntax noprefix\n");
    for (int i = 0; all_funcdefs[i]; i++) {
        FuncDef *funcdef = all_funcdefs[i];
        CodegenFunc(funcdef);
    }
    return 0;
}
