#ifndef STATE_H
#define STATE_H

#include <stdio.h>
#include <wayland-client.h>

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

  /* Use double buffering to avoid reallocating on each configure event */
  struct buffer_context *buffers[2];
  char text[BUFSIZ];

  uint32_t width, height;
  int stride;

  int anchor;
};

struct state *init_state();

#endif
