CFLAGS=-std=c11 -g -static -Wall -Wextra -Wno-builtin-declaration-mismatch
SRCS=main.c
OBJS=$(SRCS:.c=.o)

2kmcc: $(OBJS)
		$(CC) -o 2kmcc $(OBJS) $(LDFLAGS)

$(OBJS): 

2ndgen: 2kmcc
	cat main.c | xargs -0 -I XX ./2kmcc XX > 2kmcc_2ndgen.s
	$(CC) -o 2kmcc_2ndgen 2kmcc_2ndgen.s -static $(LDFLAGS)

test: 2kmcc
		python3 test_compilerbook.py
		python3 test_correctly_accepts.py 
		python3 test_correctly_rejects.py

embarrass: 2kmcc
		python3 test_miscompiles.py 
		python3 test_incorrectly_accepts.py 
		python3 test_incorrectly_rejects.py

clean:
		rm -f 2kmcc *.o *~ tmp*

.PHONY: test clean
