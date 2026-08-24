CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -g -Iinclude -Iinclude/protocols \
	-Wno-unused-parameter -D_POSIX_C_SOURCE=200809L
CFLAGS += $(shell pkg-config --cflags pango cairo pangocairo wayland-client)
LDFLAGS = $(shell pkg-config --libs pango cairo pangocairo wayland-client)
SOURCES = $(shell find src/ -name '*.c')

labline: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

install: labline
	sudo cp ./labline /usr/local/bin/labline

uninstall:
	sudo rm -f /usr/local/bin/labline

clean: uninstall
	rm -f labline
