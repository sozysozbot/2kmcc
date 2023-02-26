int printf();
void exit();
void *calloc();
int strcmp();
int strncmp();
char *strchr();
char *strncpy();
char *strstr();

struct Type {
    int kind;
    struct Type *ptr_to;
    int array_size;
    char *struct_name;
};

struct StructMember {
    char *struct_name;
    char *member_name;
    int member_offset;
    struct Type *member_type;
};

struct StructSizeAndAlign {
    char *struct_name;
    int size;
    int align;
};

struct Expr {
    int op_kind;
    int expr_kind;
    int value;
    struct Expr *first_child;
    struct Expr *second_child;
    struct Expr **func_args_start;
    int func_arg_len;
    char *func_or_ident_name_or_string_content;
    struct Type *typ;
};

struct NameAndType {
    char *name;
    struct Type *type;
};

struct Stmt {
    int stmt_kind;
    struct Expr *expr;
    struct Expr *for_cond;
    struct Expr *for_after;
    struct Stmt *first_child;
    struct Stmt *second_child;
};

struct FuncDef {
    struct Stmt *content;
    char *name;
    struct NameAndType *params_start;
    int param_len;
    struct NameAndType *lvar_table_start;
    struct NameAndType *lvar_table_end;
    struct Type *return_type;
};

struct LVar {
    struct LVar *next;
    char *name;
    int offset_from_rbp;
};

struct Token {
    int kind;
    int value_or_string_size;  // includes the null terminator, so length+1
    char *identifier_name_or_escaped_string_content;
    char *position;
};

int enum2(int a, int b) {
    return a + b * 256;
}

int enum3(int a, int b, int c) {
    return enum2(a, enum2(b, c));
}

int enum4(int a, int b, int c, int d) {
    return enum2(a, enum3(b, c, d));
}

char *decode_kind(int kind) {
    void *r = &kind;
    char *q = r;
    char *ans = calloc(5, sizeof(char));
    strncpy(ans, q, 4);
    return ans;
}

void panic(const char *msg) {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! %s", msg);
    exit(1);
}

void panic_kind(const char *msg, int kind) {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! %s `%s`\n", msg, decode_kind(kind));
    exit(1);
}

/*** ^ LIB | v PARSE ***/
struct Token tokens_start[50000];
struct Token *tokens_end;
struct Token *tokens_cursor;
char *string_literals_start[10000];
char **string_literals_cursor;
struct StructMember *struct_members_start[10000];
struct StructMember **struct_members_cursor;
struct StructSizeAndAlign *struct_sizes_and_alignments_start[100];
struct StructSizeAndAlign **struct_sizes_and_alignments_cursor;
int currently_handling_function_returning_void;
char *entire_input_start;

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

struct Token *tokenize(char *str) {
    for (int i = 0; str[i];) {
        char c = str[i];
        tokens_cursor->position = str + i;
        if (is_reserved_then_handle(str + i, &i, "return", 6, enum3('R', 'E', 'T'))) {
        } else if (is_reserved_then_handle(str + i, &i, "sizeof", 6, enum4('S', 'Z', 'O', 'F'))) {
        } else if (is_reserved_then_handle(str + i, &i, "struct", 6, enum4('S', 'T', 'R', 'U'))) {
        } else if (is_reserved_then_handle(str + i, &i, "if", 2, enum2('i', 'f'))) {
        } else if (is_reserved_then_handle(str + i, &i, "while", 5, enum4('W', 'H', 'I', 'L'))) {
        } else if (is_reserved_then_handle(str + i, &i, "const", 5, enum4('C', 'N', 'S', 'T'))) {
        } else if (is_reserved_then_handle(str + i, &i, "else", 4, enum4('e', 'l', 's', 'e'))) {
        } else if (is_reserved_then_handle(str + i, &i, "void", 4, enum4('v', 'o', 'i', 'd'))) {
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
                printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
                printf("! Not supported: Unsupported escape sequence within a character literal: `\\%c`\n", str[i + 2]);
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
                    if (str[i + parsed_length + 1] == 'x') {
                        panic("Not supported: hexadecimal escape sequence within a string literal\n");
                    } else if (strchr("01234567", str[i + parsed_length + 1])) {
                        parsed_length++;
                        if (strchr("01234567", str[i + parsed_length + 1])) {
                            parsed_length++;
                            if (strchr("01234567", str[i + parsed_length + 1]))
                                parsed_length++;
                        }
                    } else
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
            strncpy(name, start, length);
            tokens_cursor->kind = enum4('I', 'D', 'N', 'T');
            (tokens_cursor++)->identifier_name_or_escaped_string_content = name;
        } else if (strchr(" \n", c)) {
            i += 1;
        } else {
            printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf("! unknown character `%c` (%d)\n", c, c);
            exit(1);
        }
    }
    return tokens_cursor;
}

/*** ^ TOKENIZE | v PARSE ***/

struct Type *type(int kind) {
    struct Type *t = calloc(1, sizeof(struct Type));
    t->kind = kind;
    return t;
}

struct Type *ptr_of(struct Type *t) {
    struct Type *new_t = type('*');
    new_t->ptr_to = t;
    return new_t;
}

struct Type *arr_of(struct Type *t, int array_size) {
    struct Type *new_t = type(enum2('[', ']'));
    new_t->ptr_to = t;
    new_t->array_size = array_size;
    return new_t;
}

void display_type(struct Type *t) {
    if (t->kind == enum2('[', ']')) {
        printf("array (length: %d) of ", t->array_size);
        display_type(t->ptr_to);
    } else if (t->kind == '*') {
        printf("pointer to ");
        display_type(t->ptr_to);
    } else if (t->kind == enum4('S', 'T', 'R', 'U')) {
        printf("struct %s", t->struct_name);
    } else
        printf("%s", decode_kind(t->kind));
}

void panic_single_type(const char *msg, struct Type *t) {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! %s `", msg);
    display_type(t);
    printf("`.\n");
    exit(1);
}

struct Type *deref(struct Type *t) {
    if (t->kind != '*')
        panic_single_type("cannot deref a non-pointer type", t);
    return t->ptr_to;
}

