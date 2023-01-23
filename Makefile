
BIN:=iso-chooser-menu

URL:=http://cdimage.ubuntu.com/streams/v1/com.ubuntu.cdimage.daily:ubuntu-server.json

CFLAGS+=-Wall -Werror -Wfatal-errors -std=c11

# CFLAGS+=-g

CFLAGS+=$(shell pkg-config --cflags ncursesw)
LDFLAGS+=$(shell pkg-config --libs ncursesw)

CFLAGS+=$(shell pkg-config --cflags json-c)
LDFLAGS+=$(shell pkg-config --libs json-c)

SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)

TEST_SRCS:=$(wildcard test/*.c)
TEST_OBJS:=$(TEST_SRCS:.c=.o)

TEST_CFLAGS:=$(CFLAGS) $(shell pkg-config --cflags cmocka)
TEST_LDFLAGS:=$(LDFLAGS) $(shell pkg-config --libs cmocka)

default: new

new: clean build

ubuntu-server.json:
	wget "$(URL)" -O $@

clean:
	rm -f $(BIN) $(OBJS) $(TEST_OBJS) out.vars

distclean: clean
	rm -f ubuntu-server.json

build: $(OBJS)
	$(CC) -o $(BIN) $^ $(CFLAGS) $(LDFLAGS)

run: ubuntu-server.json
	./$(BIN) --input "$^" --output "out.vars"

.PHONY: test
test: runtests

runtests: json.o common.o $(TEST_OBJS)
	$(CC) -o $@ $^ $(TEST_CFLAGS) $(TEST_LDFLAGS)
	./$@
