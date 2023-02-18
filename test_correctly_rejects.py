from test import bcolors, should_not_compile

print(f"{bcolors.OKBLUE}Checking the inputs that should NOT work:{bcolors.ENDC}")

############################################################
print(f"{bcolors.OKBLUE}tokenization error:{bcolors.ENDC}")
assert should_not_compile(
    "int main() { 3 $ 5; }",
    "unknown character `$` (36)"
)
assert should_not_compile("int main() { return @3; }", "unknown character `@` (64)")

############################################################
print(f"{bcolors.OKBLUE}parse error:{bcolors.ENDC}")
assert should_not_compile('int main(){main(1}')
assert should_not_compile('int main(){return (123}')
assert should_not_compile('int main(){return 123}')
assert should_not_compile('int main(){if a}')
assert should_not_compile('int main(){while}')
assert should_not_compile('int main(){while(}')
assert should_not_compile('int main(){while()}')
assert should_not_compile('int main(){while(1}')
assert should_not_compile('int main(){while(1)}')
assert should_not_compile(
    "int main() { 3 5; }",
    "parse error: expected an operator; got a number"
)
assert should_not_compile(
    "struct Foo { a; }; int main() { return 3; }", 
    "expected a type specifier or a type qualifier; got TokenKind `IDNT`"
)

############################################################
print(f"{bcolors.OKBLUE}type error: pointer required{bcolors.ENDC}")
assert should_not_compile("int main(){return 1[2];}", "cannot deref a non-pointer type")
assert should_not_compile(
    "int main() { int x; *x; return 0; }",
    "cannot deref a non-pointer type"
)

############################################################
print(f"{bcolors.OKBLUE}type error: integer required{bcolors.ENDC}")
assert should_not_compile(
    "struct A{int a; int b;}; int main(){struct A a; struct A b; b *= a; return 3;}",
    "int/char is expected, but not an int/char")

############################################################
print(f"{bcolors.OKBLUE}incorrect use of void:{bcolors.ENDC}")
assert should_not_compile("struct A { void a; }; int main() { return 0; }")

############################################################
print(f"{bcolors.OKBLUE}incorrect struct use:{bcolors.ENDC}")
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

############################################################
print(f"{bcolors.OKBLUE}type mismatch:{bcolors.ENDC}")
assert should_not_compile(
    "int main() { int x; int y; x = 3; y = &x; return *y; }",
    "invalid operands to binary `=`: types are `int` and `pointer to int`."
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
    "invalid operands to binary `=`: types are `pointer to int` and `int`."
)
assert should_not_compile(
    "int main() { int *p = 3; return 0;}",
    "invalid operands to binary `=`: types are `pointer to int` and `int`."
)

############################################################
print(f"{bcolors.OKBLUE}other type errors:{bcolors.ENDC}")
assert should_not_compile(
    "int main() { int a; return a.b; }",
    "tried to access a member of a non-struct type"
)

print(f"""
{bcolors.OKGREEN}
************
*    OK    *
************
{bcolors.ENDC}
""")
