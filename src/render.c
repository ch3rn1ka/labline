#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <stdio.h>
#include <wayland-util.h>
#include <wayland-client.h>

#include "render.h"
#include "buffer.h"
#include "state.h"
#include "shm.h"
#include "util.h"
#include "wayland.h"

static int
draw_workspaces(struct buffer_context *buf_ctx, struct labline_state *state)
{
	pango_layout_set_width(buf_ctx->pango_layout, -1);

	struct workspace *ws;
	int ws_name_width, ws_name_height;
	double x_offset = 0;

	wl_list_for_each_reverse(ws, &state->workspaces, node) {
		struct face *current_face;
		if (ws->state == 1) {
			current_face = &state->faces.active_ws;
		} else {
			current_face = &state->faces.inactive_ws;
		}

		cairo_set_source_rgb(buf_ctx->cairo_ctx,
			current_face->bg.r,
			current_face->bg.g,
			current_face->bg.b);
		pango_layout_set_text(buf_ctx->pango_layout, ws->name, -1);
		pango_layout_get_pixel_size(buf_ctx->pango_layout,
			&ws_name_width, &ws_name_height);

		int box_width = ws_name_width + 8;
		cairo_rectangle(buf_ctx->cairo_ctx, x_offset, 0,
			box_width, state->height);
		cairo_fill(buf_ctx->cairo_ctx);

		cairo_set_source_rgb(buf_ctx->cairo_ctx,
			current_face->fg.r,
			current_face->fg.g,
			current_face->fg.b);
		cairo_move_to(buf_ctx->cairo_ctx, x_offset + 4,
			(state->height - ws_name_height) / 2.0);
		pango_cairo_show_layout(buf_ctx->cairo_ctx,
			buf_ctx->pango_layout);

		x_offset += box_width;
	}
	pango_layout_set_width(buf_ctx->pango_layout,
		state->width * PANGO_SCALE);

	return x_offset;
}

static int
draw_windows(struct buffer_context *buf_ctx, struct labline_state *state,
		int workspaces_offset)
{
	if (!state->active_toplevel) {
		return 0;
	}

	struct face *toplevel = &state->faces.toplevel;

	/* TODO: macro for passing rgb more easily */
	cairo_set_source_rgb(buf_ctx->cairo_ctx,
		toplevel->bg.r,
		toplevel->bg.g,
		toplevel->bg.b);

	int toplevel_title_width, toplevel_title_height;

	/* TODO: change wrapping limit */
	pango_layout_set_text(buf_ctx->pango_layout, state->active_toplevel->title, -1);
	pango_layout_get_pixel_size(buf_ctx->pango_layout,
		&toplevel_title_width, &toplevel_title_height);

	int box_width = toplevel_title_width + 8;
	cairo_rectangle(buf_ctx->cairo_ctx, workspaces_offset, 0,
		box_width, state->height);
	cairo_fill(buf_ctx->cairo_ctx);

	cairo_set_source_rgb(buf_ctx->cairo_ctx,
		toplevel->fg.r,
		toplevel->fg.g,
		toplevel->fg.b);
	cairo_move_to(buf_ctx->cairo_ctx, workspaces_offset + 4,
		(state->height - toplevel_title_height) / 2.0);
	pango_cairo_show_layout(buf_ctx->cairo_ctx,
		buf_ctx->pango_layout);

	return workspaces_offset + box_width;
}

static void
draw_status(struct buffer_context *buf_ctx, struct labline_state *state,
		uint32_t windows_offset)
{
	pango_layout_set_text(buf_ctx->pango_layout, state->statusline, -1);

	int text_width, text_height;
	pango_layout_get_pixel_size(buf_ctx->pango_layout, &text_width,
		&text_height);

	double x_offset = state->width - text_width;
	double y_offset = (state->height - text_height) / 2.0;
	cairo_set_source_rgb(buf_ctx->cairo_ctx,
		state->faces.status.fg.r,
		state->faces.status.fg.g,
		state->faces.status.fg.b);
	cairo_move_to(buf_ctx->cairo_ctx, x_offset, y_offset);
	pango_cairo_show_layout(buf_ctx->cairo_ctx, buf_ctx->pango_layout);
}

/* Draw the contents of the statusline with pango and cairo. */
void
draw_panel(struct buffer_context *buf_ctx, struct labline_state *state)
{
	cairo_set_source_rgb(buf_ctx->cairo_ctx,
		state->faces.status.bg.r,
		state->faces.status.bg.g,
		state->faces.status.bg.b);
	cairo_paint(buf_ctx->cairo_ctx);

	int workspaces_offset = draw_workspaces(buf_ctx, state);
	int windows_offset = draw_windows(buf_ctx, state, workspaces_offset);
	draw_status(buf_ctx, state, windows_offset);

	cairo_surface_flush(buf_ctx->cairo_surface);
}

/* Choose a buffer and draw in it, attach and commit */
void
render(struct labline_state *state)
{
	if (state->width == 0) {
		/* The surface is not initialized */
		return;
	}

	struct buffer_context *buf_ctx = state->buffer;

	if (!buf_ctx->busy) {
		if (buf_ctx->stale) {
			buffer_realloc(buf_ctx, state);
			wayland_buffer_add_listener(buf_ctx);
		}
		draw_panel(buf_ctx, state);
		wl_surface_attach(state->surface, buf_ctx->wl_buffer,
			0, 0);
		wl_surface_damage_buffer(state->surface, 0, 0,
			state->width, state->height);
		wl_surface_commit(state->surface);
		buf_ctx->busy = true;
	}
}
