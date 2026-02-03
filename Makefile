CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -g -Iprotocols -Wno-unused-parameter \
	$(shell pkg-config --cflags pango cairo pangocairo wayland-client)
LDFLAGS = $(shell pkg-config --libs pango cairo pangocairo wayland-client)

SOURCES = labline.c \
	protocols/wlr-layer-shell-unstable-v1-protocol.c \
	protocols/xdg-shell-protocol.c \
	protocols/ext-workspace-v1-protocol.c

HEADERS = protocols/wlr-layer-shell-unstable-v1-client-protocol.h \
	protocols/xdg-shell-client-protocol.h \
	protocols/ext-workspace-v1-client-protocol.h

labline: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f labline
