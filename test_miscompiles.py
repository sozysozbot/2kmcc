
from test import bcolors, check

print(f"{bcolors.WARNING}Checking the inputs that should work but DOESN'T:{bcolors.ENDC}")

print(f"{bcolors.OKCYAN}octal literal{bcolors.ENDC}")
check("int main() { return 011; }" , 9)

print(f"{bcolors.OKCYAN}incorrect variable scope when nested{bcolors.ENDC}")
check('int main(){int a; a = 174; {int a; a = 3;} return a;}' , 174)
check("""
int printf();
int main() 
{
    for (int i = 0; i < 3; i++) {
        printf(" %d:", i);
        for (int i = 0; i < 3; i++) {
            printf("%d", i);
        }
    }
    return 0;
}""", 0, expected_stdout=" 0:012 1:012 2:012")