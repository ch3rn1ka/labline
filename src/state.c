#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
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
state_set_default_values(struct state *state)
{
	state->faces = (struct faces) {
		.status = {
			.bg = {0.0f, 0.0f, 0.0f},
			.fg = {1.0f, 1.0f, 1.0f},
			.br = {0.0f, 0.0f, 0.0f}
		},
		.active_ws = {
			.bg = {1.0f, 1.0f, 1.0f},
			.fg = {0.0f, 0.0f, 0.0f},
			.br = {1.0f, 1.0f, 1.0f},
		},
		.inactive_ws = {
			.bg = {0.1f, 0.1f, 0.1f},
			.fg = {1.0f, 1.0f, 1.0f},
			.br = {0.0f, 0.0f, 0.0f},
		},
		.urgent_ws = {
			.bg = {1.0f, 0.0f, 0.0f},
			.fg = {0.0f, 0.0f, 0.0f},
			.br = {1.0f, 0.0f, 0.0f}
		}
	};

	state->width = 0;
	state->stride = 0;
	state->font = "Monospace 10";
	state->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
}

static bool
is_valid_hex(char *color)
{
	if (!color || strlen(color) != 7 || color[0] != '#') {
		return false;
	}

	for (int i = 1; i < 7; i++) {
		if (!isxdigit(color[i])) {
			return false;
		}
	}
	return true;
}

static struct rgb
hex_to_rgb(char *hex_code)
{
	unsigned int r, g, b;
	struct rgb color = {0.0f, 0.0f, 0.0f};

	if (!is_valid_hex(hex_code)) {
		fprintf(stderr, "Warning: wrong HEX color code %s\n", hex_code);
		return (struct rgb) {1.0, 0.0, 0.0};
	}
	hex_code++;

	if (sscanf(hex_code, "%02x%02x%02x", &r, &g, &b) == 3) {
		color.r = r / 255.0f;
		color.g = g / 255.0f;
		color.b = b / 255.0f;
	}

	return color;
}

static void
parse_args(struct state *state, int argc, char **argv)
{
	enum {
		OPT_STATUSLINE_BG = 256,
		OPT_STATUSLINE_FG,
		OPT_ACTIVE_WORKSPACE_BG,
		OPT_ACTIVE_WORKSPACE_FG,
		OPT_ACTIVE_WORKSPACE_BORDER,
		OPT_INACTIVE_WORKSPACE_BG,
		OPT_INACTIVE_WORKSPACE_FG,
		OPT_INACTIVE_WORKSPACE_BORDER,
		OPT_URGENT_WORKSPACE_BG,
		OPT_URGENT_WORKSPACE_FG,
		OPT_URGENT_WORKSPACE_BORDER,
	};

	const struct option long_options[] = {
		{"anchor", required_argument, NULL, 'a'},
		{"font", required_argument, NULL, 'f'},
		{"sbg", required_argument, NULL, OPT_STATUSLINE_BG},
		{"sfg", required_argument, NULL, OPT_STATUSLINE_FG},
		{"awsbg", required_argument, NULL, OPT_ACTIVE_WORKSPACE_BG},
		{"awsfg", required_argument, NULL, OPT_ACTIVE_WORKSPACE_FG},
		{"awsbr", required_argument, NULL, OPT_ACTIVE_WORKSPACE_BORDER},
		{"iwsbg", required_argument, NULL, OPT_INACTIVE_WORKSPACE_BG},
		{"iwsfg", required_argument, NULL, OPT_INACTIVE_WORKSPACE_FG},
		{"iwsbr", required_argument, NULL, OPT_INACTIVE_WORKSPACE_BORDER},
		{"uwsbg", required_argument, NULL, OPT_URGENT_WORKSPACE_BG},
		{"uwsfg", required_argument, NULL, OPT_URGENT_WORKSPACE_FG},
		{"uwsbr", required_argument, NULL, OPT_URGENT_WORKSPACE_BORDER},
		{0, 0, 0, 0}
	};

	int option;
	while ((option = getopt_long(argc, argv, "a:f:",
			long_options, NULL)) != -1) {
		switch (option) {
			case 'a':
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
				break;
			case 'f':
				state->font = optarg;
				break;
			case OPT_STATUSLINE_BG:
				state->faces.status.bg = hex_to_rgb(optarg);
				break;
			case OPT_STATUSLINE_FG:
				state->faces.status.fg = hex_to_rgb(optarg);
				break;
			case OPT_ACTIVE_WORKSPACE_BG:
				state->faces.active_ws.bg = hex_to_rgb(optarg);
				break;
			case OPT_ACTIVE_WORKSPACE_FG:
				state->faces.active_ws.fg = hex_to_rgb(optarg);
				break;
			case OPT_ACTIVE_WORKSPACE_BORDER:
				state->faces.active_ws.br = hex_to_rgb(optarg);
				break;
			case OPT_INACTIVE_WORKSPACE_BG:
				state->faces.inactive_ws.bg = hex_to_rgb(optarg);
				break;
			case OPT_INACTIVE_WORKSPACE_FG:
				state->faces.inactive_ws.fg = hex_to_rgb(optarg);
				break;
			case OPT_INACTIVE_WORKSPACE_BORDER:
				state->faces.inactive_ws.br = hex_to_rgb(optarg);
				break;
			case OPT_URGENT_WORKSPACE_BG:
				state->faces.urgent_ws.bg = hex_to_rgb(optarg);
				break;
			case OPT_URGENT_WORKSPACE_FG:
				state->faces.urgent_ws.fg = hex_to_rgb(optarg);
				break;
			case OPT_URGENT_WORKSPACE_BORDER:
				state->faces.urgent_ws.br = hex_to_rgb(optarg);
				break;
		}
	}
}

struct state *
state_init(int argc, char **argv)
{
	struct state *state = calloc(1, sizeof(struct state));

	state_set_default_values(state);
	parse_args(state, argc, argv);

	state->font_height = get_font_height(state->font);
	state->height = state->font_height + 10;
	/* printf("DEBUG: The height is: %d\n", state->height); */

	wl_list_init(&state->workspaces);
	buffers_init(state);
	wayland_init(state);

	return state;
}
