# Spherepop — simple Makefile for quick builds (prefer CMake for full builds)

CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -O2 -Isrc
LDFLAGS = -lm

SRCS = src/main.c src/repl.c src/config.c \
       src/lexer/lexer.c src/lexer/token.c \
       src/parser/parser.c src/parser/ast.c \
       src/runtime/bubble.c src/runtime/history.c \
       src/runtime/constraints.c src/runtime/provenance.c \
       src/runtime/scope.c src/runtime/value.c \
       src/runtime/evaluator.c src/runtime/admissibility.c \
       src/runtime/collapse.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean test fmt

all: spherepop

spherepop: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: spherepop
	@echo "Running .sp test suite..."
	@for f in tests/integration/*.sp; do \
		echo "  $$f"; \
		./spherepop "$$f" || exit 1; \
	done
	@echo "All tests passed."

fmt:
	clang-format -i $(SRCS) $(wildcard src/**/*.h)

clean:
	rm -f $(OBJS) spherepop
