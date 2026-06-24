#ifndef BUFFER_H
#define BUFFER_H

#include <cairo/cairo.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pango/pangocairo.h>

#include "state.h"

struct buffer_context {
	void *buf;
	size_t buf_size;

	struct wl_buffer *wl_buffer;
	cairo_surface_t *cairo_surface;
	cairo_t *cairo_ctx;
	PangoLayout *pango_layout;
	PangoFontDescription *pango_font_desc;

	bool stale;
	bool busy;
};

void buffers_init(struct labline_state *state);

void buffer_realloc(struct buffer_context *buf_ctx,
	struct labline_state *state);

#endif	/* BUFFER_H */
