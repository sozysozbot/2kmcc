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
    struct Expr **func_args_start;
    int func_arg_len;
    char *func_or_ident_name_or_string_content;
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
} Stmt;

typedef struct LVar {
    struct LVar *next;
    char *name;
    int offset_from_rbp;
} LVar;

typedef struct Token {
    Kind kind;
    int value_or_string_size;  // includes the null terminator, so length+1
    char *identifier_name_or_escaped_string_content;
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

void panic(const char *msg) {
    fprintf(stderr, "%s", msg);
    exit(1);
}

/*** ^ LIB | v PARSE ***/
Token tokens_start[1000];
Token *tokens_end;
Token *tokens_cursor;
char *string_literals_start[10000];
char **string_literals_cursor;
int is_alnum(char c) {
    return strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", c) != 0;
}
int is_reserved_then_handle(char *ptr, int *ptr_i, const char *keyword, int keyword_len, int kind) {
    if (strncmp(ptr, keyword, keyword_len) != 0)
        return 0;
    if (is_alnum(ptr[keyword_len]))
        return 0;
    (tokens_cursor++)->kind = kind;
    *ptr_i += keyword_len;
    return 1;
}
Token *tokenize(char *str) {
    for (int i = 0; str[i];) {
        char c = str[i];
        if (is_reserved_then_handle(str + i, &i, "return", 6, enum3('R', 'E', 'T'))) {
        } else if (is_reserved_then_handle(str + i, &i, "sizeof", 6, enum4('S', 'Z', 'O', 'F'))) {
        } else if (is_reserved_then_handle(str + i, &i, "struct", 6, enum4('S', 'T', 'R', 'U'))) {
        } else if (is_reserved_then_handle(str + i, &i, "if", 2, enum2('i', 'f'))) {
        } else if (is_reserved_then_handle(str + i, &i, "while", 5, enum4('W', 'H', 'I', 'L'))) {
        } else if (is_reserved_then_handle(str + i, &i, "else", 4, enum4('e', 'l', 's', 'e'))) {
        } else if (is_reserved_then_handle(str + i, &i, "for", 3, enum3('f', 'o', 'r'))) {
        } else if (is_reserved_then_handle(str + i, &i, "int", 3, enum3('i', 'n', 't'))) {
        } else if (is_reserved_then_handle(str + i, &i, "char", 4, enum4('c', 'h', 'a', 'r'))) {
        } else if (strncmp(str + i, "//", 2) == 0) {
            i += 2;
            while (str[i] && str[i] != '\n')
                i += 1;
        } else if (strncmp(str + i, "/*", 2) == 0) {
            char *q = strstr(str + i + 2, "*/");
            if (!q)
                panic("unclosed block comment\n");
            i = q + 2 - str;
        } else if (strncmp(str + i, "->", 2) == 0) {
            (tokens_cursor++)->kind = enum2('-', '>');
            i += 2;
        } else if (c == '\'') {
            if (str[i + 1] != '\\') {
                tokens_cursor->kind = enum3('N', 'U', 'M');
                (tokens_cursor++)->value_or_string_size = str[i + 1];
                i += 3;
            } else if (strchr("\\'\"?", str[i + 2])) {
                tokens_cursor->kind = enum3('N', 'U', 'M');
                (tokens_cursor++)->value_or_string_size = str[i + 2];
                i += 4;
            } else if (str[i + 2] == 'n') {
                tokens_cursor->kind = enum3('N', 'U', 'M');
                (tokens_cursor++)->value_or_string_size = '\n';
                i += 4;
            } else {
                fprintf(stderr, "Not supported: Unsupported escape sequence within a character literal: `\\%c`\n", str[i + 2]);
                exit(1);
            }
        } else if (c == '"') {
            int parsed_length = 0;
            int true_semantic_length = 0;
            for (i += 1; str[i + parsed_length] != '"'; parsed_length++) {
                true_semantic_length++;
                if (!str[i + parsed_length])
                    panic("unterminated string literal");
                if (str[i + parsed_length] == '\\') {
                    if (strchr("01234567x", str[i + parsed_length + 1]))
                        panic("Not supported: Unsupported escape sequence within a string literal\n");
                    parsed_length++;
                }
            }
            char *escaped_string_content = calloc(parsed_length + 1, sizeof(char));
            strncpy(escaped_string_content, str + i, parsed_length);
            i += parsed_length + 1;  // must also skip the remaining double-quote
            tokens_cursor->kind = enum3('S', 'T', 'R');
            tokens_cursor->value_or_string_size = true_semantic_length + 1;
            (tokens_cursor++)->identifier_name_or_escaped_string_content = escaped_string_content;
            *(string_literals_cursor++) = escaped_string_content;
        } else if (strchr(";(){},[]~.", c)) {  // these chars do not start a multichar token
            (tokens_cursor++)->kind = c;
            i += 1;
        } else if (strchr("+-*/&><=!%^|", c)) {
            i += 1;
            if (str[i] == '=') {  // compound assign, equality, compare
                i += 1;
                (tokens_cursor++)->kind = enum2(c, '=');
            } else if (str[i] != c)  // all remaining operators have the same 1st & 2nd char
                (tokens_cursor++)->kind = c;
            else if (strchr("+-&|", c)) {
                i += 1;
                (tokens_cursor++)->kind = enum2(c, c);
            } else if (strchr("<>", c))
                panic(">>, <<, >>=, <<= not supported");
            else
                (tokens_cursor++)->kind = c;
        } else if (strchr("0123456789", c)) {
            int parsed_num;
            for (parsed_num = 0; strchr("0123456789", str[i]); i++)
                parsed_num = parsed_num * 10 + (str[i] - '0');
            tokens_cursor->kind = enum3('N', 'U', 'M');
            (tokens_cursor++)->value_or_string_size = parsed_num;
        } else if (is_alnum(c)) {  // 0-9 already excluded in the previous `if`
            char *start = &str[i];
            for (i++; is_alnum(str[i]); i++) {
            }
            int length = &str[i] - start;
            char *name = calloc(length + 1, sizeof(char));
            memcpy(name, start, length);
            tokens_cursor->kind = enum4('I', 'D', 'N', 'T');
            (tokens_cursor++)->identifier_name_or_escaped_string_content = name;
        } else if (strchr(" \n", c)) {
            i += 1;
        } else {
            fprintf(stderr, "unknown character %c(%d)\n", c, c);
            exit(1);
        }
    }
    return tokens_cursor;
}

/*** ^ TOKENIZE | v PARSE ***/

Type *type(Kind kind) {
    Type *t = calloc(1, sizeof(Type));
    t->kind = kind;
    return t;
}

Type *ptr_of(Type *t) {
    Type *new_t = type('*');
    new_t->ptr_to = t;
    return new_t;
}

Type *arr_of(Type *t, int array_size) {
    Type *new_t = type(enum2('[', ']'));
    new_t->ptr_to = t;
    new_t->array_size = array_size;
    return new_t;
}

Type *deref(Type *t) {
    if (t->kind == '*')
        return t->ptr_to;
    fprintf(stderr, "Cannot deref a non-pointer type\n");
    exit(1);
}

int size(Type *t) {
    if (t->kind == '*') {
        return 8;
    } else if (t->kind == enum3('i', 'n', 't')) {
        return 4;
    } else if (t->kind == enum4('c', 'h', 'a', 'r')) {
        return 1;
    } else if (t->kind == enum2('[', ']')) {
        return t->array_size * size(t->ptr_to);
    }
    fprintf(stderr, "unknown size\n");
    exit(1);
}

Expr *numberExpr(int value) {
    Expr *numberexp = calloc(1, sizeof(Expr));
    numberexp->value = value;
    if (value) {
        numberexp->expr_kind = enum3('N', 'U', 'M');
    } else {
        numberexp->expr_kind = '0';  //  An integer constant expression with the value 0 ... is called a null pointer constant
    }
    numberexp->typ = type(enum3('i', 'n', 't'));
    return numberexp;
}

Expr *unaryExpr(Expr *first_child, Kind unaryop, Type *typ) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('1', 'A', 'R', 'Y');
    newexp->op = unaryop;
    newexp->typ = typ;
    return newexp;
}

