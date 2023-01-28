
import os
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

def msg(input: str, expected: int, returned_value: int):
    if expected == returned_value:
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s")
        return True

    print(f"{bcolors.FAIL}FAIL:check:{input=} {expected=} {returned_value=}{bcolors.ENDC}")
    print(f"{bcolors.FAIL}Consult tmp.s to find out what when wrong{bcolors.ENDC}")
    return False

def check(input: str, expected: int):
    os.system(f'./9cc "{input}" > tmp.s')
    os.system("cc -o tmp tmp.s")
    returned_value = (os.system("./tmp") >> 8) & 0xff
    return msg(input, expected, returned_value)


def check_and_link_with(input: str, linked_lib: str, expected: int):
    os.system(f'./9cc "{input}" > tmp.s')
    lib_file = open("libtest.c", "w")
    lib_file.write(linked_lib)
    lib_file.close()
    os.system("cc -S -o libtest.s libtest.c")
    os.system("cc -o tmp tmp.s libtest.s")
    os.system("rm libtest.c libtest.s")
    returned_value = (os.system("./tmp") >> 8) & 0xff
    return msg(input, expected, returned_value)

assert check("main() { return 0; }", 0)

assert check("main() { return 42; }", 42)

assert check("main() { return 0+10+3; }", 0+10+3)

assert check("main() { return 111+10-42; }", 111+10-42)

assert check("main() { return    111   + 10 -     42; }", 111+10-42)

assert check("main() { return    0 +    10+    3; }",  0 +    10+    3)

assert check("main() { return 10*2; }", 10*2)

assert check("main() { return 10+1*2; }", 10+1*2)
assert check("main() { return 10+3*2+10-5; }", 10+3*2+10-5)

assert check("main() { return (10+3)*2+10-5; }", (10+3)*2+10-5)
assert check("main() { return (10+1)*2; }", (10+1)*2)


assert check("main() { return (10+1)/2; }", (10+1)//2)
assert check("main() { return (15+1)/2+3; }", (15+1)//2+3)
assert check("main() { return 10+1 /2/5; }", 10+1//2//5)

#unary
assert check("main() { return -10+1 /2/5+30; }", -10+1//2//5+30)
assert check("main() { return +10+1 /2/5; }", +10+1//2//5)
assert check("main() { return -2*-3; }", -2*-3)

#equality
assert check("main() { return 1==0; }", 0)
assert check("main() { return 1==1; }", 1)
assert check("main() { return 1==1+5; }", 0)
assert check("main() { return 1+(1+1==1+1); }",2)

assert check("main() { return 1!=0; }", 1)
assert check("main() { return 1!=1; }", 0)
assert check("main() { return 1!=1+5; }", 1)
assert check("main() { return 1+(1+1!=1+1); }",1)


#relational
assert check("main() { return 1>0; }", 1)
assert check("main() { return 1>1; }", 0)
assert check("main() { return 1<0; }", 0)
assert check("main() { return 1<1; }", 0)
assert check("main() { return 1>=0; }", 1)
assert check("main() { return 1>=1; }", 1)
assert check("main() { return 1<=0; }", 0)
assert check("main() { return 1<=1; }", 1)


#semicolon
assert check("main() { 1+1;return 5-2; }",3)

#variables
assert check("main() { a=3;return a; }",3)
assert check("main() { a=3;b=4;return a+b; }",7)

assert check("main() { ab=3;bd=4;return ab+bd; }",7)
assert check("main() { abz=3;bdz =4;return abz+bdz; }",7)

assert check("main() { return 1;return 2; }",1)
assert check("main() { return 1;return 2+3; }",1)
assert check("main() { a=0;if(1)a=1;return a; }",1)
assert check("main() { a=0;if(0)a=1;return a; }",0)

assert check("main() { a=1;if(a)a=5;return a; }",5)
assert check("main() { a=0;if(a)a=5;return a; }",0)

assert check("main() { a=1;if(a)return 5;return 10; }",5)
assert check("main() { a=0;if(a)return 5;return 10; }",10)

assert check("main() { a=0;if(a)return 5;a=1;if(a)return 3;return 10; }",3)
assert check("main() { a=0;while(a)return 1; return 3; }",3)
assert check("main() { a=0;while(a<5)a=a+1; return a; }",5)

assert check("main() { a=0;if(a)return 5;else a=10;return a; }",10)
assert check("main() { a=1;if(a)a=0;else return 10;return a; }",0)

assert check("main() { for(a=0;a<10;a=a+1)b=a;return b; }",9)
assert check("main() { for(;;)return 0; }",0)

#block
assert check("main() { { { { return 3; } } } }", 3)
assert check("main() { a = 3; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 3)
assert check("main() { a = 0; if (a) { b = 1; c = 2; } else { b = 5; c = 7; } return b + c; }", 12)
assert check("main() { a = 0; b = 0; c = 3; if (a) { } return c; }", 3)
assert check("main() { a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } return c; }", 3)
assert check("main() { a = 0; b = 0; c = 3; if (a) { if (b) { c = 2; } } else { c = 7; } return c; }", 7)
assert check("main() { a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; } else { c = 7; }} return c; }", 3)
assert check("main() { a = 0; b = 0; c = 3; if (a) if (b) { c = 2; } else { c = 7; } return c; }", 3)
assert check("main() { a = 0; b = 0; c = 3; if (a) {if (b) { c = 2; }} else { c = 7; } return c; }", 7)

#link with what's built in gcc
assert check_and_link_with(
    "main() { return identity(3); }", 
    linked_lib="int identity(int a) { return a; }",
    expected=3)

assert check_and_link_with(
    "main() { return three(); }", 
    linked_lib="int three() { return 3; }",
    expected=3)

assert check_and_link_with(
    "main() { return add(2, 3); }", 
    linked_lib="int add(int a, int b) { return a + b; }",
    expected=5)

assert check_and_link_with(
    "main() { return add6(1, 2, 3, 4, 5, 6); }", 
    linked_lib="int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }",
    expected=21)

assert check("three() { return 3; } main() { return three(); }", 3)
assert check("one() { return 1; } three() { return one() + 2; } main() { return three() + three(); }", 6)
assert check("identity(a) { return a; } main() { return identity(3); }", 3)
assert check("add2(a, b) { return a + b; } main() { return add2(1, 2); }", 3)
assert check("add6(a,b,c,d,e,f) { return a + b + c + d + e + f; } main() { return add6(1, 2, 3, 4, 5, 6); }", 21)
assert check("fib(n) { if (n <= 1) { return n; } return fib(n-1) + fib(n-2); } main() { return fib(8); }", 21)

print("OK")