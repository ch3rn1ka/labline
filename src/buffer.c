#include <cairo/cairo.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pango/pangocairo.h>
#include <unistd.h>

#include "buffer.h"
#include "shm.h"
#include "state.h"
#include "util.h"

/*
 * Allocate two `buffer_context` structs and mark them as stale so that their
 * fields will get allocated during the next `prepare_buffer()` call.
 */
void
buffers_init(struct labline_state *state)
{
	for (int i = 0; i < 2; i++) {
		state->buffers[i] = calloc(1, sizeof(struct buffer_context));
		state->buffers[i]->stale = true;
	}
}

/*
 * Realloc all fields of the `buffer_context` struct. Called during
 * configure events to match the buffer dimensions to the new surface size
 * so that `render()` could be safely called after.
 */
void
buffer_realloc(struct buffer_context *buf_ctx, struct labline_state *state)
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
	if (buf_ctx->wl_buffer) {
		wl_buffer_destroy(buf_ctx->wl_buffer);
	}
	if (buf_ctx->buf) {
		munmap(buf_ctx->buf, buf_ctx->buf_size);
	}

	/* Buffer */
	buf_ctx->buf_size = state->height * state->stride;
	int fd = create_shm_file(buf_ctx->buf_size);

	void *buf = mmap(NULL, buf_ctx->buf_size,
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		die("Failed to run mmap()");
	}
	buf_ctx->buf = buf;

	struct wl_shm_pool *pool = wl_shm_create_pool(state->shm, fd,
		buf_ctx->buf_size);
	buf_ctx->wl_buffer = wl_shm_pool_create_buffer(pool, 0, state->width,
		state->height, state->stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);

	/* Pango/cairo globals */
	buf_ctx->cairo_surface =
		cairo_image_surface_create_for_data(buf, CAIRO_FORMAT_ARGB32,
			state->width, state->height, state->stride);
	buf_ctx->cairo_ctx = cairo_create(buf_ctx->cairo_surface);
	buf_ctx->pango_layout = pango_cairo_create_layout(buf_ctx->cairo_ctx);

	buf_ctx->pango_font_desc =
		pango_font_description_from_string(state->font);

	pango_layout_set_font_description(buf_ctx->pango_layout,
		buf_ctx->pango_font_desc);
	pango_layout_set_width(buf_ctx->pango_layout, state->width * PANGO_SCALE);

	buf_ctx->stale = false;
	buf_ctx->busy = false;
}