Expr *binaryExpr(Expr *first_child, Expr *second_child, Kind binaryop, Type *typ) {
    Expr *newexp = unaryExpr(first_child, binaryop, typ);
    newexp->expr_kind = enum4('2', 'A', 'R', 'Y');
    newexp->second_child = second_child;
    return newexp;
}

Expr *decay_if_arr(Expr *first_child) {
    if (first_child->typ->kind != enum2('[', ']'))
        return first_child;
    Type *t = calloc(1, sizeof(Type));
    t->ptr_to = first_child->typ->ptr_to;
    t->kind = '*';
    return unaryExpr(first_child, enum4('[', ']', '>', '*'), t);
}

int maybe_consume(Kind kind) {
    if (tokens_cursor->kind != kind)
        return 0;
    tokens_cursor += 1;
    return 1;
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
        fprintf(stderr, "expected TokenKind `%s`; got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens_cursor->kind));
        exit(1);
    }
}

void expect_otherwise_panic(int kind) {
    if (tokens_cursor->kind != kind) {
        fprintf(stderr, "expected TokenKind `%s`; got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens_cursor->kind));
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
FuncDef **funcdefs_cursor;
NameAndType *global_vars_start[100];
NameAndType **global_vars_cursor;

Type *lookup_ident_type(char *name) {
    for (int i = 0; lvars_start[i].name; i++)
        if (strcmp(lvars_start[i].name, name) == 0)
            return lvars_start[i].type;
    for (int i = 0; global_vars_start[i]; i++)
        if (strcmp(global_vars_start[i]->name, name) == 0)
            return global_vars_start[i]->type;
    fprintf(stderr, "cannot find an identifier named `%s`; cannot determine the type\n", name);
    exit(1);
}

Type *lookup_func_type(char *name) {
    for (int i = 0; funcdecls_start[i]; i++)
        if (strcmp(funcdecls_start[i]->name, name) == 0)
            return funcdecls_start[i]->type;
    fprintf(stderr, "cannot find a function named `%s`. Implicitly assumes that it return an int\n", name);
    return type(enum3('i', 'n', 't'));
}

Expr *callingExpr(char *name, Expr **arguments, int len) {
    Expr *callexp = calloc(1, sizeof(Expr));
    callexp->func_or_ident_name_or_string_content = name;
    callexp->expr_kind = enum4('C', 'A', 'L', 'L');
    callexp->func_args_start = arguments;
    callexp->func_arg_len = len;
    callexp->typ = lookup_func_type(name);
    return callexp;
}

Expr *identExpr(char *name) {
    Expr *ident_exp = calloc(1, sizeof(Expr));
    ident_exp->func_or_ident_name_or_string_content = name;
    ident_exp->expr_kind = enum4('I', 'D', 'N', 'T');
    ident_exp->typ = lookup_ident_type(name);
    return ident_exp;
}

Expr *parsePrimary() {
    panic_if_eof();
    if (tokens_cursor->kind == enum3('N', 'U', 'M'))
        return numberExpr((tokens_cursor++)->value_or_string_size);
    else if (tokens_cursor->kind == enum3('S', 'T', 'R')) {
        int str_size = tokens_cursor->value_or_string_size;
        char *str_content = (tokens_cursor++)->identifier_name_or_escaped_string_content;
        Expr *string_literal_exp = calloc(1, sizeof(Expr));
        string_literal_exp->func_or_ident_name_or_string_content = str_content;
        string_literal_exp->expr_kind = enum3('S', 'T', 'R');
        string_literal_exp->typ = arr_of(type(enum4('c', 'h', 'a', 'r')), str_size);
        return string_literal_exp;
    } else if (tokens_cursor->kind == enum4('I', 'D', 'N', 'T')) {
        char *name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
        if (maybe_consume('(')) {
            Expr **arguments = calloc(6, sizeof(Expr *));
            if (maybe_consume(')'))
                return callingExpr(name, arguments, 0);
            int i = 0;
            for (; i < 6; i++) {
                arguments[i] = decay_if_arr(parseExpr());
                if (maybe_consume(')'))
                    break;
                consume_otherwise_panic(',');
            }
            return callingExpr(name, arguments, i + 1);
        }
        return identExpr(name);
    }
    consume_otherwise_panic('(');
    Expr *expr = parseExpr();  // NO DECAY
    consume_otherwise_panic(')');
    return expr;
}

int is_int_or_char(Kind kind) {
    return (kind == enum3('i', 'n', 't')) + (kind == enum4('c', 'h', 'a', 'r'));
}

int starts_a_type(Kind kind) {
    return is_int_or_char(kind) + (kind == enum4('S', 'T', 'R', 'U'));
}

int is_integer(Type *typ) {
    return is_int_or_char(typ->kind);
}

Expr *assert_integer(Expr *e) {
    if (is_integer(e->typ)) {
        return e;
    }
    fprintf(stderr, "int/char is expected, but not an int/char\n");
    exit(1);
}

void display_type(Type *t) {
    if (t->kind == enum2('[', ']')) {
        fprintf(stderr, "array (length: %d) of ", t->array_size);
        display_type(t->ptr_to);
    } else if (t->kind == '*') {
        fprintf(stderr, "pointer to ");
        display_type(t->ptr_to);
    } else
        fprintf(stderr, "%s", decode_kind(t->kind));
}

int is_same_type(Type *t1, Type *t2) {
    if (t1->kind == '*' && t2->kind == '*') {
        return is_same_type(t1->ptr_to, t2->ptr_to);
    }
    return t1->kind == t2->kind;
}

void panic_two_types(const char *msg, Type *t1, Type *t2) {
    fprintf(stderr, "%s: `", msg);
    display_type(t1);
    fprintf(stderr, "` and `");
    display_type(t2);
    fprintf(stderr, "`.\n");
    exit(1);
}

int is_compatible_type(Type *t1, Type *t2) {
    if (t1->kind == '*' && t2->kind == '*') {
        return is_same_type(t1->ptr_to, t2->ptr_to);
    }
    return !(t1->kind != t2->kind && !(is_integer(t1) && is_integer(t2)));
}

Expr *expr_add(Expr *lhs, Expr *rhs) {
    if (is_integer(lhs->typ)) {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, rhs, '+', type(enum3('i', 'n', 't')));
        else if (rhs->typ->kind == '*')
            return expr_add(rhs, lhs);
        else
            panic("unknown type in addition\n");
    } else if (lhs->typ->kind == '*') {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, binaryExpr(numberExpr(size(deref(lhs->typ))), rhs, '*', type(enum3('i', 'n', 't'))), '+', lhs->typ);
        else
            panic("cannot add\n");
    }
    fprintf(stderr, "unknown type\n");
    exit(1);
}

Expr *expr_subtract(Expr *lhs, Expr *rhs) {
    if (is_integer(lhs->typ)) {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, rhs, '-', type(enum3('i', 'n', 't')));
        else if (rhs->typ->kind == '*')
            panic("cannot subtract a pointer from an integer\n");
    } else if (lhs->typ->kind == '*') {
        if (is_integer(rhs->typ)) {
            return binaryExpr(lhs, binaryExpr(numberExpr(size(deref(lhs->typ))), rhs, '*', type(enum3('i', 'n', 't'))), '-', lhs->typ);
        } else if (rhs->typ->kind == '*') {
            if (!is_same_type(lhs->typ, rhs->typ))
                panic_two_types("cannot subtract two expressions with different pointer types", lhs->typ, rhs->typ);
            return binaryExpr(binaryExpr(lhs, rhs, '-', type(enum3('i', 'n', 't'))), numberExpr(size(deref(lhs->typ))), '/', type(enum3('i', 'n', 't')));
        } else
            panic("cannot subtract: invalid type in the second operand\n");
    }
    fprintf(stderr, "unknown type\n");
    exit(1);
}

