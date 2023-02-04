from test import bcolors, should_not_compile

print(f"{bcolors.OKBLUE}Checking the inputs that should NOT work:{bcolors.ENDC}")

assert should_not_compile('int main(){main(1}')
assert should_not_compile('int main(){return (123}')
assert should_not_compile('int main(){return 123}')
assert should_not_compile('int main(){if a}')
assert should_not_compile('int main(){while}')
assert should_not_compile('int main(){while(}')
assert should_not_compile('int main(){while()}')
assert should_not_compile('int main(){while(1}')
assert should_not_compile('int main(){while(1)}')

assert should_not_compile("int main(){return 1[2];}", "cannot deref a non-pointer type")

assert should_not_compile(
    "struct A{int a; int b;}; int main(){struct A a; struct A b; b *= a; return 3;}",
    "int/char is expected, but not an int/char")

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
