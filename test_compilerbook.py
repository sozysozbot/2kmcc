from test import bcolors, check, compile_with_2kmcc, get_compiler_name, run_resulting_binary
import os

def check_stepN_test_case(n: int, step_n: str, input_to_step_n: str, expected_output: int):
    return check_stepN_that_gcc_compiled(n, step_n, input_to_step_n, expected_output) and check_stepN_that_2kmcc_compiled(
        n, step_n, input_to_step_n, expected_output)


def check_stepN_that_gcc_compiled(n: int, step_n: str, input_to_step_n: str, expected_output: int):
    open("tmp_gcc_stepn.c", "w").write(step_n)
    os.system("gcc -Wno-builtin-declaration-mismatch -std=c11 -g -static -o tmp_gcc_stepn tmp_gcc_stepn.c")
    value_returned_from_stepN = run_resulting_binary(
        "./tmp_gcc_stepn", stdin=input_to_step_n, stdout_path="tmp_final.s")
    return rest(n, value_returned_from_stepN, expected_output, step_n, input_to_step_n, "gcc")


def check_stepN_that_2kmcc_compiled(n: int, step_n: str, input_to_step_n: str, expected_output: int):
    compiler_name = get_compiler_name()
    compiler_returns = compile_with_2kmcc(step_n, f"tmp_{compiler_name}_stepn.s")
    if compiler_returns != 0:
        print(
            f"{bcolors.FAIL}FAIL:check ({compiler_name} gave a compile error):{step_n=}{bcolors.ENDC}")
        msg = open("tmp_compile_errmsg.txt", "r").read()
        print(f"  The error message is:\n{bcolors.FAIL}{msg}{bcolors.ENDC}")
        return False
    os.system(f"cc -o tmp_{compiler_name}_stepn tmp_{compiler_name}_stepn.s -static")
    value_returned_from_stepN = run_resulting_binary(
        f"./tmp_{compiler_name}_stepn", stdin=input_to_step_n, stdout_path="tmp_final.s")
    return rest(n, value_returned_from_stepN, expected_output, step_n, input_to_step_n, compiler_name)

def rest(n: int, value_returned_from_stepN: int, expected_output: int, step_n: str, input_to_step_n: str, compiler: str):
    os.system("cc -o tmp_final tmp_final.s -static")
    actual_output = run_resulting_binary("./tmp_final")

    if 0 != value_returned_from_stepN:
        print(f"{bcolors.FAIL}FAIL:check (stepN returned with non-zero):{bcolors.ENDC}")
        print(f"  {step_n=}")
        return False
    elif expected_output != actual_output:
        print(
            f"{bcolors.FAIL}FAIL:check (stepN returned with zero, but gives wrong result):{bcolors.ENDC}")
        print(f"  {step_n=}")
        print(f"{bcolors.FAIL}  {expected_output=}\n  {actual_output=}{bcolors.ENDC}")
        return False
    else:
        print(f"{bcolors.OKGREEN}step #{n} passed (when compiled with {compiler}):\n  {input_to_step_n=}\n  {expected_output=} {bcolors.ENDC}")
        os.system(
            f"rm tmp_{compiler}_stepn* tmp_final tmp_final.s tmp_run_stdout.txt")
        return True


print(f"{bcolors.OKBLUE}今までのステップをコンパイルできるか確認していく{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Checking the earlier steps from the compilerbook:{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}================================================={bcolors.ENDC}")

###################################################################################################
print(f"{bcolors.OKBLUE}ステップ1：整数1個をコンパイルする言語の作成{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Step 1: making a language that can compile a single integer{bcolors.ENDC}")
###################################################################################################
step1 = """
int printf();
int atoi(); 

int main(int argc, char **argv) {
    if (argc != 2) {
        return 3;
    }
    printf(".intel_syntax noprefix\\n");
    printf(".globl main\\n");
    printf("main:\\n");
    printf("  mov rax, %d\\n", atoi(argv[1]));
    printf("  ret\\n");
    return 0;
}
"""

assert check_stepN_test_case(1, step1, '0', 0)
assert check_stepN_test_case(1, step1, '42', 42)
assert check(step1, 0, stdin="42", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 42
  ret
""")
assert check(step1, 0, stdin="0", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 0
  ret
""")

###################################################################################################
print(f"{bcolors.OKBLUE}-------------------------------------------------{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}ステップ2：加減算のできるコンパイラの作成{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Step 2: making a compiler that can add and subtract{bcolors.ENDC}")
###################################################################################################
step2 = """int printf();
int isDigit(char c);
int parseInt(char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int intLength(char *str) {
    int length = 0;
    while (isDigit(*str)) {
        length++;
        str++;
    }
    return length;
}
int isDigit(char c) {
    return '0' <= c && c <= '9';
}
int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }
    char *p = argv[1];
    printf(".intel_syntax noprefix\\n");
    printf(".globl main\\n");
    printf("main:\\n");
    int parsednum_ = parseInt(p);
    int parsedlength_ = intLength(p);
    p += parsedlength_;
    printf("  mov rax, %d\\n", parsednum_);
    while (*p) {
        if (*p == '+') {
            p++;
            int parsednum = parseInt(p);
            int parsedlength = intLength(p);
            p += parsedlength;
            printf("  add rax, %d\\n", parsednum);
        } else if (*p == '-') {
            p++;
            int parsednum2 = parseInt(p);
            int parsedlength2 = intLength(p);
            p += parsedlength2;
            printf("  sub rax, %d\\n", parsednum2);
        } else {
            return 2;
        }
    }
    printf("  ret\\n");
    return 0;
}"""

assert check_stepN_test_case(2, step2, '0', 0)
assert check_stepN_test_case(2, step2, '42', 42)
assert check_stepN_test_case(2, step2, '0+10+3', 13)
assert check_stepN_test_case(2, step2, '111+10-42', 79)

assert check(step2, 0, stdin="0", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 0
  ret
""")
assert check(step2, 0, stdin="42", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 42
  ret
""")
assert check(step2, 0, stdin="0+10+3", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 0
  add rax, 10
  add rax, 3
  ret
""")
assert check(step2, 0, stdin="111+10-42", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 111
  add rax, 10
  sub rax, 42
  ret
""")


###################################################################################################
print(f"{bcolors.OKBLUE}-------------------------------------------------{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}ステップ3：トークナイザを導入{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Step 3: introducing a tokenizer{bcolors.ENDC}")
###################################################################################################
step3 = """
int printf();
struct Token {
    char kind;
    int value;
};

int isDigit(char c);
int intLength(char *str);
int parseInt(char *str);

struct Token tokens[1000];
int tokenize(char *str) {
    int token_index = 0;
    for (int i = 0; str[i];) {
        char c = str[i];
        if (c == '+') {
            tokens[token_index].kind = '+';
            token_index++;
            i++;
        } else if (c == '-') {
            tokens[token_index].kind = '-';
            token_index++;
            i++;
        } else if ('0' <= c && c <= '9') {
            int parsednum = parseInt(&str[i]);
            int parsedlength = intLength(&str[i]);
            i += parsedlength;
            tokens[token_index].kind = '#';
            tokens[token_index].value = parsednum;
            token_index++;
        } else if (c == ' ') {
            i++;
        } else {
            return -1;
        }
    }
    return token_index;
}

int parseInt(char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int intLength(char *str) {
    int length = 0;
    while (isDigit(*str)) {
        length++;
        str++;
    }
    return length;
}
int isDigit(char c) {
    return '0' <= c && c <= '9';
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    char *p = argv[1];
    int token_length = tokenize(p);
    if (token_length == 0) {
        return 1;
    }

    printf(".intel_syntax noprefix\\n");
    printf(".globl main\\n");
    printf("main:\\n");

    struct Token token = tokens[0];
    if (token.kind != '#') {
        return 1;
    }
    printf("  mov rax, %d\\n", token.value);

    for (int i = 1; i < token_length;) {
        struct Token maybe_operator = tokens[i];
        if (maybe_operator.kind == '#') {
            return 1;
        } else if (maybe_operator.kind == '+') {
            i++;
            if (i >= token_length) {
                return 1;
            }

            struct Token maybe_number = tokens[i];
            if (maybe_number.kind != '#') {
                return 1;
            }
            printf("  add rax, %d\\n", maybe_number.value);
            i++;
        } else if (maybe_operator.kind == '-') {
            i++;
            if (i >= token_length) {
                return 1;
            }
            struct Token maybe_number = tokens[i];
            if (maybe_number.kind != '#') {
                return 1;
            }
            printf("  sub rax, %d\\n", maybe_number.value);
            i++;
        } else {
            return 1;
        }
    }
    printf("  ret\\n");
    return 0;
}
"""

