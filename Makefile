CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -g -Iinclude -Iinclude/protocols \
	-Wno-unused-parameter -D_POSIX_C_SOURCE=200809L
CFLAGS += $(shell pkg-config --cflags pango cairo pangocairo wayland-client)
LDFLAGS = $(shell pkg-config --libs pango cairo pangocairo wayland-client)

SOURCES = src/main.c \
	src/wayland.c \
	src/state.c \
	src/shm.c \
	src/render.c \
	src/protocols/wlr-layer-shell-unstable-v1-protocol.c \
	src/protocols/xdg-shell-protocol.c \
	src/protocols/ext-workspace-v1-protocol.c

HEADERS = include/wayland.h \
	include/state.h \
	include/shm.h \
	include/render.h \
	include/protocols/wlr-layer-shell-unstable-v1-client-protocol.h \
	include/protocols/xdg-shell-client-protocol.h \
	include/protocols/ext-workspace-v1-client-protocol.h

labline: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f labline
