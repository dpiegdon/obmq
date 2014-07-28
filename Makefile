.PHONY: clean all

all: example

example: example.c obmq.c obmq.h

clean:
	-rm *.o
	-rm example
