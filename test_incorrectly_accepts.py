from test import bcolors, should_not_compile

print(f"{bcolors.WARNING}Checking the inputs that should NOT work, but is currently ACCEPTED:{bcolors.ENDC}")

# duplicate definition
should_not_compile("int foo() { return 0; } int foo() { return 1; } int main() { return 0; }")
should_not_compile("int main() {int i; int i; return 0;}")
should_not_compile("struct A {int a;}; struct A {int b;}; int main() { return 0; }")

# nonexistent member
should_not_compile("struct A {int a;}; int main() { struct A a; a.b; return 0; }")
should_not_compile("struct A {int a;}; int main() { struct A a; return a.b; }")

# scalar required
should_not_compile("struct A {int a;}; int main() { struct A a; a&&a; return 0; }")
