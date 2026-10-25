CC ?= cc
CFLAGS ?= -O2
WARNINGS = -std=c11 -Wall -Wextra -Werror
OUT = build-native

.PHONY: all test
all: $(OUT)/hidctl

$(OUT):
	mkdir -p $@

$(OUT)/hidctl: hidctl.c protocol.c protocol.h | $(OUT)
	$(CC) $(CFLAGS) $(WARNINGS) hidctl.c protocol.c -o $@

$(OUT)/test_protocol: test_protocol.c protocol.c protocol.h | $(OUT)
	$(CC) $(CFLAGS) $(WARNINGS) test_protocol.c protocol.c -o $@

$(OUT)/test_hidctl: test_hidctl.c | $(OUT)
	$(CC) $(CFLAGS) $(WARNINGS) test_hidctl.c -o $@

test: $(OUT)/test_protocol $(OUT)/test_hidctl $(OUT)/hidctl
	./$(OUT)/test_protocol
	./$(OUT)/test_hidctl
