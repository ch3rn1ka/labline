#include "shm.h"
#include "wayland.h"

int main()
{
  struct state *state = init_state();

  while (wl_display_dispatch(state->display) != -1)
    {
      /* redraw logic */
    }

  return 0;
}
