CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -Iinclude
LDFLAGS =

SRC = src/error.c src/ast.c src/iterator.c src/document.c src/parser.c src/extractor.c
OBJ = $(SRC:src/%.c=bin/%.o)
TARGET = bin/libmarkdown.a
TEST_BIN = bin/test

.PHONY: all clean test

all: | bin $(TARGET)

bin:
	mkdir -p bin

$(TARGET): $(OBJ)
	ar rcs $@ $(OBJ)

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_BIN)

test: $(TARGET)
	$(CC) $(CFLAGS) -o $(TEST_BIN) tests/test.c $(TARGET)
	$(TEST_BIN)
