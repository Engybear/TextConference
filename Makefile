# Makefile for compiling server.c and client.c

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Source files and output executables
SRC_DIR = .
SERVER_SRC = server.c
DELIVER_SRC = client.c
SERVER_EXEC = server
DELIVER_EXEC = client

# Targets and rules
all: $(SERVER_EXEC) $(DELIVER_EXEC)

$(SERVER_EXEC): $(SERVER_SRC)
	$(CC) -o $@ $<

$(DELIVER_EXEC): $(DELIVER_SRC)
	$(CC) -o $@ $<

clean:
	rm -f $(SERVER_EXEC) $(DELIVER_EXEC)

.PHONY: all clean
