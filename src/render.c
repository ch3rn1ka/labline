#include <pango/pangocairo.h>
#include <cairo/cairo.h>

#include "render.h"
#include "state.h"

/*
 * Draw the contents of the statusline with pango and cairo.
 */
void
render(void *buffer, struct state *state)
{
  cairo_surface_t *surface =
    cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32,
                                        state->width, state->height,
                                        state->stride);
  cairo_t *ctx = cairo_create(surface);
  cairo_set_source_rgb(ctx, 0.0, 0.0, 0.0);
  cairo_paint_with_alpha(ctx, 1.0);
}