int size(struct Type *t) {
    if (t->kind == '*') {
        return 8;
    } else if (t->kind == enum3('i', 'n', 't')) {
        return 4;
    } else if (t->kind == enum4('c', 'h', 'a', 'r')) {
        return 1;
    } else if (t->kind == enum2('[', ']')) {
        return t->array_size * size(t->ptr_to);
    } else if (t->kind == enum4('S', 'T', 'R', 'U'))
        for (int i = 0; struct_sizes_and_alignments_start[i]; i++)
            if (strcmp(t->struct_name, struct_sizes_and_alignments_start[i]->struct_name) == 0)
                return struct_sizes_and_alignments_start[i]->size;
    panic_single_type("cannot calculate the size for type", t);
    exit(1);
}

int align(struct Type *t) {
    if (t->kind == '*') {
        return 8;
    } else if (t->kind == enum3('i', 'n', 't')) {
        return 4;
    } else if (t->kind == enum4('c', 'h', 'a', 'r')) {
        return 1;
    } else if (t->kind == enum2('[', ']')) {
        return align(t->ptr_to);
    } else if (t->kind == enum4('S', 'T', 'R', 'U'))
        for (int i = 0; struct_sizes_and_alignments_start[i]; i++)
            if (strcmp(t->struct_name, struct_sizes_and_alignments_start[i]->struct_name) == 0)
                return struct_sizes_and_alignments_start[i]->align;
    panic_single_type("cannot calculate the alignment for type", t);
    exit(1);
}

struct Expr *numberExpr(int value) {
    struct Expr *numberexp = calloc(1, sizeof(struct Expr));
    numberexp->value = value;
    if (value)
        numberexp->expr_kind = enum3('N', 'U', 'M');
    else
        numberexp->expr_kind = '0';  //  An integer constant expression with the value 0 ... is called a null pointer constant
    numberexp->typ = type(enum3('i', 'n', 't'));
    return numberexp;
}

