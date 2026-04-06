#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>

#include "state.h"
#include "render.h"
#include "wayland.h"

#include "ext-workspace-v1-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

/*
 * Allocate two `buffer_context` structs and mark them as stale so that their
 * fields will get allocated during the next `prepare_buffer()` call.
 */
static void
buffers_init(struct state *state)
{
	for (int i = 0; i < 2; i++) {
		state->buffers[i] = calloc(1, sizeof(struct buffer_context));
		state->buffers[i]->fd = -1;
		state->buffers[i]->stale = true;
	}
}

static int
get_font_height(const char *fontname)
{
	PangoFontMap *fontmap = pango_cairo_font_map_get_default();
	PangoContext *context = pango_font_map_create_context(fontmap);
	PangoFontDescription *desc = pango_font_description_from_string(fontname);
	PangoFont *font = pango_font_map_load_font(fontmap, context, desc);
	PangoFontMetrics *metrics = pango_font_get_metrics(font, NULL);
	int height = pango_font_metrics_get_height(metrics) / PANGO_SCALE;
	pango_font_metrics_unref(metrics);
	g_object_unref(font);
	pango_font_description_free(desc);
	g_object_unref(context);
	return height;
}

static void
parse_args(struct state *state, int argc, char **argv)
{
	int opt;
	while ((opt = getopt(argc, argv, "f:a:")) != -1) {
		switch (opt) {
			case 'f': {
				state->font = optarg;
				break;
			}
			case 'a': {
				if (strcmp(optarg, "bottom") == 0) {
					state->anchor =
						ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
						| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
						| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
				} else if (strcmp(optarg, "top") == 0) {
					state->anchor =
						ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
						| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
						| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
				} else {
					fprintf(stderr, "Incorrect anchor.");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}

struct state *
state_init(int argc, char **argv)
{
	struct state *state = calloc(1, sizeof(struct state));

	/* Default values: */
	state->width = 0;
	state->stride = 0;
	state->font = "Monospace 10";
	state->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;

	parse_args(state, argc, argv);

	state->font_height = get_font_height(state->font);
	state->height = state->font_height + 6;
	printf("The height is: %d\n", state->height);

	wl_list_init(&state->workspaces);
	buffers_init(state);
	wayland_init(state);

	return state;
}
