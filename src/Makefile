# Define the compiler
CC=gcc

# Define any compile-time flags
CFLAGS=-Wall -g -Wextra -pedantic

# Define the object files
OBJ=main.o scanner.o string.o error.o symtable.o expr.o parser.o instructions.o generator.o

# Define the executable file
EXEC=ifj23

# Linking rule
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation rule
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Include dependencies for each .o file on its .c and any .h files
main.o: main.c scanner.h string.h error.h symtable.h expr.h parser.h generator.h
scanner.o: scanner.c scanner.h
string.o: string.c string.h
error.o: error.c error.h
symtable.o: symtable.c symtable.h
expr.o: expr.c expr.h
parser.o: parser.c parser.h
generator.o: generator.c generator.h
instructions.0: instructions.c instructions.h

# Clean rule
clean:
	rm -f $(OBJ) $(EXEC)