struct Expr *unaryExpr(struct Expr *first_child, int op_kind, struct Type *typ) {
    struct Expr *newexp = calloc(1, sizeof(struct Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('1', 'A', 'R', 'Y');
    newexp->op_kind = op_kind;
    newexp->typ = typ;
    return newexp;
}

struct Expr *binaryExpr(struct Expr *first_child, struct Expr *second_child, int op_kind, struct Type *typ) {
    struct Expr *newexp = unaryExpr(first_child, op_kind, typ);
    newexp->expr_kind = enum4('2', 'A', 'R', 'Y');
    newexp->second_child = second_child;
    return newexp;
}

struct Expr *decay_if_arr(struct Expr *first_child) {
    if (first_child->typ->kind != enum2('[', ']'))
        return first_child;
    struct Type *t = calloc(1, sizeof(struct Type));
    t->ptr_to = first_child->typ->ptr_to;
    t->kind = '*';
    return unaryExpr(first_child, enum4('[', ']', '>', '*'), t);
}

int maybe_consume(int kind) {
    if (tokens_cursor->kind != kind)
        return 0;
    tokens_cursor += 1;
    return 1;
}

void show_error_at(char *location, const char *msg) {
    char *line = location;
    while (entire_input_start < line && line[-1] != '\n')
        line--;

    char *end = location;
    while (*end != '\n' && *end != 0)
        end++;

    int line_num = 1;
    for (char *p = entire_input_start; p < line; p++)
        if (*p == '\n')
            line_num++;
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    int indent = printf("! line #%d: ", line_num) - 2;
    int offset = end - line;
    printf("%.*s\n", offset, line);
    int pos = location - line + indent;  // ptrdiff_t -> int
    printf("! %*s", pos, "");
    printf("^ %s\n", msg);
}

void consume_otherwise_panic(int kind) {
    if (!maybe_consume(kind)) {
        show_error_at(tokens_cursor->position, "");
        printf("! parse error: expected TokenKind `%s`; got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens_cursor->kind));
        exit(1);
    }
}

void expect_otherwise_panic(int kind) {
    if (tokens_cursor->kind != kind) {
        show_error_at(tokens_cursor->position, "");
        printf("! parse error: expected TokenKind `%s`; got TokenKind `%s`\n", decode_kind(kind), decode_kind(tokens_cursor->kind));
        exit(1);
    }
}

void panic_if_eof() {
    if (tokens_cursor >= tokens_end)
        panic("EOF encountered");
}

struct Expr *parseExpr(void);
struct NameAndType *lvars_start;
struct NameAndType *lvars_cursor;
struct NameAndType *funcdecls_start[100];
struct NameAndType **funcdecls_cursor;
struct FuncDef *funcdefs_start[100];
struct FuncDef **funcdefs_cursor;
struct NameAndType *global_vars_start[100];
struct NameAndType **global_vars_cursor;

struct Type *lookup_ident_type(char *name) {
    for (int i = 0; lvars_start[i].name; i++)
        if (strcmp(lvars_start[i].name, name) == 0)
            return lvars_start[i].type;
    for (int i = 0; global_vars_start[i]; i++)
        if (strcmp(global_vars_start[i]->name, name) == 0)
            return global_vars_start[i]->type;
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! cannot find an identifier named `%s`; cannot determine the type\n", name);
    exit(1);
}

struct Type *lookup_func_type(char *name) {
    for (int i = 0; funcdecls_start[i]; i++)
        if (strcmp(funcdecls_start[i]->name, name) == 0)
            return funcdecls_start[i]->type;
    printf("# cannot find a function named `%s`. Implicitly assumes that it return an int\n", name);
    return type(enum3('i', 'n', 't'));
}

struct Expr *callingExpr(char *name, struct Expr **arguments, int len) {
    struct Expr *callexp = calloc(1, sizeof(struct Expr));
    callexp->func_or_ident_name_or_string_content = name;
    callexp->expr_kind = enum4('C', 'A', 'L', 'L');
    callexp->func_args_start = arguments;
    callexp->func_arg_len = len;
    callexp->typ = lookup_func_type(name);
    return callexp;
}

struct Expr *identExpr(char *name) {
    struct Expr *ident_exp = calloc(1, sizeof(struct Expr));
    ident_exp->func_or_ident_name_or_string_content = name;
    ident_exp->expr_kind = enum4('I', 'D', 'N', 'T');
    ident_exp->typ = lookup_ident_type(name);
    return ident_exp;
}

struct Expr *parsePrimary() {
    panic_if_eof();
    if (tokens_cursor->kind == enum3('N', 'U', 'M'))
        return numberExpr((tokens_cursor++)->value_or_string_size);
    else if (tokens_cursor->kind == enum3('S', 'T', 'R')) {
        int str_size = tokens_cursor->value_or_string_size;
        char *str_content = (tokens_cursor++)->identifier_name_or_escaped_string_content;
        struct Expr *string_literal_exp = calloc(1, sizeof(struct Expr));
        string_literal_exp->func_or_ident_name_or_string_content = str_content;
        string_literal_exp->expr_kind = enum3('S', 'T', 'R');
        string_literal_exp->typ = arr_of(type(enum4('c', 'h', 'a', 'r')), str_size);
        return string_literal_exp;
    } else if (tokens_cursor->kind == enum4('I', 'D', 'N', 'T')) {
        char *name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
        if (maybe_consume('(')) {
            struct Expr **arguments = calloc(6, sizeof(struct Expr *));
            if (maybe_consume(')'))
                return callingExpr(name, arguments, 0);
            for (int i = 0; i < 6; i++) {
                arguments[i] = decay_if_arr(parseExpr());
                if (maybe_consume(')'))
                    return callingExpr(name, arguments, i + 1);
                consume_otherwise_panic(',');
            }
            panic("not supported: more than 6 arguments\n");
        }
        return identExpr(name);
    }
    consume_otherwise_panic('(');
    struct Expr *expr = parseExpr();  // NO DECAY
    consume_otherwise_panic(')');
    return expr;
}

int is_int_or_char(int kind) {
    return kind == enum3('i', 'n', 't') || kind == enum4('c', 'h', 'a', 'r');
}

int starts_a_type(int kind) {
    return is_int_or_char(kind) || kind == enum4('v', 'o', 'i', 'd') || kind == enum4('S', 'T', 'R', 'U') || kind == enum4('C', 'N', 'S', 'T');
}

int is_integer(struct Type *typ) {
    return is_int_or_char(typ->kind);
}

struct Expr *require_integer(struct Expr *e) {
    if (!is_integer(e->typ))
        panic_single_type("int/char is expected, but not an int/char; the type is instead", e->typ);
    return e;
}

struct Expr *require_scalar(struct Expr *e) {
    if (e && !is_integer(e->typ) && e->typ->kind != '*')  // so that it can be used to handle `for(;;)`
        panic_single_type("a scalar value is expected, but the type is instead", e->typ);
    return e;
}

int is_same_type(struct Type *t1, struct Type *t2) {
    if (t1->kind == '*' && t2->kind == '*')
        return is_same_type(t1->ptr_to, t2->ptr_to);
    return t1->kind == t2->kind;
}

int is_compatible_type(struct Type *t1, struct Type *t2) {
    if (t1->kind != '*' || t2->kind != '*')
        return t1->kind == t2->kind || (is_integer(t1) && is_integer(t2));
    if ((t1->ptr_to->kind == enum4('v', 'o', 'i', 'd')) || (t2->ptr_to->kind == enum4('v', 'o', 'i', 'd')))
        return 1;
    return is_same_type(t1->ptr_to, t2->ptr_to);
}

void panic_invalid_binary_operand_types(struct Type *lhs_type, struct Expr *rhs, int op_kind) {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! invalid operands to binary `%s`: types are `", decode_kind(op_kind));
    display_type(lhs_type);
    printf("` and `");
    display_type(rhs->typ);
    printf("`.\n");
    exit(1);
}

struct Expr *expr_add(struct Expr *lhs, struct Expr *rhs) {
    if (is_integer(lhs->typ)) {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, rhs, '+', type(enum3('i', 'n', 't')));
        else if (rhs->typ->kind == '*')
            return expr_add(rhs, lhs);
        else
            panic_invalid_binary_operand_types(lhs->typ, rhs, '+');
    } else if (lhs->typ->kind == '*') {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, binaryExpr(numberExpr(size(deref(lhs->typ))), rhs, '*', type(enum3('i', 'n', 't'))), '+', lhs->typ);
        else
            panic_invalid_binary_operand_types(lhs->typ, rhs, '+');
    }
    panic_invalid_binary_operand_types(lhs->typ, rhs, '+');
    exit(1);
}

struct Expr *expr_subtract(struct Expr *lhs, struct Expr *rhs) {
    if (lhs->typ->kind == '*') {
        if (is_integer(rhs->typ)) {
            return binaryExpr(lhs, binaryExpr(numberExpr(size(deref(lhs->typ))), rhs, '*', type(enum3('i', 'n', 't'))), '-', lhs->typ);
        } else if (rhs->typ->kind == '*' && is_same_type(lhs->typ, rhs->typ))
            return binaryExpr(binaryExpr(lhs, rhs, '-', type(enum3('i', 'n', 't'))), numberExpr(size(deref(lhs->typ))), '/', type(enum3('i', 'n', 't')));
    } else if (is_integer(lhs->typ))
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, rhs, '-', type(enum3('i', 'n', 't')));
    panic_invalid_binary_operand_types(lhs->typ, rhs, '-');
    exit(1);
}

struct StructMember *get_member(struct Type *struct_type, char *member_name) {
    for (int i = 0; struct_members_start[i]; i++)
        if (strcmp(struct_members_start[i]->struct_name, struct_type->struct_name) == 0 && strcmp(struct_members_start[i]->member_name, member_name) == 0)
            return struct_members_start[i];
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! cannot find a struct type `");
    display_type(struct_type);
    printf("` which has a member named `%s`\n", member_name);
    exit(1);
}

struct Expr *arrowExpr(struct Expr *lhs, char *member_name) {
    struct Type *struct_type = deref(lhs->typ);
    if (struct_type->kind != enum4('S', 'T', 'R', 'U'))
        panic("tried to access a member of a non-struct type\n");
    struct StructMember *member = get_member(struct_type, member_name);
    struct Expr *expr = binaryExpr(lhs, numberExpr(member->member_offset), '+', ptr_of(member->member_type));
    return unaryExpr(expr, '*', deref(expr->typ));
}

