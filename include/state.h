#ifndef STATE_H
#define STATE_H

#include <stddef.h>
#include <stdio.h>
#include <wayland-client.h>

struct rgb {
	float r;
	float g;
	float b;
};

struct face {
	struct rgb bg;
	struct rgb fg;
	struct rgb br;
};

struct faces {
	struct face status;
	struct face active_ws;
	struct face inactive_ws;
	struct face urgent_ws;
};

struct state
{
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct wl_surface *surface;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct ext_workspace_manager_v1 *workspace_manager;

	struct wl_list workspaces;

	/* Swap between two buffers to draw the panel */
	struct buffer_context *buffers[2];
	char statusline[BUFSIZ];

	struct faces faces;

	char *font;
	int font_height;

	uint32_t width, height, stride;
	int anchor;
};

struct state *state_init(int argc, char **argv);

#endif
