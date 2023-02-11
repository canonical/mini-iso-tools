
BIN:=iso-chooser-menu

URL_CDIMAGE:=https://cdimage.ubuntu.com/streams/v1/com.ubuntu.cdimage.daily:ubuntu-server.json
URL_RELEASES:=https://releases.ubuntu.com/streams/v1/com.ubuntu.releases:ubuntu-server.json

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
TEST_OBJS:=$(TEST_SRCS:.c=.o)

TEST_CFLAGS:=$(CFLAGS) $(shell pkg-config --cflags cmocka)
TEST_LDFLAGS:=$(LDFLAGS) $(shell pkg-config --libs cmocka)

.PHONY: default new
default new: clean build

ubuntu-server-cdimage.json:
	wget "$(URL_CDIMAGE)" -O $@

ubuntu-server-releases.json:
	wget "$(URL_RELEASES)" -O $@

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJS) $(TEST_OBJS) test/runtests out.vars

.PHONY: distclean
distclean: clean
	rm -f ubuntu-server-cdimage.json ubuntu-server-releases.json

.PHONY: build
build: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $^ $(CFLAGS) $(LDFLAGS)

.PHONY: run
run: $(BIN) ubuntu-server-cdimage.json ubuntu-server-releases.json
	./$(BIN) "out.vars" $^

.PHONY: test
test: test/runtests

.PHONY: runtests
test/runtests: json.o common.o $(TEST_OBJS)
	$(CC) -o $@ $^ $(TEST_CFLAGS) $(TEST_LDFLAGS)
	./$@
