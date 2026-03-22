#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

#include "state.h"
#include "render.h"
#include "wayland.h"

struct state *
init_state()
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

  /* TODO: get text from stdin */
  strcpy(state->text, "Hello, world!");

  init_buffers(state);
  wl_list_init(&state->workspaces);
  init_wayland_globals(state);

  return state;
}
