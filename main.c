#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int Kind;

typedef struct Type {
    Kind kind;
    struct Type *ptr_to;
    int array_size;
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
    NameAndType *params_start;
    NameAndType *params_end;
    int param_len;
    NameAndType *lvar_table_start;
    NameAndType *lvar_table_end;
    Type *return_type;
} FuncDef;

typedef struct Stmt {
    Kind stmt_kind;
    struct Expr *expr;
    struct Expr *for_cond;
    struct Expr *for_after;
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
Token tokens_start[1000];
Token *tokens_end;
Token *tokens_cursor;
int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}
int is_reserved_then_handle(char *ptr, int *ptr_token_index, int *ptr_i, const char *keyword, int keyword_len, int kind) {
    if (strncmp(ptr, keyword, keyword_len) == 0 && !is_alnum(ptr[keyword_len])) {
        Token token = {kind, 0, 0};
        tokens_start[*ptr_token_index] = token;
        (*ptr_token_index)++;
        *ptr_i += keyword_len;
        return 1;
    }
    return 0;
}
int tokenize(char *str) {
    int token_index = 0;
    for (int i = 0; str[i];) {
        char c = str[i];
        char *ptr = str + i;
        if (is_reserved_then_handle(ptr, &token_index, &i, "return", 6, enum3('R', 'E', 'T'))) {
            continue;
        }
        if (is_reserved_then_handle(ptr, &token_index, &i, "sizeof", 6, enum4('S', 'Z', 'O', 'F'))) {
            continue;
        }
        if (is_reserved_then_handle(ptr, &token_index, &i, "if", 2, enum2('i', 'f'))) {
            continue;
        }
        if (is_reserved_then_handle(ptr, &token_index, &i, "while", 5, enum4('W', 'H', 'I', 'L'))) {
            continue;
        }
        if (is_reserved_then_handle(ptr, &token_index, &i, "else", 4, enum4('e', 'l', 's', 'e'))) {
            continue;
        }
        if (is_reserved_then_handle(ptr, &token_index, &i, "for", 3, enum3('f', 'o', 'r'))) {
            continue;
        }
        if (is_reserved_then_handle(ptr, &token_index, &i, "int", 3, enum3('i', 'n', 't'))) {
            continue;
        }
        if (c == '+') {
            Token token = {'+', 0, 0};
            tokens_start[token_index] = token;
            token_index++;
            i++;
        } else if (c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == '[' || c == ']') {
            tokens_start[token_index].kind = c;
            token_index++;
            i++;
        } else if (c == '-') {
            Token token = {'-', 0, 0};
            tokens_start[token_index] = token;
            token_index++;
            i++;
        } else if (c == '*') {
            Token token = {'*', 0, 0};
            tokens_start[token_index] = token;
            token_index++;
            i++;
        } else if (c == '&') {
            Token token = {'&', 0, 0};
            tokens_start[token_index] = token;
            token_index++;
            i++;
        } else if (c == '/') {
            Token token = {'/', 0, 0};
            tokens_start[token_index] = token;
            token_index++;
            i++;
        } else if (c == '>') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'>', 0, 0};
                tokens_start[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('>', '='), 0, 0};
                tokens_start[token_index] = token;
                token_index++;
            }
        } else if (c == '<') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'<', 0, 0};
                tokens_start[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('<', '='), 0, 0};
                tokens_start[token_index] = token;
                token_index++;
            }
        } else if (c == '=') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'=', 0, 0};
                tokens_start[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('=', '='), 0, 0};
                tokens_start[token_index] = token;
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
            tokens_start[token_index] = token;
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
            tokens_start[token_index] = token;
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
            tokens_start[token_index] = token;
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
    t->kind = enum3('i', 'n', 't');
    return t;
}

Type *ptr_of(Type *t) {
    Type *new_t = calloc(1, sizeof(Type));
    new_t->kind = '*';
    new_t->ptr_to = t;
    return new_t;
}

Type *deref(Type *t) {
    if (t->kind == '*') {
        return t->ptr_to;
    } else {
        fprintf(stderr, "Cannot deref a non-pointer type\n");
        exit(1);
    }
}

