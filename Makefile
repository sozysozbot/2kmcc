CFLAGS=-std=c11 -g -static -Wall -Wextra -Wno-builtin-declaration-mismatch
SRCS=main.c
OBJS=$(SRCS:.c=.o)

2kmcc: $(OBJS)
		$(CC) -o 2kmcc $(OBJS) $(LDFLAGS)

$(OBJS): 

2kmcc_2ndgen: 2kmcc
	cat main.c | xargs -0 -I XX ./2kmcc XX > 2kmcc_2ndgen.s
	$(CC) -o 2kmcc_2ndgen 2kmcc_2ndgen.s -static $(LDFLAGS)

2kmcc_3rdgen: 2kmcc_2ndgen
	cat main.c | xargs -0 -I XX ./2kmcc_2ndgen XX > 2kmcc_3rdgen.s
	$(CC) -o 2kmcc_3rdgen 2kmcc_3rdgen.s -static $(LDFLAGS)
	diff 2kmcc_2ndgen.s 2kmcc_3rdgen.s

test: 2kmcc
		echo "2kmcc" > tmp_which_compiler_to_test.txt
		python3 test_compilerbook.py
		python3 test_correctly_accepts.py 
		python3 test_correctly_rejects.py
		rm tmp_which_compiler_to_test.txt

test_2ndgen: 2kmcc_2ndgen
		echo "2kmcc_2ndgen" > tmp_which_compiler_to_test.txt
		python3 test_compilerbook.py
		python3 test_correctly_accepts.py 
		python3 test_correctly_rejects.py
		rm tmp_which_compiler_to_test.txt

embarrass: 2kmcc
		echo "2kmcc" > tmp_which_compiler_to_test.txt
		python3 test_miscompiles.py 
		python3 test_incorrectly_rejects.py
		python3 test_incorrectly_accepts.py 
		rm tmp_which_compiler_to_test.txt

clean:
		rm -f 2kmcc *.o *~ tmp*

.PHONY: test clean
