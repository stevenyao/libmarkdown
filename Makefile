CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -Iinclude
LDFLAGS =

SRC = src/error.c src/ast.c src/iterator.c src/document.c src/parser.c src/extractor.c
OBJ = $(SRC:src/%.c=bin/%.o)
TARGET = bin/libmarkdown.a
TEST_BIN = bin/test
EXAMPLE_BIN = bin/example

COV_OBJ = $(SRC:src/%.c=bin/cov/%.o)
COV_TARGET = bin/cov/libmarkdown.a
COV_TEST_BIN = bin/cov/test

.PHONY: all clean test example coverage

all: | bin $(TARGET) example

bin:
	mkdir -p bin

bin/cov:
	mkdir -p bin/cov

$(TARGET): $(OBJ)
	ar rcs $@ $(OBJ)

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf bin/cov
	rm -f $(OBJ) $(TARGET) $(TEST_BIN) $(EXAMPLE_BIN) bin/*.gcda bin/*.gcno bin/*.gcov

test: $(TARGET)
	$(CC) $(CFLAGS) -o $(TEST_BIN) tests/test.c $(TARGET)
	$(TEST_BIN)

example: $(TARGET)
	$(CC) $(CFLAGS) -o $(EXAMPLE_BIN) examples/example.c $(TARGET)
	@echo "Example program built: $(EXAMPLE_BIN)"
	@echo "Run with: ./$(EXAMPLE_BIN)"

coverage: bin/cov $(COV_TARGET)
	$(CC) -Wall -Wextra -std=c11 -O0 -Iinclude --coverage -o $(COV_TEST_BIN) tests/test.c $(COV_TARGET) --coverage
	$(COV_TEST_BIN)
	@echo ""
	@echo "=== libmarkdown Code Coverage Report ==="
	@cd bin/cov && \
		total_lines=$$(for f in error.c ast.c iterator.c document.c parser.c extractor.c; do \
			gcov $$f 2>/dev/null | grep -o "of [0-9]*" | head -1 | sed 's/of //'; \
		done | awk '{s+=$$1} END {print s}') && \
		executed_lines=$$(for f in error.c ast.c iterator.c document.c parser.c extractor.c; do \
			percent=$$(gcov $$f 2>/dev/null | grep -o "[0-9]*\.[0-9]*%" | head -1 | sed 's/%//'); \
			lines=$$(gcov $$f 2>/dev/null | grep -o "of [0-9]*" | head -1 | sed 's/of //'); \
			echo "$$percent $$lines" | awk '{print $$1 * $$2 / 100}'; \
		done | awk '{s+=$$1} END {printf "%.2f", s}') && \
		coverage_percent=$$(echo "scale=2; $$executed_lines * 100 / $$total_lines" | bc) && \
		echo "Total lines in source files: $$total_lines" && \
		echo "Total lines executed: $$coverage_percent%" && \
		echo "" && \
		echo "File-by-file coverage:" && \
		echo "----------------------" && \
		for f in error.c ast.c iterator.c document.c parser.c extractor.c; do \
			echo -n "$$f: "; \
			gcov $$f 2>/dev/null | grep "Lines executed" | head -1 | sed 's/Lines executed://'; \
		done

$(COV_TARGET): $(COV_OBJ) | bin/cov
	ar rcs $@ $(COV_OBJ)

bin/cov/error.o: src/error.c
	$(CC) -Wall -Wextra -std=c11 -O0 -Iinclude --coverage -c $< -o $@

bin/cov/ast.o: src/ast.c
	$(CC) -Wall -Wextra -std=c11 -O0 -Iinclude --coverage -c $< -o $@

bin/cov/iterator.o: src/iterator.c
	$(CC) -Wall -Wextra -std=c11 -O0 -Iinclude --coverage -c $< -o $@

bin/cov/document.o: src/document.c
	$(CC) -Wall -Wextra -std=c11 -O0 -Iinclude --coverage -c $< -o $@

bin/cov/parser.o: src/parser.c
	$(CC) -Wall -Wextra -std=c11 -O0 -Iinclude --coverage -c $< -o $@

bin/cov/extractor.o: src/extractor.c
	$(CC) -Wall -Wextra -std=c11 -O0 -Iinclude --coverage -c $< -o $@
