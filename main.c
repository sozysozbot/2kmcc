#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int BinaryOpKind;
typedef int UnaryOpKind;
typedef int BaseTypeKind;
typedef int TokenKind;
typedef int StmtKind;
typedef int ExprKind;

typedef struct Type {
    BaseTypeKind ty;
    struct Type *ptr_to;
} Type;

typedef struct Expr {
    BinaryOpKind op;
    ExprKind expr_kind;
    int value;
    struct Expr *first_child;
    struct Expr *second_child;
    struct Expr **func_args;
    int func_arg_len;
    char *name;
} Expr;

typedef struct FuncDef {
    struct Stmt *content;
    char *name;
    char **params;
    int param_len;
    char **lvar_names_start;
    char **lvar_names_end;
} FuncDef;

typedef struct Stmt {
    StmtKind stmt_kind;
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
    TokenKind kind;
    int value;
    char *identifier_name;
} Token;

Expr *parseMultiplicative(void);
Expr *parseAdditive(void);
Expr *parseExpr(void);
Expr *parseUnary(void);
void parseProgram(void);
Expr *parseAssign(void);
Stmt *parseFor(void);
Stmt *parseStmt(void);
FuncDef *parseFunction(void);

void CodegenFunc(FuncDef *funcdef);

int tokenize(char *str);

void EvaluateExprIntoRax(Expr *expr);
extern FuncDef *all_funcdefs[100];

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

Expr *numberexpr(int value) {
    Expr *numberexp = calloc(1, sizeof(Expr));
    numberexp->value = value;
    numberexp->expr_kind = enum3('N', 'U', 'M');
    return numberexp;
}

Expr *binaryExpr(Expr *first_child, Expr *second_child, BinaryOpKind binaryop) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('2', 'A', 'R', 'Y');
    newexp->op = binaryop;
    newexp->second_child = second_child;
    return newexp;
}

Expr *unaryExpr(Expr *first_child, UnaryOpKind unaryop) {
    Expr *newexp = calloc(1, sizeof(Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('1', 'A', 'R', 'Y');
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

char *decode_kind(int kind) {
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
                callexp->name = name;
                callexp->expr_kind = enum4('C', 'A', 'L', 'L');
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
            callexp->expr_kind = enum4('C', 'A', 'L', 'L');
            callexp->func_args = arguments;
            callexp->func_arg_len = i + 1;
            return callexp;
        } else {
            Expr *numberexp = calloc(1, sizeof(Expr));
            numberexp->name = name;
            numberexp->expr_kind = enum4('1', 'D', 'N', 'T');
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
        if (tokens->kind == enum3('N', 'U', 'M')) {
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
        if (tokens->kind == enum3('N', 'U', 'M')) {
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

char **lvar_names_start;
char **lvar_names;

Expr *parseOptionalExprAndToken(TokenKind target) {
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
        Type *new_t = calloc(1, sizeof(Type));
        new_t->ty = '*';
        new_t->ptr_to = t;
        t = new_t;
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
        if (lvar_names == lvar_names_start + 100) {
            fprintf(stderr, "too many local variables");
            exit(1);
        }
        char *name = expect_identifier_and_get_name();
        *lvar_names = name;
        lvar_names++;
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
    Type *t = consume_type_otherwise_panic();
    char *name = expect_identifier_and_get_name();
    char **params = calloc(6, sizeof(char *));
    consume_otherwise_panic('(');
    if (maybe_consume(')')) {
        lvar_names = lvar_names_start = calloc(100, sizeof(char *));
        Stmt *content = parseFunctionContent();
        FuncDef *funcdef = calloc(1, sizeof(FuncDef));
        funcdef->content = content;
        funcdef->name = name;
        funcdef->param_len = 0;
        funcdef->params = params;
        funcdef->lvar_names_start = lvar_names_start;
        funcdef->lvar_names_end = lvar_names;
        return funcdef;
    }

    int i = 0;
    for (; i < 6; i++) {
        Type *t = consume_type_otherwise_panic();
        char *name = expect_identifier_and_get_name();
        if (maybe_consume(')')) {
            params[i] = name;
            break;
        }
        consume_otherwise_panic(',');
        params[i] = name;
    }

    lvar_names = lvar_names_start = calloc(100, sizeof(char *));
    Stmt *content = parseFunctionContent();
    FuncDef *funcdef = calloc(1, sizeof(FuncDef));
    funcdef->content = content;
    funcdef->name = name;
    funcdef->param_len = i + 1;
    funcdef->params = params;
    funcdef->lvar_names_start = lvar_names_start;
    funcdef->lvar_names_end = lvar_names;
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

void EvaluateLValueAddressIntoRax(Expr *expr) {
    if (expr->expr_kind == enum4('1', 'D', 'N', 'T')) {
        if (!findLVar(expr->name)) {
            fprintf(stderr, "undefined variable %s\n", expr->name);
            exit(1);
        }
        LVar *local = findLVar(expr->name);
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
    for (char **names = funcdef->lvar_names_start; names != funcdef->lvar_names_end; names++) {
        insertLVar(*names);
    }
    CodegenStmt(funcdef->content);
}

void EvaluateExprIntoRax(Expr *expr) {
    if (expr->expr_kind == enum4('1', 'D', 'N', 'T')) {
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
        printf(" call %s\n", expr->name);
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

FuncDef *all_funcdefs[100];

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
    parseProgram();
    printf(".intel_syntax noprefix\n");
    for (int i = 0; all_funcdefs[i]; i++) {
        FuncDef *funcdef = all_funcdefs[i];
        CodegenFunc(funcdef);
    }
    return 0;
}
