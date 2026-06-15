#ifndef BUFFER_H
#define BUFFER_H

#include <cairo/cairo.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pango/pangocairo.h>

#include "state.h"

struct buffer_context {
	void *map;
	size_t map_size;
	int fd;

	struct wl_buffer *buf;
	cairo_surface_t *cairo_surface;
	cairo_t *cairo_ctx;
	PangoLayout *pango_layout;
	PangoFontDescription *pango_font_desc;

	bool stale;
	bool busy;
};

void buffers_init(struct state *state);
void buffer_realloc(struct buffer_context *buf_ctx, struct state *state);

#endif
