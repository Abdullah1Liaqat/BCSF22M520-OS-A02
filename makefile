# ===============================
#  Makefile for OS Assignment 02
#  Author: Abdullah Liaqat (BCSF22M520)
# ===============================

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Files
TARGET = $(BIN_DIR)/ls
SRC = $(SRC_DIR)/ls-v1.0.0.c
OBJ = $(OBJ_DIR)/ls-v1.0.0.o

# Default rule
all: $(TARGET)

# Link the object file to create the binary
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile source into object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: all
	@echo "Running program..."
	./$(TARGET)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean run