struct Expr *parsePostfix() {
    struct Expr *result = parsePrimary();
    while (1) {
        if (maybe_consume('[')) {
            struct Expr *addition = expr_add(decay_if_arr(result), decay_if_arr(parseExpr()));
            consume_otherwise_panic(']');
            struct Expr *expr = decay_if_arr(addition);
            result = unaryExpr(expr, '*', deref(expr->typ));
        } else if (maybe_consume(enum2('+', '+'))) {  // `a++` is `(a ++) - 1
            struct Expr *addition = expr_add(decay_if_arr(result), numberExpr(1));
            addition->op_kind = enum2('+', '=');
            result = expr_subtract(addition, numberExpr(1));
        } else if (maybe_consume(enum2('-', '-'))) {  // `a--` is `(a -= 1) + 1
            struct Expr *subtraction = expr_subtract(decay_if_arr(result), numberExpr(1));
            subtraction->op_kind = enum2('-', '=');
            result = expr_add(subtraction, numberExpr(1));
        } else if (maybe_consume(enum2('-', '>'))) {
            expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
            char *member_name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
            result = arrowExpr(result, member_name);
        } else if (maybe_consume('.')) {
            expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
            char *member_name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
            result = arrowExpr(unaryExpr(result, '&', ptr_of(result->typ)), member_name);
        } else
            return result;
    }
}

struct Expr *parseUnary(void);
struct Expr *parseCast() {
    return parseUnary();
}

struct Type *consume_simple_type(void);
struct Expr *equalityExpr(struct Expr *lhs, struct Expr *rhs, int kind);

struct Expr *parseUnary() {
    panic_if_eof();
    if (maybe_consume('+')) {
        return binaryExpr(numberExpr(0), require_integer(parseCast()), '+', type(enum3('i', 'n', 't')));
    } else if (maybe_consume('-')) {
        return binaryExpr(numberExpr(0), require_integer(parseCast()), '-', type(enum3('i', 'n', 't')));
    } else if (maybe_consume('!')) {
        return equalityExpr(numberExpr(0), parseCast(), enum2('=', '='));  // The expression !E is equivalent to (0==E)
    } else if (maybe_consume('*')) {
        struct Expr *expr = decay_if_arr(parseCast());
        return unaryExpr(expr, '*', deref(expr->typ));
    } else if (maybe_consume('&')) {
        struct Expr *expr = parseCast();                 // NO DECAY
        return unaryExpr(expr, '&', ptr_of(expr->typ));  // NO DECAY
    } else if (maybe_consume(enum4('S', 'Z', 'O', 'F'))) {
        if (tokens_cursor->kind != '(') {
            return numberExpr(size(parseUnary()->typ));  // NO DECAY
        } else if (starts_a_type((tokens_cursor + 1)->kind)) {
            tokens_cursor++;
            struct Type *typ = consume_simple_type();
            consume_otherwise_panic(')');
            return numberExpr(size(typ));
        } else
            return numberExpr(size(parseUnary()->typ));  // NO DECAY
    } else
        return parsePostfix();
}

void assert_compatible_in_equality(struct Expr *e1, struct Expr *e2, int op_kind) {
    if (is_compatible_type(e1->typ, e2->typ))
        return;
    if (e1->expr_kind == '0' && e2->typ->kind == '*')  // one operand is a pointer and the other is a null pointer constant
        return;
    if (e2->expr_kind == '0' && e1->typ->kind == '*')  // one operand is a pointer and the other is a null pointer constant
        return;
    panic_invalid_binary_operand_types(e1->typ, e2, op_kind);
}

struct Expr *equalityExpr(struct Expr *lhs, struct Expr *rhs, int kind) {
    assert_compatible_in_equality(decay_if_arr(lhs), decay_if_arr(rhs), kind);
    return binaryExpr(decay_if_arr(lhs), decay_if_arr(rhs), kind, type(enum3('i', 'n', 't')));
}

int getPrecedence() {
    int kind = tokens_cursor->kind;
    if (kind == enum3('N', 'U', 'M'))
        panic("parse error: expected an operator; got a number\n");
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

struct Expr *parseLeftToRightInfix(int level) {
    panic_if_eof();
    struct Expr *expr = parseUnary();
    while (tokens_cursor < tokens_end) {
        int precedence = getPrecedence();
        if (precedence < level)
            return expr;
        int op = (tokens_cursor++)->kind;
        if (precedence == 10)
            expr = binaryExpr(require_integer(expr), require_integer(parseUnary()), op, type(enum3('i', 'n', 't')));
        else if (precedence == 9)
            if (op == '-')
                expr = expr_subtract(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)));
            else
                expr = expr_add(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)));
        else if (op == '<' || op == enum2('<', '='))  // children & operator swapped
            expr = binaryExpr(decay_if_arr(parseLeftToRightInfix(precedence + 1)), decay_if_arr(expr), op - '<' + '>', type(enum3('i', 'n', 't')));
        else if (precedence == 6)
            expr = equalityExpr(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)), op);
        else if (precedence <= 2)
            expr = binaryExpr(require_scalar(decay_if_arr(expr)), require_scalar(decay_if_arr(parseLeftToRightInfix(precedence + 1))), op, type(enum3('i', 'n', 't')));
        else
            expr = binaryExpr(decay_if_arr(expr), decay_if_arr(parseLeftToRightInfix(precedence + 1)), op, type(enum3('i', 'n', 't')));
    }
    return expr;
}

void assert_compatible_in_simple_assignment(struct Type *lhs_type, struct Expr *rhs) {
    if (is_compatible_type(lhs_type, rhs->typ))
        return;
    if (lhs_type->kind == '*' && rhs->expr_kind == '0')  // the left operand is an atomic, qualified, or unqualified pointer, and the right is a null pointer constant
        return;
    panic_invalid_binary_operand_types(lhs_type, rhs, '=');
}

