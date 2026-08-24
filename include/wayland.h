/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef WAYLAND_H
#define WAYLAND_H

#include "state.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif	/* MIN */

struct workspace {
	struct ext_workspace_handle_v1 *handle;
	char *name;
	uint32_t state;
	struct wl_list node;
};

struct workspace_callback_data {
	struct labline_state *state;
	struct workspace *workspace;
};

struct toplevel {
	struct zwlr_foreign_toplevel_handle_v1 *handle;
	char *title;
};

struct toplevel_callback_data {
	struct labline_state *state;
	struct toplevel *toplevel;
};

void wayland_buffer_add_listener(struct buffer_context *buf_ctx);

void wayland_init(struct labline_state *state);

#endif	/* WAYLAND_H */
