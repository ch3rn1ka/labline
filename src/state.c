#include <stdlib.h>

#include "wayland.h"
#include "state.h"

struct state *
state_init()
{
  struct state *state = malloc(sizeof(struct state));
  wl_list_init(&state->workspaces);
  state->width = 1920;
  state->height = 26;
  state->stride = 1920*4;

  wayland_init_globals(state);

  return state;
}
