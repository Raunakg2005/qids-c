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

# Link the archive by path: after `make`, libqids.so is also in ., so -lqids
# picked the shared library and ./test_qids then failed to load it at runtime.
$(TEST_BIN): $(TEST_SRCS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $< $(STATIC_LIB) $(LDFLAGS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

# The README promises C99: build the library and the tests as strict ISO C99
# with every warning an error, so a compiler extension cannot slip back in.
STRICT_CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -Iinclude

strict:
	$(CC) $(STRICT_CFLAGS) -c src/qids.c -o /dev/null
	$(CC) $(STRICT_CFLAGS) $(TEST_SRCS) $(SRCS) $(LDFLAGS) -o $(TEST_BIN)_strict
	./$(TEST_BIN)_strict
	rm -f $(TEST_BIN)_strict

clean:
	rm -f $(OBJS) $(STATIC_LIB) $(SHARED_LIB) $(TEST_BIN)

.PHONY: all test strict clean
