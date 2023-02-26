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
assert should_not_compile('int main(){int a; while(1){a}}')
assert should_not_compile('int main(){for}')
assert should_not_compile('int main(){for(}')
assert should_not_compile('int main(){for(1)}')
assert should_not_compile('int main(){for(1;1)}')
assert should_not_compile('int main(){for(1;1;1)}')
assert should_not_compile('int main(){for(1;1;1}')
assert should_not_compile('int main(){1}')

############################################################
print(f"{bcolors.OKBLUE}type error: pointer required{bcolors.ENDC}")
assert should_not_compile("int main(){return 1[2];}", "cannot deref a non-pointer type `int`.")
assert should_not_compile(
    "int main() { int x; *x; return 0; }",
    "cannot deref a non-pointer type `int`."
)

############################################################
print(f"{bcolors.OKBLUE}type error: integer required{bcolors.ENDC}")
assert should_not_compile(
    "struct A{int a; int b;}; int main(){struct A a; struct A b; b *= a; return 3;}",
    "int/char is expected, but not an int/char; the type is instead `struct A`.")
assert should_not_compile(
    "int main() { int a = 0; int *p = &a; +p; return 0; }",
    "int/char is expected, but not an int/char; the type is instead `pointer to int`.")

############################################################
print(f"{bcolors.OKBLUE}incorrect use of void:{bcolors.ENDC}")
assert should_not_compile("struct A { void a; }; int main() { return 0; }")
assert should_not_compile('int main(void){void *p = 0; p += 3;}')

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
assert should_not_compile(
    'struct A{int x; int y;}; int main() {struct A a; return 3 - a; }',
    "invalid operands to binary `-`: types are `int` and `struct A`."
)

############################################################
print(f"{bcolors.OKBLUE}assigning to an array:{bcolors.ENDC}")
assert should_not_compile('int main(){int a[1]; int *b; a = b;}', "invalid operands to binary `=`: types are `array (length: 1) of int` and `pointer to int`.")

############################################################
print(f"{bcolors.OKBLUE}undefined identifier:{bcolors.ENDC}")
assert should_not_compile(
    "int main() { return p;}",
    "cannot find an identifier named `p`; cannot determine the type"
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
    "int main() { int *p; p = 3; return 0;}",
    "invalid operands to binary `=`: types are `pointer to int` and `int`."
)
assert should_not_compile(
    "int main() { int *p = 3; return 0;}",
    "invalid operands to binary `=`: types are `pointer to int` and `int`."
)
assert should_not_compile(
    'int main(){int x;int *y;*y = &x; return x;}', 
    "invalid operands to binary `=`: types are `int` and `pointer to int`."
)
assert should_not_compile(
    'int main(){int x; int *y; y = &x;int **z; *z = x;}', 
    "invalid operands to binary `=`: types are `pointer to int` and `int`."
)
assert should_not_compile('int main() {int a; int *b; b = a; return a;}', "invalid operands to binary `=`: types are `pointer to int` and `int`.")
assert should_not_compile('int main() {int a; *a;}', "cannot deref a non-pointer type `int`.")
assert should_not_compile(
    'int main(){int x; int *y; y = &x;int **z; z = y;}', 
    "invalid operands to binary `=`: types are `pointer to pointer to int` and `pointer to int`."
)
assert should_not_compile('int main(){int x; int *y; y = &x; **y;}', "cannot deref a non-pointer type `int`.")
assert should_not_compile('int *foo(){int *x; return x;}int main(){int x; x= foo();}', "invalid operands to binary `=`: types are `int` and `pointer to int`.")
assert should_not_compile(
    'int main(void){char a[5]; a[1] = 74; int *p = a + 3; p -= 2; return *p;}', 
    "invalid operands to binary `=`: types are `pointer to int` and `pointer to char`."
)
assert should_not_compile(
    'int main(){int *p; int a[4][1][2]; p = a;}', 
    "invalid operands to binary `=`: types are `pointer to int` and `pointer to array (length: 1) of array (length: 2) of int`."
)
assert should_not_compile(
    'int main(){int *x; int *y; x+y;}', 
    "invalid operands to binary `+`: types are `pointer to int` and `pointer to int`."
)

############################################################
print(f"{bcolors.OKBLUE}not an lvalue:{bcolors.ENDC}")
assert should_not_compile('struct A{int a;}; int main() { struct A x; struct A y; struct A z; y.a = 100; z.a = 2; (x = y) = z; return 0; }', "not an lvalue")
assert should_not_compile('struct A{int a;}; int main() { struct A x; struct A y; y.a = 100; &(x = y); return 0; }', "not an lvalue")

############################################################
print(f"{bcolors.OKBLUE}nonexistent member{bcolors.ENDC}")
assert should_not_compile(
    "int main() { int a; return a.b; }",
    "tried to access a member of a non-struct type"
)
assert should_not_compile(
    "struct A {int a;}; int main() { struct A a; a.b; return 0; }", 
    "cannot find a struct type `struct A` which has a member named `b`"
)
assert should_not_compile(
    "struct A {int a;}; int main() { struct A a; return a.b; }",
    "cannot find a struct type `struct A` which has a member named `b`"
)

############################################################
print(f"{bcolors.OKCYAN}scalar required{bcolors.ENDC}")
assert should_not_compile("struct A {int a;}; int main() { struct A a; a&&a; return 0; }", "a scalar value is expected, but the type is instead `struct A`.")
assert should_not_compile('struct A{int a; int b;}; int main(){struct A a; if(a){return 12;} return 3;}', "a scalar value is expected, but the type is instead `struct A`.")
assert should_not_compile('struct A{int a; int b;}; int main(){struct A a; for(;a;){return 12;} return 3;}', "a scalar value is expected, but the type is instead `struct A`.")
assert should_not_compile('struct A{int a; int b;}; int main(){struct A a; while(a){return 12;} return 3;}', "a scalar value is expected, but the type is instead `struct A`.")
assert should_not_compile('struct A{int a; int b;}; int main(){struct A a; struct A b; b || a; return 3;}', "a scalar value is expected, but the type is instead `struct A`.")

############################################################
print(f"{bcolors.WARNING}!!!possible memory violation while detecting the error!!!{bcolors.ENDC}")
assert should_not_compile('int main(') # somehow says "expected a type specifier or a type qualifier; got TokenKind ``"
assert should_not_compile('int main(int a') # somehow says `unknown character `:` (58)`
assert should_not_compile('int main const(){return const 123 const; const} const') # somehow says `unknown character `:` (58)`

print(f"""
{bcolors.OKGREEN}
************
*    OK    *
************
{bcolors.ENDC}
""")
