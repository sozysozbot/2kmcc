
from test import bcolors, check, check_and_link_with


print(f"{bcolors.WARNING}Checking the inputs that should work but DOESN'T:{bcolors.ENDC}")

######################################
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