struct Expr *parseAssign() {
    panic_if_eof();
    struct Expr *result = parseLeftToRightInfix(1);
    if (maybe_consume('=')) {
        struct Expr *rhs = decay_if_arr(parseAssign());
        assert_compatible_in_simple_assignment(result->typ, rhs);  // no decay of lhs, since we cannot assign to an array
        return binaryExpr(result, rhs, '=', result->typ);
    } else if (maybe_consume(enum2('+', '='))) {
        result = expr_add(decay_if_arr(result), require_integer(parseAssign()));
        result->op_kind = enum2('+', '=');
    } else if (maybe_consume(enum2('-', '='))) {
        result = expr_subtract(decay_if_arr(result), require_integer(parseAssign()));
        result->op_kind = enum2('-', '=');
    } else if (maybe_consume(enum2('*', '=')))
        result = binaryExpr(require_integer(result), require_integer(parseUnary()), enum2('*', '='), type(enum3('i', 'n', 't')));
    else if (maybe_consume(enum2('/', '=')))
        result = binaryExpr(require_integer(result), require_integer(parseUnary()), enum2('/', '='), type(enum3('i', 'n', 't')));
    return result;
}

struct Expr *parseExpr() {
    return parseAssign();
}

struct Expr *parseOptionalExprAndToken(int token_kind) {
    if (maybe_consume(token_kind))
        return 0;
    struct Expr *expr = decay_if_arr(parseExpr());
    consume_otherwise_panic(token_kind);
    return expr;
}

struct Type *consume_simple_type() {
    struct Type *type = calloc(1, sizeof(struct Type));
    if (maybe_consume(enum4('C', 'N', 'S', 'T')))
        return consume_simple_type();  // ignore const for now
    else if (maybe_consume(enum3('i', 'n', 't')))
        type->kind = enum3('i', 'n', 't');
    else if (maybe_consume(enum4('c', 'h', 'a', 'r')))
        type->kind = enum4('c', 'h', 'a', 'r');
    else if (maybe_consume(enum4('v', 'o', 'i', 'd')))
        type->kind = enum4('v', 'o', 'i', 'd');
    else if (maybe_consume(enum4('S', 'T', 'R', 'U'))) {
        type->kind = enum4('S', 'T', 'R', 'U');
        expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
        char *name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
        type->struct_name = name;
    } else
        panic_kind("expected a type specifier or a type qualifier; got TokenKind", tokens_cursor->kind);
    while (maybe_consume('*'))
        type = ptr_of(type);
    return type;
}

struct NameAndType *consume_type_and_ident_1st_half() {
    struct Type *type = consume_simple_type();
    expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
    char *name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
    struct NameAndType *ans = calloc(1, sizeof(struct NameAndType));
    ans->name = name;
    ans->type = type;
    return ans;
}

struct NameAndType *consume_type_and_ident_2nd_half(struct NameAndType *ans) {
    struct Type *elem_t = ans->type;
    struct Type *insertion_point;
    if (maybe_consume('[')) {
        expect_otherwise_panic(enum3('N', 'U', 'M'));
        struct Type *t = calloc(1, sizeof(struct Type));
        t->ptr_to = elem_t;
        t->kind = enum2('[', ']');
        t->array_size = (tokens_cursor++)->value_or_string_size;
        insertion_point = t;
        consume_otherwise_panic(']');
        ans->type = t;
    }
    while (maybe_consume('[')) {
        expect_otherwise_panic(enum3('N', 'U', 'M'));
        struct Type *t = calloc(1, sizeof(struct Type));
        t->ptr_to = elem_t;
        t->kind = enum2('[', ']');
        t->array_size = (tokens_cursor++)->value_or_string_size;
        insertion_point->ptr_to = t;
        insertion_point = t;
        consume_otherwise_panic(']');
    }
    return ans;
}

struct NameAndType *consume_type_and_ident() {
    struct NameAndType *ans = consume_type_and_ident_1st_half();
    return consume_type_and_ident_2nd_half(ans);
}

struct Stmt *parse_var_def_maybe_with_initializer() {
    struct NameAndType *var = consume_type_and_ident();
    lvars_cursor->name = var->name;
    (lvars_cursor++)->type = var->type;
    if (maybe_consume(';')) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
        stmt->expr = numberExpr(42);
        return stmt;
    }
    consume_otherwise_panic('=');
    if (maybe_consume('{')) 
        panic("not supported: initializer list\n");
    struct Expr *rhs = decay_if_arr(parseExpr());
    consume_otherwise_panic(';');
    struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
    stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
    assert_compatible_in_simple_assignment(var->type, rhs);
    stmt->expr = binaryExpr(identExpr(var->name), rhs, '=', var->type);
    return stmt;
}

