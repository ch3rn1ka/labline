#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

#include "wayland.h"
#include "render.h"
#include "shm.h"
#include "state.h"
#include "util.h"

#include "ext-workspace-v1-client-protocol.h"
#include "wlr-foreign-management-unstable-v1-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static void *
_bind_global(struct wl_registry *wl_registry, uint32_t iface_id,
		const struct wl_interface *iface, uint32_t server_iface_version)
{
	uint32_t library_iface_version = iface->version;
	uint32_t bind_version =
		MIN(server_iface_version, library_iface_version);

	return wl_registry_bind(wl_registry, iface_id, iface, bind_version);
}

#define bind_global(interface) \
	_bind_global(wl_registry, iface_id, &(interface), version)

static void
registry_global(void *data, struct wl_registry *wl_registry,
		uint32_t iface_id, const char *iface_name,
		uint32_t version)
{
	struct labline_state *state = data;

	if (strcmp(iface_name, wl_compositor_interface.name) == 0) {
		state->compositor = bind_global(wl_compositor_interface);
	} else if (strcmp(iface_name, wl_shm_interface.name) == 0) {
		state->shm = bind_global(wl_shm_interface);
	} else if (strcmp(iface_name,
			zwlr_layer_shell_v1_interface.name) == 0) {
		state->layer_shell = bind_global(zwlr_layer_shell_v1_interface);
	} else if (strcmp(iface_name,
			ext_workspace_manager_v1_interface.name) == 0) {
		state->workspace_manager =
			bind_global(ext_workspace_manager_v1_interface);
	} else if (strcmp(iface_name,
			zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
		state->toplevel_manager =
			bind_global(zwlr_foreign_toplevel_manager_v1_interface);
	}
}

#undef bind_global

static void
registry_global_remove() {}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove
};

static void
workspace_handle_name(void *data, struct ext_workspace_handle_v1 *handle,
		const char *name)
{
	struct workspace_callback_data *callback_data = data;
	struct workspace *workspace = callback_data->workspace;

	if (workspace->name) {
		free(workspace->name);
	}
	workspace->name = strdup(name);
}

static void
workspace_handle_state(void *data, struct ext_workspace_handle_v1 *handle,
		uint32_t workspace_state)
{
	struct workspace_callback_data *callback_data = data;
	struct workspace *workspace = callback_data->workspace;
	workspace->state = workspace_state;

	/*
	 * Disambiguation: "state" as in "program state" vs. the "state" event
	 * for ws handles
	 */
	struct labline_state *labline_state = callback_data->state;
	labline_state->needs_render = true;
}

static void
workspace_handle_id() {}

static void
workspace_handle_coordinates() {}

static void
workspace_handle_capabilities() {}

static void
workspace_handle_removed(void *data, struct ext_workspace_handle_v1 *handle)
{
	struct workspace_callback_data *callback_data = data;
	struct workspace *workspace = callback_data->workspace;

	wl_list_remove(&workspace->node);
	ext_workspace_handle_v1_destroy(handle);
}

static const struct ext_workspace_handle_v1_listener
workspace_handle_listener = {
	.id = workspace_handle_id,
	.name = workspace_handle_name,
	.coordinates = workspace_handle_coordinates,
	.state = workspace_handle_state,
	.capabilities = workspace_handle_capabilities,
	.removed = workspace_handle_removed
};

static void
workspace_manager_workspace(void *data,
		struct ext_workspace_manager_v1 *manager,
		struct ext_workspace_handle_v1 *handle)
{
	struct labline_state *state = data;
	struct workspace *new_workspace = calloc(1, sizeof(struct workspace));
	new_workspace->handle = handle;
	wl_list_insert(&state->workspaces, &new_workspace->node);

	struct workspace_callback_data *callback_data =
		calloc(1, sizeof(struct workspace_callback_data));
	callback_data->workspace = new_workspace;
	callback_data->state = state;

	ext_workspace_handle_v1_add_listener(handle,
		&workspace_handle_listener, callback_data);
}

/*
 * We don't need to track workspace groups since there's only one in Labwc.
 * Will probably need to do it anyway for compatibility with other compositors.
 */
static void
workspace_manager_group() {}

static void
workspace_manager_done() {}

static void
workspace_manager_finished() {}

static const struct ext_workspace_manager_v1_listener
workspace_manager_listener = {
	.workspace = workspace_manager_workspace,
	.workspace_group = workspace_manager_group,
	.done = workspace_manager_done,
	.finished = workspace_manager_finished
};

static void
layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface,
		uint32_t serial, uint32_t width, uint32_t height)
{
	struct labline_state *state = data;

	if (width != state->width || height != state->height) {
		state->width = width;
		state->stride = width * 4;
		state->height = height;

		state->buffer->stale = true;
	}

	zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
	state->needs_render = true;
}

static void
layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *layer_surface)
{
	/* TODO: deallocate stuff */
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed
};

