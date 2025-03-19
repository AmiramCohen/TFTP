# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I../common -D_GNU_SOURCE

# Source files
SRCS = server.c prog.c tftp_server.c ../common/common.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: server

# Compile the client
server: $(OBJS)
	$(CC) $(CFLAGS) -o server $(OBJS)

# Generic rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -f server *.o