#include <stdio.h>
#include <stdlib.h>

#include "shm.h"
#include "wayland.h"

int main()
{
  struct state *state = state_init();
  /* wl_surface_attach(state->surface, create_buffer(state), 0, 0); */
  /* wl_surface_damage_buffer(state->surface, 0, 0, state->width, state->height); */
  /* wl_surface_commit(state->surface); */

  while (wl_display_dispatch(state->display) != -1)
    {
      /* TODO: implement redraw logic */
    }

  return 0;
}
