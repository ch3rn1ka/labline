#ifndef RENDER_H
#define RENDER_H

#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <stdbool.h>
#include <unistd.h>
#include <wayland-client.h>

#include "buffer.h"
#include "state.h"

#define PADDING 4

void render(struct labline_state *state);

#endif	/* RENDER_H */
