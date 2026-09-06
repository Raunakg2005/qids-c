CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -O3 -fPIC -Iinclude
LDFLAGS ?= -lm

AR ?= ar
ARFLAGS = rcs

SRCS = src/qids.c
OBJS = $(SRCS:.c=.o)
TEST_SRCS = tests/test_qids.c
TEST_BIN = test_qids

STATIC_LIB = libqids.a
SHARED_LIB = libqids.so

all: $(STATIC_LIB) $(SHARED_LIB)

$(STATIC_LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SHARED_LIB): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c include/qids.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_SRCS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -L. -lqids $(LDFLAGS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(OBJS) $(STATIC_LIB) $(SHARED_LIB) $(TEST_BIN)

.PHONY: all test clean
