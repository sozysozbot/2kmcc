from test import bcolors, should_not_compile

print(f"{bcolors.WARNING}Checking the inputs that should NOT work, but is currently ACCEPTED:{bcolors.ENDC}")

print(f"{bcolors.OKCYAN}duplicate definition{bcolors.ENDC}")
should_not_compile("int foo() { return 0; } int foo() { return 1; } int main() { return 0; }")
should_not_compile("int main() {int i; int i; return 0;}")
should_not_compile("struct A {int a;}; struct A {int b;}; int main() { return 0; }")
should_not_compile('int a(); char *a(); int main(void){return 174;}')
should_not_compile('int a(void); int a(int b); int main(void){return 174;}')

print(f"{bcolors.OKCYAN}wrong scope{bcolors.ENDC}")
should_not_compile( 'int main(){int a; {int b;} return b;}')

print(f"{bcolors.OKCYAN}wrong type of return value{bcolors.ENDC}")
should_not_compile('int *foo(){return 1;}int main(){return 0;}')
should_not_compile('int *foo(){int x; return x;}int main(){return 0;}')
should_not_compile('int foo(){int *x; return x;}int main(){return 0;}')
should_not_compile('int *foo(){int *x; return x;}int main(){return foo();}')

print(f"{bcolors.OKCYAN}wrong type of an argument{bcolors.ENDC}")
should_not_compile('int f(int a); int f(int a){return a;} int main(){int a; f(&a); return 3;}')
