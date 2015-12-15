CC = clang
CFLAGS = -g

SRCS = lib/linkedlist.o main.c lib/talloc.o lib/tokenizer.o lib/parser.o interpreter.c
HDRS = linkedlist.h value.h talloc.h tokenizer.h parser.h interpreter.h
OBJS = $(SRCS:.c=.o)

interpreter: $(OBJS)
	$(CC)  $(CFLAGS) $^  -o $@

%.o : %.c $(HDRS)
	$(CC)  $(CFLAGS) -c $<  -o $@

clean:
	rm *.o
	rm interpreter

