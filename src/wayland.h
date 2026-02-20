#ifndef WAYLAND_H
#define WAYLAND_H

#include "state.h"

void wayland_init_globals(struct state *state);
struct wl_buffer * create_buffer(struct state *state);

#endif
