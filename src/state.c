#include <stdlib.h>
#include <string.h>

#include "wayland.h"
#include "state.h"

struct state *
state_init()
{
  struct state *state = malloc(sizeof(struct state));
  wl_list_init(&state->workspaces);

  /*
   * The width of the surface is left blank until `layer_surface_listener`
   * catches a configure event from the compositor (it knows the horizontal
   * dimension of the surface because we anchor it to the sides of the screen).
   * As for the height, however, it's important to define it here because the
   * compositor can't deduce it by itself, and we later communicate it in the
   * `zwlr_layer_surface_v1_set_size()` call.
   */
  state->width = 0;
  state->stride = 0;

  /* TODO: get height from the fontsize */
  state->height = 26;

  /* TODO: get text from stdin */
  strcpy(state->text, "Hello, world!");

  wayland_init_globals(state);

  return state;
}
