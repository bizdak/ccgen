
CC=gcc
CFLAGS=
OBJ=ccgen.o

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $< 

ccgen: $(OBJ)
	$(CC) -o ccgen ccgen.o 

.PHONY: clean

clean:
	rm -f *.o ccgen

