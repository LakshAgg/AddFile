# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Source files
SRCS = main.c Helper.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = AddFile

# Installation directory
INSTALL_DIR = /usr/local/bin

# Default target
all: $(TARGET)

# Rule to compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Install rule
install: $(TARGET)
	cp $(TARGET) $(INSTALL_DIR)

# Uninstall rule
uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)
