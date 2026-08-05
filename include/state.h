#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <wayland-client.h>

#define LABLINE_VERSION "labline 0.1"

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
	/* Accented sections: active workspace & toplevel */
	struct face primary;
	/* Dim sections: inactive workspaces & statusline */
	struct face secondary;
};

struct labline_state
{
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct wl_surface *surface;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager;
	struct ext_workspace_manager_v1 *workspace_manager;

	struct wl_list workspaces;
	struct toplevel *active_toplevel;

	struct buffer_context *buffer;
	char statusline[BUFSIZ];

	struct faces faces;

	char *font;
	int font_height;

	bool needs_render;

	uint32_t width, height, stride;
	int anchor;
};

struct labline_state *state_init(int argc, char **argv);

#endif	/* STATE_H */
