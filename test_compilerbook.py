from test import bcolors, check

print(f"{bcolors.OKBLUE}Checking the earlier steps from the compilerbook:{bcolors.ENDC}")

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
    int parsednum = parseInt(p);
    int parsedlength = intLength(p);
    p += parsedlength;
    printf("  mov rax, %d\\n", parsednum);
    while (*p) {
        if (*p == '+') {
            p++;
            int parsednum = parseInt(p);
            int parsedlength = intLength(p);
            p += parsedlength;
            printf("  add rax, %d\\n", parsednum);
        } else if (*p == '-') {
            p++;
            int parsednum = parseInt(p);
            int parsedlength = intLength(p);
            p += parsedlength;
            printf("  sub rax, %d\\n", parsednum);
        } else {
            return 1;
        }
    }
    printf("  ret\\n");
    return 0;
}"""


assert check(step2, 0, stdin="0", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 42
  ret
""")
assert check(step2, 0, stdin="42", expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 0
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
