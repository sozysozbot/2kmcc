
import os
import subprocess

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def compile(input: str):
    f = open("tmp.s", "w")
    return subprocess.call(["./2kmcc", input], stdout=f) # to handle double-quotes correctly

def run():
    f = open("stdout.txt", "w")
    return subprocess.call(["./tmp"], stdout=f)

def check(input: str, expected: int, expected_stdout: str=None):
    compiler_returns = compile(input)
    if compiler_returns != 0:
        print(f"{bcolors.FAIL}FAIL:check (compile error):{input=}{bcolors.ENDC}")
        return False
    os.system("cc -o tmp tmp.s -static")
    returned_value = run()
    actual_stdout = open("stdout.txt","r").read()

    if expected != returned_value:
        print(f"{bcolors.FAIL}FAIL:check (wrong answer):{input=} {expected=} {returned_value=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False
    elif expected_stdout != None and actual_stdout != expected_stdout:
        print(f"{bcolors.FAIL}FAIL:check (correct answer but wrong stdout):{input=} {expected_stdout=} {actual_stdout=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False  
    elif expected_stdout != None:   
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {expected_stdout=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s stdout.txt")
        return True
    else:   
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s stdout.txt")
        return True

def should_not_compile(input: str):
    compiler_returns = compile(input)
    if compiler_returns != 0:
        print(f"{bcolors.OKGREEN}passed: should give compile error:{input=}{bcolors.ENDC}")
        return True
    else:
        print(f"{bcolors.FAIL}FAIL: compiled what should not compile:{input=}{bcolors.ENDC}")
        return False

def check_and_link_with(input: str, linked_lib: str, expected: int):
    compiler_returns = compile(input)
    if compiler_returns != 0:
        print(f"{bcolors.FAIL}FAIL:check (compile error):{input=}{bcolors.ENDC}")
        return False
    lib_file = open("libtest.c", "w")
    lib_file.write(linked_lib)
    lib_file.close()
    os.system("cc -S -o libtest.s libtest.c")
    os.system("cc -o tmp tmp.s libtest.s -static")
    os.system("rm libtest.c libtest.s")
    returned_value = (os.system("./tmp") >> 8) & 0xff
    if expected == returned_value:
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        print(f"{bcolors.OKGREEN}       {linked_lib=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s")
        return True

    print(f"{bcolors.FAIL}FAIL:check (wrong answer):{input=} {expected=} {returned_value=}{bcolors.ENDC}")
    print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
    return False

print(f"{bcolors.OKBLUE}Checking the inputs that should work:{bcolors.ENDC}")

######################################

assert check("""
int main() {
    for (int i = 1; i <= 3; i++) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0, expected_stdout="a-1a-2a-3")

assert check("int main() { int a=1; a*=3; return a; }",3)


assert check("int main() { return sizeof(int); }", 4)
assert check("int main() { return sizeof(int *); }", 8)
assert check("int main() { return sizeof(char); }", 1)
assert check("int main() { return sizeof(char *); }", 8)
assert check("int main() { return sizeof(char **); }", 8)

assert check("int main() { int a; int b; a=1; b=a++; printf(\"b=%d\", b); return a; }",2, expected_stdout="b=1")
assert check("int main() { int a; int b; a=2; b=a--; printf(\"b=%d\", b); return a; }",1, expected_stdout="b=2")

assert check("int main() { int a; a=1; a*=3; return a; }",3)

assert check("""
int main() {
    int i;
    for (i = 1; i <= 3; i += 1) { 
        printf("a%d", -i); 
    }
    return 0;
}
""", 0, expected_stdout="a-1a-2a-3")

assert check("""
int main() {
    int i;
    for (i = 256; i > 1; i /= 2) { 
        printf("%d,", i); 
    }
    return 0;
}
""", 0, expected_stdout="256,128,64,32,16,8,4,2,")


assert check("""
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
int main() {
    int i; 
    for (i = 1; i <= 3; i = i + 1) { 
        printf("a"); 
    }
    return 0;
}
""", 0, expected_stdout="aaa")

assert check("""
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

assert check("int main() { return    0 +    10+    3; }",  0 +    10+    3)

assert check("int main() { return 10*2; }", 10*2)

assert check("int main() { return 10+1*2; }", 10+1*2)
assert check("int main() { return 10+3*2+10-5; }", 10+3*2+10-5)

assert check("int main() { return (10+3)*2+10-5; }", (10+3)*2+10-5)
assert check("int main() { return (10+1)*2; }", (10+1)*2)


assert check("int main() { return (10+1)/2; }", (10+1)//2)
assert check("int main() { return (15+1)/2+3; }", (15+1)//2+3)
assert check("int main() { return 10+1 /2/5; }", 10+1//2//5)

#unary
assert check("int main() { return -10+1 /2/5+30; }", -10+1//2//5+30)
assert check("int main() { return +10+1 /2/5; }", +10+1//2//5)
assert check("int main() { return -2*-3; }", -2*-3)

#equality
assert check("int main() { return 1==0; }", 0)
assert check("int main() { return 1==1; }", 1)
assert check("int main() { return 1==1+5; }", 0)
assert check("int main() { return 1+(1+1==1+1); }",2)

assert check("int main() { return 1!=0; }", 1)
assert check("int main() { return 1!=1; }", 0)
assert check("int main() { return 1!=1+5; }", 1)
assert check("int main() { return 1+(1+1!=1+1); }",1)


#relational
assert check("int main() { return 1>0; }", 1)
assert check("int main() { return 1>1; }", 0)
assert check("int main() { return 1<0; }", 0)
assert check("int main() { return 1<1; }", 0)
assert check("int main() { return 1>=0; }", 1)
assert check("int main() { return 1>=1; }", 1)
assert check("int main() { return 1<=0; }", 0)
assert check("int main() { return 1<=1; }", 1)


#semicolon
assert check("int main() { 1+1;return 5-2; }",3)

#variables
assert check("int main() { int a; a=3;return a; }",3)
assert check("int main() { int a; int b; a=3;b=4;return a+b; }",7)

assert check("int main() { int ab; int bd; ab=3;bd=4;return ab+bd; }",7)
assert check("int main() { int abz; int bdz; abz=3;bdz =4;return abz+bdz; }",7)

assert check("int main() { return 1;return 2; }",1)
assert check("int main() { return 1;return 2+3; }",1)
assert check("int main() { int a; a=0;if(1)a=1;return a; }",1)
assert check("int main() { int a; a=0;if(0)a=1;return a; }",0)

assert check("int main() { int a; a=1;if(a)a=5;return a; }",5)
assert check("int main() { int a; a=0;if(a)a=5;return a; }",0)

assert check("int main() { int a; a=1;if(a)return 5;return 10; }",5)
assert check("int main() { int a; a=0;if(a)return 5;return 10; }",10)

assert check("int main() { int a; a=0;if(a)return 5;a=1;if(a)return 3;return 10; }",3)
assert check("int main() { int a; a=0;while(a)return 1; return 3; }",3)
assert check("int main() { int a; a=0;while(a<5)a=a+1; return a; }",5)

assert check("int main() { int a; a=0;if(a)return 5;else a=10;return a; }",10)
assert check("int main() { int a; a=1;if(a)a=0;else return 10;return a; }",0)

assert check("int main() { int a; int b; for(a=0;a<10;a=a+1)b=a;return b; }",9)
assert check("int main() { for(;;)return 0; }",0)

#block
assert check("int main() { { { { return 3; } } } }", 3)
assert check("int main() { int a; int b; int c; a = 3; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 3)
assert check("int main() { int a; int b; int c; a = 0; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 12)
assert check("int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { } return c; }", 3)
assert check("int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } return c; }", 3)
assert check("int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } else { c = 7; } return c; }", 7)
assert check("int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; } else { c = 7; }} return c; }", 3)
assert check("int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) if (b) { c = 2; } else { c = 7; } return c; }", 3)
assert check("int main() { int a; int b; int c; a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; }} else { c = 7; } return c; }", 7)

#link with what's built in gcc
assert check_and_link_with(
    "int main() { return identity(3); }", 
    linked_lib="int identity(int a) { return a; }",
    expected=3)

assert check_and_link_with(
    "int main() { return three(); }", 
    linked_lib="int three() { return 3; }",
    expected=3)

assert check_and_link_with(
    "int main() { return add(2, 3); }", 
    linked_lib="int add(int a, int b) { return a + b; }",
    expected=5)

assert check_and_link_with(
    "int main() { return add6(1, 2, 3, 4, 5, 6); }", 
    linked_lib="int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }",
    expected=21)

assert check("int three() { return 3; } int main() { return three(); }", 3)
assert check("int one() { return 1; } int three() { return one() + 2; } int main() { return three() + three(); }", 6)
assert check("int identity(int a) { return a; } int main() { return identity(3); }", 3)
assert check("int add2(int a, int b) { return a + b; } int main() { return add2(1, 2); }", 3)
assert check("int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; } int main() { return add6(1, 2, 3, 4, 5, 6); }", 21)
assert check("int fib(int n) { if (n <= 1) { return n; } return fib(n-1) + fib(n-2); } int main() { return fib(8); }", 21)
assert check("int main() { int x; int *y; x = 3; y = &x; return *y; }", 3)
assert check_and_link_with("int main() { int x; x = 3; write4(&x); return x; }",
    linked_lib="int write4(int *p) { return *p = 4; } ", expected= 4)

assert check("int main() { int x; int *y; y = &x; *y = 3; return x; }", 3)


assert check_and_link_with("int main() { int *p; alloc4(&p, 1, 2, 4, 8); return *(p + 0) + *(p + 1) + *(p + 2) + *(p + 3); }",
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
""", expected= 15)

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

assert check("int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }", 3)
assert check("int main() { int a[2]; *(a + 1) = 2; *a = 1; int *p; p = a; return *p + *(p + 1); }", 3)

assert check_and_link_with("int main() { int a[2][4]; **a = 3; return foo(&a); }",
    linked_lib="int foo(int (*p)[2][4]) { return (*p)[0][0] == 3; }",
    expected=1)

assert check_and_link_with("int main() { int a[2][4]; **(a+0) = 3; return foo(&a); }",
    linked_lib="int foo(int (*p)[2][4]) { return (*p)[0][0] == 3; }",
    expected=1)

assert check_and_link_with("int main() { int a[2][4]; **(a+1) = 3; return foo(&a); }",
    linked_lib="int foo(int (*p)[2][4]) { return (*p)[1][0] == 3; }",
    expected=1)

assert check_and_link_with("int main() { int a[2][4]; *(*(a+1)) = 3; return foo(&a); }",
    linked_lib="int foo(int (*p)[2][4]) { return (*p)[1][0] == 3; }",
    expected=1)

assert check_and_link_with("int main() { int a[2][4]; *(*(a+1)+2) = 3; return foo(&a); }",
    linked_lib="int foo(int (*p)[2][4]) { return (*p)[1][2] == 3; }",
    expected=1)

assert check_and_link_with("int main() { int a[3][4]; *(*(a+2)+1) = 4; *(*(a+1)+2) = 3; return foo(&a); }",
    linked_lib="int foo(int (*p)[3][4]) { return (*p)[2][1] == 4 && (*p)[1][2] == 3; }",
    expected=1)

assert check_and_link_with("int main() { int a[1][2]; *(*(a+0)+0) = 4; *(*(a+0)+1) = 3; return foo(&a); }",
    linked_lib="int foo(int (*p)[1][2]) { return (*p)[0][0] == 4 && (*p)[0][1] == 3; }",
    expected=1)

assert check_and_link_with("int main() { int a[2][2]; *(*(a+0)+0) = 4; *(*(a+0)+1) = 3; *(*(a+1)+0) = 2; *(*(a+1)+1) = 1; return foo(&a); }",
    linked_lib="int foo(int (*p)[1][2]) { return (*p)[0][0] == 4 && (*p)[0][1] == 3 && (*p)[1][0] == 2 && (*p)[1][1] == 1; }",
    expected=1)

assert check_and_link_with("int main() { int i; int j; foo(5); for(i=0;i<2;i=i+1) { foo(i); } return 1; }",
    linked_lib='''
#include <stdio.h>
int foo(int i) { printf("i=%d\\n", i); return 0; }''',
    expected=1)

assert check_and_link_with("int main() { int p[1]; int j; *p=3; for(j=0;j<2;j=j+1) { foo(p, j); } return *p; }",
    linked_lib='''
#include <stdio.h>
int foo(int *p, int j) { *p = j; return 0; }''',
    expected=1)

assert check_and_link_with("int main() { int i; int j; foo(5); for(j=0;j<2;j=j+1) { foo(j); } return 1; }",
    linked_lib='''
#include <stdio.h>
int foo(int j) { printf("j=%d\\n", j); return 0; }''',
    expected=1)

assert check_and_link_with("int main() { int i; int j; for(i=0;i<2;i=i+1) {for(i=0;i<4;i=i+1) { foo(i, i); } } return 1; }",
    linked_lib='''
#include <stdio.h>
int foo(int i, int j) { printf("i=%d, j=%d\\n", i, j); return 0; }''',
    expected=1)

assert check_and_link_with("int main() { int i; int j; for(i=0;i<2;i=i+1) {for(j=0;j<4;j=j+1) { foo(i, j); } } return 1; }",
    linked_lib='''
#include <stdio.h>
int foo(int i, int j) { printf("i=%d, j=%d\\n", i, j); return 0; }''',
    expected=1)


assert check_and_link_with("int main() { int a[2][4]; int i; int j; for(i=0;i<2;1) {for(j=0;j<4;1) {*(*(a + i) + j) = i * 10 + j; j=j+1;}i=i+1;} return foo(&a); }",
    linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
    expected=42)


assert check_and_link_with("int main() { int a[2][4]; int i; int j; for(i=0;i<2;i=i+1) {for(j=0;j<4;j=j+1) {*(*(a + i) + j) = i * 10 + j;}} return foo(&a); }",
    linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
    expected=42)


assert check_and_link_with("int main() { int a[2][4]; int i; int j; for(i=0;i<2;i=i+1) for(j=0;j<4;j=j+1) *(*(a + i) + j) = i * 10 + j; return foo(&a); }",
    linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
    expected=42)

assert check_and_link_with("int main() { int a[2][4]; int i; int j; for(i=0;i<2;i=i+1) {for(j=0;j<4;j=j+1) {a[i][j] = i * 10 + j;}} return foo(&a); }",
    linked_lib='''
#include <stdio.h>
int foo(int (*p)[2][4]) { int i; int j; for(i=0;i<2;i++) for(j=0;j<4;j++) { printf("i=%d, j=%d, (*p)[i][j]=%d\\n", i, j, (*p)[i][j]); if ((*p)[i][j] != i * 10 + j) return i * 10 + j * 3;} return 42; }''',
    expected=42)

assert check("int main() { int a; int b; a = b = 3; return a + b; }", 6)

assert check("int *foo; int bar[10]; int main() { return 0; }", 0)

assert check("int *foo; int bar[10]; int main() { foo = bar; bar[3] = 7; return foo[3]; }", 7)

assert check("int main() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }", 3)

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

print(f"{bcolors.OKBLUE}Checking the inputs that should NOT work:{bcolors.ENDC}")

assert should_not_compile("int main() { int x; int y; x = 3; y = &x; return *y; }")

assert should_not_compile("int main() { int *p; char *q; return p-q;}")

print(f"""
{bcolors.OKGREEN}
************
*    OK    *
************
{bcolors.ENDC}
""")
