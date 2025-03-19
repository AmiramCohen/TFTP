# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -I../common -D_GNU_SOURCE

# Source files
SRCS = client.c prog.c tftp_client.c ../common/common.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: client

# Compile the client
client: $(OBJS)
	$(CC) $(CFLAGS) -o client $(OBJS)

# Generic rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -f client $(OBJS)
