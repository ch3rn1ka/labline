CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -g -Iprotocols
LDFLAGS = -lwayland-client -lm

SOURCES = labline.c \
	protocols/wlr-layer-shell-unstable-v1-protocol.c \
	protocols/xdg-shell-protocol.c \
	protocols/ext-workspace-v1-protocol.c

HEADERS = protocols/wlr-layer-shell-unstable-v1-client-protocol.h \
	protocols/xdg-shell-client-protocol.h \
	protocols/ext-workspace-v1-client-protocol.h

TARGET = labline

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