struct Stmt *parseStmt() {
    if (maybe_consume('{')) {
        struct Stmt *result = calloc(1, sizeof(struct Stmt));
        result->stmt_kind = enum4('e', 'x', 'p', 'r');
        result->expr = numberExpr(42);
        while (!maybe_consume('}')) {
            struct Stmt *newstmt = calloc(1, sizeof(struct Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
            newstmt->second_child = parseStmt();
            result = newstmt;
        }
        return result;
    }
    if (maybe_consume(enum3('R', 'E', 'T'))) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        stmt->stmt_kind = enum3('R', 'E', 'T');
        if (maybe_consume(';')) {
            if (currently_handling_function_returning_void) {
                stmt->expr = numberExpr(42);
                return stmt;
            } else
                panic("`return` with no value found in a function not returning void");
        }
        stmt->expr = decay_if_arr(parseExpr());
        consume_otherwise_panic(';');
        return stmt;
    }
    if (maybe_consume(enum2('i', 'f'))) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        consume_otherwise_panic('(');
        stmt->expr = require_scalar(decay_if_arr(parseExpr()));
        consume_otherwise_panic(')');
        stmt->stmt_kind = enum2('i', 'f');
        stmt->first_child = parseStmt();  // then-block
        if (maybe_consume(enum4('e', 'l', 's', 'e')))
            stmt->second_child = parseStmt();  // else-block
        return stmt;
    }
    if (maybe_consume(enum4('W', 'H', 'I', 'L'))) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        consume_otherwise_panic('(');
        stmt->expr = require_scalar(decay_if_arr(parseExpr()));
        consume_otherwise_panic(')');
        stmt->stmt_kind = enum4('W', 'H', 'I', 'L');
        struct Stmt *statement = parseStmt();
        stmt->second_child = statement;
        return stmt;
    }
    if (maybe_consume(enum3('f', 'o', 'r'))) {
        struct Stmt *for_stmt = calloc(1, sizeof(struct Stmt));
        for_stmt->stmt_kind = enum3('f', 'o', 'r');
        consume_otherwise_panic('(');
        if (starts_a_type(tokens_cursor->kind)) {
            struct Stmt *initializer = parse_var_def_maybe_with_initializer();
            for_stmt->expr = numberExpr(42);
            for_stmt->for_cond = require_scalar(parseOptionalExprAndToken(';'));
            for_stmt->for_after = parseOptionalExprAndToken(')');
            for_stmt->second_child = parseStmt();
            struct Stmt *combined_stmt = calloc(1, sizeof(struct Stmt));
            combined_stmt->first_child = initializer;
            combined_stmt->stmt_kind = enum4('n', 'e', 'x', 't');
            combined_stmt->second_child = for_stmt;
            return combined_stmt;
        } else {
            for_stmt->expr = parseOptionalExprAndToken(';');
            for_stmt->for_cond = require_scalar(parseOptionalExprAndToken(';'));
            for_stmt->for_after = parseOptionalExprAndToken(')');
            for_stmt->second_child = parseStmt();
            return for_stmt;
        }
    }
    if (starts_a_type(tokens_cursor->kind))
        return parse_var_def_maybe_with_initializer();
    struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
    stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
    stmt->expr = decay_if_arr(parseExpr());
    consume_otherwise_panic(';');
    return stmt;
}

struct Stmt *parseFunctionContent() {
    consume_otherwise_panic('{');
    struct Stmt *result = calloc(1, sizeof(struct Stmt));
    result->stmt_kind = enum4('e', 'x', 'p', 'r');
    result->expr = numberExpr(1);
    while (!maybe_consume('}')) {
        struct Stmt *statement = parseStmt();
        struct Stmt *newstmt = calloc(1, sizeof(struct Stmt));
        newstmt->first_child = result;
        newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
        newstmt->second_child = statement;
        result = newstmt;
    }
    struct Stmt *implicit_return = calloc(1, sizeof(struct Stmt));
    implicit_return->stmt_kind = enum3('R', 'E', 'T');
    implicit_return->expr = numberExpr(42);
    struct Stmt *finalstmt = calloc(1, sizeof(struct Stmt));
    finalstmt->first_child = result;
    finalstmt->stmt_kind = enum4('n', 'e', 'x', 't');
    finalstmt->second_child = implicit_return;
    return finalstmt;
}

struct FuncDef *constructFuncDef(struct Stmt *content, struct NameAndType *rettype_and_funcname, int len, struct NameAndType *params_start) {
    struct FuncDef *funcdef = calloc(1, sizeof(struct FuncDef));
    funcdef->content = content;
    funcdef->name = rettype_and_funcname->name;
    funcdef->return_type = rettype_and_funcname->type;
    funcdef->param_len = len;
    funcdef->params_start = params_start;
    funcdef->lvar_table_start = lvars_start;
    funcdef->lvar_table_end = lvars_cursor;
    return funcdef;
}

void store_func_decl(struct NameAndType *rettype_and_funcname) {
    struct NameAndType *decl = calloc(1, sizeof(struct NameAndType));
    decl->type = rettype_and_funcname->type;
    decl->name = rettype_and_funcname->name;
    *(funcdecls_cursor++) = decl;
}

int roundup(int sz, int align) {
    return (sz + align - 1) / align * align;
}

void parseToplevel() {
    if (maybe_consume(enum4('S', 'T', 'R', 'U'))) {
        expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
        char *struct_name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
        int overall_alignment = 1;
        int next_member_offset = 0;
        if (maybe_consume('{')) {  // `struct Foo { int a; };
            while (!maybe_consume('}')) {
                struct NameAndType *member = consume_type_and_ident();
                consume_otherwise_panic(';');
                struct StructMember *q = calloc(1, sizeof(struct StructMember));
                q->member_name = member->name;
                q->struct_name = struct_name;
                q->member_type = member->type;
                q->member_offset = roundup(next_member_offset, align(member->type));
                next_member_offset = q->member_offset + size(member->type);
                if (overall_alignment < align(member->type))
                    overall_alignment = align(member->type);
                *(struct_members_cursor++) = q;
            }
            struct StructSizeAndAlign *sa = calloc(1, sizeof(struct StructSizeAndAlign));
            sa->struct_name = struct_name;
            sa->align = overall_alignment;
            sa->size = roundup(next_member_offset, overall_alignment);
            *(struct_sizes_and_alignments_cursor++) = sa;
            consume_otherwise_panic(';');
            return;
        } else                   // global var definition: `struct Foo **id[5];`
            tokens_cursor -= 2;  // thus bring back the cursor and let the rest of the function do the job
    }
    struct NameAndType *first_half = consume_type_and_ident_1st_half();
    if (maybe_consume('(')) {
        struct NameAndType *rettype_and_funcname = first_half;
        struct NameAndType *params_start = calloc(6, sizeof(struct NameAndType));
        if (maybe_consume(')')) {  // fixme: `int foo();` should say nothing about its argument types
            lvars_cursor = lvars_start = calloc(100, sizeof(struct NameAndType));
            store_func_decl(rettype_and_funcname);
            if (maybe_consume(';'))
                return;
            currently_handling_function_returning_void = rettype_and_funcname->type->kind == enum4('v', 'o', 'i', 'd');
            *(funcdefs_cursor++) = constructFuncDef(parseFunctionContent(), rettype_and_funcname, 0, params_start);
            return;
        } else if (tokens_cursor->kind == enum4('v', 'o', 'i', 'd') && (tokens_cursor[1].kind == ')')) {
            tokens_cursor += 2;
            lvars_cursor = lvars_start = calloc(100, sizeof(struct NameAndType));
            store_func_decl(rettype_and_funcname);
            if (maybe_consume(';'))
                return;
            currently_handling_function_returning_void = rettype_and_funcname->type->kind == enum4('v', 'o', 'i', 'd');
            *(funcdefs_cursor++) = constructFuncDef(parseFunctionContent(), rettype_and_funcname, 0, params_start);
            return;
        }
        lvars_cursor = lvars_start = calloc(100, sizeof(char *));
        for (int i = 0; i < 6; i++) {
            struct NameAndType *param = consume_type_and_ident();
            if (maybe_consume(')')) {
                params_start[i].name = param->name;
                params_start[i].type = param->type;
                lvars_cursor->name = param->name;
                (lvars_cursor++)->type = param->type;
                store_func_decl(rettype_and_funcname);
                if (maybe_consume(';'))
                    return;
                currently_handling_function_returning_void = rettype_and_funcname->type->kind == enum4('v', 'o', 'i', 'd');
                *(funcdefs_cursor++) = constructFuncDef(parseFunctionContent(), rettype_and_funcname, i + 1, params_start);
                return;
            }
            consume_otherwise_panic(',');
            params_start[i].name = param->name;
            params_start[i].type = param->type;
            lvars_cursor->name = param->name;
            (lvars_cursor++)->type = param->type;
        }
        panic("not supported: more than 6 parameters\n");
    } else {
        struct NameAndType *global_var_type_and_name = consume_type_and_ident_2nd_half(first_half);
        *(global_vars_cursor++) = global_var_type_and_name;
        consume_otherwise_panic(';');
        return;
    }
}

