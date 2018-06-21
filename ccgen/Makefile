
CC=gcc
CFLAGS=-I.
OBJ=ccgen.o getopt.o

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $< 

ccgen: $(OBJ)
	$(CC) -o ccgen ccgen.o getopt.o

.PHONY: clean

clean:
	rm -f *.o ccgen

