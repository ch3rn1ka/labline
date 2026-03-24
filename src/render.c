#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <sys/mman.h>
#include <wayland-client.h>

#include "render.h"
#include "state.h"
#include "shm.h"

static void
buffer_release(void *data, struct wl_buffer *buf)
{
	struct buffer_context *buf_ctx = data;
	buf_ctx->busy = false;
}

static const struct wl_buffer_listener buffer_listener = {
	.release = buffer_release
};

/*
 * Realloc all fields of the `buffer_context` struct. Called during
 * configure events to match the buffer dimensions to the new surface size
 * so that `render()` could be safely called after.
 */
static void
buffer_realloc(struct buffer_context *buf_ctx, struct state *state)
{
	if (buf_ctx->pango_layout) {
		g_object_unref(buf_ctx->pango_layout);
	}
	if (buf_ctx->pango_font_desc) {
		pango_font_description_free(buf_ctx->pango_font_desc);
	}
	if (buf_ctx->cairo_ctx) {
		cairo_destroy(buf_ctx->cairo_ctx);
	}
	if (buf_ctx->cairo_surface) {
		cairo_surface_destroy(buf_ctx->cairo_surface);
	}
	if (buf_ctx->buf) {
		wl_buffer_destroy(buf_ctx->buf);
	}
	if (buf_ctx->map) {
		munmap(buf_ctx->map, buf_ctx->map_size);
	}
	if (buf_ctx->fd >= 0) {
		close(buf_ctx->fd);
	}

	buf_ctx->map_size = state->height * state->stride;
	int fd = create_shm_file(buf_ctx->map_size);
	/* TODO: handle errors */

	void *map = mmap(NULL, buf_ctx->map_size,
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	/* TODO: handle errors */
	buf_ctx->map = map;

	struct wl_shm_pool *pool = wl_shm_create_pool(state->shm, fd,
		buf_ctx->map_size);
	buf_ctx->buf = wl_shm_pool_create_buffer(pool, 0, state->width,
		state->height, state->stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);

	buf_ctx->cairo_surface =
		cairo_image_surface_create_for_data(map, CAIRO_FORMAT_ARGB32,
			state->width, state->height, state->stride);
	buf_ctx->cairo_ctx = cairo_create(buf_ctx->cairo_surface);
	buf_ctx->pango_layout = pango_cairo_create_layout(buf_ctx->cairo_ctx);

	/* TODO: custom fonts */
	buf_ctx->pango_font_desc =
		pango_font_description_from_string("Monospace 10");

	pango_layout_set_font_description(buf_ctx->pango_layout,
		buf_ctx->pango_font_desc);
	pango_layout_set_width(buf_ctx->pango_layout, state->width * PANGO_SCALE);
	pango_layout_set_alignment(buf_ctx->pango_layout, PANGO_ALIGN_RIGHT);

	buf_ctx->fd = fd;
	buf_ctx->stale = false;
	buf_ctx->busy = false;
}

/* Draw the contents of the statusline with pango and cairo. */
void
buffer_redraw(struct buffer_context *buf_ctx, struct state *state)
{
	cairo_set_source_rgb(buf_ctx->cairo_ctx, 0.0, 0.0, 0.0);
	cairo_paint_with_alpha(buf_ctx->cairo_ctx, 1.0);

	pango_layout_set_text(buf_ctx->pango_layout, state->text, -1);

	int text_width, text_height;
	pango_layout_get_pixel_size(buf_ctx->pango_layout,
		&text_width, &text_height);

	double y_offset = (state->height - text_height) / 2.0;
	cairo_set_source_rgb(buf_ctx->cairo_ctx, 1.0, 1.0, 1.0);
	cairo_move_to(buf_ctx->cairo_ctx, 0, y_offset);
	pango_cairo_show_layout(buf_ctx->cairo_ctx, buf_ctx->pango_layout);
	cairo_surface_flush(buf_ctx->cairo_surface);
}

/* Choose a buffer and draw in it, attach and commit */
void
render(struct state *state)
{
	for (int i = 0; i < 2; ++i)
	{
		struct buffer_context *buf_ctx = state->buffers[i];

		if (!buf_ctx->busy)
		{
			if (buf_ctx->stale) {
				buffer_realloc(buf_ctx, state);
				wl_buffer_add_listener(buf_ctx->buf,
					&buffer_listener, buf_ctx);
			}
			buffer_redraw(buf_ctx, state);
			wl_surface_attach(state->surface, buf_ctx->buf, 0, 0);
			wl_surface_damage_buffer(state->surface, 0, 0,
				state->width, state->height);
			wl_surface_commit(state->surface);
			buf_ctx->busy = true;
			break;
		}
	}
}
