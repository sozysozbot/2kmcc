from test import check, check_and_link_with

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
