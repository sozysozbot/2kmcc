
from test import bcolors, check, check_and_link_with


print(f"{bcolors.WARNING}Checking the inputs that should work but DOESN'T:{bcolors.ENDC}")

######################################
check("""
int printf();
int enum2(int a, int b) {
    printf("a=%d, b=%d; ", a, b);
    return a + b * 10;
}

int enum3(int a, int b, int c) {
    printf("a=%d, b=%d, c=%d; ", a, b, c);
    return enum2(a, b + c * 10);
}

int main() {
    printf("res=%d", enum3(1, 2, 3));
    return 0;
}
""", expected=0, expected_stdout="a=1, b=2, c=3; a=1, b=32; res=321")

check("""
int printf();
int enum2(int a, int b) {
    return a + b * 10;
}

int enum3(int a, int b, int c) {
    return enum2(a, enum2(b, c));
}

int main() {
    printf("%d", enum3(1, 2, 3));
    return 0;
}
""", expected=0, expected_stdout="321")


check("""
void *calloc();
char *strncpy();
int printf();
char *decode_kind(int kind) {
    void *r = &kind;
    char *q = r;
    char *ans = calloc(5, sizeof(char));
    strncpy(ans, q, 4);
    return ans;
}

int enum2(int a, int b) {
    return a + b * 256;
}

int enum3(int a, int b, int c) {
    return enum2(a, enum2(b, c));
}

int enum4(int a, int b, int c, int d) {
    return enum2(a, enum3(b, c, d));
}

int main() {
    printf("%s", decode_kind(enum3('a', 'b', 'c')));
    return 0;
}
""", expected=0, expected_stdout="abc")

check("""
void *calloc();
char *strncpy();
int printf();
char *decode_kind(int kind) {
    void *r = &kind;
    char *q = r;
    char *ans = calloc(5, sizeof(char));
    strncpy(ans, q, 4);
    return ans;
}

int enum2(int a, int b) {
    return a + b * 256;
}

int enum3(int a, int b, int c) {
    return enum2(a, enum2(b, c));
}

int enum4(int a, int b, int c, int d) {
    return enum2(a, enum3(b, c, d));
}

int main() {
    printf("%s", decode_kind(enum4('v', 'o', 'i', 'd')));
    return 0;
}
""", expected=0, expected_stdout="void")

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
