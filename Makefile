
BIN:=iso-chooser-menu

CFLAGS+=-Wall -Werror -Wfatal-errors -std=c11 -I.

# CFLAGS+=-g -O0

CFLAGS+=$(shell pkg-config --cflags ncursesw)
LDFLAGS+=$(shell pkg-config --libs ncursesw)

# This just adds an include path I don't want
# CFLAGS+=$(shell pkg-config --cflags json-c)
LDFLAGS+=$(shell pkg-config --libs json-c)

SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)

TEST_SRCS:=$(wildcard test/*.c)
TEST_PRGS:=$(TEST_SRCS:.c=.test)

TEST_CFLAGS:=$(CFLAGS) $(shell pkg-config --cflags cmocka)
TEST_LDFLAGS:=$(LDFLAGS) $(shell pkg-config --libs cmocka)

.PHONY: default new
default new: clean build

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJS) $(TEST_PRGS) out.vars

.PHONY: distclean
distclean: clean

.PHONY: build
build: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $^ $(CFLAGS) $(LDFLAGS)

.PHONY: run
run: $(BIN) ubuntu-server-cdimage.json ubuntu-server-releases.json
	./$(BIN) "out.vars" test/data/com*json

debug: $(BIN) ubuntu-server-cdimage.json
	gdb --args ./$(BIN) "out.vars" ubuntu-server-cdimage.json

.PHONY: test
test: clean $(OBJS) $(TEST_PRGS)

test/test_json.test: test/test_json.c
	$(CC) -o $@ $^ $(TEST_CFLAGS) $(TEST_LDFLAGS) json.o common.o
	./$@

test/test_args.test: test/test_args.c
	$(CC) -o $@ $^ $(TEST_CFLAGS) $(TEST_LDFLAGS) args.o common.o
	./$@
