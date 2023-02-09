
from test import bcolors, check, check_and_link_with


print(f"{bcolors.OKBLUE}Checking the inputs that should work:{bcolors.ENDC}")

######################################

assert check("""
struct Token {
    char kind;
    int value;
};

struct Token tokens[5];

int main(int argc, char **argv) {
    return sizeof(tokens);
}
""", 40)

assert check("""int printf();
int isDigit(char c) {
    return '0' <= c && c <= '9';
}
int parseInt(const char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int main() {
    return parseInt("42");
}""", 42)

assert check("""int printf();
int isDigit(char c) {
    return '0' <= c && c <= '9';
}
int parseInt(const char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int main() {
    printf("%d", parseInt("42")); 
    return 0;
}""", 0, expected_stdout="""42""")

assert check("""int printf();
int isDigit(char c);
void *calloc();
int parseInt(char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int intLength(char *str) {
    int length = 0;
    while (isDigit(*str)) {
        length++;
        str++;
    }
    return length;
}
int isDigit(char c) {
    return '0' <= c && c <= '9';
}
int main() {
    char *p = calloc(3, 1);
    p[0] = '0';
    p[1] = 0;
    printf(".intel_syntax noprefix\\n");
    printf(".globl main\\n");
    printf("main:\\n");
    int parsednum_ = parseInt(p);
    int parsedlength_ = intLength(p);
    p += parsedlength_;
    printf("  mov rax, %d\\n", parsednum_);
    while (*p) {
        if (*p == '+') {
            p++;
            int parsednum = parseInt(p);
            int parsedlength = intLength(p);
            p += parsedlength;
            printf("  add rax, %d\\n", parsednum);
        } else if (*p == '-') {
            p++;
            int parsednum2 = parseInt(p);
            int parsedlength2 = intLength(p);
            p += parsedlength2;
            printf("  sub rax, %d\\n", parsednum2);
        } else {
            return 2;
        }
    }
    printf("  ret\\n");
    return 0;
}""", 0, expected_stdout=""".intel_syntax noprefix
.globl main
main:
  mov rax, 0
  ret
""")

assert check("""
int isDigit(char c);
int parseInt(char *str) {
    int result = 0;
    while (isDigit(*str)) {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    return result;
}
int intLength(char *str) {
    int length = 0;
    while (isDigit(*str)) {
        length++;
        str++;
    }
    return length;
}
int isDigit(char c) {
    return '0' <= c && c <= '9';
}
int main() { return parseInt("123"); }""", 123)

assert check("""
int isDigit(char c) {
    return '0' <= c && c <= '9';
}
int intLength(char *str) {
    int length = 0;
    while (isDigit(*str)) {
        length++;
        str++;
    }
    return length;
}
int main() { return intLength("012a"); }
""", 3)

assert check("""
int isDigit(char c) {
    return '0' <= c && c <= '9';
}
int main() { for(int a = ' '; a <= '~'; a++) { if (isDigit(a)) {printf("%c", a);} } return 0; }
""", 0, expected_stdout="0123456789")

assert check("void foo(int *p) { *p = 3; return; } int main() { int a; foo(&a); return a; }", 3)

