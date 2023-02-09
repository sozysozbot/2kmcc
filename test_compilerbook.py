import subprocess
from test import bcolors, check, compile_with_2kmcc, run_resulting_binary
import os

def compile_with_stepn(input: str, output_assembly_path: str = "tmp.s"):
    assembly = open(output_assembly_path, "w")
    msg = open("tmp_compile_stderr.txt", "w")
    return subprocess.call(["./stepn", input], stdout=assembly, stderr=msg)

def check_stepN_that_2kmcc_compiled(n: int, step_n: str, input_to_step_n: str, expected_output: int):
    compiler_returns = compile_with_2kmcc(step_n, "stepn.s")
    if compiler_returns != 0:
        print(
            f"{bcolors.FAIL}FAIL:check (2kmcc gave a compile error):{step_n=}{bcolors.ENDC}")
        msg = open("tmp_compile_stderr.txt", "r").read()
        print(f"  The error message is: {bcolors.FAIL}{msg}{bcolors.ENDC}")
        return False
    os.system("cc -o stepn stepn.s -static")
    value_returned_from_stepN = run_resulting_binary("./stepn", stdin=input_to_step_n, stdout_path="tmp_final.s")
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
        print(f"{bcolors.OKGREEN}step #{n} passed:\n  {input_to_step_n=}\n  {expected_output=} {bcolors.ENDC}")
        os.system("rm stepn stepn.s tmp_final tmp_final.s tmp_run_stdout.txt tmp_compile_stderr.txt")
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

assert check_stepN_that_2kmcc_compiled(1, step1, '0', 0)
assert check_stepN_that_2kmcc_compiled(1, step1, '42', 42)
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

assert check_stepN_that_2kmcc_compiled(2, step2, '0', 0)
assert check_stepN_that_2kmcc_compiled(2, step2, '42', 42)
assert check_stepN_that_2kmcc_compiled(2, step2, '0+10+3', 13)
assert check_stepN_that_2kmcc_compiled(2, step2, '111+10-42', 79)

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

assert check_stepN_that_2kmcc_compiled(3, step3, '0', 0)
assert check_stepN_that_2kmcc_compiled(3, step3, '42', 42)
assert check_stepN_that_2kmcc_compiled(3, step3, '0+10+3', 13)
assert check_stepN_that_2kmcc_compiled(3, step3, '111+10-42', 79)
assert check_stepN_that_2kmcc_compiled(3, step3, '   111   + 10 -     42', 79)
assert check_stepN_that_2kmcc_compiled(3, step3, '   0 +    10+    3', 13)
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
assert check_stepN_that_2kmcc_compiled(5, step5, '0', 0)
assert check_stepN_that_2kmcc_compiled(5, step5, '42', 42)
assert check_stepN_that_2kmcc_compiled(5, step5, '0+10+3', 13)
assert check_stepN_that_2kmcc_compiled(5, step5, '111+10-42', 79)
assert check_stepN_that_2kmcc_compiled(5, step5, '   111   + 10 -     42', 79)
assert check_stepN_that_2kmcc_compiled(5, step5, '   0 +    10+    3', 13)
assert check_stepN_that_2kmcc_compiled(5, step5, '10*2', 20)
assert check_stepN_that_2kmcc_compiled(5, step5, '10+1*2', 12)
assert check_stepN_that_2kmcc_compiled(5, step5, '10+3*2+10-5', 21)
assert check_stepN_that_2kmcc_compiled(5, step5, '(10+3)*2+10-5', 31)
assert check_stepN_that_2kmcc_compiled(5, step5, '(10+1)*2', 22)
assert check_stepN_that_2kmcc_compiled(5, step5, '(10+1)/2', 5)
assert check_stepN_that_2kmcc_compiled(5, step5, '(15+1)/2+3', 11)
assert check_stepN_that_2kmcc_compiled(5, step5, '10+1 /2/5', 10)
