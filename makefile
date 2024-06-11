CC=gcc
CFLAGS=-Wall -Werror -D_DEFAULT_SOURCE -std=c99

all: a3

a3: main.o
	$(CC) $(CFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $<

.PHONY: help
help: 
	@echo "all		Building the program"
	@echo "clean	Remove object files"
	@echo "help		Display help message"
clean:
	rm -f *.o printing_table