Expr *parsePostfix() {
    Expr *result = parsePrimary();
    while (1) {
        if (maybe_consume('[')) {
            Expr *addition = expr_add(decay_if_arr(result), decay_if_arr(parseExpr()));
            consume_otherwise_panic(']');
            Expr *expr = decay_if_arr(addition);
            result = unaryExpr(expr, '*', deref(expr->typ));
        } else if (maybe_consume(enum2('+', '+'))) {  // `a++` is `(a ++) - 1
            Expr *addition = expr_add(decay_if_arr(result), numberExpr(1));
            addition->op = enum2('+', '=');
            result = expr_subtract(addition, numberExpr(1));
        } else if (maybe_consume(enum2('-', '-'))) {  // `a--` is `(a -= 1) + 1
            Expr *subtraction = expr_subtract(decay_if_arr(result), numberExpr(1));
            subtraction->op = enum2('-', '=');
            result = expr_add(subtraction, numberExpr(1));
        } else
            break;
    }
    return result;
}

Expr *parseUnary();
Expr *parseCast() {
    return parseUnary();
}

Type *consume_simple_type();
Expr *equalityExpr(Expr *lhs, Expr *rhs, Kind kind);

Expr *parseUnary() {
    panic_if_eof();
    if (maybe_consume('+')) {
        return assert_integer(parseCast());
    } else if (maybe_consume('-')) {
        return binaryExpr(numberExpr(0), assert_integer(parseCast()), '-', type(enum3('i', 'n', 't')));
    } else if (maybe_consume('!')) {
        return equalityExpr(numberExpr(0), parseCast(), enum2('=', '='));  // The expression !E is equivalent to (0==E)
    } else if (maybe_consume('*')) {
        Expr *expr = decay_if_arr(parseCast());
        return unaryExpr(expr, '*', deref(expr->typ));
    } else if (maybe_consume('&')) {
        Expr *expr = parseCast();                        // NO DECAY
        return unaryExpr(expr, '&', ptr_of(expr->typ));  // NO DECAY
    } else if (maybe_consume(enum4('S', 'Z', 'O', 'F'))) {
        if (tokens_cursor->kind == '(') {
            if (starts_a_type((tokens_cursor + 1)->kind)) {
                tokens_cursor++;
                Type *typ = consume_simple_type();
                consume_otherwise_panic(')');
                return numberExpr(size(typ));
            } else {
                Expr *expr = parseUnary();  // NO DECAY
                return numberExpr(size(expr->typ));
            }
        } else {
            Expr *expr = parseUnary();  // NO DECAY
            return numberExpr(size(expr->typ));
        }
    }
    return parsePostfix();
}

