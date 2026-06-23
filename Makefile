CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -g -Iinclude -Iinclude/protocols \
	-Wno-unused-parameter -D_POSIX_C_SOURCE=200809L
CFLAGS += $(shell pkg-config --cflags pango cairo pangocairo wayland-client)
LDFLAGS = $(shell pkg-config --libs pango cairo pangocairo wayland-client)

SOURCES = src/buffer.c \
	src/main.c \
	src/render.c \
	src/shm.c \
	src/state.c \
	src/util.c \
	src/wayland.c \
	src/protocols/ext-workspace-v1-protocol.c \
	src/protocols/wlr-layer-shell-unstable-v1-protocol.c \
	src/protocols/xdg-shell-protocol.c

HEADERS = include/buffer.h \
	include/render.h \
	include/shm.h \
	include/state.h \
	include/util.h \
	include/wayland.h \
	include/protocols/ext-workspace-v1-client-protocol.h \
	include/protocols/wlr-layer-shell-unstable-v1-client-protocol.h \
	include/protocols/xdg-shell-client-protocol.h

labline: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f labline
