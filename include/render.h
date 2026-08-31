/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef RENDER_H
#define RENDER_H

#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <stdbool.h>
#include <unistd.h>
#include <wayland-client.h>

#include "buffer.h"
#include "state.h"

#define PADDING 6

void render(struct labline_state *state);

#endif	/* RENDER_H */