/*** ^ PARSE | v CODEGEN ***/

int labelCounter;

struct LVar *locals;

struct LVar *findLVar(char *name) {
    struct LVar *local = locals;
    if (!local)
        return 0;
    while (local) {
        if (!strcmp(name, local->name))
            return local;
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
    for (int i = 0; global_vars_start[i]; i++)
        if (strcmp(name, global_vars_start[i]->name) == 0)
            return 1;
    return 0;
}

struct LVar *lastLVar() {
    struct LVar *local = locals;
    if (!local)
        return 0;
    while (1) {
        if (!local->next)
            return local;
        local = local->next;
    }
}

struct LVar *insertLVar(char *name, int sz) {
    sz = roundup(sz, 8);
    struct LVar *newlocal = calloc(1, sizeof(struct LVar));
    struct LVar *last = lastLVar();
    newlocal->name = name;
    printf("# inserting a variable named `%s` at offset", name);
    if (!last)
        newlocal->offset_from_rbp = sz;
    else
        newlocal->offset_from_rbp = last->offset_from_rbp + sz;
    printf(" %d\n", newlocal->offset_from_rbp);
    newlocal->next = 0;
    if (!last)
        locals = newlocal;
    else
        last->next = newlocal;
    return newlocal;
}

void EvaluateExprIntoRax(struct Expr *expr);

void EvaluateLValueAddressIntoRax(struct Expr *expr) {
    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        char *name = expr->func_or_ident_name_or_string_content;
        struct LVar *local = findLVar(name);
        if (local) {
            printf("  lea rax, [rbp - %d] # rax = &%s\n", local->offset_from_rbp, name);
        } else if (isGVar(name)) {
            printf("  mov eax, OFFSET FLAT:%s\n", name);
        } else {
            printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf("! Internal compiler error at codegen: undefined variable %s\n", name);
            exit(1);
        }
    } else if (expr->expr_kind == enum3('S', 'T', 'R')) {
        printf("  mov eax, OFFSET FLAT:.LC%d\n", find_strlit(expr->func_or_ident_name_or_string_content));
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y') && expr->op_kind == '*') {
        EvaluateExprIntoRax(expr->first_child);
    } else
        panic("not an lvalue\n");
}

void CodegenStmt(struct Stmt *stmt) {
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
        return &"rdi\0rsi\0rdx\0rcx\0r8 \0r9"[4 * n];
    else if (sz == 4)
        return &"edi\0esi\0edx\0ecx\0r8d\0r9d"[4 * n];
    else if (sz == 1)
        return &"dil\0sil\0dl \0cl \0r8b\0r9b"[4 * n];
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! Internal compiler error at codegen: unhandlable size %d\n", sz);
    exit(1);
}

const char *rax_eax_al(int sz) {
    if (sz == 8)
        return "rax";
    else if (sz == 4)
        return "eax";
    else if (sz == 1)
        return "al";
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! Internal compiler error at codegen: unhandlable size %d\n", sz);
    exit(1);
}

const char *rdi_edi_dil(int sz) {
    return nth_arg_reg(0, sz);
}

void CodegenFunc(struct FuncDef *funcdef) {
    locals = 0;
    printf(".globl %s\n", funcdef->name);
    printf("%s:\n", funcdef->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    int stack_adjust = 0;
    for (int i = 0; i < funcdef->param_len; i++)
        stack_adjust += 8;
    for (struct NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++)
        stack_adjust += roundup(size(ptr->type), 8);
    printf("  sub rsp, %d\n", stack_adjust);
    for (int i = 0; i < funcdef->param_len; i++) {
        char *param_name = funcdef->params_start[i].name;
        insertLVar(param_name, 8);
        struct LVar *local = findLVar(param_name);
        printf("  mov rax, %s\n", nth_arg_reg(i, 8));
        printf("  mov [rbp - %d], %s\n", local->offset_from_rbp, rax_eax_al(size(funcdef->params_start[i].type)));
    }
    for (struct NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++)
        if (!findLVar(ptr->name))  // avoid duplicate insertion of parameters
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
        printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("! Internal compiler error at codegen: unhandlable size %d\n", sz);
        exit(1);
    }
}

void write_rax_to_where_rdi_points(int sz) {
    if (sz == 8)
        printf("  mov [rdi], rax\n");
    else if (sz == 4)
        printf("  mov [rdi], eax\n");
    else if (sz == 1) {
        printf("  mov ecx, eax\n");
        printf("  mov [rdi], cl\n");
    } else {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("! Internal compiler error at codegen: unhandlable size %d\n", sz);
        exit(1);
    }
}

