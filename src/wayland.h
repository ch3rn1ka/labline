#ifndef WAYLAND_H
#define WAYLAND_H

#include "state.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

void wayland_init(struct state *state);

#endif
