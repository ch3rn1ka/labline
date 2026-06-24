#ifndef WAYLAND_H
#define WAYLAND_H

#include "state.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

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

void wayland_buffer_add_listener(struct buffer_context *buf_ctx);

void wayland_init(struct labline_state *state);

#endif	/* WAYLAND_H */