void assert_compatible_in_equality(Expr *e1, Expr *e2) {
    if (is_compatible_type(e1->typ, e2->typ))
        return;
    if (e1->expr_kind == '0' && e2->typ->kind == '*')  // one operand is a pointer and the other is a null pointer constant
        return;
    if (e2->expr_kind == '0' && e1->typ->kind == '*')  // one operand is a pointer and the other is a null pointer constant
        return;
    panic_two_types("cannot compare (un)equal two operands with incompatible types", e1->typ, e2->typ);
}

Expr *equalityExpr(Expr *lhs, Expr *rhs, Kind kind) {
    assert_compatible_in_equality(decay_if_arr(lhs), decay_if_arr(rhs));
    return binaryExpr(decay_if_arr(lhs), decay_if_arr(rhs), kind, type(enum3('i', 'n', 't')));
}

int getPrecedence() {
    int kind = tokens_cursor->kind;
    if (kind == enum3('N', 'U', 'M'))
        panic("expected an operator; got a number");
    if (kind == '*' || kind == '/' || kind == '%') return 10;
    if (kind == '+' || kind == '-') return 9;
    if (kind == enum2('<', '<') || kind == enum2('>', '>')) return 8;
    if (kind == '<' || kind == enum2('<', '=') || kind == '>' || kind == enum2('>', '=')) return 7;
    if (kind == enum2('=', '=') || kind == enum2('!', '=')) return 6;
    if (kind == '&') return 5;
    if (kind == '^') return 4;
    if (kind == '|') return 3;
    if (kind == enum2('&', '&')) return 2;
    if (kind == enum2('|', '|')) return 1;
    return 0;
}