int size(Type *t) {
    if (t->kind == '*') {
        return 8;
    } else if (t->kind == enum3('i', 'n', 't')) {
        return 4;
    } else if (t->kind == enum2('[', ']')) {
        return t->array_size * size(t->ptr_to);
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

Expr *decay_if_arr(Expr *first_child) {
    if (first_child->typ->kind == enum2('[', ']')) {
        Type *t = calloc(1, sizeof(Type));
        t->ptr_to = first_child->typ->ptr_to;
        t->kind = '*';
        return unaryExpr(first_child, enum4('[', ']', '>', '*'), t);
    }
    return first_child;
}

int maybe_consume(Kind kind) {
    if (tokens_cursor->kind == kind) {
        tokens_cursor++;
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
        fprintf(stderr, "expected TokenKind `%s`, got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens_cursor->kind));
        exit(1);
    }
}

void expect_otherwise_panic(int kind) {
    if (tokens_cursor->kind != kind) {
        fprintf(stderr, "expected TokenKind `%s`, got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens_cursor->kind));
        exit(1);
    }
}

void panic_if_eof() {
    if (tokens_cursor >= tokens_end) {
        fprintf(stderr, "EOF encountered");
        exit(1);
    }
}

Expr *parseExpr(void);

NameAndType *lvars_start;
NameAndType *lvars_cursor;
NameAndType *funcdecls_start[100];
NameAndType **funcdecls_cursor;
FuncDef *funcdefs_start[100];

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
    for (int i = 0; funcdecls_start[i]; i++) {
        if (strcmp(funcdecls_start[i]->name, name) == 0) {
            return funcdecls_start[i]->type;
        }
    }
    fprintf(stderr, "cannot find a function named `%s`; cannot determine the return type. Implicitly assumes that it return an int\n", name);
    return type_int();
}

Expr *callingExpr(char *name, Expr **arguments, int len) {
    Expr *callexp = calloc(1, sizeof(Expr));
    callexp->func_or_ident_name = name;
    callexp->expr_kind = enum4('C', 'A', 'L', 'L');
    callexp->func_args = arguments;
    callexp->func_arg_len = len;
    callexp->typ = lookup_func_type(name);
    return callexp;
}

Expr *parsePrimary() {
    panic_if_eof();
    if (tokens_cursor->kind == enum3('N', 'U', 'M')) {
        int value = tokens_cursor->value;
        tokens_cursor++;
        return numberexpr(value);
    } else if (tokens_cursor->kind == enum4('I', 'D', 'N', 'T')) {
        char *name = tokens_cursor->identifier_name;
        tokens_cursor++;
        if (maybe_consume('(')) {
            Expr **arguments = calloc(6, sizeof(Expr *));
            if (maybe_consume(')')) {
                return callingExpr(name, arguments, 0);
            }
            int i = 0;
            for (; i < 6; i++) {
                Expr *expr = decay_if_arr(parseExpr());
                if (maybe_consume(')')) {
                    arguments[i] = expr;
                    break;
                }
                consume_otherwise_panic(',');
                arguments[i] = decay_if_arr(expr);
            }
            return callingExpr(name, arguments, i + 1);
        } else {
            Expr *ident_exp = calloc(1, sizeof(Expr));
            ident_exp->func_or_ident_name = name;
            ident_exp->expr_kind = enum4('I', 'D', 'N', 'T');
            ident_exp->typ = lookup_ident_type(name);
            return ident_exp;
        }
    }

    consume_otherwise_panic('(');
    Expr *expr = parseExpr();  // NO DECAY
    consume_otherwise_panic(')');
    return expr;
}

Expr *assert_int(Expr *e) {
    if (e->typ->kind != enum3('i', 'n', 't')) {
        fprintf(stderr, "int is expected, but not an int\n");
        exit(1);
    }
    return e;
}

void display_type(Type *t) {
    if (t->kind == enum2('[', ']')) {
        fprintf(stderr, "[%d] ", t->array_size);
        display_type(t->ptr_to);
    } else if (t->kind == '*') {
        fprintf(stderr, "* ");
        display_type(t->ptr_to);
    } else {
        fprintf(stderr, "%s", decode_kind(t->kind));
    }
}

void assert_same_type(Type *t1, Type *t2) {
    if (t1->kind == '*' && t2->kind == '*') {
        assert_same_type(t1->ptr_to, t2->ptr_to);
    } else if (t1->kind != t2->kind) {
        fprintf(stderr, "two different types detected: `");
        display_type(t1);
        fprintf(stderr, "` and `");
        display_type(t2);
        fprintf(stderr, "`.\n");
        exit(1);
    }
}

Expr *expr_add(Expr *lhs, Expr *rhs) {
    if (lhs->typ->kind == enum3('i', 'n', 't')) {
        if (rhs->typ->kind == enum3('i', 'n', 't')) {
            return binaryExpr(lhs, rhs, '+', type_int());
        } else if (rhs->typ->kind == '*') {
            return expr_add(rhs, lhs);
        } else {
            fprintf(stderr, "unknown type in addition\n");
            exit(1);
        }
    } else if (lhs->typ->kind == '*') {
        if (rhs->typ->kind == enum3('i', 'n', 't')) {
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

Expr *expr_subtract(Expr *lhs, Expr *rhs) {
    if (lhs->typ->kind == enum3('i', 'n', 't')) {
        if (rhs->typ->kind == enum3('i', 'n', 't')) {
            return binaryExpr(lhs, rhs, '-', type_int());
        } else if (rhs->typ->kind == '*') {
            fprintf(stderr, "cannot subtract\n");
            exit(1);
        }
    } else if (lhs->typ->kind == '*') {
        if (rhs->typ->kind == enum3('i', 'n', 't')) {
            return binaryExpr(lhs, binaryExpr(numberexpr(size(deref(lhs->typ))), rhs, '*', type_int()), '-', lhs->typ);
        } else if (rhs->typ->kind == '*') {
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

Expr *parsePostfix() {
    Expr *result = parsePrimary();
    while (maybe_consume('[')) {
        Expr *addition = expr_add(decay_if_arr(result), decay_if_arr(parseExpr()));
        consume_otherwise_panic(']');
        Expr *expr = decay_if_arr(addition);
        result = unaryExpr(expr, '*', deref(expr->typ));
    }
    return result;
}

Expr *parseUnary();
Expr *parseCast() {
    return parseUnary();
}

Expr *parseUnary() {
    panic_if_eof();
    if (maybe_consume('+')) {
        return assert_int(parseCast());
    } else if (maybe_consume('-')) {
        return binaryExpr(numberexpr(0), assert_int(parseCast()), '-', type_int());
    } else if (maybe_consume('*')) {
        Expr *expr = decay_if_arr(parseCast());
        return unaryExpr(expr, '*', deref(expr->typ));
    } else if (maybe_consume('&')) {
        Expr *expr = parseCast();                        // NO DECAY
        return unaryExpr(expr, '&', ptr_of(expr->typ));  // NO DECAY
    } else if (maybe_consume(enum4('S', 'Z', 'O', 'F'))) {
        Expr *expr = parseUnary();  // NO DECAY
        return numberexpr(size(expr->typ));
    }
    return parsePostfix();
}

Expr *parseMultiplicative() {
    panic_if_eof();
    Expr *result = parseUnary();
    while (tokens_cursor < tokens_end) {
        if (tokens_cursor->kind == enum3('N', 'U', 'M')) {
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

Expr *parseAdditive() {
    panic_if_eof();
    Expr *result = parseMultiplicative();
    while (tokens_cursor < tokens_end) {
        if (tokens_cursor->kind == enum3('N', 'U', 'M')) {
            fprintf(stderr, "Expected operator, got Number");
            exit(1);
        } else if (maybe_consume('-')) {
            result = expr_subtract(decay_if_arr(result), decay_if_arr(parseMultiplicative()));
        } else if (maybe_consume('+')) {
            result = expr_add(decay_if_arr(result), decay_if_arr(parseMultiplicative()));
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseRelational() {
    panic_if_eof();
    Expr *result = parseAdditive();
    while (tokens_cursor < tokens_end) {
        if (maybe_consume('>')) {
            result = binaryExpr(decay_if_arr(result), decay_if_arr(parseAdditive()), '>', type_int());
        } else if (maybe_consume(enum2('>', '='))) {
            result = binaryExpr(decay_if_arr(result), decay_if_arr(parseAdditive()), enum2('>', '='), type_int());
        } else if (maybe_consume('<')) {
            result = binaryExpr(decay_if_arr(parseAdditive()), decay_if_arr(result), '>', type_int());  // children & operator swapped
        } else if (maybe_consume(enum2('<', '='))) {
            result = binaryExpr(decay_if_arr(parseAdditive()), decay_if_arr(result), enum2('>', '='), type_int());  // children & operator swapped
        } else {
            return result;
        }
    }
    return result;
}

Expr *parseEquality() {
    panic_if_eof();
    Expr *result = parseRelational();
    while (tokens_cursor < tokens_end) {
        if (maybe_consume(enum2('=', '='))) {
            result = binaryExpr(decay_if_arr(result), decay_if_arr(parseRelational()), enum2('=', '='), type_int());
        } else if (maybe_consume(enum2('!', '='))) {
            result = binaryExpr(decay_if_arr(result), decay_if_arr(parseRelational()), enum2('!', '='), type_int());
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
        Expr *rhs = decay_if_arr(parseAssign());
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
    Expr *expr = decay_if_arr(parseExpr());
    consume_otherwise_panic(target);
    return expr;
}

NameAndType *consume_type_and_ident_1st_half() {
    consume_otherwise_panic(enum3('i', 'n', 't'));

    Type *type = calloc(1, sizeof(Type));
    type->kind = enum3('i', 'n', 't');

    while (maybe_consume('*')) {
        type = ptr_of(type);
    }

    expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
    char *name = tokens_cursor->identifier_name;
    tokens_cursor++;

    NameAndType *ans = calloc(1, sizeof(NameAndType));
    ans->name = name;
    ans->type = type;
    return ans;
}

NameAndType *consume_type_and_ident_2nd_half(NameAndType *ans) {
    Type *elem_t = ans->type;
    Type *insertion_point;

    if (maybe_consume('[')) {
        expect_otherwise_panic(enum3('N', 'U', 'M'));
        Type *t = calloc(1, sizeof(Type));
        t->array_size = tokens_cursor->value;
        t->ptr_to = elem_t;
        t->kind = enum2('[', ']');
        insertion_point = t;
        tokens_cursor++;
        consume_otherwise_panic(']');
        ans->type = t;
    }
    while (maybe_consume('[')) {
        expect_otherwise_panic(enum3('N', 'U', 'M'));
        Type *t = calloc(1, sizeof(Type));
        t->array_size = tokens_cursor->value;
        t->ptr_to = elem_t;
        t->kind = enum2('[', ']');
        insertion_point->ptr_to = t;
        insertion_point = t;
        tokens_cursor++;
        consume_otherwise_panic(']');
    }

    return ans;
}

NameAndType *consume_type_and_ident() {
    NameAndType *ans = consume_type_and_ident_1st_half();
    return consume_type_and_ident_2nd_half(ans);
}

Stmt *parseStmt() {
    if (maybe_consume('{')) {
        Stmt *result = calloc(1, sizeof(Stmt));
        result->stmt_kind = enum4('e', 'x', 'p', 'r');
        result->expr = numberexpr(42);
        while (tokens_cursor->kind != '}') {
            Stmt *newstmt = calloc(1, sizeof(Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
            newstmt->second_child = parseStmt();
            result = newstmt;
        }
        tokens_cursor++;
        return result;
    }
    if (maybe_consume(enum3('R', 'E', 'T'))) {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        stmt->stmt_kind = enum3('R', 'E', 'T');
        stmt->expr = decay_if_arr(parseExpr());
        consume_otherwise_panic(';');
        return stmt;
    }
    if (maybe_consume(enum2('i', 'f'))) {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        consume_otherwise_panic('(');
        stmt->expr = decay_if_arr(parseExpr());
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
        stmt->expr = decay_if_arr(parseExpr());
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
        stmt->for_cond = parseOptionalExprAndToken(';');
        stmt->for_after = parseOptionalExprAndToken(')');
        stmt->second_child = parseStmt();
        return stmt;
    }
    if (maybe_consume(enum3('i', 'n', 't'))) {
        tokens_cursor--;
        NameAndType *nt = consume_type_and_ident();
        if (lvars_cursor == lvars_start + 100) {
            fprintf(stderr, "too many local variables");
            exit(1);
        }
        lvars_cursor->name = nt->name;
        lvars_cursor->type = nt->type;
        lvars_cursor++;
        consume_otherwise_panic(';');
        Stmt *stmt = calloc(1, sizeof(Stmt));
        stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
        stmt->expr = numberexpr(42);
        return stmt;
    }
    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
    stmt->expr = decay_if_arr(parseExpr());
    consume_otherwise_panic(';');
    return stmt;
}

Stmt *parseFunctionContent() {
    consume_otherwise_panic('{');
    Stmt *result = calloc(1, sizeof(Stmt));
    result->stmt_kind = enum4('e', 'x', 'p', 'r');
    result->expr = numberexpr(1);
    while (tokens_cursor->kind != '}') {
        Stmt *statement = parseStmt();
        Stmt *newstmt = calloc(1, sizeof(Stmt));
        newstmt->first_child = result;
        newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
        newstmt->second_child = statement;
        result = newstmt;
    }
    tokens_cursor++;
    return result;
}

FuncDef *constructFuncDef(Stmt *content, NameAndType *rettype_and_funcname, int len, NameAndType *params_start) {
    FuncDef *funcdef = calloc(1, sizeof(FuncDef));
    funcdef->content = content;
    funcdef->name = rettype_and_funcname->name;
    funcdef->return_type = rettype_and_funcname->type;
    funcdef->param_len = len;
    funcdef->params_start = params_start;
    funcdef->params_end = params_start + len;
    funcdef->lvar_table_start = lvars_start;
    funcdef->lvar_table_end = lvars_cursor;
    return funcdef;
}

void store_func_decl(NameAndType *rettype_and_funcname) {
    NameAndType *decl = calloc(1, sizeof(NameAndType));
    decl->type = rettype_and_funcname->type;
    decl->name = rettype_and_funcname->name;
    *funcdecls_cursor = decl;
    funcdecls_cursor++;
}

FuncDef *parseToplevel() {
    NameAndType *first_half = consume_type_and_ident_1st_half();
    NameAndType *rettype_and_funcname = consume_type_and_ident_2nd_half(first_half);
    NameAndType *params_start = calloc(6, sizeof(NameAndType));
    consume_otherwise_panic('(');
    if (maybe_consume(')')) {
        lvars_cursor = lvars_start = calloc(100, sizeof(NameAndType));
        store_func_decl(rettype_and_funcname);
        return constructFuncDef(parseFunctionContent(), rettype_and_funcname, 0, params_start);
    }

    lvars_cursor = lvars_start = calloc(100, sizeof(char *));
    int i = 0;
    for (; i < 6; i++) {
        NameAndType *param = consume_type_and_ident();
        if (maybe_consume(')')) {
            params_start[i].name = param->name;
            params_start[i].type = param->type;
            lvars_cursor->name = param->name;
            lvars_cursor->type = param->type;
            lvars_cursor++;
            break;
        }
        consume_otherwise_panic(',');
        params_start[i].name = param->name;
        params_start[i].type = param->type;
        lvars_cursor->name = param->name;
        lvars_cursor->type = param->type;
        lvars_cursor++;
    }
    store_func_decl(rettype_and_funcname);
    return constructFuncDef(parseFunctionContent(), rettype_and_funcname, i + 1, params_start);
}

void parseProgram() {
    FuncDef **funcdefs_cursor = funcdefs_start;
    while (tokens_cursor < tokens_end) {
        *funcdefs_cursor = parseToplevel();
        funcdefs_cursor++;
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

LVar *insertLVar(char *name, int sz) {
    sz = (sz + 7) / 8 * 8;
    LVar *newlocal = calloc(1, sizeof(LVar));
    LVar *last = lastLVar();
    newlocal->name = name;
    if (!last) {
        newlocal->offset_from_rbp = sz;
    } else {
        newlocal->offset_from_rbp = last->offset_from_rbp + sz;
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
        if (stmt->for_cond) {
            EvaluateExprIntoRax(stmt->for_cond);
        } else {
            printf("  mov rax, 1\n");
        }
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        CodegenStmt(stmt->second_child);
        if (stmt->for_after) {
            EvaluateExprIntoRax(stmt->for_after);
        }
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    }
}

const char *nth_arg_reg(int n, int sz) {
    if (sz == 8) {
        return "rdi\0rsi\0rdx\0rcx\0r8 \0r9" + 4 * n;
    } else if (sz == 4) {
        return "edi\0esi\0edx\0ecx\0r8d\0r9d" + 4 * n;
    } else {
        fprintf(stderr, "unhandlable size %d\n", sz);
        exit(1);
    }
}

void CodegenFunc(FuncDef *funcdef) {
    printf(".globl %s\n", funcdef->name);
    printf("%s:\n", funcdef->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    int stack_adjust = 0;
    for (int i = 0; i < funcdef->param_len; i++) {
        stack_adjust += 8;
    }
    for (NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++) {
        stack_adjust += (size(ptr->type) + 7) / 8 * 8;
    }
    printf("  sub rsp, %d\n", stack_adjust);
    for (int i = 0; i < funcdef->param_len; i++) {
        char *param_name = funcdef->params_start[i].name;
        insertLVar(param_name, 8);
        LVar *local = findLVar(param_name);
        printf("  mov [rbp - %d], %s\n", local->offset_from_rbp, nth_arg_reg(i, 8));
    }
    for (NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++) {
        insertLVar(ptr->name, size(ptr->type));
    }
    CodegenStmt(funcdef->content);
}

const char *eax_or_rax(int sz) {
    if (sz == 8) {
        return "rax";
    } else if (sz == 4) {
        return "eax";
    } else {
        fprintf(stderr, "unhandlable size %d\n", sz);
        exit(1);
    }
}

void EvaluateExprIntoRax(Expr *expr) {
    if (expr->typ->kind == enum2('[', ']')) {
        EvaluateLValueAddressIntoRax(expr);
        return;
    }

    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        EvaluateLValueAddressIntoRax(expr);
        printf("  mov %s,[rax]\n", eax_or_rax(size(expr->typ)));
        return;
    } else if (expr->expr_kind == enum4('C', 'A', 'L', 'L')) {
        for (int i = 0; i < expr->func_arg_len; i++) {
            EvaluateExprIntoRax(expr->func_args[i]);
            printf("    push rax\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--) {
            printf("    pop %s\n", nth_arg_reg(i, 8));
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
        } else if (expr->op == enum4('[', ']', '>', '*')) {
            EvaluateExprIntoRax(expr->first_child);
        } else {
            fprintf(stderr, "Invalid unaryop kind:%d", expr->op);
            exit(1);
        }
    } else if (expr->expr_kind == enum4('2', 'A', 'R', 'Y')) {
        if (expr->op == '=') {
            EvaluateLValueAddressIntoRax(expr->first_child);
            printf("    push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    pop rdi\n");
            printf("    mov [rdi], %s\n", eax_or_rax(size(expr->second_child->typ)));
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
    tokens_cursor = tokens_start;
    tokens_end = tokens_start + tokens_length;

    funcdecls_cursor = funcdecls_start;
    parseProgram();
    printf(".intel_syntax noprefix\n");
    for (int i = 0; funcdefs_start[i]; i++) {
        FuncDef *funcdef = funcdefs_start[i];
        CodegenFunc(funcdef);
    }
    return 0;
}