static void
toplevel_handle_title(void *data,
		struct zwlr_foreign_toplevel_handle_v1 *handle,
		const char *title)
{
	struct toplevel_callback_data *callback_data = data;
	struct toplevel *toplevel = callback_data->toplevel;

	if (toplevel->title) {
		free(toplevel->title);
	}
	toplevel->title = strdup(title);

	callback_data->state->needs_render = true;
}

static void toplevel_handle_app_id() {}

static void toplevel_handle_output_enter() {}

static void toplevel_handle_output_leave() {}

static void toplevel_handle_state(void *data,
		struct zwlr_foreign_toplevel_handle_v1 *handle,
		struct wl_array *state)
{
	struct toplevel_callback_data *callback_data = data;
	struct labline_state *labline_state = callback_data->state;
	struct toplevel *this_toplevel = callback_data->toplevel;

	bool activated = false;
	uint32_t *state_elem;
	wl_array_for_each(state_elem, state) {
		switch(*state_elem) {
			/* case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED: */
			/* case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED: */
			/* case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN: */
			/* 	break; */
			case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED:
				labline_state->active_toplevel = this_toplevel;
				activated = true;
				break;
		}
	}

	if (!activated && labline_state->active_toplevel == this_toplevel) {
		/* Previously active toplevel became inactive */
		labline_state->active_toplevel = NULL;
	}

	labline_state->needs_render = true;
}

static void toplevel_handle_done() {}

static void toplevel_handle_closed() {}

static void toplevel_handle_parent() {}

static const struct zwlr_foreign_toplevel_handle_v1_listener
toplevel_handle_listener = {
	.title = toplevel_handle_title,
	.app_id = toplevel_handle_app_id,
	.output_enter = toplevel_handle_output_enter,
	.output_leave = toplevel_handle_output_leave,
	.state = toplevel_handle_state,
	.done = toplevel_handle_done,
	.closed = toplevel_handle_closed,
	.parent = toplevel_handle_parent
};

static void
toplevel_manager_toplevel(void *data,
		struct zwlr_foreign_toplevel_manager_v1 *manager,
		struct zwlr_foreign_toplevel_handle_v1 *handle)
{
	struct labline_state *state = data;
	struct toplevel *new_toplevel = calloc(1, sizeof(struct toplevel));
	new_toplevel->handle = handle;

	struct toplevel_callback_data *callback_data =
		calloc(1, sizeof(struct toplevel_callback_data));
	callback_data->toplevel = new_toplevel;
	callback_data->state = state;

	zwlr_foreign_toplevel_handle_v1_add_listener(handle,
		&toplevel_handle_listener, callback_data);
}

static void
toplevel_manager_finished()
{
	/* TODO: implement this */
}

static const struct zwlr_foreign_toplevel_manager_v1_listener
toplevel_manager_listener = {
	.toplevel = toplevel_manager_toplevel,
	.finished = toplevel_manager_finished
};

static void
buffer_release(void *data, struct wl_buffer *buf)
{
	struct buffer_context *buf_ctx = data;
	buf_ctx->busy = false;
}

static const struct wl_buffer_listener buffer_listener = {
	.release = buffer_release
};

void
wayland_buffer_add_listener(struct buffer_context *buf_ctx)
{
	wl_buffer_add_listener(buf_ctx->wl_buffer, &buffer_listener, buf_ctx);
}

void
wayland_init(struct labline_state *state)
{
	/* Display */
	state->display = wl_display_connect(NULL);
	if (!state->display) {
		die("Failed to connect to display");
	}

	/* Registry */
	state->registry = wl_display_get_registry(state->display);
	if (!state->registry) {
		die("Failed to get registry");
	}
	wl_registry_add_listener(state->registry, &registry_listener, state);
	wl_display_roundtrip(state->display);

	/* Layer surface */
	state->surface = wl_compositor_create_surface(state->compositor);
	state->layer_surface =
		zwlr_layer_shell_v1_get_layer_surface(state->layer_shell,
			state->surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_TOP,
			"labline");
	zwlr_layer_surface_v1_set_size(state->layer_surface, 0, state->height);
	zwlr_layer_surface_v1_set_anchor(state->layer_surface, state->anchor);
	zwlr_layer_surface_v1_set_exclusive_zone(state->layer_surface,
		state->height);
	zwlr_layer_surface_v1_add_listener(state->layer_surface,
		&layer_surface_listener, state);

	/* Workspace manager */
	if (!state->workspace_manager) {
		die("Workspace manager not supported by the compositor");
	}
	ext_workspace_manager_v1_add_listener(state->workspace_manager,
		&workspace_manager_listener, state);

	wl_list_init(&state->workspaces);

	/* Toplevel manager */
	if (!state->toplevel_manager) {
		die("Toplevel manager not supported by the compositor");
	}
	zwlr_foreign_toplevel_manager_v1_add_listener(state->toplevel_manager,
		&toplevel_manager_listener, state);

	state->active_toplevel = NULL;

	wl_display_roundtrip(state->display);
	wl_surface_commit(state->surface);
	wl_display_flush(state->display);
	wl_display_roundtrip(state->display);
}
