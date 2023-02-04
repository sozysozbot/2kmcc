
from os import system
from test import bcolors, compile


def should_not_compile(input: str, expected_stderr: str = None):
    compiler_returns = compile(input)
    actual_stderr = open("tmp_stderr.txt", "r").read()
    if compiler_returns != 0:
        print(
            f"{bcolors.OKGREEN}passed: should give compile error\n  {input=}{bcolors.ENDC}")
        if expected_stderr != None:
            if actual_stderr == expected_stderr + "\n":
                print(
                    f"{bcolors.OKGREEN}   msg: {actual_stderr}{bcolors.ENDC}")
            else:
                print(
                    f"{bcolors.OKCYAN}error message was not as expected:\n  {expected_stderr=}\n  {actual_stderr=}{bcolors.ENDC}")
        else:
            print(
                f"{bcolors.OKGREEN}error message was:{actual_stderr=}{bcolors.ENDC}")
        system("rm tmp.s tmp_stderr.txt")
        return True
    else:
        print(
            f"{bcolors.FAIL}FAIL: compiled what should not compile:{input=}{bcolors.ENDC}")
        return False


print(f"{bcolors.OKBLUE}Checking the inputs that should NOT work:{bcolors.ENDC}")

assert should_not_compile(
    "int main() { 3 5; }",
    "parse error: expected an operator; got a number"
)

assert should_not_compile(
    "int main() { 3 $ 5; }",
    "unknown character $(36)"
)

assert should_not_compile(
    "int main() { int a; return a.b; }",
    "tried to access a member of a non-struct type"
)

assert should_not_compile(
    "int main() { return sizeof(struct A); }",
    "cannot calculate the size for type `struct A`."
)

assert should_not_compile(
    "struct A {int a;}; int main() { struct A a; 3+a; return 0; }",
    "invalid operands to binary `+`: types are `int` and `struct A`."
)


assert should_not_compile(
    "struct A {int a;}; int main() { struct A a; 3 == a; return 0; }",
    "invalid operands to binary `==`: types are `int` and `struct A`."
)

assert should_not_compile(
    "struct A {int a;}; int main() { struct A a; a+3; return 0; }",
    "invalid operands to binary `+`: types are `struct A` and `int`."
)

assert should_not_compile(
    "struct A {int a;}; int main() { struct A a; a-3; return 0; }",
    "invalid operands to binary `-`: types are `struct A` and `int`."
)

assert should_not_compile(
    "int main() { int x; *x; return 0; }",
    "cannot deref a non-pointer type"
)
assert should_not_compile(
    "int main() { int x; int y; x = 3; y = &x; return *y; }",
    "cannot assign/initialize because two incompatible types are detected: `int` and `pointer to int`."
)
assert should_not_compile(
    "int main() { int *p; char *q; return p-q;}",
    "invalid operands to binary `-`: types are `pointer to int` and `pointer to char`."
)
assert should_not_compile(
    "int main() { return p;}",
    "cannot find an identifier named `p`; cannot determine the type"
)
assert should_not_compile(
    "int main() { int *p; p = 3; return 0;}",
    "cannot assign/initialize because two incompatible types are detected: `pointer to int` and `int`."
)
assert should_not_compile(
    "int main() { int *p = 3; return 0;}",
    "cannot assign/initialize because two incompatible types are detected: `pointer to int` and `int`."
)

print(f"""
{bcolors.OKGREEN}
************
*    OK    *
************
{bcolors.ENDC}
""")