const char *AddSubMulDivAssign_rdi_into_rax(int kind) {
    if (kind == enum2('+', '='))
        return "  add rax,rdi\n";
    else if (kind == enum2('-', '='))
        return "  sub rax,rdi\n";
    else if (kind == enum2('*', '='))
        return "  imul rax,rdi\n";
    else if (kind == enum2('/', '='))
        return "  cqo\n  idiv rdi\n";
    return 0;
}

void EvaluateExprIntoRax(struct Expr *expr) {
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
            printf("  push rax\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--)
            if (size(expr->func_args_start[i]->typ) == 1) {
                printf("  pop rax\n");
                printf("  movsx rax, al\n");
                printf("  mov %s, rax\n", nth_arg_reg(i, 8));
            } else
                printf("  pop %s\n", nth_arg_reg(i, 8));
        printf("  mov rax, 0\n");
        printf("  call %s\n", expr->func_or_ident_name_or_string_content);
    } else if (expr->expr_kind == enum3('N', 'U', 'M')) {
        printf("  mov rax, %d\n", expr->value);
    } else if (expr->expr_kind == '0') {
        printf("  mov rax, 0\n");
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y')) {
        if (expr->op_kind == '*') {
            EvaluateExprIntoRax(expr->first_child);
            printf("  mov %s, [rax]\n", rax_eax_al(size(expr->typ)));
            if (size(expr->typ) == 1) {
                printf("  movsx rax, al\n");
            }
        } else if (expr->op_kind == '&') {
            EvaluateLValueAddressIntoRax(expr->first_child);
        } else if (expr->op_kind == enum4('[', ']', '>', '*')) {
            EvaluateExprIntoRax(expr->first_child);
        } else
            panic_kind("Internal compiler error at codegen: Invalid unaryop kind:", expr->op_kind);
    } else if (expr->expr_kind == enum4('2', 'A', 'R', 'Y')) {
        if (expr->op_kind == '=') {
            EvaluateLValueAddressIntoRax(expr->first_child);
            printf("  push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("  pop rdi\n");
            write_rax_to_where_rdi_points(size(expr->first_child->typ));  // second_child might be a 0 meaning a null pointer
        } else if (AddSubMulDivAssign_rdi_into_rax(expr->op_kind)) {      // x @= i
            EvaluateExprIntoRax(expr->second_child);
            printf("  push rax\n");                                        // stack: i
            EvaluateLValueAddressIntoRax(expr->first_child);               // rax: &x
            printf("  mov rsi, rax\n");                                    // rsi: &x
            printf("  mov rax, [rax]\n");                                  // rsi: &x, rax: x
            printf("  pop rdi\n");                                         // rsi: &x, rax: x, rdi: i
            printf("%s", AddSubMulDivAssign_rdi_into_rax(expr->op_kind));  // rsi: &x, rax: x@i
            printf("  mov rdi, rsi\n");                                    // rdi: &x, rax: x@i
            write_rax_to_where_rdi_points(size(expr->second_child->typ));
        } else if (expr->op_kind == enum2('&', '&')) {
            int label = (labelCounter++);
            EvaluateExprIntoRax(expr->first_child);
            printf("  test rax, rax\n");
            printf("  je .Landfalse%d\n", label);
            EvaluateExprIntoRax(expr->second_child);
            printf("  test rax, rax\n");
            printf("  je  .Landfalse%d\n", label);
            printf("  mov eax, 1\n");
            printf("  jmp .Landend%d\n", label);
            printf(".Landfalse%d:\n", label);
            printf("  mov     eax, 0\n");
            printf(".Landend%d:\n", label);
        } else if (expr->op_kind == enum2('|', '|')) {
            int label = (labelCounter++);
            EvaluateExprIntoRax(expr->first_child);
            printf("  test rax, rax\n");
            printf("  jne .Lorleft%d\n", label);
            EvaluateExprIntoRax(expr->second_child);
            printf("  test rax, rax\n");
            printf("  je  .Lorright%d\n", label);
            printf(".Lorleft%d:\n", label);
            printf("  mov eax, 1\n");
            printf("  jmp .Lorend%d\n", label);
            printf(".Lorright%d:\n", label);
            printf("  mov     eax, 0\n");
            printf(".Lorend%d:\n", label);
        } else {
            EvaluateExprIntoRax(expr->first_child);
            printf("  push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("  mov rdi, rax\n");
            printf("  pop rax\n");
            int siz = size(expr->first_child->typ);
            if (AddSubMulDivAssign_rdi_into_rax(enum2(expr->op_kind, '='))) {
                printf("%s", AddSubMulDivAssign_rdi_into_rax(enum2(expr->op_kind, '=')));
            } else if (expr->op_kind == enum2('=', '=')) {
                printf("  cmp %s, %s\n", rax_eax_al(siz), rdi_edi_dil(siz));
                printf("  sete al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op_kind == enum2('!', '=')) {
                printf("  cmp %s, %s\n", rax_eax_al(siz), rdi_edi_dil(siz));
                printf("  setne al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op_kind == '>') {
                printf("  cmp %s, %s\n", rax_eax_al(siz), rdi_edi_dil(siz));
                printf("  setg al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op_kind == enum2('>', '=')) {
                printf("  cmp %s, %s\n", rax_eax_al(siz), rdi_edi_dil(siz));
                printf("  setge al\n");
                printf("  movzb rax, al\n");
            } else
                panic_kind("Internal compiler error at codegen: Invalid binaryop kind:", expr->op_kind);
        }
    } else
        panic_kind("Internal compiler error at codegen: Invalid expr kind:", expr->expr_kind);
}

/*** ^ CODEGEN | v MAIN ***/

int main(int argc, char **argv) {
    if (argc != 2)
        panic("incorrect cmd line arg\n");
    string_literals_cursor = string_literals_start;
    tokens_cursor = tokens_start;  // the 1st tokens_cursor is for storing the tokens
    entire_input_start = argv[1];
    tokens_end = tokenize(argv[1]);
    if (tokens_start == tokens_end)
        panic("no token found\n");
    tokens_cursor = tokens_start;  // the 2nd tokens_cursor is for parsing
    struct_members_cursor = struct_members_start;
    struct_sizes_and_alignments_cursor = struct_sizes_and_alignments_start;
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