assert check_stepN_test_case(3, step3, '0', 0)
assert check_stepN_test_case(3, step3, '42', 42)
assert check_stepN_test_case(3, step3, '0+10+3', 13)
assert check_stepN_test_case(3, step3, '111+10-42', 79)
assert check_stepN_test_case(3, step3, '   111   + 10 -     42', 79)
assert check_stepN_test_case(3, step3, '   0 +    10+    3', 13)
assert check(step3, 0, stdin="0", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 0
  ret
""")
assert check(step3, 0, stdin="42", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 42
  ret
""")
assert check(step3, 0, stdin="0+10+3", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 0
  add rax, 10
  add rax, 3
  ret
""")
assert check(step3, 0, stdin="111+10-42", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 111
  add rax, 10
  sub rax, 42
  ret
""")
assert check(step3, 0, stdin="    111  +   10 -   42", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 111
  add rax, 10
  sub rax, 42
  ret
""")

###################################################################################################
print(f"{bcolors.OKBLUE}-------------------------------------------------{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}ステップ5：四則演算のできる言語の作成{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Step 5: creating a language that can handle basic arithmetics{bcolors.ENDC}")
###################################################################################################
step5 = """
int printf();
char *strchr();
void exit();
void *calloc();

struct Expr {
    char binary_op;
    char expr_kind;
    int value;
    struct Expr *first_child;
    struct Expr *second_child;
};

struct Token {
    char kind;
    int value;
};

int isDigit(char c);
int intLength(char *str);
int parseInt(char *str);
struct Expr *parseMultiplicative(struct Token **ptrptr, struct Token *token_end);
struct Expr *parseAdditive(struct Token **ptrptr, struct Token *token_end);
struct Expr *parseExpr(struct Token **ptrptr, struct Token *token_end);

struct Token tokens[1000];

void EvaluateExprIntoRax(struct Expr *expr) {
    if (expr->expr_kind == '#') {
        printf("  mov rax, %d\\n", expr->value);
    } else if (expr->expr_kind == '@') {
        EvaluateExprIntoRax(expr->first_child);
        printf("    push rax\\n");
        EvaluateExprIntoRax(expr->second_child);
        printf("    push rax\\n");
        printf("    pop rdi\\n");
        printf("    pop rax\\n");
        if (expr->binary_op == '+') {
            printf("    add rax,rdi\\n");
        } else if (expr->binary_op == '-') {
            printf("    sub rax,rdi\\n");
        } else if (expr->binary_op == '*') {
            printf("    imul rax,rdi\\n");
        } else if (expr->binary_op == '/') {
            printf("  cqo\\n");
            printf("  idiv rdi\\n");
        } else {
            exit(1);
        }
    } else {
        exit(1);
    }
}

struct Expr *numberexpr(int value) {
    struct Expr *numberexp = calloc(1, sizeof(struct Expr));
    numberexp->value = value;
    numberexp->expr_kind = '#';
    return numberexp;
}

struct Expr *binaryExpr(struct Expr *first_child, struct Expr *second_child, char binaryop) {
    struct Expr *newexp = calloc(1, sizeof(struct Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = '@';
    newexp->binary_op = binaryop;
    newexp->second_child = second_child;
    return newexp;
}

struct Expr *parsePrimary(struct Token **ptrptr, struct Token *token_end) {
    struct Token *maybe_number = *ptrptr;
    if (maybe_number >= token_end) {
        exit(1);
    }
    if (maybe_number->kind != '#') {
        struct Token *maybe_leftparenthesis = maybe_number;
        if (maybe_leftparenthesis->kind == '(') {
            *ptrptr += 1;
            struct Expr *expr = parseExpr(ptrptr, token_end);
            struct Token *maybe_rightparenthesis = *ptrptr;
            if (maybe_rightparenthesis->kind != ')') {
                exit(1);
            }
            *ptrptr += 1;
            return expr;
        }
        exit(1);
    }
    *ptrptr += 1;
    return numberexpr(maybe_number->value);
}

struct Expr *parseExpr(struct Token **ptrptr, struct Token *token_end) {
    return parseAdditive(ptrptr, token_end);
}
struct Expr *parseMultiplicative(struct Token **ptrptr, struct Token *token_end) {
    struct Token *tokens = *ptrptr;
    if (token_end == tokens) {
        exit(1);
    }
    struct Expr *result = parsePrimary(&tokens, token_end);

    for (; tokens < token_end;) {
        struct Token maybe_operator = *tokens;
        if (maybe_operator.kind == '#') {
            exit(1);
        } else if (maybe_operator.kind == '*') {
            tokens++;
            struct Expr *numberexp = parsePrimary(&tokens, token_end);
            result = binaryExpr(result, numberexp, '*');
        } else if (maybe_operator.kind == '/') {
            tokens++;
            struct Expr *numberexp = parsePrimary(&tokens, token_end);
            result = binaryExpr(result, numberexp, '/');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

struct Expr *parseAdditive(struct Token **ptrptr, struct Token *token_end) {
    struct Token *tokens = *ptrptr;
    if (token_end == tokens) {
        exit(1);
    }
    struct Expr *result = parseMultiplicative(&tokens, token_end);

    for (; tokens < token_end;) {
        struct Token maybe_operator = *tokens;
        if (maybe_operator.kind == '#') {
            exit(1);
        } else if (maybe_operator.kind == '-') {
            tokens++;
            struct Expr *numberexp = parseMultiplicative(&tokens, token_end);
            result = binaryExpr(result, numberexp, '-');
        }
        if (maybe_operator.kind == '+') {
            tokens++;
            struct Expr *numberexp = parseMultiplicative(&tokens, token_end);
            result = binaryExpr(result, numberexp, '+');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

int tokenize(char *str) {
    int token_index = 0;
    for (int i = 0; str[i];) {
        char c = str[i];
        if (strchr("+()-*/", c)) {
            tokens[token_index].kind = c;
            token_index++;
            i++;
        } else if ('0' <= c && c <= '9') {
            int parsednum = parseInt(&str[i]);
            int parsedlength = intLength(&str[i]);
            i += parsedlength;
            tokens[token_index].kind = '#';
            tokens[token_index].value = parsednum;
            token_index++;
        } else if (c == ' ') {
            i++;
        } else {
            exit(1);
        }
    }
    return token_index;
}

int parseInt(char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int intLength(char *str) {
    int length = 0;
    while (isDigit(*str)) {
        length++;
        str++;
    }
    return length;
}
int isDigit(char c) {
    return '0' <= c && c <= '9';
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    char *p = argv[1];

    int token_length = tokenize(p);

    if (token_length == 0) {
        return 1;
    }

    printf(".intel_syntax noprefix\\n");
    printf(".globl main\\n");
    printf("main:\\n");
    struct Token *ptr = tokens;
    struct Token *token_end = tokens + token_length;
    struct Expr *expr = parseExpr(&ptr, token_end);
    EvaluateExprIntoRax(expr);
    printf("  ret\\n");
    return 0;
}
"""
assert check_stepN_test_case(5, step5, '0', 0)
assert check_stepN_test_case(5, step5, '42', 42)
assert check_stepN_test_case(5, step5, '0+10+3', 13)
assert check_stepN_test_case(5, step5, '111+10-42', 79)
assert check_stepN_test_case(5, step5, '   111   + 10 -     42', 79)
assert check_stepN_test_case(5, step5, '   0 +    10+    3', 13)
assert check_stepN_test_case(5, step5, '10*2', 20)
assert check_stepN_test_case(5, step5, '10+1*2', 12)
assert check_stepN_test_case(5, step5, '10+3*2+10-5', 21)
assert check_stepN_test_case(5, step5, '(10+3)*2+10-5', 31)
assert check_stepN_test_case(5, step5, '(10+1)*2', 22)
assert check_stepN_test_case(5, step5, '(10+1)/2', 5)
assert check_stepN_test_case(5, step5, '(15+1)/2+3', 11)
assert check_stepN_test_case(5, step5, '10+1 /2/5', 10)

###################################################################################################
print(f"{bcolors.OKBLUE}-------------------------------------------------{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}ステップ6：単項プラスと単項マイナス{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Step 6: unary plus / unary minus{bcolors.ENDC}")
###################################################################################################
step6 = """int printf();
char *strchr();
void exit();
void *calloc();

struct Expr {
    char binary_op;
    char expr_kind;
    int value;
    struct Expr *first_child;
    struct Expr *second_child;
};

struct Token {
    char kind;
    int value;
};

int isDigit(char c);
int intLength(char *str);
int parseInt(char *str);
struct Expr *parseMultiplicative(struct Token **ptrptr, struct Token *token_end);
struct Expr *parseAdditive(struct Token **ptrptr, struct Token *token_end);
struct Expr *parseExpr(struct Token **ptrptr, struct Token *token_end);

struct Token tokens[1000];

void EvaluateExprIntoRax(struct Expr *expr) {
    if (expr->expr_kind == '#') {
        printf("  mov rax, %d\\n", expr->value);
    } else if (expr->expr_kind == '@') {
        EvaluateExprIntoRax(expr->first_child);
        printf("    push rax\\n");
        EvaluateExprIntoRax(expr->second_child);
        printf("    push rax\\n");
        printf("    pop rdi\\n");
        printf("    pop rax\\n");
        if (expr->binary_op == '+') {
            printf("    add rax,rdi\\n");
        } else if (expr->binary_op == '-') {
            printf("    sub rax,rdi\\n");
        } else if (expr->binary_op == '*') {
            printf("    imul rax,rdi\\n");
        } else if (expr->binary_op == '/') {
            printf("  cqo\\n");
            printf("  idiv rdi\\n");
        } else {
            exit(1);
        }
    } else {
        exit(1);
    }
}

struct Expr *numberexpr(int value) {
    struct Expr *numberexp = calloc(1, sizeof(struct Expr));
    numberexp->value = value;
    numberexp->expr_kind = '#';
    return numberexp;
}

struct Expr *binaryExpr(struct Expr *first_child, struct Expr *second_child, char binaryop) {
    struct Expr *newexp = calloc(1, sizeof(struct Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = '@';
    newexp->binary_op = binaryop;
    newexp->second_child = second_child;
    return newexp;
}

struct Expr *parsePrimary(struct Token **ptrptr, struct Token *token_end) {
    struct Token *maybe_number = *ptrptr;
    if (maybe_number >= token_end) {
        exit(1);
    }
    if (maybe_number->kind != '#') {
        struct Token *maybe_leftparenthesis = maybe_number;
        if (maybe_leftparenthesis->kind == '(') {
            *ptrptr += 1;
            struct Expr *expr = parseExpr(ptrptr, token_end);
            struct Token *maybe_rightparenthesis = *ptrptr;
            if (maybe_rightparenthesis->kind != ')') {
                exit(1);
            }
            *ptrptr += 1;
            return expr;
        }
        exit(1);
    }
    *ptrptr += 1;
    return numberexpr(maybe_number->value);
}

struct Expr *parseUnary(struct Token **ptrptr, struct Token *token_end) {
    struct Token *maybe_unary = *ptrptr;
    if (maybe_unary >= token_end) {
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

struct Expr *parseExpr(struct Token **ptrptr, struct Token *token_end) {
    return parseAdditive(ptrptr, token_end);
}

struct Expr *parseMultiplicative(struct Token **ptrptr, struct Token *token_end) {
    struct Token *tokens = *ptrptr;
    if (token_end == tokens) {
        exit(1);
    }
    struct Expr *result = parseUnary(&tokens, token_end);

    for (; tokens < token_end;) {
        struct Token maybe_operator = *tokens;
        if (maybe_operator.kind == '#') {
            exit(1);
        } else if (maybe_operator.kind == '*') {
            tokens++;
            struct Expr *numberexp = parseUnary(&tokens, token_end);
            result = binaryExpr(result, numberexp, '*');
        } else if (maybe_operator.kind == '/') {
            tokens++;
            struct Expr *numberexp = parseUnary(&tokens, token_end);
            result = binaryExpr(result, numberexp, '/');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

struct Expr *parseAdditive(struct Token **ptrptr, struct Token *token_end) {
    struct Token *tokens = *ptrptr;
    if (token_end == tokens) {
        exit(1);
    }
    struct Expr *result = parseMultiplicative(&tokens, token_end);

    for (; tokens < token_end;) {
        struct Token maybe_operator = *tokens;
        if (maybe_operator.kind == '#') {
            exit(1);
        } else if (maybe_operator.kind == '-') {
            tokens++;
            struct Expr *numberexp = parseMultiplicative(&tokens, token_end);
            result = binaryExpr(result, numberexp, '-');
        }
        if (maybe_operator.kind == '+') {
            tokens++;
            struct Expr *numberexp = parseMultiplicative(&tokens, token_end);
            result = binaryExpr(result, numberexp, '+');
        } else {
            *ptrptr = tokens;
            return result;
        }
    }
    *ptrptr = tokens;
    return result;
}

int tokenize(char *str) {
    int token_index = 0;
    for (int i = 0; str[i];) {
        char c = str[i];
        if (strchr("+()-*/", c)) {
            tokens[token_index].kind = c;
            token_index++;
            i++;
        } else if ('0' <= c && c <= '9') {
            int parsednum = parseInt(&str[i]);
            int parsedlength = intLength(&str[i]);
            i += parsedlength;
            tokens[token_index].kind = '#';
            tokens[token_index].value = parsednum;
            token_index++;
        } else if (c == ' ') {
            i++;
        } else {
            exit(1);
        }
    }
    return token_index;
}

int parseInt(char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int intLength(char *str) {
    int length = 0;
    while (isDigit(*str)) {
        length++;
        str++;
    }
    return length;
}
int isDigit(char c) {
    return '0' <= c && c <= '9';
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    char *p = argv[1];

    int token_length = tokenize(p);

    if (token_length == 0) {
        return 1;
    }

    printf(".intel_syntax noprefix\\n");
    printf(".globl main\\n");
    printf("main:\\n");
    struct Token *ptr = tokens;
    struct Token *token_end = tokens + token_length;
    struct Expr *expr = parseExpr(&ptr, token_end);
    EvaluateExprIntoRax(expr);
    printf("  ret\\n");
    return 0;
}"""

assert check_stepN_test_case(6, step6, '0', 0)
assert check_stepN_test_case(6, step6, '42', 42)
assert check_stepN_test_case(6, step6, '0+10+3', 13)
assert check_stepN_test_case(6, step6, '111+10-42', 79)
assert check_stepN_test_case(6, step6, '   111   + 10 -     42', 79)
assert check_stepN_test_case(6, step6, '   0 +    10+    3', 13)
assert check_stepN_test_case(6, step6, '10*2', 20)
assert check_stepN_test_case(6, step6, '10+1*2', 12)
assert check_stepN_test_case(6, step6, '10+3*2+10-5', 21)
assert check_stepN_test_case(6, step6, '(10+3)*2+10-5', 31)
assert check_stepN_test_case(6, step6, '(10+1)*2', 22)
assert check_stepN_test_case(6, step6, '(10+1)/2', 5)
assert check_stepN_test_case(6, step6, '(15+1)/2+3', 11)
assert check_stepN_test_case(6, step6, '10+1 /2/5', 10)
assert check_stepN_test_case(6, step6, '-2*-3', 6)

###################################################################################################
print(f"{bcolors.OKBLUE}ステップ18: ポインタ型を導入する{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Step 18: introduce pointer types{bcolors.ENDC}")
###################################################################################################
step18 = """
int printf();
void exit();
void *calloc();
int strcmp();
int strncmp();
char *strchr();
void *memcpy();
char * strncpy();

struct Type {
    int ty_kind;
    struct Type *ptr_to;
};

struct Expr {
    int op_kind;
    int expr_kind;
    int value;
    struct Expr *first_child;
    struct Expr *second_child;
    struct Expr **func_args;
    int func_arg_len;
    char *name;
};

struct FuncDef {
    struct Stmt *content;
    char *name;
    char **params;
    int param_len;
    char **lvar_names_start;
    char **lvar_names_end;
};

struct Stmt {
    int stmt_kind;
    struct Expr *expr;
    struct Expr *expr1;
    struct Expr *expr2;
    struct Stmt *first_child;
    struct Stmt *second_child;
    struct Stmt *third_child;
};

struct LVar {
    struct LVar *next;
    char *name;
    int offset_from_rbp;
};

struct Token {
    int kind;
    int value;
    char *identifier_name;
};

struct Expr *parseMultiplicative(void);
struct Expr *parseAdditive(void);
struct Expr *parseExpr(void);
struct Expr *parseUnary(void);
void parseProgram(void);
struct Expr *parseAssign(void);
struct Stmt *parseFor(void);
struct Stmt *parseStmt(void);
struct FuncDef *parseFunction(void);

void CodegenFunc(struct FuncDef *funcdef);

int tokenize(char *str);
struct LVar *findLVar(char *name);
struct LVar *insertLVar(char *name);
struct LVar *lastLVar();
int is_alnum(char c) {
    return strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", c) != 0;
}

void EvaluateExprIntoRax(struct Expr *expr);

int enum2(int a, int b);
int enum3(int a, int b, int c);
int enum4(int a, int b, int c, int d);

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
struct Token all_tokens[1000];
struct FuncDef *all_funcdefs[100];
struct Token *tokens_end;
struct Token *tokens;
int tokenize(char *str) {
    int token_index = 0;
    for (int i = 0; str[i];) {
        char c = str[i];
        char *ptr = str + i;
        if (strncmp(ptr, "return", 6) == 0 && !is_alnum(ptr[6])) {
            all_tokens[token_index].kind = enum3('R', 'E', 'T');
            token_index++;
            i += 6;
        } else if (strncmp(ptr, "if", 2) == 0 && !is_alnum(ptr[2])) {
            all_tokens[token_index].kind = enum2('i', 'f');
            token_index++;
            i += 2;
        } else if (strncmp(ptr, "while", 5) == 0 && !is_alnum(ptr[5])) {
            all_tokens[token_index].kind = enum4('W', 'H', 'I', 'L');
            token_index++;
            i += 5;
        } else if (strncmp(ptr, "else", 4) == 0 && !is_alnum(ptr[4])) {
            all_tokens[token_index].kind = enum4('e', 'l', 's', 'e');
            token_index++;
            i += 4;
        } else if (strncmp(ptr, "for", 3) == 0 && !is_alnum(ptr[3])) {
            all_tokens[token_index].kind = enum3('f', 'o', 'r');
            token_index++;
            i += 3;
        } else if (strncmp(ptr, "int", 3) == 0 && !is_alnum(ptr[3])) {
            all_tokens[token_index].kind = enum3('i', 'n', 't');
            token_index++;
            i += 3;
        } else if (strchr("+-*&/;(){},", c)) {
            all_tokens[token_index].kind = c;
            token_index++;
            i++;
        } else if (c == '>') {
            i++;
            char c = str[i];
            if (c != '=') {
                all_tokens[token_index].kind = '>';
                token_index++;
            } else {
                i++;
                all_tokens[token_index].kind = enum2('>', '=');
                token_index++;
            }
        } else if (c == '<') {
            i++;
            char c = str[i];
            if (c != '=') {
                all_tokens[token_index].kind = '<';
                token_index++;
            } else {
                i++;
                all_tokens[token_index].kind = enum2('<', '=');
                token_index++;
            }
        } else if (c == '=') {
            i++;
            char c = str[i];
            if (c != '=') {
                all_tokens[token_index].kind = '=';
                token_index++;
            } else {
                i++;
                all_tokens[token_index].kind = enum2('=', '=');
                token_index++;
            }
        } else if (c == '!') {
            i++;
            char c = str[i];
            if (c != '=') {
                exit(1);
            }
            i++;
            all_tokens[token_index].kind = enum2('!', '=');
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
            all_tokens[token_index].kind = enum3('N', 'U', 'M');
            all_tokens[token_index].value = parsed_num;
            token_index++;
        } else if (c == ' ') {
            i++;
        } else if (strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", c)) {
            char *start = &str[i];
            for (i++; is_alnum(str[i]); i++) {
            }
            int length = &str[i] - start;
            char *name = calloc(length + 1, sizeof(char));
            memcpy(name, start, length);
            all_tokens[token_index].kind = enum4('I', 'D', 'N', 'T');
            all_tokens[token_index].identifier_name = name;
            token_index++;
        } else {
            exit(1);
        }
    }
    return token_index;
}

/*** ^ TOKENIZE | v PARSE ***/

struct Expr *numberexpr(int value) {
    struct Expr *numberexp = calloc(1, sizeof(struct Expr));
    numberexp->value = value;
    numberexp->expr_kind = enum3('N', 'U', 'M');
    return numberexp;
}

struct Expr *binaryExpr(struct Expr *first_child, struct Expr *second_child, int binaryop) {
    struct Expr *newexp = calloc(1, sizeof(struct Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('2', 'A', 'R', 'Y');
    newexp->op_kind = binaryop;
    newexp->second_child = second_child;
    return newexp;
}

struct Expr *unaryExpr(struct Expr *first_child, int unaryop) {
    struct Expr *newexp = calloc(1, sizeof(struct Expr));
    newexp->first_child = first_child;
    newexp->expr_kind = enum4('1', 'A', 'R', 'Y');
    newexp->op_kind = unaryop;
    return newexp;
}

int maybe_consume(int kind) {
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
        exit(1);
    }
}

void expect_otherwise_panic(int kind) {
    if (tokens->kind != kind) {
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
        exit(1);
    }
}

struct Expr *parsePrimary() {
    panic_if_eof();
    if (tokens->kind == enum3('N', 'U', 'M')) {
        int value = tokens->value;
        tokens += 1;
        return numberexpr(value);
    } else if (tokens->kind == enum4('I', 'D', 'N', 'T')) {
        char *name = tokens->identifier_name;
        tokens += 1;
        if (maybe_consume('(')) {
            struct Expr **arguments = calloc(6, sizeof(struct Expr *));
            if (maybe_consume(')')) {
                struct Expr *callexp = calloc(1, sizeof(struct Expr));
                callexp->name = name;
                callexp->expr_kind = enum4('C', 'A', 'L', 'L');
                callexp->func_args = arguments;
                callexp->func_arg_len = 0;
                return callexp;
            }

            int i = 0;
            for (; i < 6; i++) {
                struct Expr *expr = parseExpr();
                if (maybe_consume(')')) {
                    arguments[i] = expr;
                    struct Expr *callexp = calloc(1, sizeof(struct Expr));
                    callexp->name = name;
                    callexp->expr_kind = enum4('C', 'A', 'L', 'L');
                    callexp->func_args = arguments;
                    callexp->func_arg_len = i + 1;
                    return callexp;
                }
                consume_otherwise_panic(',');
                arguments[i] = expr;
            }
            exit(1);
        } else {
            struct Expr *numberexp = calloc(1, sizeof(struct Expr));
            numberexp->name = name;
            numberexp->expr_kind = enum4('I', 'D', 'N', 'T');
            return numberexp;
        }
    }

    consume_otherwise_panic('(');
    struct Expr *expr = parseExpr();
    consume_otherwise_panic(')');
    return expr;
}

struct Expr *parseUnary() {
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

struct Expr *parseMultiplicative() {
    panic_if_eof();
    struct Expr *result = parseUnary();
    while (tokens < tokens_end) {
        if (tokens->kind == enum3('N', 'U', 'M')) {
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

struct Expr *parseAdditive() {
    panic_if_eof();
    struct Expr *result = parseMultiplicative();
    while (tokens < tokens_end) {
        if (tokens->kind == enum3('N', 'U', 'M')) {
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

struct Expr *parseRelational() {
    panic_if_eof();
    struct Expr *result = parseAdditive();
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

struct Expr *parseEquality() {
    panic_if_eof();
    struct Expr *result = parseRelational();
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

struct Expr *parseAssign() {
    panic_if_eof();
    struct Expr *result = parseEquality();
    if (maybe_consume('=')) {
        return binaryExpr(result, parseAssign(), '=');
    }
    return result;
}

struct Expr *parseExpr() {
    return parseAssign();
}

char **lvar_names_start;
char **lvar_names;

struct Expr *parseOptionalExprAndToken(int target) {
    if (maybe_consume(target)) {
        return 0;
    }
    struct Expr *expr = parseExpr();
    consume_otherwise_panic(target);
    return expr;
}

struct Type *consume_type_otherwise_panic() {
    consume_otherwise_panic(enum3('i', 'n', 't'));

    struct Type *t = calloc(1, sizeof(struct Type));
    t->ty_kind = enum3('i', 'n', 't');

    while (maybe_consume('*')) {
        struct Type *new_t = calloc(1, sizeof(struct Type));
        new_t->ty_kind = '*';
        new_t->ptr_to = t;
        t = new_t;
    }
    return t;
}

struct Stmt *parseStmt() {
    if (maybe_consume('{')) {
        struct Stmt *result = calloc(1, sizeof(struct Stmt));
        result->stmt_kind = enum4('e', 'x', 'p', 'r');
        result->expr = numberexpr(42);
        while (tokens->kind != '}') {
            struct Stmt *newstmt = calloc(1, sizeof(struct Stmt));
            newstmt->first_child = result;
            newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
            newstmt->second_child = parseStmt();
            result = newstmt;
        }
        tokens++;
        return result;
    }
    if (maybe_consume(enum3('R', 'E', 'T'))) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        stmt->stmt_kind = enum3('R', 'E', 'T');
        stmt->expr = parseExpr();
        consume_otherwise_panic(';');
        return stmt;
    }
    if (maybe_consume(enum2('i', 'f'))) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
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
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        consume_otherwise_panic('(');
        stmt->expr = parseExpr();
        consume_otherwise_panic(')');
        stmt->stmt_kind = enum4('W', 'H', 'I', 'L');
        struct Stmt *statement = parseStmt();
        stmt->second_child = statement;
        return stmt;
    }
    if (maybe_consume(enum3('f', 'o', 'r'))) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
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
        struct Type *t = consume_type_otherwise_panic();
        if (lvar_names == lvar_names_start + 100) {
            exit(1);
        }
        char *name = expect_identifier_and_get_name();
        *lvar_names = name;
        lvar_names++;
        consume_otherwise_panic(';');
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
        stmt->expr = numberexpr(42);
        return stmt;
    }
    struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
    stmt->stmt_kind = enum4('e', 'x', 'p', 'r');
    stmt->expr = parseExpr();
    consume_otherwise_panic(';');
    return stmt;
}

struct Stmt *parseFunctionContent() {
    consume_otherwise_panic('{');
    struct Stmt *result = calloc(1, sizeof(struct Stmt));
    result->stmt_kind = enum4('e', 'x', 'p', 'r');
    result->expr = numberexpr(1);
    while (tokens->kind != '}') {
        struct Stmt *statement = parseStmt();
        struct Stmt *newstmt = calloc(1, sizeof(struct Stmt));
        newstmt->first_child = result;
        newstmt->stmt_kind = enum4('n', 'e', 'x', 't');
        newstmt->second_child = statement;
        result = newstmt;
    }
    tokens++;
    return result;
}

struct FuncDef *parseFunction() {
    struct Type *t = consume_type_otherwise_panic();
    char *name = expect_identifier_and_get_name();
    char **params = calloc(6, sizeof(char *));
    consume_otherwise_panic('(');
    if (maybe_consume(')')) {
        lvar_names = lvar_names_start = calloc(100, sizeof(char *));
        struct Stmt *content = parseFunctionContent();
        struct FuncDef *funcdef = calloc(1, sizeof(struct FuncDef));
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
        struct Type *t = consume_type_otherwise_panic();
        char *inner_name = expect_identifier_and_get_name();
        if (maybe_consume(')')) {
            params[i] = inner_name;
            lvar_names = lvar_names_start = calloc(100, sizeof(char *));
            struct Stmt *content = parseFunctionContent();
            struct FuncDef *funcdef = calloc(1, sizeof(struct FuncDef));
            funcdef->content = content;
            funcdef->name = name;
            funcdef->param_len = i + 1;
            funcdef->params = params;
            funcdef->lvar_names_start = lvar_names_start;
            funcdef->lvar_names_end = lvar_names;
            return funcdef;
        }
        consume_otherwise_panic(',');
        params[i] = inner_name;
    }

    exit(1);
}

void parseProgram() {
    int i = 0;
    while (tokens < tokens_end) {
        all_funcdefs[i] = parseFunction();
        i++;
    }
}

/*** ^ PARSE | v CODEGEN ***/

int labelCounter;

struct LVar *locals;

struct LVar *findLVar(char *name) {
    struct LVar *local = locals;
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

struct LVar *insertLVar(char *name) {
    struct LVar *newlocal = calloc(1, sizeof(struct LVar));
    struct LVar *last = lastLVar();
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

struct LVar *lastLVar() {
    struct LVar *local = locals;
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

void EvaluateLValueAddressIntoRax(struct Expr *expr) {
    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        if (!findLVar(expr->name)) {
            exit(1);
        }
        struct LVar *local = findLVar(expr->name);
        printf("  mov rax, rbp\\n");
        printf("  sub rax, %d\\n", local->offset_from_rbp);
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y') && expr->op_kind == '*') {
        EvaluateExprIntoRax(expr->first_child);
    } else {
        exit(1);
    }
}

void CodegenStmt(struct Stmt *stmt) {
    if (stmt->stmt_kind == enum4('e', 'x', 'p', 'r')) {
        EvaluateExprIntoRax(stmt->expr);
    } else if (stmt->stmt_kind == enum4('n', 'e', 'x', 't')) {
        CodegenStmt(stmt->first_child);
        CodegenStmt(stmt->second_child);
    } else if (stmt->stmt_kind == enum3('R', 'E', 'T')) {
        EvaluateExprIntoRax(stmt->expr);
        printf("  mov rsp, rbp\\n");
        printf("  pop rbp\\n");
        printf("  ret\\n");
    } else if (stmt->stmt_kind == enum2('i', 'f')) {
        int label = labelCounter++;

        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\\n");
        printf("  je  .Lelse%d\\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp .Lend%d\\n", label);
        printf(".Lelse%d:\\n", label);
        if (stmt->third_child != 0) {
            CodegenStmt(stmt->third_child);
        }
        printf(".Lend%d:\\n", label);
    } else if (stmt->stmt_kind == enum4('W', 'H', 'I', 'L')) {
        int label = labelCounter++;

        printf(".Lbegin%d:\\n", label);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\\n");
        printf("  je  .Lend%d\\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp  .Lbegin%d\\n", label);
        printf(".Lend%d:\\n", label);
    } else if (stmt->stmt_kind == enum3('f', 'o', 'r')) {
        int label = labelCounter++;

        if (stmt->expr) {
            EvaluateExprIntoRax(stmt->expr);
        }
        printf(".Lbegin%d:\\n", label);
        if (stmt->expr1) {
            EvaluateExprIntoRax(stmt->expr1);
        } else {
            printf("  mov rax, 1\\n");
        }
        printf("  cmp rax, 0\\n");
        printf("  je  .Lend%d\\n", label);
        CodegenStmt(stmt->second_child);
        if (stmt->expr2) {
            EvaluateExprIntoRax(stmt->expr2);
        }
        printf("  jmp  .Lbegin%d\\n", label);
        printf(".Lend%d:\\n", label);
    }
}

const char *nth_arg_reg(int n) {
    if (n == 0) {
        return "rdi";
    } else if (n == 1) {
        return "rsi";
    } else if (n == 2) {
        return "rdx";
    } else if (n == 3) {
        return "rcx";
    } else if (n == 4) {
        return "r8";
    } else if (n == 5) {
        return "r9";
    } else {
        printf("!!!!!!!!!error: incorrect n: %d\\n", n);
        exit(1);
    }
}

void CodegenFunc(struct FuncDef *funcdef) {
    printf(".globl %s\\n", funcdef->name);
    printf("%s:\\n", funcdef->name);
    printf("  push rbp\\n");
    printf("  mov rbp, rsp\\n");
    printf("  sub rsp, 208\\n");
    for (int i = 0; i < funcdef->param_len; i++) {
        char *param_name = funcdef->params[i];
        insertLVar(param_name);
        struct LVar *local = findLVar(param_name);
        printf("  mov rax, rbp\\n");
        printf("  sub rax, %d\\n", local->offset_from_rbp);
        printf("  mov [rax], %s\\n", nth_arg_reg(i));
    }
    for (char **names = funcdef->lvar_names_start; names != funcdef->lvar_names_end; names++) {
        insertLVar(*names);
    }
    CodegenStmt(funcdef->content);
}

void EvaluateExprIntoRax(struct Expr *expr) {
    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        printf("#1\\n");
        EvaluateLValueAddressIntoRax(expr);
        printf("  mov rax,[rax]\\n");
        return;
    } else if (expr->expr_kind == enum4('C', 'A', 'L', 'L')) {
        for (int i = 0; i < expr->func_arg_len; i++) {
            printf("#2\\n");
            EvaluateExprIntoRax(expr->func_args[i]);
            printf("    push rax\\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--) {
            printf("#3\\n");
            printf("    pop %s\\n", nth_arg_reg(i));
        }
        printf("#4\\n");
        printf(" call %s\\n", expr->name);
        return;
    } else if (expr->expr_kind == enum3('N', 'U', 'M')) {
        printf("#5\\n");
        printf("  mov rax, %d\\n", expr->value);
        return;
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y')) {
        if (expr->op_kind == '*') {
            printf("#6\\n");
            EvaluateExprIntoRax(expr->first_child);
            printf("  mov rax, [rax]\\n");
        } else if (expr->op_kind == '&') {
            printf("#7\\n");
            EvaluateLValueAddressIntoRax(expr->first_child);
        } else {
            exit(1);
        }
    } else if (expr->expr_kind == enum4('2', 'A', 'R', 'Y')) {
        if (expr->op_kind == '=') {
            printf("#8\\n");
            EvaluateLValueAddressIntoRax(expr->first_child);
            printf("    push rax\\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\\n");
            printf("    pop rdi\\n");
            printf("    pop rax\\n");
            printf("    mov [rax], rdi\\n");
        } else {
            printf("#9\\n");
            EvaluateExprIntoRax(expr->first_child);
            printf("    push rax\\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\\n");
            printf("    pop rdi\\n");
            printf("    pop rax\\n");

            if (expr->op_kind == '+') {
                printf("    add rax,rdi\\n");
            } else if (expr->op_kind == '-') {
                printf("    sub rax,rdi\\n");

            } else if (expr->op_kind == '*') {
                printf("    imul rax,rdi\\n");
            } else if (expr->op_kind == '/') {
                printf("  cqo\\n");
                printf("  idiv rdi\\n");
            } else if (expr->op_kind == enum2('=', '=')) {
                printf("  cmp rax, rdi\\n");
                printf("  sete al\\n");
                printf("  movzb rax, al\\n");
            } else if (expr->op_kind == enum2('!', '=')) {
                printf("  cmp rax, rdi\\n");
                printf("  setne al\\n");
                printf("  movzb rax, al\\n");
            } else if (expr->op_kind == '>') {
                printf("  cmp rax, rdi\\n");
                printf("  setg al\\n");
                printf("  movzb rax, al\\n");
            } else if (expr->op_kind == enum2('>', '=')) {
                printf("  cmp rax, rdi\\n");
                printf("  setge al\\n");
                printf("  movzb rax, al\\n");
            } else {
                exit(1);
            }
        }
    } else {
        exit(1);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }
    char *p = argv[1];
    int tokens_length = tokenize(p);
    if (tokens_length == 0) {
        return 1;
    }
    tokens = all_tokens;
    tokens_end = all_tokens + tokens_length;
    parseProgram();
    printf(".intel_syntax noprefix\\n");
    for (int i = 0; all_funcdefs[i]; i++) {
        struct FuncDef *funcdef = all_funcdefs[i];
        CodegenFunc(funcdef);
    }
    return 0;
}
"""

assert  check_stepN_test_case(18, step18, "int main() { return 0; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 42; }", 42)
assert  check_stepN_test_case(18, step18, "int main() { return 0+10+3; }", 0+10+3)
assert  check_stepN_test_case(18, step18, "int main() { return 111+10-42; }", 111+10-42)
assert  check_stepN_test_case(18, step18, "int main() { return    111   + 10 -     42; }", 111+10-42)
assert  check_stepN_test_case(18, step18, "int main() { return    0 +    10+    3; }",  0 +    10+    3)
assert  check_stepN_test_case(18, step18, "int main() { return 10*2; }", 10*2)
assert  check_stepN_test_case(18, step18, "int main() { return 10+1*2; }", 10+1*2)
assert  check_stepN_test_case(18, step18, "int main() { return 10+3*2+10-5; }", 10+3*2+10-5)
assert  check_stepN_test_case(18, step18, "int main() { return (10+3)*2+10-5; }", (10+3)*2+10-5)
assert  check_stepN_test_case(18, step18, "int main() { return (10+1)*2; }", (10+1)*2)
assert  check_stepN_test_case(18, step18, "int main() { return (10+1)/2; }", (10+1)//2)
assert  check_stepN_test_case(18, step18, "int main() { return (15+1)/2+3; }", (15+1)//2+3)
assert  check_stepN_test_case(18, step18, "int main() { return 10+1 /2/5; }", 10+1//2//5)

#unary
assert  check_stepN_test_case(18, step18, "int main() { return -10+1 /2/5+30; }", -10+1//2//5+30)
assert  check_stepN_test_case(18, step18, "int main() { return +10+1 /2/5; }", +10+1//2//5)
assert  check_stepN_test_case(18, step18, "int main() { return -2*-3; }", -2*-3)

#equality
assert  check_stepN_test_case(18, step18, "int main() { return 1==0; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 1==1; }", 1)
assert  check_stepN_test_case(18, step18, "int main() { return 1==1+5; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 1+(1+1==1+1); }",2)
assert  check_stepN_test_case(18, step18, "int main() { return 1!=0; }", 1)
assert  check_stepN_test_case(18, step18, "int main() { return 1!=1; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 1!=1+5; }", 1)
assert  check_stepN_test_case(18, step18, "int main() { return 1+(1+1!=1+1); }",1)


#relational
assert  check_stepN_test_case(18, step18, "int main() { return 1>0; }", 1)
assert  check_stepN_test_case(18, step18, "int main() { return 1>1; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 1<0; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 1<1; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 1>=0; }", 1)
assert  check_stepN_test_case(18, step18, "int main() { return 1>=1; }", 1)
assert  check_stepN_test_case(18, step18, "int main() { return 1<=0; }", 0)
assert  check_stepN_test_case(18, step18, "int main() { return 1<=1; }", 1)


#semicolon
assert  check_stepN_test_case(18, step18, "int main() { 1+1;return 5-2; }",3)

#variables
assert  check_stepN_test_case(18, step18, "int main() { int a; a=3;return a; }",3)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; a=3;b=4;return a+b; }",7)
assert  check_stepN_test_case(18, step18, "int main() { int ab; int bd; ab=3;bd=4;return ab+bd; }",7)
assert  check_stepN_test_case(18, step18, "int main() { int abz; int bdz; abz=3;bdz =4;return abz+bdz; }",7)
assert  check_stepN_test_case(18, step18, "int main() { return 1;return 2; }",1)
assert  check_stepN_test_case(18, step18, "int main() { return 1;return 2+3; }",1)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;if(1)a=1;return a; }",1)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;if(0)a=1;return a; }",0)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=1;if(a)a=5;return a; }",5)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;if(a)a=5;return a; }",0)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=1;if(a)return 5;return 10; }",5)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;if(a)return 5;return 10; }",10)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;if(a)return 5;a=1;if(a)return 3;return 10; }",3)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;while(a)return 1; return 3; }",3)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;while(a<5)a=a+1; return a; }",5)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=0;if(a)return 5;else a=10;return a; }",10)
assert  check_stepN_test_case(18, step18, "int main() { int a; a=1;if(a)a=0;else return 10;return a; }",0)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; for(a=0;a<10;a=a+1)b=a;return b; }",9)
assert  check_stepN_test_case(18, step18, "int main() { for(;;)return 0; }",0)

#block
assert  check_stepN_test_case(18, step18, "int main() { { { { return 3; } } } }", 3)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 3; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 3)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 0; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 12)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { } return c; }", 3)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } return c; }", 3)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } else { c = 7; } return c; }", 7)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; } else { c = 7; }} return c; }", 3)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) if (b) { c = 2; } else { c = 7; } return c; }", 3)
assert  check_stepN_test_case(18, step18, "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; }} else { c = 7; } return c; }", 7)
assert  check_stepN_test_case(18, step18, "int three() { return 3; } int main() { return three(); }", 3)
assert  check_stepN_test_case(18, step18, "int one() { return 1; } int three() { return one() + 2; } int main() { return three() + three(); }", 6)
assert  check_stepN_test_case(18, step18, "int identity(int a) { return a; } int main() { return identity(3); }", 3)
assert  check_stepN_test_case(18, step18, "int add2(int a, int b) { return a + b; } int main() { return add2(1, 2); }", 3)
assert  check_stepN_test_case(18, step18, "int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; } int main() { return add6(1, 2, 3, 4, 5, 6); }", 21)
assert  check_stepN_test_case(18, step18, "int fib(int n) { if (n <= 1) { return n; } return fib(n-1) + fib(n-2); } int main() { return fib(8); }", 21)
assert  check_stepN_test_case(18, step18, "int main() { int x; int y; x = 3; y = &x; return *y; }", 3)
assert  check_stepN_test_case(18, step18, "int main() { int x; int *y; y = &x; *y = 3; return x; }", 3)

###################################################################################################
print(f"{bcolors.OKBLUE}ステップ42{bcolors.ENDC}")
print(f"{bcolors.OKBLUE}Step 42{bcolors.ENDC}")
###################################################################################################
step42 = """
int printf();
void exit();
void *calloc();
int strcmp();
int strncmp();
char *strchr();
void *memcpy();
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

void panic(const char *msg) {
    exit(1);
}

/*** ^ LIB | v PARSE ***/
struct Token tokens_start[1000];
struct Token *tokens_end;
struct Token *tokens_cursor;
char *string_literals_start[10000];
char **string_literals_cursor;
struct StructMember *struct_members_start[10000];
struct StructMember **struct_members_cursor;
struct StructSizeAndAlign *struct_sizes_and_alignments_start[100];
struct StructSizeAndAlign **struct_sizes_and_alignments_cursor;

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
        if (is_reserved_then_handle(str + i, &i, "return", 6, enum3('R', 'E', 'T'))) {
        } else if (is_reserved_then_handle(str + i, &i, "sizeof", 6, enum4('S', 'Z', 'O', 'F'))) {
        } else if (is_reserved_then_handle(str + i, &i, "struct", 6, enum4('S', 'T', 'R', 'U'))) {
        } else if (is_reserved_then_handle(str + i, &i, "if", 2, enum2('i', 'f'))) {
        } else if (is_reserved_then_handle(str + i, &i, "while", 5, enum4('W', 'H', 'I', 'L'))) {
        } else if (is_reserved_then_handle(str + i, &i, "const", 5, enum4('C', 'N', 'S', 'T'))) {
        } else if (is_reserved_then_handle(str + i, &i, "else", 4, enum4('e', 'l', 's', 'e'))) {
        } else if (is_reserved_then_handle(str + i, &i, "for", 3, enum3('f', 'o', 'r'))) {
        } else if (is_reserved_then_handle(str + i, &i, "int", 3, enum3('i', 'n', 't'))) {
        } else if (is_reserved_then_handle(str + i, &i, "char", 4, enum4('c', 'h', 'a', 'r'))) {
        } else if (strncmp(str + i, "//", 2) == 0) {
            i += 2;
            while (str[i] && str[i] != '\\n')
                i += 1;
        } else if (strncmp(str + i, "/*", 2) == 0) {
            char *q = strstr(str + i + 2, "*/");
            if (!q)
                panic("unclosed block comment\\n");
            i = q + 2 - str;
        } else if (strncmp(str + i, "->", 2) == 0) {
            (tokens_cursor++)->kind = enum2('-', '>');
            i += 2;
        } else if (c == '\\'') {
            if (str[i + 1] != '\\\\') {
                tokens_cursor->kind = enum3('N', 'U', 'M');
                (tokens_cursor++)->value_or_string_size = str[i + 1];
                i += 3;
            } else if (strchr("\\\\'\\"?", str[i + 2])) {
                tokens_cursor->kind = enum3('N', 'U', 'M');
                (tokens_cursor++)->value_or_string_size = str[i + 2];
                i += 4;
            } else if (str[i + 2] == 'n') {
                tokens_cursor->kind = enum3('N', 'U', 'M');
                (tokens_cursor++)->value_or_string_size = '\\n';
                i += 4;
            } else {
                exit(1);
            }
        } else if (c == '"') {
            int parsed_length = 0;
            int true_semantic_length = 0;
            for (i += 1; str[i + parsed_length] != '"'; parsed_length++) {
                true_semantic_length++;
                if (!str[i + parsed_length])
                    panic("unterminated string literal");
                if (str[i + parsed_length] == '\\\\') {
                    if (strchr("01234567x", str[i + parsed_length + 1]))
                        panic("Not supported: Unsupported escape sequence within a string literal\\n");
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
        } else if (strchr(" \\n", c)) {
            i += 1;
        } else {
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

struct Type *deref(struct Type *t) {
    if (t->kind == '*')
        return t->ptr_to;
    exit(1);
}

void display_type(struct Type *t);

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
    display_type(t);
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

char *decode_kind(int kind) {
    void *r = &kind;
    char *q = r;
    char *ans = calloc(5, sizeof(char));
    strncpy(ans, q, 4);
    return ans;
}

void consume_otherwise_panic(int kind) {
    if (!maybe_consume(kind)) {
        exit(1);
    }
}

void expect_otherwise_panic(int kind) {
    if (tokens_cursor->kind != kind) {
        exit(1);
    }
}

void panic_if_eof() {
    if (tokens_cursor >= tokens_end) {
        exit(1);
    }
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
    exit(1);
}

struct Type *lookup_func_type(char *name) {
    for (int i = 0; funcdecls_start[i]; i++)
        if (strcmp(funcdecls_start[i]->name, name) == 0)
            return funcdecls_start[i]->type;
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
            int i = 0;
            while (1) {
                arguments[i] = decay_if_arr(parseExpr());
                if (maybe_consume(')'))
                    return callingExpr(name, arguments, i + 1);
                consume_otherwise_panic(',');
                i++;
                if (i >= 6)
                    panic("not supported: more than arguments\\n");
            }
        }
        return identExpr(name);
    }
    consume_otherwise_panic('(');
    struct Expr *expr = parseExpr();  // NO DECAY
    consume_otherwise_panic(')');
    return expr;
}

int is_int_or_char(int kind) {
    return (kind == enum3('i', 'n', 't')) + (kind == enum4('c', 'h', 'a', 'r'));
}

int starts_a_type(int kind) {
    return is_int_or_char(kind) + (kind == enum4('S', 'T', 'R', 'U')) + (kind == enum4('C', 'N', 'S', 'T'));
}

int is_integer(struct Type *typ) {
    return is_int_or_char(typ->kind);
}

struct Expr *assert_integer(struct Expr *e) {
    if (is_integer(e->typ)) {
        return e;
    }
    exit(1);
}

void display_type(struct Type *t) {
    if (t->kind == enum2('[', ']')) {
        display_type(t->ptr_to);
    } else if (t->kind == '*') {
        display_type(t->ptr_to);
    } else if (t->kind == enum4('S', 'T', 'R', 'U')) {
    } else {

    }
}

int is_same_type(struct Type *t1, struct Type *t2) {
    if (t1->kind == '*' && t2->kind == '*') {
        return is_same_type(t1->ptr_to, t2->ptr_to);
    }
    return t1->kind == t2->kind;
}

void panic_two_types(const char *msg, struct Type *t1, struct Type *t2) {
    exit(1);
}

int is_compatible_type(struct Type *t1, struct Type *t2) {
    if (t1->kind == '*' && t2->kind == '*') {
        return is_same_type(t1->ptr_to, t2->ptr_to);
    }
    return !(t1->kind != t2->kind && !(is_integer(t1) && is_integer(t2)));
}

struct Expr *expr_add(struct Expr *lhs, struct Expr *rhs) {
    if (is_integer(lhs->typ)) {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, rhs, '+', type(enum3('i', 'n', 't')));
        else if (rhs->typ->kind == '*')
            return expr_add(rhs, lhs);
        else
            panic("unknown type encountered in addition\\n");
    } else if (lhs->typ->kind == '*') {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, binaryExpr(numberExpr(size(deref(lhs->typ))), rhs, '*', type(enum3('i', 'n', 't'))), '+', lhs->typ);
        else
            panic("cannot add\\n");
    }
    exit(1);
}

struct Expr *expr_subtract(struct Expr *lhs, struct Expr *rhs) {
    if (is_integer(lhs->typ)) {
        if (is_integer(rhs->typ))
            return binaryExpr(lhs, rhs, '-', type(enum3('i', 'n', 't')));
        else if (rhs->typ->kind == '*')
            panic("cannot subtract a pointer from an integer\\n");
    } else if (lhs->typ->kind == '*') {
        if (is_integer(rhs->typ)) {
            return binaryExpr(lhs, binaryExpr(numberExpr(size(deref(lhs->typ))), rhs, '*', type(enum3('i', 'n', 't'))), '-', lhs->typ);
        } else if (rhs->typ->kind == '*') {
            if (!is_same_type(lhs->typ, rhs->typ))
                panic_two_types("cannot subtract two expressions with different pointer types", lhs->typ, rhs->typ);
            return binaryExpr(binaryExpr(lhs, rhs, '-', type(enum3('i', 'n', 't'))), numberExpr(size(deref(lhs->typ))), '/', type(enum3('i', 'n', 't')));
        } else
            panic("cannot subtract: invalid type in the second operand\\n");
    }
    exit(1);
}

int offset_of(struct Type *t, char *member_name) {
    if (t->kind != enum4('S', 'T', 'R', 'U'))
        panic("tried to make a member access to a non-struct type\\n");
    for (int i = 0; struct_members_start[i]; i++)
        if (strcmp(struct_members_start[i]->struct_name, t->struct_name) == 0)
            if (strcmp(struct_members_start[i]->member_name, member_name) == 0)
                return struct_members_start[i]->member_offset;
    exit(1);
}

struct Expr *arrowExpr(struct Expr *lhs, char *member_name) {
    struct Type *struct_type = deref(lhs->typ);
    if (struct_type->kind != enum4('S', 'T', 'R', 'U'))
        panic("tried to access a member of a non-struct type");
    int offset;
    struct Type *member_type;
    for (int i = 0; struct_members_start[i]; i++)
        if (strcmp(struct_members_start[i]->struct_name, struct_type->struct_name) == 0)
            if (strcmp(struct_members_start[i]->member_name, member_name) == 0) {
                offset = struct_members_start[i]->member_offset;
                member_type = struct_members_start[i]->member_type;
            }
    struct Expr *expr = binaryExpr(lhs, numberExpr(offset), '+', ptr_of(member_type));
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

struct Expr *parseUnary();
struct Expr *parseCast() {
    return parseUnary();
}

struct Type *consume_simple_type();
struct Expr *equalityExpr(struct Expr *lhs, struct Expr *rhs, int kind);

struct Expr *parseUnary() {
    panic_if_eof();
    if (maybe_consume('+')) {
        return assert_integer(parseCast());
    } else if (maybe_consume('-')) {
        return binaryExpr(numberExpr(0), assert_integer(parseCast()), '-', type(enum3('i', 'n', 't')));
    } else if (maybe_consume('!')) {
        return equalityExpr(numberExpr(0), parseCast(), enum2('=', '='));  // The expression !E is equivalent to (0==E)
    } else if (maybe_consume('*')) {
        struct Expr *expr = decay_if_arr(parseCast());
        return unaryExpr(expr, '*', deref(expr->typ));
    } else if (maybe_consume('&')) {
        struct Expr *expr = parseCast();                 // NO DECAY
        return unaryExpr(expr, '&', ptr_of(expr->typ));  // NO DECAY
    } else if (maybe_consume(enum4('S', 'Z', 'O', 'F'))) {
        if (tokens_cursor->kind == '(') {
            if (starts_a_type((tokens_cursor + 1)->kind)) {
                tokens_cursor++;
                struct Type *typ = consume_simple_type();
                consume_otherwise_panic(')');
                return numberExpr(size(typ));
            } else {
                struct Expr *expr = parseUnary();  // NO DECAY
                return numberExpr(size(expr->typ));
            }
        } else {
            struct Expr *expr = parseUnary();  // NO DECAY
            return numberExpr(size(expr->typ));
        }
    }
    return parsePostfix();
}

void assert_compatible_in_equality(struct Expr *e1, struct Expr *e2) {
    if (is_compatible_type(e1->typ, e2->typ))
        return;
    if (e1->expr_kind == '0' && e2->typ->kind == '*')  // one operand is a pointer and the other is a null pointer constant
        return;
    if (e2->expr_kind == '0' && e1->typ->kind == '*')  // one operand is a pointer and the other is a null pointer constant
        return;
    panic_two_types("cannot compare (un)equal two operands with incompatible types", e1->typ, e2->typ);
}

struct Expr *equalityExpr(struct Expr *lhs, struct Expr *rhs, int kind) {
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

struct Expr *parseLeftToRightInfix(int level) {
    panic_if_eof();
    struct Expr *expr = parseUnary();
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

void assert_compatible_in_simple_assignment(struct Type *lhs_type, struct Expr *rhs) {
    if (is_compatible_type(lhs_type, rhs->typ))
        return;
    if (lhs_type->kind == '*' && rhs->expr_kind == '0')  // the left operand is an atomic, qualified, or unqualified pointer, and the right is a null pointer constant
        return;
    panic_two_types("cannot assign/initialize because two incompatible types are detected", lhs_type, rhs->typ);
}

struct Expr *parseAssign() {
    panic_if_eof();
    struct Expr *result = parseLeftToRightInfix(1);
    if (maybe_consume('=')) {
        struct Expr *rhs = decay_if_arr(parseAssign());
        assert_compatible_in_simple_assignment(result->typ, rhs);  // no decay, since we cannot assign to an array
        return binaryExpr(result, rhs, '=', result->typ);
    } else if (maybe_consume(enum2('+', '='))) {
        result = expr_add(decay_if_arr(result), assert_integer(parseAssign()));
        result->op_kind = enum2('+', '=');
    } else if (maybe_consume(enum2('-', '='))) {
        result = expr_subtract(decay_if_arr(result), assert_integer(parseAssign()));
        result->op_kind = enum2('-', '=');
    } else if (maybe_consume(enum2('*', '=')))
        result = binaryExpr(assert_integer(result), assert_integer(parseUnary()), enum2('*', '='), type(enum3('i', 'n', 't')));
    else if (maybe_consume(enum2('/', '=')))
        result = binaryExpr(assert_integer(result), assert_integer(parseUnary()), enum2('/', '='), type(enum3('i', 'n', 't')));
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
        return consume_simple_type(); // ignore const for now
    else if (maybe_consume(enum3('i', 'n', 't')))
        type->kind = enum3('i', 'n', 't');
    else if (maybe_consume(enum4('c', 'h', 'a', 'r')))
        type->kind = enum4('c', 'h', 'a', 'r');
    else if (maybe_consume(enum4('S', 'T', 'R', 'U'))) {
        type->kind = enum4('S', 'T', 'R', 'U');
        expect_otherwise_panic(enum4('I', 'D', 'N', 'T'));
        char *name = (tokens_cursor++)->identifier_name_or_escaped_string_content;
        type->struct_name = name;
    } else {
        exit(1);
    }
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
    if (maybe_consume('{')) {
        panic("not supported: initializer list\\n");
    }
    struct Expr *rhs = parseExpr();
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
        stmt->expr = decay_if_arr(parseExpr());
        consume_otherwise_panic(';');
        return stmt;
    }
    if (maybe_consume(enum2('i', 'f'))) {
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
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
        struct Stmt *stmt = calloc(1, sizeof(struct Stmt));
        consume_otherwise_panic('(');
        stmt->expr = decay_if_arr(parseExpr());
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
            for_stmt->for_cond = parseOptionalExprAndToken(';');
            for_stmt->for_after = parseOptionalExprAndToken(')');
            for_stmt->second_child = parseStmt();
            struct Stmt *combined_stmt = calloc(1, sizeof(struct Stmt));
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
    return result;
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
        consume_otherwise_panic('{');
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
    }
    struct NameAndType *first_half = consume_type_and_ident_1st_half();
    if (maybe_consume('(')) {
        struct NameAndType *rettype_and_funcname = first_half;
        struct NameAndType *params_start = calloc(6, sizeof(struct NameAndType));
        if (maybe_consume(')')) {
            lvars_cursor = lvars_start = calloc(100, sizeof(struct NameAndType));
            store_func_decl(rettype_and_funcname);
            if (maybe_consume(';'))
                return;
            *(funcdefs_cursor++) = constructFuncDef(parseFunctionContent(), rettype_and_funcname, 0, params_start);
            return;
        }
        lvars_cursor = lvars_start = calloc(100, sizeof(char *));
        int i = 0;
        while (1) {
            struct NameAndType *param = consume_type_and_ident();
            if (maybe_consume(')')) {
                params_start[i].name = param->name;
                params_start[i].type = param->type;
                lvars_cursor->name = param->name;
                (lvars_cursor++)->type = param->type;
                store_func_decl(rettype_and_funcname);
                if (maybe_consume(';'))
                    return;
                *(funcdefs_cursor++) = constructFuncDef(parseFunctionContent(), rettype_and_funcname, i + 1, params_start);
                return;
            }
            consume_otherwise_panic(',');
            params_start[i].name = param->name;
            params_start[i].type = param->type;
            lvars_cursor->name = param->name;
            (lvars_cursor++)->type = param->type;
            i++;
            if (i >= 6)
                panic("not supported: more than 6 parameters\\n");
        }
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

struct LVar *lastLVar() {
    struct LVar *local = locals;
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

struct LVar *insertLVar(char *name, int sz) {
    sz = roundup(sz, 8);
    struct LVar *newlocal = calloc(1, sizeof(struct LVar));
    struct LVar *last = lastLVar();
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

void EvaluateExprIntoRax(struct Expr *expr);

void EvaluateLValueAddressIntoRax(struct Expr *expr) {
    if (expr->expr_kind == enum4('I', 'D', 'N', 'T')) {
        struct LVar *local = findLVar(expr->func_or_ident_name_or_string_content);
        if (local) {
            printf("  lea rax, [rbp - %d]\\n", local->offset_from_rbp);
        } else if (isGVar(expr->func_or_ident_name_or_string_content)) {
            printf("  mov eax, OFFSET FLAT:%s\\n", expr->func_or_ident_name_or_string_content);
        } else {
            exit(1);
        }
    } else if (expr->expr_kind == enum3('S', 'T', 'R')) {
        printf("  mov eax, OFFSET FLAT:.LC%d\\n", find_strlit(expr->func_or_ident_name_or_string_content));
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y') && expr->op_kind == '*') {
        EvaluateExprIntoRax(expr->first_child);
    } else
        panic("not lvalue");
}

void CodegenStmt(struct Stmt *stmt) {
    if (stmt->stmt_kind == enum4('e', 'x', 'p', 'r')) {
        EvaluateExprIntoRax(stmt->expr);
    } else if (stmt->stmt_kind == enum4('n', 'e', 'x', 't')) {
        CodegenStmt(stmt->first_child);
        CodegenStmt(stmt->second_child);
    } else if (stmt->stmt_kind == enum3('R', 'E', 'T')) {
        EvaluateExprIntoRax(stmt->expr);
        printf("  mov rsp, rbp\\n");
        printf("  pop rbp\\n");
        printf("  ret\\n");
    } else if (stmt->stmt_kind == enum2('i', 'f')) {
        int label = (labelCounter++);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\\n");
        printf("  je  .Lelse%d\\n", label);
        CodegenStmt(stmt->first_child);
        printf("  jmp .Lend%d\\n", label);
        printf(".Lelse%d:\\n", label);
        if (stmt->second_child != 0)
            CodegenStmt(stmt->second_child);
        printf(".Lend%d:\\n", label);
    } else if (stmt->stmt_kind == enum4('W', 'H', 'I', 'L')) {
        int label = (labelCounter++);
        printf(".Lbegin%d:\\n", label);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\\n");
        printf("  je  .Lend%d\\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp  .Lbegin%d\\n", label);
        printf(".Lend%d:\\n", label);
    } else if (stmt->stmt_kind == enum3('f', 'o', 'r')) {
        int label = (labelCounter++);
        if (stmt->expr)
            EvaluateExprIntoRax(stmt->expr);
        printf(".Lbegin%d:\\n", label);
        if (stmt->for_cond)
            EvaluateExprIntoRax(stmt->for_cond);
        else
            printf("  mov rax, 1\\n");
        printf("  cmp rax, 0\\n");
        printf("  je  .Lend%d\\n", label);
        CodegenStmt(stmt->second_child);
        if (stmt->for_after)
            EvaluateExprIntoRax(stmt->for_after);
        printf("  jmp  .Lbegin%d\\n", label);
        printf(".Lend%d:\\n", label);
    }
}

const char *nth_arg_reg(int n, int sz) {
    if (sz == 8)
        return "rdi\\0rsi\\0rdx\\0rcx\\0r8 \\0r9" + 4 * n;
    else if (sz == 4)
        return "edi\\0esi\\0edx\\0ecx\\0r8d\\0r9d" + 4 * n;
    exit(1);
}

void CodegenFunc(struct FuncDef *funcdef) {
    printf(".globl %s\\n", funcdef->name);
    printf("%s:\\n", funcdef->name);
    printf("  push rbp\\n");
    printf("  mov rbp, rsp\\n");
    int stack_adjust = 0;
    for (int i = 0; i < funcdef->param_len; i++)
        stack_adjust += 8;
    for (struct NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++)
        stack_adjust += roundup(size(ptr->type), 8);
    printf("  sub rsp, %d\\n", stack_adjust);
    for (int i = 0; i < funcdef->param_len; i++) {
        char *param_name = funcdef->params_start[i].name;
        insertLVar(param_name, 8);
        struct LVar *local = findLVar(param_name);
        printf("  mov [rbp - %d], %s\\n", local->offset_from_rbp, nth_arg_reg(i, 8));
    }
    for (struct NameAndType *ptr = funcdef->lvar_table_start; ptr != funcdef->lvar_table_end; ptr++)
        insertLVar(ptr->name, size(ptr->type));
    CodegenStmt(funcdef->content);
}

void deref_rax(int sz) {
    if (sz == 8)
        printf("  mov rax,[rax]\\n");
    else if (sz == 4)
        printf("  mov eax,[rax]\\n");
    else if (sz == 1) {
        printf("  movzx ecx, BYTE PTR [rax]\\n");
        printf("  mov eax, ecx\\n");
    } else {
        exit(1);
    }
}

void write_rax_to_where_rdi_points(int sz) {
    if (sz == 8)
        printf("    mov [rdi], rax\\n");
    else if (sz == 4)
        printf("    mov [rdi], eax\\n");
    else if (sz == 1) {
        printf("    mov ecx, eax\\n");
        printf("    mov [rdi], cl\\n");
    } else {
        exit(1);
    }
}

const char *AddSubMulDivAssign_rdi_into_rax(int kind) {
    if (kind == enum2('+', '=')) {
        return "    add rax,rdi\\n";
    } else if (kind == enum2('-', '=')) {
        return "    sub rax,rdi\\n";
    } else if (kind == enum2('*', '=')) {
        return "    imul rax,rdi\\n";
    } else if (kind == enum2('/', '=')) {
        return "  cqo\\n  idiv rdi\\n";
    }
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
            printf("    push rax\\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--)
            printf("    pop %s\\n", nth_arg_reg(i, 8));
        printf("  mov rax, 0\\n");
        printf(" call %s\\n", expr->func_or_ident_name_or_string_content);
    } else if (expr->expr_kind == enum3('N', 'U', 'M')) {
        printf("  mov rax, %d\\n", expr->value);
    } else if (expr->expr_kind == '0') {
        printf("  mov rax, 0\\n");
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y')) {
        if (expr->op_kind == '*') {
            EvaluateExprIntoRax(expr->first_child);
            printf("  mov rax, [rax]\\n");
        } else if (expr->op_kind == '&') {
            EvaluateLValueAddressIntoRax(expr->first_child);
        } else if (expr->op_kind == enum4('[', ']', '>', '*')) {
            EvaluateExprIntoRax(expr->first_child);
        } else {
            exit(1);
        }
    } else if (expr->expr_kind == enum4('2', 'A', 'R', 'Y')) {
        if (expr->op_kind == '=') {
            EvaluateLValueAddressIntoRax(expr->first_child);
            printf("    push rax\\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    pop rdi\\n");
            write_rax_to_where_rdi_points(size(expr->first_child->typ));  // second_child might be a 0 meaning a null pointer
        } else if (AddSubMulDivAssign_rdi_into_rax(expr->op_kind)) {      // x @= i
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\\n");                                      // stack: i
            EvaluateLValueAddressIntoRax(expr->first_child);               // rax: &x
            printf("    mov rsi, rax\\n");                                  // rsi: &x
            printf("    mov rax, [rax]\\n");                                // rsi: &x, rax: x
            printf("    pop rdi\\n");                                       // rsi: &x, rax: x, rdi: i
            printf("%s", AddSubMulDivAssign_rdi_into_rax(expr->op_kind));  // rsi: &x, rax: x@i
            printf("    mov rdi, rsi\\n");                                  // rdi: &x, rax: x@i
            write_rax_to_where_rdi_points(size(expr->second_child->typ));
        } else if (expr->op_kind == enum2('&', '&')) {
            int label = (labelCounter++);
            EvaluateExprIntoRax(expr->first_child);
            printf("    test rax, rax\\n");
            printf("    je .Landfalse%d\\n", label);
            EvaluateExprIntoRax(expr->second_child);
            printf("    test rax, rax\\n");
            printf("    je  .Landfalse%d\\n", label);
            printf("    mov eax, 1\\n");
            printf("    jmp .Landend%d\\n", label);
            printf(".Landfalse%d:\\n", label);
            printf("    mov     eax, 0\\n");
            printf(".Landend%d:\\n", label);
        } else {
            EvaluateExprIntoRax(expr->first_child);
            printf("    push rax\\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\\n");
            printf("    pop rdi\\n");
            printf("    pop rax\\n");
            if (AddSubMulDivAssign_rdi_into_rax(enum2(expr->op_kind, '='))) {
                printf("%s", AddSubMulDivAssign_rdi_into_rax(enum2(expr->op_kind, '=')));
            } else if (expr->op_kind == enum2('=', '=')) {
                printf("  cmp rax, rdi\\n");
                printf("  sete al\\n");
                printf("  movzb rax, al\\n");
            } else if (expr->op_kind == enum2('!', '=')) {
                printf("  cmp rax, rdi\\n");
                printf("  setne al\\n");
                printf("  movzb rax, al\\n");
            } else if (expr->op_kind == '>') {
                printf("  cmp rax, rdi\\n");
                printf("  setg al\\n");
                printf("  movzb rax, al\\n");
            } else if (expr->op_kind == enum2('>', '=')) {
                printf("  cmp rax, rdi\\n");
                printf("  setge al\\n");
                printf("  movzb rax, al\\n");
            } else {
                exit(1);
            }
        }
    } else {
        exit(1);
    }
}

/*** ^ CODEGEN | v MAIN ***/

int main(int argc, char **argv) {
    if (argc != 2)
        panic("incorrect cmd line arg\\n");
    string_literals_cursor = string_literals_start;
    tokens_cursor = tokens_start;  // the 1st tokens_cursor is for storing the tokens
    tokens_end = tokenize(argv[1]);
    if (tokens_start == tokens_end)
        panic("no token found\\n");
    tokens_cursor = tokens_start;  // the 2nd tokens_cursor is for parsing
    struct_members_cursor = struct_members_start;
    struct_sizes_and_alignments_cursor = struct_sizes_and_alignments_start;
    funcdecls_cursor = funcdecls_start;
    funcdefs_cursor = funcdefs_start;
    global_vars_cursor = global_vars_start;
    while (tokens_cursor < tokens_end)
        parseToplevel();
    printf(".intel_syntax noprefix\\n");
    printf("  .text\\n");
    printf("  .section .rodata\\n");
    for (int i = 0; string_literals_start[i]; i++) {
        printf(".LC%d:\\n", i);
        printf("  .string \\"%s\\"\\n", string_literals_start[i]);
    }
    printf("  .text\\n");
    for (int i = 0; global_vars_start[i]; i++) {
        printf(".globl %s\\n", global_vars_start[i]->name);
        printf(".data\\n");
        printf("%s:\\n", global_vars_start[i]->name);
        printf("  .zero %d\\n", size(global_vars_start[i]->type));
    }
    printf(".text\\n");
    for (int i = 0; funcdefs_start[i]; i++)
        CodegenFunc(funcdefs_start[i]);
    return 0;
}
"""
assert check_stepN_test_case(42, step42, "struct A { int a; int b; }; int main() { return 0; }", 0)
assert check_stepN_test_case(42, step42, "struct A { int a; int b; }; int main() { return sizeof(struct A); }", 8)
assert check_stepN_test_case(42, step42, "struct A { char a; int b; }; int main() { return sizeof(struct A); }", 8)
assert check_stepN_test_case(42, step42, "struct A { char a[5]; int b; }; int main() { return sizeof(struct A); }", 12)

assert check_stepN_test_case(42, step42, "int main() { return sizeof(struct A*); }", 8)
assert check_stepN_test_case(42, step42, "int main() { struct A *p; return 0; }", 0)

assert check_stepN_test_case(42, step42, "int main() { return 1+(2!=1+1); }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 5+(8+(7!=2)); }", 14)

assert check_stepN_test_case(42, step42, "int main() { return 8*7!=2; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 8+7!=2; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1+(1+1!=1+1); }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1+(1+1!=2); }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 5+(8+7!=2); }", 6)


assert check_stepN_test_case(42, step42, """
int printf();
int first() {
    printf("first, ");
    return 0;
}
int second() {
    printf("second, ");
    return 0;
}
int main() { return first() && second(); }
""", 0)

assert check_stepN_test_case(42, step42, """
int printf();
int first() {
    printf("first, ");
    return 2;
}
int second() {
    printf("second, ");
    return 0;
}
int main() { return first() && second(); }
""", 0)

assert check_stepN_test_case(42, step42, """
int printf();
int first() {
    printf("first, ");
    return 2;
}
int second() {
    printf("second, ");
    return 2;
}
int main() { return first() && second(); }
""", 1)

assert check_stepN_test_case(42, step42, r'int main() { return "\\"[0]; }', ord('\\'))
assert check_stepN_test_case(42, step42, r'int main() { return "\""[0]; }', ord('\"'))
assert check_stepN_test_case(42, step42, r'int main() { return "\'"[0]; }', ord('\''))
assert check_stepN_test_case(42, step42, r'int main() { return "\n"[0]; }', ord('\n'))
## According to https://docs.oracle.com/cd/E19120-01/open.solaris/817-5477/eoqka/index.html 
## \a is not supported in the assembly
# assert check_stepN_test_case(42, step42, r'int main() { return "\a"[0]; }', ord('\a')) 
assert check_stepN_test_case(42, step42, r'int main() { return "\b"[0]; }', ord('\b'))
assert check_stepN_test_case(42, step42, r'int main() { return "\t"[0]; }', ord('\t'))
assert check_stepN_test_case(42, step42, r'int main() { return "\f"[0]; }', ord('\f'))
assert check_stepN_test_case(42, step42, r'int main() { return "\v"[0]; }', ord('\v'))

assert check_stepN_test_case(42, step42, r'int main() { return sizeof("abc\\"); }', 5)
assert check_stepN_test_case(42, step42, r'int main() { return sizeof("\\abc\\"); }', 6)

assert check_stepN_test_case(42, step42, r"int main() { int p = '\\'; return p; }", ord('\\'))
assert check_stepN_test_case(42, step42, r"int main() { int p = '\"'; return p; }", ord('\"'))
assert check_stepN_test_case(42, step42, r"int main() { int p = '\''; return p; }", ord('\''))
assert check_stepN_test_case(42, step42, r"int main() { int p = '\n'; return p; }", ord('\n'))

assert check_stepN_test_case(42, step42, "int main() { int q; int *p = &q; return p == 0;}", 0)

assert check_stepN_test_case(42, step42, "int main() { int *p; p = 0; return 0;}", 0)
assert check_stepN_test_case(42, step42, "int main() { int *p = 0; return 0;}", 0)

assert check_stepN_test_case(42, step42, "int main() { int *p = 0; return !p;}", 1)
assert check_stepN_test_case(42, step42, "int main() { int q; int *p = &q; return !p;}", 0)

assert check_stepN_test_case(42, step42, """
int printf();
int main() {
    for (int i = 1; i <= 3; i++) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0)

assert check_stepN_test_case(42, step42, "int main() { int a=1; a*=3; return a; }", 3)


assert check_stepN_test_case(42, step42, "int main() { return sizeof(int); }", 4)
assert check_stepN_test_case(42, step42, "int main() { return sizeof(int *); }", 8)
assert check_stepN_test_case(42, step42, "int main() { return sizeof(char); }", 1)
assert check_stepN_test_case(42, step42, "int main() { return sizeof(char *); }", 8)
assert check_stepN_test_case(42, step42, "int main() { return sizeof(char **); }", 8)

assert check_stepN_test_case(42, step42, 
    "int printf(); int main() { int a; int b; a=1; b=a++; printf(\"b=%d\", b); return a; }", 2)
assert check_stepN_test_case(42, step42, 
    "int printf(); int main() { int a; int b; a=2; b=a--; printf(\"b=%d\", b); return a; }", 1)

assert check_stepN_test_case(42, step42, "int main() { int a; a=1; a*=3; return a; }", 3)

assert check_stepN_test_case(42, step42, """
int printf();
int main() {
    int i;
    for (i = 1; i <= 3; i += 1) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0)

assert check_stepN_test_case(42, step42, """
int printf();
int main() {
    int i;
    for (i = 256; i > 1; i /= 2) { 
        printf("%d,", i); 
    }
    return 0;
}
""", 0)


assert check_stepN_test_case(42, step42, """
int puts();
// line comment
int main() {
    int i; 
    /*********
     * block *
     *********/
    for (i = 1; i <= 3; i = i + 1) { 
        puts("a");//*
        // */ b 
    }
    return 0;
}
""", 0)

assert check_stepN_test_case(42, step42, """
int printf();
int main() {
    int i; 
    for (i = 1; i <= 3; i = i + 1) { 
        printf("a"); 
    }
    return 0;
}
""", 0)

assert check_stepN_test_case(42, step42, """
int printf();
int main() {
    int i;
    for (i = 1; i <= 3; i = i + 1) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0)

#################################

assert check_stepN_test_case(42, step42, "int main() { return 0; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 42; }", 42)
assert check_stepN_test_case(42, step42, "int main() { return 0+10+3; }", 0+10+3)
assert check_stepN_test_case(42, step42, "int main() { return 111+10-42; }", 111+10-42)
assert check_stepN_test_case(42, step42, "int main() { return    111   + 10 -     42; }", 111+10-42)
assert check_stepN_test_case(42, step42, "int main() { return    0 +    10+    3; }",  0 + 10 + 3)
assert check_stepN_test_case(42, step42, "int main() { return 10*2; }", 10*2)
assert check_stepN_test_case(42, step42, "int main() { return 10+1*2; }", 10+1*2)
assert check_stepN_test_case(42, step42, "int main() { return 10+3*2+10-5; }", 10+3*2+10-5)
assert check_stepN_test_case(42, step42, "int main() { return (10+3)*2+10-5; }", (10+3)*2+10-5)
assert check_stepN_test_case(42, step42, "int main() { return (10+1)*2; }", (10+1)*2)
assert check_stepN_test_case(42, step42, "int main() { return (10+1)/2; }", (10+1)//2)
assert check_stepN_test_case(42, step42, "int main() { return (15+1)/2+3; }", (15+1)//2+3)
assert check_stepN_test_case(42, step42, "int main() { return 10+1 /2/5; }", 10+1//2//5)

# unary
assert check_stepN_test_case(42, step42, "int main() { return -10+1 /2/5+30; }", -10+1//2//5+30)
assert check_stepN_test_case(42, step42, "int main() { return +10+1 /2/5; }", +10+1//2//5)
assert check_stepN_test_case(42, step42, "int main() { return -2*-3; }", -2*-3)

# equality
assert check_stepN_test_case(42, step42, "int main() { return 1==0; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 1==1; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1==1+5; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 1+(1+1==1+1); }", 2)

assert check_stepN_test_case(42, step42, "int main() { return 1!=0; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1!=1; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 1!=1+5; }", 1)

# relational
assert check_stepN_test_case(42, step42, "int main() { return 1>0; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1>1; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 1<0; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 1<1; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 1>=0; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1>=1; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1<=0; }", 0)
assert check_stepN_test_case(42, step42, "int main() { return 1<=1; }", 1)


# semicolon
assert check_stepN_test_case(42, step42, "int main() { 1+1;return 5-2; }", 3)

# variables
assert check_stepN_test_case(42, step42, "int main() { int a; a=3;return a; }", 3)
assert check_stepN_test_case(42, step42, "int main() { int a; int b; a=3;b=4;return a+b; }", 7)

assert check_stepN_test_case(42, step42, "int main() { int ab; int bd; ab=3;bd=4;return ab+bd; }", 7)
assert check_stepN_test_case(42, step42, 
    "int main() { int abz; int bdz; abz=3;bdz =4;return abz+bdz; }", 7)

assert check_stepN_test_case(42, step42, "int main() { return 1;return 2; }", 1)
assert check_stepN_test_case(42, step42, "int main() { return 1;return 2+3; }", 1)
assert check_stepN_test_case(42, step42, "int main() { int a; a=0;if(1)a=1;return a; }", 1)
assert check_stepN_test_case(42, step42, "int main() { int a; a=0;if(0)a=1;return a; }", 0)

assert check_stepN_test_case(42, step42, "int main() { int a; a=1;if(a)a=5;return a; }", 5)
assert check_stepN_test_case(42, step42, "int main() { int a; a=0;if(a)a=5;return a; }", 0)

assert check_stepN_test_case(42, step42, "int main() { int a; a=1;if(a)return 5;return 10; }", 5)
assert check_stepN_test_case(42, step42, "int main() { int a; a=0;if(a)return 5;return 10; }", 10)

assert check_stepN_test_case(42, step42, 
    "int main() { int a; a=0;if(a)return 5;a=1;if(a)return 3;return 10; }", 3)
assert check_stepN_test_case(42, step42, "int main() { int a; a=0;while(a)return 1; return 3; }", 3)
assert check_stepN_test_case(42, step42, "int main() { int a; a=0;while(a<5)a=a+1; return a; }", 5)

assert check_stepN_test_case(42, step42, "int main() { int a; a=0;if(a)return 5;else a=10;return a; }", 10)
assert check_stepN_test_case(42, step42, "int main() { int a; a=1;if(a)a=0;else return 10;return a; }", 0)

assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; for(a=0;a<10;a=a+1)b=a;return b; }", 9)
assert check_stepN_test_case(42, step42, "int main() { for(;;)return 0; }", 0)

# block
assert check_stepN_test_case(42, step42, "int main() { { { { return 3; } } } }", 3)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 3; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 3)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 0; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 12)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { } return c; }", 3)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } return c; }", 3)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } else { c = 7; } return c; }", 7)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; } else { c = 7; }} return c; }", 3)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) if (b) { c = 2; } else { c = 7; } return c; }", 3)
assert check_stepN_test_case(42, step42, 
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; }} else { c = 7; } return c; }", 7)

assert check_stepN_test_case(42, step42, "int three() { return 3; } int main() { return three(); }", 3)
assert check_stepN_test_case(42, step42, 
    "int one() { return 1; } int three() { return one() + 2; } int main() { return three() + three(); }", 6)
assert check_stepN_test_case(42, step42, 
    "int identity(int a) { return a; } int main() { return identity(3); }", 3)
assert check_stepN_test_case(42, step42, 
    "int add2(int a, int b) { return a + b; } int main() { return add2(1, 2); }", 3)
assert check_stepN_test_case(42, step42, 
    "int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; } int main() { return add6(1, 2, 3, 4, 5, 6); }", 21)
assert check_stepN_test_case(42, step42, 
    "int fib(int n) { if (n <= 1) { return n; } return fib(n-1) + fib(n-2); } int main() { return fib(8); }", 21)
assert check_stepN_test_case(42, step42, "int main() { int x; int *y; x = 3; y = &x; return *y; }", 3)

assert check_stepN_test_case(42, step42, "int main() { int x; int *y; y = &x; *y = 3; return x; }", 3)

assert check_stepN_test_case(42, step42, "int main() { int x; return sizeof x; }", 4)
assert check_stepN_test_case(42, step42, "int main() { int *p; return sizeof p; }", 8)
assert check_stepN_test_case(42, step42, "int main() { int x; return sizeof(x+3); }", 4)
assert check_stepN_test_case(42, step42, "int main() { int *p; return sizeof(p+3); }", 8)

assert check_stepN_test_case(42, step42, "int main() { int arr[10]; return 8; }", 8)
assert check_stepN_test_case(42, step42, "int main() { int arr[10]; return sizeof(arr); }", 40)
assert check_stepN_test_case(42, step42, "int main() { int arr[5][2]; return sizeof(arr); }", 40)
assert check_stepN_test_case(42, step42, "int main() { int *arr[5][2]; return sizeof(arr); }", 80)

assert check_stepN_test_case(42, step42, "int main() { int arr[5][2]; return sizeof((arr)); }", 40)
assert check_stepN_test_case(42, step42, "int main() { int arr[5][2]; return sizeof(arr + 0); }", 8)
assert check_stepN_test_case(42, step42, "int main() { int arr[5][2]; return sizeof(*&arr); }", 40)

assert check_stepN_test_case(42, step42, "int main() { int arr[10]; return sizeof(*arr); }", 4)
assert check_stepN_test_case(42, step42, "int main() { int arr[5][2]; return sizeof(*arr); }", 8)
assert check_stepN_test_case(42, step42, "int main() { int arr[2][5]; return sizeof(*arr); }", 20)

assert check_stepN_test_case(42, step42, 
    "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }", 3)
assert check_stepN_test_case(42, step42, 
    "int main() { int a[2]; *(a + 1) = 2; *a = 1; int *p; p = a; return *p + *(p + 1); }", 3)

assert check_stepN_test_case(42, step42, "int main() { int a; int b; a = b = 3; return a + b; }", 6)

assert check_stepN_test_case(42, step42, "int *foo; int bar[10]; int main() { return 0; }", 0)

assert check_stepN_test_case(42, step42, 
    "int *foo; int bar[10]; int main() { foo = bar; bar[3] = 7; return foo[3]; }", 7)

assert check_stepN_test_case(42, step42, 
    "int main() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }", 3)

assert check_stepN_test_case(42, step42, "int main() { char *x; x = \"@A\"; return x[1] - x[0]; }", 1)
assert check_stepN_test_case(42, step42, "int main() { char *x; x = \"az\"; return x[1] - x[0]; }", 25)
assert check_stepN_test_case(42, step42, "int main() { return \"az\"[1] - \"ab\"[0]; }", 25)
