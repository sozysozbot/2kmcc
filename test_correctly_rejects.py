
from os import system
from test import bcolors, compile

def should_not_compile(input: str):
    compiler_returns = compile(input)
    if compiler_returns != 0:
        print(
            f"{bcolors.OKGREEN}passed: should give compile error:{input=}{bcolors.ENDC}")
        system("rm tmp.s")
        return True
    else:
        print(
            f"{bcolors.FAIL}FAIL: compiled what should not compile:{input=}{bcolors.ENDC}")
        return False

print(f"{bcolors.OKBLUE}Checking the inputs that should NOT work:{bcolors.ENDC}")

assert should_not_compile(
    "int main() { int x; int y; x = 3; y = &x; return *y; }")

assert should_not_compile("int main() { int *p; char *q; return p-q;}")

assert should_not_compile("int main() { return p;}")

assert should_not_compile("int main() { int *p; p = 3; return 0;}")

assert should_not_compile("int main() { int *p = 3; return 0;}")

print(f"""
{bcolors.OKGREEN}
************
*    OK    *
************
{bcolors.ENDC}
""")
