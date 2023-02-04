from test import bcolors, should_not_compile

print(f"{bcolors.WARNING}Checking the inputs that should NOT work, but is currently ACCEPTED:{bcolors.ENDC}")

should_not_compile(
    "struct A {int a;}; int main() { struct A a; a.b; return 0; }",
    "cannot calculate the size for type `struct A`."
)

should_not_compile(
    "struct A {int a;}; int main() { struct A a; return a.b; }",
    "cannot calculate the size for type `struct A`."
)
