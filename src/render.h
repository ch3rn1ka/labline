#ifndef RENDER_H
#define RENDER_H

#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <stdbool.h>
#include <unistd.h>
#include <wayland-client.h>

#include "state.h"

struct buffer_context
{
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

void init_buffers(struct state *state);
bool realloc_buffer(struct buffer_context *buf_ctx, struct state *state);
void redraw_buffer(struct buffer_context *buf_ctx, struct state *state);
void render(struct state *state);

#endif
