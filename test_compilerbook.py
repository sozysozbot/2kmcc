import subprocess
from test import bcolors, check, compile_with_2kmcc, run_resulting_binary
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
    compiler_returns = compile_with_2kmcc(step_n, "tmp_2kmcc_stepn.s")
    if compiler_returns != 0:
        print(
            f"{bcolors.FAIL}FAIL:check (2kmcc gave a compile error):{step_n=}{bcolors.ENDC}")
        msg = open("tmp_compile_stderr.txt", "r").read()
        print(f"  The error message is: {bcolors.FAIL}{msg}{bcolors.ENDC}")
        return False
    os.system("rm tmp_compile_stderr.txt")
    os.system("cc -o tmp_2kmcc_stepn tmp_2kmcc_stepn.s -static")
    value_returned_from_stepN = run_resulting_binary(
        "./tmp_2kmcc_stepn", stdin=input_to_step_n, stdout_path="tmp_final.s")
    return rest(n, value_returned_from_stepN, expected_output, step_n, input_to_step_n, "2kmcc")

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
        } else if (('a' <= c && c <= 'z') || c == '_') {
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

int labelCounter = 0;

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
        int label = ++labelCounter;

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
        int label = ++labelCounter;

        printf(".Lbegin%d:\\n", label);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\\n");
        printf("  je  .Lend%d\\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp  .Lbegin%d\\n", label);
        printf(".Lend%d:\\n", label);
    } else if (stmt->stmt_kind == enum3('f', 'o', 'r')) {
        int label = ++labelCounter;

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
    return "rdi\\0rsi\\0rdx\\0rcx\\0r8 \\0r9" + 4 * n;
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
        EvaluateLValueAddressIntoRax(expr);
        printf("  mov rax,[rax]\\n");
        return;
    } else if (expr->expr_kind == enum4('C', 'A', 'L', 'L')) {
        for (int i = 0; i < expr->func_arg_len; i++) {
            EvaluateExprIntoRax(expr->func_args[i]);
            printf("    push rax\\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--) {
            printf("    pop %s\\n", nth_arg_reg(i));
        }
        printf(" call %s\\n", expr->name);
        return;
    } else if (expr->expr_kind == enum3('N', 'U', 'M')) {
        printf("  mov rax, %d\\n", expr->value);
        return;
    } else if (expr->expr_kind == enum4('1', 'A', 'R', 'Y')) {
        if (expr->op_kind == '*') {
            EvaluateExprIntoRax(expr->first_child);
            printf("  mov rax, [rax]\\n");
        } else if (expr->op_kind == '&') {
            EvaluateLValueAddressIntoRax(expr->first_child);
        } else {
            exit(1);
        }
    } else if (expr->expr_kind == enum4('2', 'A', 'R', 'Y')) {
        if (expr->op_kind == '=') {
            EvaluateLValueAddressIntoRax(expr->first_child);
            printf("    push rax\\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\\n");
            printf("    pop rdi\\n");
            printf("    pop rax\\n");
            printf("    mov [rax], rdi\\n");
        } else {
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
