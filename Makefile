.PHONY: clean all

all: example

example: example.c obmq.c

clean:
	-rm *.o
	-rm example
