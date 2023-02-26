
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

def get_compiler_name():
    try:
        return open("tmp_which_compiler_to_test.txt", "r").read().strip()
    except:
        os.system("make 2kmcc")
        os.system('echo "2kmcc" > tmp_which_compiler_to_test.txt')
        return "2kmcc"

def compile_with_2kmcc(input: str, output_assembly_path: str = "tmp.s"):
    assembly = open(output_assembly_path, "w")
    compiler_name = get_compiler_name()
    ret = subprocess.call([f"./{compiler_name}", input], stdout=assembly)
    if ret != 0:
        result = open(output_assembly_path, "r").read()
        msg = result.split("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n", maxsplit=1)[1]
        open("tmp_compile_errmsg.txt", "w").write(msg)
    return ret

def run_resulting_binary(executable_path: str, stdin: str = None, stdout_path: str = "tmp_run_stdout.txt"):
    f = open(stdout_path, "w")
    if stdin == None:
        return subprocess.call([executable_path], stdout=f)
    else:
        return subprocess.call([executable_path, stdin], stdout=f)

def check(input: str, expected: int, stdin: str = None, expected_stdout: str = None):
    compiler_returns = compile_with_2kmcc(input)
    if compiler_returns != 0:
        print(f"{bcolors.FAIL}FAIL:check (compile error):{input=}{bcolors.ENDC}")
        msg = open("tmp_compile_errmsg.txt", "r").read()
        print(f"  The error message is:\n{bcolors.FAIL}{msg}{bcolors.ENDC}")
        return False
    os.system("cc -o tmp tmp.s -static")
    returned_value = run_resulting_binary("./tmp", stdin)
    actual_stdout = open("tmp_run_stdout.txt", "r").read()

    if expected != returned_value:
        print(f"{bcolors.FAIL}FAIL:check (wrong value returned):{bcolors.ENDC}")
        print(f"  {input=}")
        print(f"{bcolors.FAIL}  {expected=}\n  {returned_value=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False
    elif expected_stdout != None and actual_stdout != expected_stdout:
        print(f"{bcolors.FAIL}FAIL:check (correct value returned but wrong stdout):{bcolors.ENDC}")
        print(f"  {input=}")
        print(f"{bcolors.FAIL}  {expected_stdout=}\n  {actual_stdout=}{bcolors.ENDC}")
        print(f"{bcolors.FAIL}Consult tmp.s to find out what went wrong{bcolors.ENDC}")
        return False
    elif expected_stdout != None:
        print(
            f"{bcolors.OKGREEN}passed:{input=} {expected=} {expected_stdout=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s tmp_run_stdout.txt")
        return True
    else:
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s tmp_run_stdout.txt")
        return True

def check_and_link_with(input: str, linked_lib: str, expected: int, expected_stdout: str = None):
    compiler_returns = compile_with_2kmcc(input)
    if compiler_returns != 0:
        print(f"{bcolors.FAIL}FAIL:check (compile error):{input=}{bcolors.ENDC}")
        return False
    lib_file = open("libtest.c", "w")
    lib_file.write(linked_lib)
    lib_file.close()
    os.system("cc -S -o libtest.s libtest.c")
    os.system("cc -o tmp tmp.s libtest.s -static")
    os.system("rm libtest.c libtest.s")
    returned_value = run_resulting_binary("./tmp")
    actual_stdout = open("tmp_run_stdout.txt", "r").read()

    if expected != returned_value:
        print(f"{bcolors.FAIL}FAIL:check (wrong value returned):{input=} {expected=} {returned_value=}{bcolors.ENDC}")
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
        os.system("rm tmp tmp.s tmp_run_stdout.txt")
        return True
    else:
        print(f"{bcolors.OKGREEN}passed:{input=} {expected=} {bcolors.ENDC}")
        print(f"{bcolors.OKGREEN}       {linked_lib=} {bcolors.ENDC}")
        os.system("rm tmp tmp.s tmp_run_stdout.txt")
        return True

def should_not_compile(input: str, expected_errmsg: str = None):
    compiler_returns = compile_with_2kmcc(input)
    if compiler_returns != 0:
        actual_errmsg = open("tmp_compile_errmsg.txt", "r").read()
        print(
            f"{bcolors.OKGREEN}passed: should give compile error\n  {input=}{bcolors.ENDC}")
        if expected_errmsg != None:
            if actual_errmsg == "! " + expected_errmsg + "\n":
                print(
                    f"{bcolors.OKGREEN}   msg: {actual_errmsg}{bcolors.ENDC}")
            else:
                print(
                    f"{bcolors.OKCYAN}error message was not as expected:\n  {expected_errmsg=}\n  {actual_errmsg=}{bcolors.ENDC}")
        else:
            print(
                f"{bcolors.OKGREEN}error message was:\n```\n{actual_errmsg}```{bcolors.ENDC}")
        os.system("rm tmp.s tmp_compile_errmsg.txt")
        return True
    else:
        print(
            f"{bcolors.FAIL}FAIL: compiled what should not compile:{input=}{bcolors.ENDC}")
        return False