Expr *parseLeftToRightInfix(int level) {
    panic_if_eof();
    Expr *expr = parseUnary();
    while (tokens_cursor < tokens_end) {
        int precedence = getPrecedence();
        if (precedence < level)
            return expr;
        int op = (tokens_cursor++)->kind;
        if (precedence == 10)
            expr = binaryExpr(assert_integer(expr), assert_integer(parseUnary()), op, type(enum3('i', 'n', 't')));
        else if (precedence == 9)
            if (op == '-')
                expr = expr_subtract(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)));
            else
                expr = expr_add(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)));
        else if (op == '<' || op == enum2('<', '='))  // children & operator swapped
            expr = binaryExpr(decay_if_arr(parseLeftToRightInfix(precedence + 1)), decay_if_arr(expr), op - '<' + '>', type(enum3('i', 'n', 't')));
        else if (precedence == 6)
            expr = equalityExpr(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)), op);
        else
            expr = binaryExpr(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)), op, type(enum3('i', 'n', 't')));
    }
    return expr;
}

void assert_compatible_in_simple_assignment(Type *lhs_type, Expr *rhs) {
    if (is_compatible_type(lhs_type, rhs->typ))
        return;
    if (lhs_type->kind == '*' && rhs->expr_kind == '0')  // the left operand is an atomic, qualified, or unqualified pointer, and the right is a null pointer constant
        return;
    panic_two_types("cannot assign/initialize because two incompatible types are detected", lhs_type, rhs->typ);
}

Expr *parseAssign() {
    panic_if_eof();
    Expr *result = parseLeftToRightInfix(1);
    if (maybe_consume('=')) {
        Expr *rhs = decay_if_arr(parseAssign());
        assert_compatible_in_simple_assignment(result->typ, rhs);  // no decay, since we cannot assign to an array
        return binaryExpr(result, rhs, '=', result->typ);
    } else if (maybe_consume(enum2('+', '='))) {
        result = expr_add(decay_if_arr(result), assert_integer(parseAssign()));
        result->op = enum2('+', '=');
    } else if (maybe_consume(enum2('-', '='))) {
        result = expr_subtract(decay_if_arr(result), assert_integer(parseAssign()));
        result->op = enum2('-', '=');
    } else if (maybe_consume(enum2('*', '=')))
        result = binaryExpr(assert_integer(result), assert_integer(parseUnary()), enum2('*', '='), type(enum3('i', 'n', 't')));
    else if (maybe_consume(enum2('/', '=')))
        result = binaryExpr(assert_integer(result), assert_integer(parseUnary()), enum2('/', '='), type(enum3('i', 'n', 't')));
    return result;
}

Expr *parseExpr() {
    return parseAssign();
}

Expr *parseOptionalExprAndToken(Kind target) {
    if (maybe_consume(target))
        return 0;
    Expr *expr = decay_if_arr(parseExpr());
    consume_otherwise_panic(target);
    return expr;
}

Type *consume_simple_type() {
    Type *type = calloc(1, sizeof(Type));
    if (maybe_consume(enum3('i', 'n', 't')))
        type->kind = enum3('i', 'n', 't');
    else if (maybe_consume(enum4('c', 'h', 'a', 'r')))
        type->kind = enum4('c', 'h', 'a', 'r');
    else {
        fprintf(stderr, "expected `int` or `char`; got TokenKind `%s`\n", decode_kind(tokens_cursor->kind));
        exit(1);
    }
    while (maybe_consume('*'))
        type = ptr_of(type);
    return type;
}

NameAndType *consume_type_and_ident_1st_half() {
    Type *type = consume_simple_type();
    expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
    char *name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
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
        t->ptr_to = elem_t;
        t->kind = enum2('[', ']');
        t->array_size = (tokens_cursor++)->value_or_string_size;
        insertion_point = t;
        consume_otherwise_panic(']');
        ans->type = t;
    }
    while (maybe_consume('[')) {
        expect_otherwise_panic(enum3('N', 'U', 'M'));
        Type *t = calloc(1, sizeof(Type));
        t->ptr_to = elem_t;
        t->kind = enum2('[', ']');
        t->array_size = (tokens_cursor++)->value_or_string_size;
        insertion_point->ptr_to = t;
        insertion_point = t;
        consume_otherwise_panic(']');
    }
    return ans;
}

