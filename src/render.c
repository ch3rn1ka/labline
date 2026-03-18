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

  PangoLayout *layout = pango_cairo_create_layout(ctx);
  pango_layout_set_text(layout, state->text, -1);

  PangoFontDescription *desc = pango_font_description_from_string("Monospace");
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  pango_layout_set_width(layout, state->width * PANGO_SCALE);
  pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);

  int text_width, text_height;
  pango_layout_get_pixel_size(layout, &text_width, &text_height);

  /* used to center the text vertically */
  double y_offset = (state->height - text_height) / 2.0;

  cairo_set_source_rgb(ctx, 1.0, 1.0, 1.0);
  /*
   * Even though we start drawing from (0, y_offset), the text gets shifted
   * by (state->width - text_width)px to the right since we aligned it earlier
   * with `pango_layout_set_alignment()`.
   */
  cairo_move_to(ctx, 0, y_offset);
  pango_cairo_show_layout(ctx, layout);

  g_object_unref(layout);
  cairo_destroy(ctx);
  cairo_surface_destroy(surface);
}
