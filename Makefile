CFLAGS=-std=c11 -g -static -Wall -Wextra
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

2kmcc: $(OBJS)
		$(CC) -o 2kmcc $(OBJS) $(LDFLAGS)

$(OBJS): 

test: 2kmcc
		python3 test_correctly_accepts.py 
		python3 test_correctly_rejects.py 

clean:
		rm -f 2kmcc *.o *~ tmp*

.PHONY: test clean