NameAndType *consume_type_and_ident() {
    NameAndType *ans = consume_type_and_ident_1st_half();
    return consume_type_and_ident_2nd_half(ans);
}

Stmt *parse_var_def_maybe_with_initializer() {
    NameAndType *var = consume_type_and_ident();
    lvars_cursor->name = var->name;
    (lvars_cursor++)->type = var->type;
    if (maybe_consume(';')) {
        Stmt *stmt = calloc(1, sizeof(Stmt));
        stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
        stmt->expr = numberExpr(42);
        return stmt;
    }
    consume_otherwise_panic('=');
    if (maybe_consume('{')) {
        panic("not supported: initializer list\n");
    }
    Expr *rhs = parseExpr();
    consume_otherwise_panic(';');
    Stmt *stmt = calloc(1, sizeof(Stmt));
    stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
    assert_compatible_in_simple_assignment(var->type, rhs);
    stmt->expr = binaryExpr(identExpr(var->name), rhs, '=', var->type);
    return stmt;
}

Stmt *parseStmt() {
    if (maybe_consume('{')) {
        Stmt *result = calloc(1, sizeof(Stmt));
        result->stmt_kind = enum4('e', 'x', 'p', 'r');
        result->expr = numberExpr(42);
        while (!maybe_consume('}')) {
            Stmt *newstmt = calloc(1, sizeof(Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
            newstmt->second_child = parseStmt();
            result = newstmt;
        }
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
        stmt->first_child = parseStmt();  // then-block
        if (maybe_consume(enum4('e', 'l', 's', 'e')))
            stmt->second_child = parseStmt();  // else-block
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
        Stmt *for_stmt = calloc(1, sizeof(Stmt));
        for_stmt->stmt_kind = enum3('f', 'o', 'r');
        consume_otherwise_panic('(');
        if (starts_a_type(tokens_cursor->kind)) {
            Stmt *initializer = parse_var_def_maybe_with_initializer();
            for_stmt->expr = numberExpr(42);
            for_stmt->for_cond = parseOptionalExprAndToken(';');
            for_stmt->for_after = parseOptionalExprAndToken(')');
            for_stmt->second_child = parseStmt();
            Stmt *combined_stmt = calloc(1, sizeof(Stmt));
            combined_stmt->first_child = initializer;
            combined_stmt->stmt_kind = enum4('n', 'e', 'x', 't');
            combined_stmt->second_child = for_stmt;
            return combined_stmt;
        } else {
            for_stmt->expr = parseOptionalExprAndToken(';');
            for_stmt->for_cond = parseOptionalExprAndToken(';');
            for_stmt->for_after = parseOptionalExprAndToken(')');
            for_stmt->second_child = parseStmt();
            return for_stmt;
        }
    }
    if (starts_a_type(tokens_cursor->kind)) {
        return parse_var_def_maybe_with_initializer();
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
    result->expr = numberExpr(1);
    while (!maybe_consume('}')) {
        Stmt *statement = parseStmt();
        Stmt *newstmt = calloc(1, sizeof(Stmt));
        newstmt->first_child = result;
        newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
        newstmt->second_child = statement;
        result = newstmt;
    }
    return result;
}

FuncDef *constructFuncDef(Stmt *content, NameAndType *rettype_and_funcname, int len, NameAndType *params_start) {
    FuncDef *funcdef = calloc(1, sizeof(FuncDef));
    funcdef->content = content;
    funcdef->name = rettype_and_funcname->name;
    funcdef->return_type = rettype_and_funcname->type;
    funcdef->param_len = len;
    funcdef->params_start = params_start;
    funcdef->lvar_table_start = lvars_start;
    funcdef->lvar_table_end = lvars_cursor;
    return funcdef;
}

void store_func_decl(NameAndType *rettype_and_funcname) {
    NameAndType *decl = calloc(1, sizeof(NameAndType));
    decl->type = rettype_and_funcname->type;
    decl->name = rettype_and_funcname->name;
    *(funcdecls_cursor++) = decl;
}

void parseToplevel() {
    NameAndType *first_half = consume_type_and_ident_1st_half();
    if (maybe_consume('(')) {
        NameAndType *rettype_and_funcname = first_half;
        NameAndType *params_start = calloc(6, sizeof(NameAndType));
        if (maybe_consume(')')) {
            lvars_cursor = lvars_start = calloc(100, sizeof(NameAndType));
            store_func_decl(rettype_and_funcname);
            if (maybe_consume(';'))
                return;
            *(funcdefs_cursor++) = constructFuncDef(parseFunctionContent(), rettype_and_funcname, 0, params_start);
            return;
        }
        lvars_cursor = lvars_start = calloc(100, sizeof(char *));
        int i = 0;
        for (; i < 6; i++) {
            NameAndType *param = consume_type_and_ident();
            if (maybe_consume(')')) {
                params_start[i].name = param->name;
                params_start[i].type = param->type;
                lvars_cursor->name = param->name;
                (lvars_cursor++)->type = param->type;
                break;
            }
            consume_otherwise_panic(',');
            params_start[i].name = param->name;
            params_start[i].type = param->type;
            lvars_cursor->name = param->name;
            (lvars_cursor++)->type = param->type;
        }
        store_func_decl(rettype_and_funcname);
        if (maybe_consume(';'))
            return;
        *(funcdefs_cursor++) = constructFuncDef(parseFunctionContent(), rettype_and_funcname, i + 1, params_start);
        return;
    } else {
        NameAndType *global_var_type_and_name = consume_type_and_ident_2nd_half(first_half);
        *(global_vars_cursor++) = global_var_type_and_name;
        consume_otherwise_panic(';');
        return;
    }
}

/*** ^ PARSE | v CODEGEN ***/

int labelCounter;

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

int find_strlit(char *str) {
    for (int i = 0; string_literals_start[i]; i++)
        if (strcmp(string_literals_start[i], str) == 0)
            return i;
    return 100000;
}

int isGVar(char *name) {
    for (int i = 0; global_vars_start[i]; i++) {
        if (strcmp(name, global_vars_start[i]->name) == 0) {
            return 1;
        }
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
        LVar *local = findLVar(expr->func_or_ident_name_or_string_content);
        if (local) {
            printf("  lea rax, [rbp - %d]\n", local->offset_from_rbp);
        } else if (isGVar(expr->func_or_ident_name_or_string_content)) {
            printf("  mov eax, OFFSET FLAT:%s\n", expr->func_or_ident_name_or_string_content);
        } else {
            fprintf(stderr, "undefined variable %s\n", expr->func_or_ident_name_or_string_content);
            exit(1);
        }
    } else if (expr->expr_kind == enum3('S', 'T', 'R')) {
        printf("  mov eax, OFFSET FLAT:.LC%d\n", find_strlit(expr->func_or_ident_name_or_string_content));
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y') && expr->op == '*') {
        EvaluateExprIntoRax(expr->first_child);
    } else
        panic("not lvalue");
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
        int label = (labelCounter++);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", label);
        CodegenStmt(stmt->first_child);
        printf("  jmp .Lend%d\n", label);
        printf(".Lelse%d:\n", label);
        if (stmt->second_child != 0)
            CodegenStmt(stmt->second_child);
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == enum4('W', 'H', 'I', 'L')) {
        int label = (labelCounter++);
        printf(".Lbegin%d:\n", label);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == enum3('f', 'o', 'r')) {
        int label = (labelCounter++);
        if (stmt->expr)
            EvaluateExprIntoRax(stmt->expr);
        printf(".Lbegin%d:\n", label);
        if (stmt->for_cond)
            EvaluateExprIntoRax(stmt->for_cond);
        else
            printf("  mov rax, 1\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        CodegenStmt(stmt->second_child);
        if (stmt->for_after)
            EvaluateExprIntoRax(stmt->for_after);
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    }
}

const char *nth_arg_reg(int n, int sz) {
    if (sz == 8)
        return "rdi\0rsi\0rdx\0rcx\0r8 \0r9" + 4 * n;
    else if (sz == 4)
        return "edi\0esi\0edx\0ecx\0r8d\0r9d" + 4 * n;
    fprintf(stderr, "unhandlable size %d\n", sz);
    exit(1);
}

void CodegenFunc(FuncDef *funcdef) {
    printf(".globl %s\n", funcdef->name);
    printf("%s:\n", funcdef->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    int stack_adjust = 0;
    for (int i = 0; i < funcdef->param_len; i++)
        stack_adjust += 8;
    for (NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++)
        stack_adjust += (size(ptr->type) + 7) / 8 * 8;
    printf("  sub rsp, %d\n", stack_adjust);
    for (int i = 0; i < funcdef->param_len; i++) {
        char *param_name = funcdef->params_start[i].name;
        insertLVar(param_name, 8);
        LVar *local = findLVar(param_name);
        printf("  mov [rbp - %d], %s\n", local->offset_from_rbp, nth_arg_reg(i, 8));
    }
    for (NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++)
        insertLVar(ptr->name, size(ptr->type));
    CodegenStmt(funcdef->content);
}

void deref_rax(int sz) {
    if (sz == 8)
        printf("  mov rax,[rax]\n");
    else if (sz == 4)
        printf("  mov eax,[rax]\n");
    else if (sz == 1) {
        printf("  movzx ecx, BYTE PTR [rax]\n");
        printf("  mov eax, ecx\n");
    } else {
        fprintf(stderr, "unhandlable size %d\n", sz);
        exit(1);
    }
}

void write_rax_to_where_rdi_points(int sz) {
    if (sz == 8)
        printf("    mov [rdi], rax\n");
    else if (sz == 4)
        printf("    mov [rdi], eax\n");
    else if (sz == 1) {
        printf("    mov ecx, eax\n");
        printf("    mov [rdi], cl\n");
    } else {
        fprintf(stderr, "unhandlable size %d\n", sz);
        exit(1);
    }
}

const char *AddSubMulDivAssign_rdi_into_rax(Kind kind) {
    if (kind == enum2('+', '=')) {
        return "    add rax,rdi\n";
    } else if (kind == enum2('-', '=')) {
        return "    sub rax,rdi\n";
    } else if (kind == enum2('*', '=')) {
        return "    imul rax,rdi\n";
    } else if (kind == enum2('/', '=')) {
        return "  cqo\n  idiv rdi\n";
    }
    return 0;
}

void EvaluateExprIntoRax(Expr *expr) {
    if (expr->typ->kind == enum2('[', ']')) {
        EvaluateLValueAddressIntoRax(expr);
        return;
    }
    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        EvaluateLValueAddressIntoRax(expr);
        deref_rax(size(expr->typ));
    } else if (expr->expr_kind == enum4('C', 'A', 'L', 'L')) {
        for (int i = 0; i < expr->func_arg_len; i++) {
            EvaluateExprIntoRax(expr->func_args_start[i]);
            printf("    push rax\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--)
            printf("    pop %s\n", nth_arg_reg(i, 8));
        printf("  mov rax, 0\n");
        printf(" call %s\n", expr->func_or_ident_name_or_string_content);
    } else if (expr->expr_kind == enum3('N', 'U', 'M')) {
        printf("  mov rax, %d\n", expr->value);
    } else if (expr->expr_kind == '0') {
        printf("  mov rax, 0\n");
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
            write_rax_to_where_rdi_points(size(expr->first_child->typ));  // second_child might be a 0 meaning a null pointer
        } else if (AddSubMulDivAssign_rdi_into_rax(expr->op)) {           // x @= i
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\n");                                 // stack: i
            EvaluateLValueAddressIntoRax(expr->first_child);          // rax: &x
            printf("    mov rsi, rax\n");                             // rsi: &x
            printf("    mov rax, [rax]\n");                           // rsi: &x, rax: x
            printf("    pop rdi\n");                                  // rsi: &x, rax: x, rdi: i
            printf("%s", AddSubMulDivAssign_rdi_into_rax(expr->op));  // rsi: &x, rax: x@i
            printf("    mov rdi, rsi\n");                             // rdi: &x, rax: x@i
            write_rax_to_where_rdi_points(size(expr->second_child->typ));
        } else if (expr->op == enum2('&', '&')) {
            int label = (labelCounter++);
            EvaluateExprIntoRax(expr->first_child);
            printf("    test rax, rax\n");
            printf("    je .Landfalse%d\n", label);
            EvaluateExprIntoRax(expr->second_child);
            printf("    test rax, rax\n");
            printf("    je  .Landfalse%d\n", label);
            printf("    mov eax, 1\n");
            printf("    jmp .Landend%d\n", label);
            printf(".Landfalse%d:\n", label);
            printf("    mov     eax, 0\n");
            printf(".Landend%d:\n", label);
        } else {
            EvaluateExprIntoRax(expr->first_child);
            printf("    push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\n");
            printf("    pop rdi\n");
            printf("    pop rax\n");
            if (AddSubMulDivAssign_rdi_into_rax(enum2(expr->op, '='))) {
                printf("%s", AddSubMulDivAssign_rdi_into_rax(enum2(expr->op, '=')));
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
    if (argc != 2)
        panic("incorrect cmd line arg\n");
    string_literals_cursor = string_literals_start;
    tokens_cursor = tokens_start;  // the 1st tokens_cursor is for storing the tokens
    tokens_end = tokenize(argv[1]);
    if (tokens_start == tokens_end)
        panic("no token found\n");
    tokens_cursor = tokens_start;  // the 2nd tokens_cursor is for parsing
    funcdecls_cursor = funcdecls_start;
    funcdefs_cursor = funcdefs_start;
    global_vars_cursor = global_vars_start;
    while (tokens_cursor < tokens_end)
        parseToplevel();
    printf(".intel_syntax noprefix\n");
    printf("  .text\n");
    printf("  .section .rodata\n");
    for (int i = 0; string_literals_start[i]; i++) {
        printf(".LC%d:\n", i);
        printf("  .string \"%s\"\n", string_literals_start[i]);
    }
    printf("  .text\n");
    for (int i = 0; global_vars_start[i]; i++) {
        printf(".globl %s\n", global_vars_start[i]->name);
        printf(".data\n");
        printf("%s:\n", global_vars_start[i]->name);
        printf("  .zero %d\n", size(global_vars_start[i]->type));
    }
    printf(".text\n");
    for (int i = 0; funcdefs_start[i]; i++)
        CodegenFunc(funcdefs_start[i]);
    return 0;
}
