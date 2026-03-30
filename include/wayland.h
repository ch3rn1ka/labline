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

void wayland_init(struct state *state);

#endif
