#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

#include "state.h"
#include "render.h"
#include "wayland.h"

/*
 * Allocate two `buffer_context` structs and mark them as stale so that their
 * fields will get allocated during the next `prepare_buffer()` call.
 */
static void
buffers_init(struct state *state)
{
	for (int i = 0; i < 2; i++) {
		state->buffers[i] = calloc(1, sizeof(struct buffer_context));
		state->buffers[i]->fd = -1;
		state->buffers[i]->stale = true;
	}
}

struct state *
state_init()
{
	struct state *state = calloc(1, sizeof(struct state));

	/*
	 * The width of the surface is left blank until `layer_surface_listener`
	 * catches a configure event from the compositor.
	 */
	state->width = 0;
	state->stride = 0;

	/* TODO: get height from the fontsize */
	state->height = 26;

	wl_list_init(&state->workspaces);
	buffers_init(state);
	wayland_init(state);
	return state;
}
