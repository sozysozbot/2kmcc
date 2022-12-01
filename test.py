
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

def check(input: str, expected: int):
    os.system(f'./9cc "{input}" > tmp.s')
    os.system("cc -o tmp tmp.s")
    returned_value = (os.system("./tmp") >> 8) & 0xff

    if expected == returned_value:
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        return True

    print(f"{bcolors.FAIL}FAIL:check:{input=} {expected=} {returned_value=}{bcolors.ENDC}")
    return False


assert check("return 0;", 0)

assert check("return 42;", 42)

assert check("return 0+10+3;", 0+10+3)

assert check("return 111+10-42;", 111+10-42)

assert check("return    111   + 10 -     42;", 111+10-42)

assert check("return    0 +    10+    3;",  0 +    10+    3)

assert check("return 10*2;", 10*2)

assert check("return 10+1*2;", 10+1*2)
assert check("return 10+3*2+10-5;", 10+3*2+10-5)

assert check("return (10+3)*2+10-5;", (10+3)*2+10-5)
assert check("return (10+1)*2;", (10+1)*2)


assert check("return (10+1)/2;", (10+1)//2)
assert check("return (15+1)/2+3;", (15+1)//2+3)
assert check("return 10+1 /2/5;", 10+1//2//5)

#unary
assert check("return -10+1 /2/5+30;", -10+1//2//5+30)
assert check("return +10+1 /2/5;", +10+1//2//5)
assert check("return -2*-3;", -2*-3)

#equality
assert check("return 1==0;", 0)
assert check("return 1==1;", 1)
assert check("return 1==1+5;", 0)
assert check("return 1+(1+1==1+1);",2)

assert check("return 1!=0;", 1)
assert check("return 1!=1;", 0)
assert check("return 1!=1+5;", 1)
assert check("return 1+(1+1!=1+1);",1)


#relational
assert check("return 1>0;", 1)
assert check("return 1>1;", 0)
assert check("return 1<0;", 0)
assert check("return 1<1;", 0)
assert check("return 1>=0;", 1)
assert check("return 1>=1;", 1)
assert check("return 1<=0;", 0)
assert check("return 1<=1;", 1)


#semicolon
assert check("1+1;return 5-2;",3);

#variables
assert check("a=3;return a;",3);
assert check("a=3;b=4;return a+b;",7);

assert check("ab=3;bd=4;return ab+bd;",7);
assert check("abz=3;bdz =4;return abz+bdz;",7);

assert check("return 1;return 2;",1);
assert check("return 1;return 2+3;",1);
assert check("a=1;if(a)return 5;return 10;",5);
assert check("a=0;if(a)return 5;return 10;",10);

print("OK")