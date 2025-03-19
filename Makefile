# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -Icommon -O2 -g -D_GNU_SOURCE

# Directories
CLIENT_DIR = client
SERVER_DIR = server
COMMON_DIR = common

# Source files
CLIENT_SRCS = $(CLIENT_DIR)/client.c $(CLIENT_DIR)/prog.c $(CLIENT_DIR)/tftp_client.c
SERVER_SRCS = $(SERVER_DIR)/server.c $(SERVER_DIR)/prog.c $(SERVER_DIR)/tftp_server.c
COMMON_SRCS = $(COMMON_DIR)/common.c

# Object files
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o) $(COMMON_SRCS:.c=.o)
SERVER_OBJS = $(SERVER_SRCS:.c=.o) $(COMMON_SRCS:.c=.o)

# Executables
CLIENT_EXEC = client/client  
SERVER_EXEC = server/server

# Default target: build both client and server
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Compile the client
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS) $(LDFLAGS)

# Compile the server
$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS) $(LDFLAGS)

# Generic rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	 rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC) *~ core

