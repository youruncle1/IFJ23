# Compiler
CC = gcc

# Source files and object files
SRCS = main.c error.c scanner.c
OBJS = $(SRCS:.c=.o)
EXEC = IFJ23

# Default target
all: $(EXEC)

# Rule to build the executable
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

# Rule to build object files from source files
%.o: %.c
	$(CC) -c $< -o $@

# Clean up object files and the executable
clean:
	powershell -command "Remove-Item -Path *.o, *.exe -ErrorAction SilentlyContinue"

.PHONY: all clean