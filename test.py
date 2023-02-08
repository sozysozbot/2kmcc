
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
    assembly = open("tmp.s", "w")
    msg = open("tmp_stderr.txt", "w")
    # to handle double-quotes correctly
    return subprocess.call(["./2kmcc", input], stdout=assembly, stderr=msg)


def run():
    f = open("tmp_stdout.txt", "w")
    return subprocess.call(["./tmp"], stdout=f)


def check(input: str, expected: int, expected_stdout: str = None):
    compiler_returns = compile(input)
    if compiler_returns != 0:
        print(f"{bcolors.FAIL}FAIL:check (compile error):{input=}{bcolors.ENDC}")
        return False
    os.system("cc -o tmp tmp.s -static")
    returned_value = run()
    actual_stdout = open("tmp_stdout.txt", "r").read()

    if expected != returned_value:
        print(f"{bcolors.FAIL}FAIL:check (wrong answer):{input=} {expected=} {returned_value=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False
    elif expected_stdout != None and actual_stdout != expected_stdout:
        print(f"{bcolors.FAIL}FAIL:check (correct answer but wrong stdout):{input=} {expected_stdout=} {actual_stdout=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False
    elif expected_stdout != None:
        print(
            f"{bcolors.OKGREEN}passed:{input=} {expected=} {expected_stdout=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s tmp_stdout.txt tmp_stderr.txt")
        return True
    else:
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s tmp_stdout.txt tmp_stderr.txt")
        return True

def check_and_link_with(input: str, linked_lib: str, expected: int, expected_stdout: str = None):
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
    returned_value = run()
    actual_stdout = open("tmp_stdout.txt", "r").read()

    if expected != returned_value:
        print(f"{bcolors.FAIL}FAIL:check (wrong answer):{input=} {expected=} {returned_value=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}  {linked_lib=} {bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False
    elif expected_stdout != None and actual_stdout != expected_stdout:
        print(f"{bcolors.FAIL}FAIL:check (correct answer but wrong stdout):{input=} {expected_stdout=} {actual_stdout=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}  {linked_lib=} {bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False
    elif expected_stdout != None:
        print(
            f"{bcolors.OKGREEN}passed:{input=} {expected=} {expected_stdout=} {bcolors.ENDC}")
        print(f"{bcolors.OKGREEN}       {linked_lib=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s tmp_stdout.txt tmp_stderr.txt")
        return True
    else:
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        print(f"{bcolors.OKGREEN}       {linked_lib=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s tmp_stdout.txt tmp_stderr.txt")
        return True

def should_not_compile(input: str, expected_stderr: str = None):
    compiler_returns = compile(input)
    actual_stderr = open("tmp_stderr.txt", "r").read()
    if compiler_returns != 0:
        print(
            f"{bcolors.OKGREEN}passed: should give compile error\n  {input=}{bcolors.ENDC}")
        if expected_stderr != None:
            if actual_stderr == expected_stderr + "\n":
                print(
                    f"{bcolors.OKGREEN}   msg: {actual_stderr}{bcolors.ENDC}")
            else:
                print(
                    f"{bcolors.OKCYAN}error message was not as expected:\n  {expected_stderr=}\n  {actual_stderr=}{bcolors.ENDC}")
        else:
            print(
                f"{bcolors.OKGREEN}error message was:{actual_stderr=}{bcolors.ENDC}")
        os.system("rm tmp.s tmp_stderr.txt")
        return True
    else:
        print(
            f"{bcolors.FAIL}FAIL: compiled what should not compile:{input=}{bcolors.ENDC}")
        return False