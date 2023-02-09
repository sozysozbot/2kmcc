from test import bcolors, check

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

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    struct Token token = tokens[0];
    if (token.kind != '#') {
        return 1;
    }
    printf("  mov rax, %d\n", token.value);

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
            printf("  add rax, %d\n", maybe_number.value);
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
            printf("  sub rax, %d\n", maybe_number.value);
            i++;
        } else {
            return 1;
        }
    }
    printf("  ret\n");
    return 0;
}
"""

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