assert check("""
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

assert check("""
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


assert check("""
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

assert check("""
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

assert check("""
int printf();
int enum2(int a, int b) {
    printf("a=%d, b=%d; ", a, b);
    return a + b * 10;
}

int enum3(int a, int b, int c) {
    return enum2(a, b + c * 10);
}

int main() {
    printf("res=%d", enum3(1, 2, 3));
    return 0;
}
""", expected=0, expected_stdout="a=1, b=32; res=321")

assert check("int a; void foo(int *p) { *p = 3; return; } int main() { foo(&a); return a; }", 3)

assert check("""
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
    printf("%s", decode_kind('a'));
    return 0;
}
""", expected=0, expected_stdout="a")

assert check("""
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
    printf("%s", decode_kind(enum2('a', 'b')));
    return 0;
}
""", expected=0, expected_stdout="ab")

assert check("""
int bar(int *p, void *q) {
    return p == q;
}
int bar2(int *p, void *q) {
    p = q;
    q = p;
    return 0;
}
int main() { return 0; }
""", 0)

assert check_and_link_with(
    "struct A { int a; int b; }; int foo(); int main() { struct A s; foo(&s); return (&s)->b - (&s)->a; }",
    linked_lib="struct A { int a; int b; }; int foo(struct A *p) { p->a = 3; p->b = 24; return 0; }",
    expected=21)

assert check_and_link_with(
    "struct A { char a[5]; int b; }; int foo(); int main() { struct A s; foo(&s); return (&s)->a[2] + (&s)->b; }",
    linked_lib="""
struct A { char a[5]; int b; }; 
int foo(struct A *p) { 
    p->a[0] = 1; 
    p->a[1] = 3; 
    p->a[2] = 5; 
    p->a[3] = 7; 
    p->a[4] = 9; 
    p->b = 24; 
    return 0; 
}""",
    expected=5+24)

assert check_and_link_with(
    "struct A { char a[5]; int b; }; int foo(); int main() { struct A s; foo(&s); return s.a[2] + s.b; }",
    linked_lib="""
struct A { char a[5]; int b; }; 
int foo(struct A *p) { 
    p->a[0] = 1; 
    p->a[1] = 3; 
    p->a[2] = 5; 
    p->a[3] = 7; 
    p->a[4] = 9; 
    p->b = 24; 
    return 0; 
}""",
    expected=5+24)

assert check("struct A { int a; int b; }; int main() { return 0; }", 0)
assert check("struct A { int a; int b; }; int main() { return sizeof(struct A); }", 8)
assert check("struct A { char a; int b; }; int main() { return sizeof(struct A); }", 8)
assert check("struct A { char a[5]; int b; }; int main() { return sizeof(struct A); }", 12)

assert check("int main() { return sizeof(struct A*); }", 8)
assert check("int main() { struct A *p; return 0; }", 0)

assert check("int main() { return 1+(2!=1+1); }", 1)
assert check("int main() { return 5+(8+(7!=2)); }", 14)

# miscompiles

assert check("int main() { return 8*7!=2; }", 1)
assert check("int main() { return 8+7!=2; }", 1)
assert check("int main() { return 1+(1+1!=1+1); }", 1)
assert check("int main() { return 1+(1+1!=2); }", 1)
assert check("int main() { return 5+(8+7!=2); }", 6)


assert check("""
int printf();
int first() {
    printf("first, ");
    return 0;
}
int second() {
    printf("second, ");
    return 0;
}
int main() { return first() && second(); }
""", 0, expected_stdout="first, ")

assert check("""
int printf();
int first() {
    printf("first, ");
    return 2;
}
int second() {
    printf("second, ");
    return 0;
}
int main() { return first() && second(); }
""", 0, expected_stdout="first, second, ")

assert check("""
int printf();
int first() {
    printf("first, ");
    return 2;
}
int second() {
    printf("second, ");
    return 2;
}
int main() { return first() && second(); }
""", 1, expected_stdout="first, second, ")

assert check(r'int main() { return "\\"[0]; }', ord('\\'))
assert check(r'int main() { return "\""[0]; }', ord('\"'))
assert check(r'int main() { return "\'"[0]; }', ord('\''))
assert check(r'int main() { return "\n"[0]; }', ord('\n'))
## According to https://docs.oracle.com/cd/E19120-01/open.solaris/817-5477/eoqka/index.html 
## \a is not supported in the assembly
# assert check(r'int main() { return "\a"[0]; }', ord('\a')) 
assert check(r'int main() { return "\b"[0]; }', ord('\b'))
assert check(r'int main() { return "\t"[0]; }', ord('\t'))
assert check(r'int main() { return "\f"[0]; }', ord('\f'))
assert check(r'int main() { return "\v"[0]; }', ord('\v'))

assert check(r'int main() { return sizeof("abc\\"); }', 5)
assert check(r'int main() { return sizeof("\\abc\\"); }', 6)

assert check(r"int main() { int p = '\\'; return p; }", ord('\\'))
assert check(r"int main() { int p = '\"'; return p; }", ord('\"'))
assert check(r"int main() { int p = '\''; return p; }", ord('\''))
assert check(r"int main() { int p = '\n'; return p; }", ord('\n'))

assert check("int main() { int q; int *p = &q; return p == 0;}", 0)

assert check("int main() { int *p; p = 0; return 0;}", 0)
assert check("int main() { int *p = 0; return 0;}", 0)

assert check("int main() { int *p = 0; return !p;}", 1)
assert check("int main() { int q; int *p = &q; return !p;}", 0)

assert check("""
int printf();
int main() {
    for (int i = 1; i <= 3; i++) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0, expected_stdout="a-1a-2a-3")

assert check("int main() { int a=1; a*=3; return a; }", 3)


assert check("int main() { return sizeof(int); }", 4)
assert check("int main() { return sizeof(int *); }", 8)
assert check("int main() { return sizeof(char); }", 1)
assert check("int main() { return sizeof(char *); }", 8)
assert check("int main() { return sizeof(char **); }", 8)

assert check(
    "int printf(); int main() { int a; int b; a=1; b=a++; printf(\"b=%d\", b); return a; }", 2, expected_stdout="b=1")
assert check(
    "int printf(); int main() { int a; int b; a=2; b=a--; printf(\"b=%d\", b); return a; }", 1, expected_stdout="b=2")

assert check("int main() { int a; a=1; a*=3; return a; }", 3)

assert check("""
int printf();
int main() {
    int i;
    for (i = 1; i <= 3; i += 1) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0, expected_stdout="a-1a-2a-3")

assert check("""
int printf();
int main() {
    int i;
    for (i = 256; i > 1; i /= 2) { 
        printf("%d,", i); 
    }
    return 0;
}
""", 0, expected_stdout="256,128,64,32,16,8,4,2,")


assert check("""
int puts();
// line comment
int main() {
    int i; 
    /*********
     * block *
     *********/
    for (i = 1; i <= 3; i = i + 1) { 
        puts("a");//*
        // */ b 
    }
    return 0;
}
""", 0, expected_stdout="a\na\na\n")

assert check("""
int printf();
int main() {
    int i; 
    for (i = 1; i <= 3; i = i + 1) { 
        printf("a"); 
    }
    return 0;
}
""", 0, expected_stdout="aaa")

assert check("""
int printf();
int main() {
    int i;
    for (i = 1; i <= 3; i = i + 1) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0, expected_stdout="a-1a-2a-3")

#################################

assert check("int main() { return 0; }", 0)

assert check("int main() { return 42; }", 42)

assert check("int main() { return 0+10+3; }", 0+10+3)

assert check("int main() { return 111+10-42; }", 111+10-42)

assert check("int main() { return    111   + 10 -     42; }", 111+10-42)

assert check("int main() { return    0 +    10+    3; }",  0 + 10 + 3)

assert check("int main() { return 10*2; }", 10*2)

assert check("int main() { return 10+1*2; }", 10+1*2)
assert check("int main() { return 10+3*2+10-5; }", 10+3*2+10-5)

assert check("int main() { return (10+3)*2+10-5; }", (10+3)*2+10-5)
assert check("int main() { return (10+1)*2; }", (10+1)*2)


assert check("int main() { return (10+1)/2; }", (10+1)//2)
assert check("int main() { return (15+1)/2+3; }", (15+1)//2+3)
assert check("int main() { return 10+1 /2/5; }", 10+1//2//5)

# unary
assert check("int main() { return -10+1 /2/5+30; }", -10+1//2//5+30)
assert check("int main() { return +10+1 /2/5; }", +10+1//2//5)
assert check("int main() { return -2*-3; }", -2*-3)

# equality
assert check("int main() { return 1==0; }", 0)
assert check("int main() { return 1==1; }", 1)
assert check("int main() { return 1==1+5; }", 0)
assert check("int main() { return 1+(1+1==1+1); }", 2)

assert check("int main() { return 1!=0; }", 1)
assert check("int main() { return 1!=1; }", 0)
assert check("int main() { return 1!=1+5; }", 1)

# relational
assert check("int main() { return 1>0; }", 1)
assert check("int main() { return 1>1; }", 0)
assert check("int main() { return 1<0; }", 0)
assert check("int main() { return 1<1; }", 0)
assert check("int main() { return 1>=0; }", 1)
assert check("int main() { return 1>=1; }", 1)
assert check("int main() { return 1<=0; }", 0)
assert check("int main() { return 1<=1; }", 1)


# semicolon
assert check("int main() { 1+1;return 5-2; }", 3)

# variables
assert check("int main() { int a; a=3;return a; }", 3)
assert check("int main() { int a; int b; a=3;b=4;return a+b; }", 7)

assert check("int main() { int ab; int bd; ab=3;bd=4;return ab+bd; }", 7)
assert check(
    "int main() { int abz; int bdz; abz=3;bdz =4;return abz+bdz; }", 7)

assert check("int main() { return 1;return 2; }", 1)
assert check("int main() { return 1;return 2+3; }", 1)
assert check("int main() { int a; a=0;if(1)a=1;return a; }", 1)
assert check("int main() { int a; a=0;if(0)a=1;return a; }", 0)

assert check("int main() { int a; a=1;if(a)a=5;return a; }", 5)
assert check("int main() { int a; a=0;if(a)a=5;return a; }", 0)

assert check("int main() { int a; a=1;if(a)return 5;return 10; }", 5)
assert check("int main() { int a; a=0;if(a)return 5;return 10; }", 10)

assert check(
    "int main() { int a; a=0;if(a)return 5;a=1;if(a)return 3;return 10; }", 3)
assert check("int main() { int a; a=0;while(a)return 1; return 3; }", 3)
assert check("int main() { int a; a=0;while(a<5)a=a+1; return a; }", 5)

assert check("int main() { int a; a=0;if(a)return 5;else a=10;return a; }", 10)
assert check("int main() { int a; a=1;if(a)a=0;else return 10;return a; }", 0)

assert check(
    "int main() { int a; int b; for(a=0;a<10;a=a+1)b=a;return b; }", 9)
assert check("int main() { for(;;)return 0; }", 0)

# block
assert check("int main() { { { { return 3; } } } }", 3)
assert check(
    "int main() { int a; int b; int c; a = 3; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 3)
assert check(
    "int main() { int a; int b; int c; a = 0; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 12)
assert check(
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { } return c; }", 3)
assert check(
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } return c; }", 3)
assert check(
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } else { c = 7; } return c; }", 7)
assert check(
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; } else { c = 7; }} return c; }", 3)
assert check(
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) if (b) { c = 2; } else { c = 7; } return c; }", 3)
assert check(
    "int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; }} else { c = 7; } return c; }", 7)

# link with what's built in gcc
assert check_and_link_with(
    "int identity(); int main() { return identity(3); }",
    linked_lib="int identity(int a) { return a; }",
    expected=3)

assert check_and_link_with(
    "int three(); int main() { return three(); }",
    linked_lib="int three() { return 3; }",
    expected=3)

assert check_and_link_with(
    "int add(); int main() { return add(2, 3); }",
    linked_lib="int add(int a, int b) { return a + b; }",
    expected=5)

assert check_and_link_with(
    "int add6(); int main() { return add6(1, 2, 3, 4, 5, 6); }",
    linked_lib="int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }",
    expected=21)

assert check("int three() { return 3; } int main() { return three(); }", 3)
assert check(
    "int one() { return 1; } int three() { return one() + 2; } int main() { return three() + three(); }", 6)
assert check(
    "int identity(int a) { return a; } int main() { return identity(3); }", 3)
assert check(
    "int add2(int a, int b) { return a + b; } int main() { return add2(1, 2); }", 3)
assert check(
    "int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; } int main() { return add6(1, 2, 3, 4, 5, 6); }", 21)
assert check(
    "int fib(int n) { if (n <= 1) { return n; } return fib(n-1) + fib(n-2); } int main() { return fib(8); }", 21)
assert check("int main() { int x; int *y; x = 3; y = &x; return *y; }", 3)
assert check_and_link_with("int write4(); int main() { int x; x = 3; write4(&x); return x; }",
                           linked_lib="int write4(int *p) { return *p = 4; } ", expected=4)

assert check("int main() { int x; int *y; y = &x; *y = 3; return x; }", 3)


assert check_and_link_with("int alloc4(); int main() { int *p; alloc4(&p, 1, 2, 4, 8); return *(p + 0) + *(p + 1) + *(p + 2) + *(p + 3); }",
                           linked_lib="""
#include <stdlib.h>
int alloc4(int **p, int a, int b, int c, int d) { 
    *p = calloc(4, sizeof(int));
    (*p)[0] = a;
    (*p)[1] = b;
    (*p)[2] = c;
    (*p)[3] = d;
    return 0; 
} 
""", expected=15)

assert check("int main() { int x; return sizeof x; }", 4)
assert check("int main() { int *p; return sizeof p; }", 8)
assert check("int main() { int x; return sizeof(x+3); }", 4)
assert check("int main() { int *p; return sizeof(p+3); }", 8)

assert check("int main() { int arr[10]; return 8; }", 8)
assert check("int main() { int arr[10]; return sizeof(arr); }", 40)
assert check("int main() { int arr[5][2]; return sizeof(arr); }", 40)
assert check("int main() { int *arr[5][2]; return sizeof(arr); }", 80)

assert check("int main() { int arr[5][2]; return sizeof((arr)); }", 40)
assert check("int main() { int arr[5][2]; return sizeof(arr + 0); }", 8)
assert check("int main() { int arr[5][2]; return sizeof(*&arr); }", 40)

assert check("int main() { int arr[10]; return sizeof(*arr); }", 4)
assert check("int main() { int arr[5][2]; return sizeof(*arr); }", 8)
assert check("int main() { int arr[2][5]; return sizeof(*arr); }", 20)

assert check(
    "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }", 3)
assert check(
    "int main() { int a[2]; *(a + 1) = 2; *a = 1; int *p; p = a; return *p + *(p + 1); }", 3)

assert check_and_link_with("int foo(); int main() { int a[2][4]; **a = 3; return foo(&a); }",
                           linked_lib="int foo(int (*p)[2][4]) { return (*p)[0][0] == 3; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int a[2][4]; **(a+0) = 3; return foo(&a); }",
                           linked_lib="int foo(int (*p)[2][4]) { return (*p)[0][0] == 3; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int a[2][4]; **(a+1) = 3; return foo(&a); }",
                           linked_lib="int foo(int (*p)[2][4]) { return (*p)[1][0] == 3; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int a[2][4]; *(*(a+1)) = 3; return foo(&a); }",
                           linked_lib="int foo(int (*p)[2][4]) { return (*p)[1][0] == 3; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int a[2][4]; *(*(a+1)+2) = 3; return foo(&a); }",
                           linked_lib="int foo(int (*p)[2][4]) { return (*p)[1][2] == 3; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int a[3][4]; *(*(a+2)+1) = 4; *(*(a+1)+2) = 3; return foo(&a); }",
                           linked_lib="int foo(int (*p)[3][4]) { return (*p)[2][1] == 4 && (*p)[1][2] == 3; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int a[1][2]; *(*(a+0)+0) = 4; *(*(a+0)+1) = 3; return foo(&a); }",
                           linked_lib="int foo(int (*p)[1][2]) { return (*p)[0][0] == 4 && (*p)[0][1] == 3; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int a[2][2]; *(*(a+0)+0) = 4; *(*(a+0)+1) = 3; *(*(a+1)+0) = 2; *(*(a+1)+1) = 1; return foo(&a); }",
                           linked_lib="int foo(int (*p)[1][2]) { return (*p)[0][0] == 4 && (*p)[0][1] == 3 && (*p)[1][0] == 2 && (*p)[1][1] == 1; }",
                           expected=1)

assert check_and_link_with("int foo(); int main() { int i; int j; foo(5); for(i=0;i<2;i=i+1) { foo(i); } return 1; }",
                           linked_lib='''
#include <stdio.h>
int foo(int i) { printf("i=%d\\n", i); return 0; }''',
                           expected=1, expected_stdout="i=5\ni=0\ni=1\n")

assert check_and_link_with("int foo(); int main() { int p[1]; int j; *p=3; for(j=0;j<2;j=j+1) { foo(p, j); } return *p; }",
                           linked_lib='''
#include <stdio.h>
int foo(int *p, int j) { *p = j; return 0; }''',
                           expected=1)

assert check_and_link_with("int foo(); int main() { int i; int j; foo(5); for(j=0;j<2;j=j+1) { foo(j); } return 1; }",
                           linked_lib='''
#include <stdio.h>
int foo(int j) { printf("j=%d\\n", j); return 0; }''',
                           expected=1, expected_stdout="j=5\nj=0\nj=1\n")

assert check_and_link_with("int foo(); int main() { int i; int j; for(i=0;i<2;i=i+1) {for(i=0;i<4;i=i+1) { foo(i, i); } } return 1; }",
                           linked_lib='''
#include <stdio.h>
int foo(int i, int j) { printf("i=%d, j=%d\\n", i, j); return 0; }''',
                           expected=1, expected_stdout="i=0, j=0\ni=1, j=1\ni=2, j=2\ni=3, j=3\n")

assert check_and_link_with("int foo(); int main() { int i; int j; for(i=0;i<2;i=i+1) {for(j=0;j<4;j=j+1) { foo(i, j); } } return 1; }",
                           linked_lib='''
#include <stdio.h>
int foo(int i, int j) { printf("i=%d, j=%d\\n", i, j); return 0; }''',
                           expected=1, expected_stdout="""i=0, j=0
i=0, j=1
i=0, j=2
i=0, j=3
i=1, j=0
i=1, j=1
i=1, j=2
i=1, j=3
""")


assert check_and_link_with("int foo(); int main() { int a[2][4]; int i; int j; for(i=0;i<2;1) {for(j=0;j<4;1) {*(*(a + i) + j) = i * 10 + j; j=j+1;}i=i+1;} return foo(&a); }",
                           linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
                           expected=42, expected_stdout="""i=0, j=0, (*p)[i][j]=0
i=0, j=1, (*p)[i][j]=1
i=0, j=2, (*p)[i][j]=2
i=0, j=3, (*p)[i][j]=3
i=1, j=0, (*p)[i][j]=10
i=1, j=1, (*p)[i][j]=11
i=1, j=2, (*p)[i][j]=12
i=1, j=3, (*p)[i][j]=13
""")


assert check_and_link_with("int foo(); int main() { int a[2][4]; int i; int j; for(i=0;i<2;i=i+1) {for(j=0;j<4;j=j+1) {*(*(a + i) + j) = i * 10 + j;}} return foo(&a); }",
                           linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
                           expected=42, expected_stdout="""i=0, j=0, (*p)[i][j]=0
i=0, j=1, (*p)[i][j]=1
i=0, j=2, (*p)[i][j]=2
i=0, j=3, (*p)[i][j]=3
i=1, j=0, (*p)[i][j]=10
i=1, j=1, (*p)[i][j]=11
i=1, j=2, (*p)[i][j]=12
i=1, j=3, (*p)[i][j]=13
""")


assert check_and_link_with("int foo(); int main() { int a[2][4]; int i; int j; for(i=0;i<2;i=i+1) for(j=0;j<4;j=j+1) *(*(a + i) + j) = i * 10 + j; return foo(&a); }",
                           linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
                           expected=42, expected_stdout="""i=0, j=0, (*p)[i][j]=0
i=0, j=1, (*p)[i][j]=1
i=0, j=2, (*p)[i][j]=2
i=0, j=3, (*p)[i][j]=3
i=1, j=0, (*p)[i][j]=10
i=1, j=1, (*p)[i][j]=11
i=1, j=2, (*p)[i][j]=12
i=1, j=3, (*p)[i][j]=13
""")

assert check_and_link_with("int foo(); int main() { int a[2][4]; int i; int j; for(i=0;i<2;i=i+1) {for(j=0;j<4;j=j+1) {a[i][j] = i * 10 + j;}} return foo(&a); }",
                           linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
                           expected=42, expected_stdout="""i=0, j=0, (*p)[i][j]=0
i=0, j=1, (*p)[i][j]=1
i=0, j=2, (*p)[i][j]=2
i=0, j=3, (*p)[i][j]=3
i=1, j=0, (*p)[i][j]=10
i=1, j=1, (*p)[i][j]=11
i=1, j=2, (*p)[i][j]=12
i=1, j=3, (*p)[i][j]=13
""")

assert check("int main() { int a; int b; a = b = 3; return a + b; }", 6)

assert check("int *foo; int bar[10]; int main() { return 0; }", 0)

assert check(
    "int *foo; int bar[10]; int main() { foo = bar; bar[3] = 7; return foo[3]; }", 7)

assert check(
    "int main() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }", 3)

assert check("int main() { char *x; x = \"@A\"; return x[1] - x[0]; }", 1)
assert check("int main() { char *x; x = \"az\"; return x[1] - x[0]; }", 25)
assert check("int main() { return \"az\"[1] - \"ab\"[0]; }", 25)

print(f"""
{bcolors.OKGREEN}
************
*    OK    *
************
{bcolors.ENDC}
""")