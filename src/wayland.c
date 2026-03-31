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

#include "ext-workspace-v1-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static void
registry_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *iface, uint32_t server_iface_version)
{
	struct state *state = data;

	const struct wl_interface *i = NULL; /* shorthand for wl_*_interface */
	uint32_t bind_version, library_iface_version;
	if (strcmp(iface, wl_compositor_interface.name) == 0)
	{
		i = &wl_compositor_interface;
		library_iface_version = i->version;
		bind_version = MIN(server_iface_version, library_iface_version);
		state->compositor =
			wl_registry_bind(wl_registry, name, i, bind_version);
	} else if (strcmp(iface, wl_shm_interface.name) == 0) {
		i = &wl_shm_interface;
		library_iface_version = i->version;
		bind_version = MIN(server_iface_version, library_iface_version);
		state->shm =
			wl_registry_bind(wl_registry, name, i, bind_version);
	} else if (strcmp(iface, zwlr_layer_shell_v1_interface.name) == 0) {
		i = &zwlr_layer_shell_v1_interface;
		library_iface_version = i->version;
		bind_version = MIN(server_iface_version, library_iface_version);
		state->layer_shell =
			wl_registry_bind(wl_registry, name, i, bind_version);
	} else if (strcmp(iface,
			ext_workspace_manager_v1_interface.name) == 0) {
		i = &ext_workspace_manager_v1_interface;
		library_iface_version = i->version;
		bind_version = MIN(server_iface_version, library_iface_version);
		state->workspace_manager =
			wl_registry_bind(wl_registry, name, i, bind_version);
	}
}

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
workspace_handle_state(void *data,
		struct ext_workspace_handle_v1 *ext_workspace_handle_v1,
		uint32_t state)
{
	struct workspace_callback_data *callback_data = data;
	struct workspace *workspace = callback_data->workspace;
	struct state *program_state = callback_data->state;
	workspace->state = state;

	/* Trigger a re-render if the surface is initialized. */
	if (program_state->width != 0) {
		render(program_state);
	}
}

static void
workspace_handle_id() {}

static void
workspace_handle_coordinates() {}

static void
workspace_handle_capabilities() {}

static void
workspace_handle_removed()
{
	/* TODO: implement remove logic */
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
workspace_manager_workspace(void *data, struct ext_workspace_manager_v1 *mgr,
		struct ext_workspace_handle_v1 *handle)
{
	struct state *state = data;
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

/* Main buffer realloc logic here. */
static void
layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface,
		uint32_t serial, uint32_t width, uint32_t height)
{
	struct state *state = data;

	if (width != state->width || height != state->height) {
		state->width = width;
		state->stride = width * 4;
		state->height = height;

		state->buffers[0]->stale = true;
		state->buffers[1]->stale = true;
	}
	zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

	render(state);
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


void
wayland_init(struct state *state)
{
	/* Display */
	state->display = wl_display_connect(NULL);
	if (!state->display)
	{
		fprintf(stderr, "Failed to connect to display.\n");
		exit(EXIT_FAILURE);
	}

	/* Registry */
	state->registry = wl_display_get_registry(state->display);
	if (!state->registry)
	{
		fprintf(stderr, "Failed to get registry.\n");
		exit(EXIT_FAILURE);
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
	zwlr_layer_surface_v1_set_anchor(state->layer_surface,
		ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	zwlr_layer_surface_v1_set_exclusive_zone(state->layer_surface,
		state->height);
	zwlr_layer_surface_v1_add_listener(state->layer_surface,
		&layer_surface_listener, state);

	/* Workspace manager */
	if (!state->workspace_manager) {
		fprintf(stderr,
			"Workspace manager not supported by the compositor.\n");
		exit(EXIT_FAILURE);
	}
	ext_workspace_manager_v1_add_listener(state->workspace_manager,
		&workspace_manager_listener, state);
	wl_display_roundtrip(state->display);

	wl_surface_commit(state->surface);
	wl_display_flush(state->display);
	wl_display_roundtrip(state->display);
}
