from test import bcolors, check

print(f"{bcolors.WARNING}Checking the inputs that SHOULD work, but is currently REJECTED:{bcolors.ENDC}")

check(r"int main() { return '\a';}", 7)