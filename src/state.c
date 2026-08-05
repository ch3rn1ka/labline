#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>

#include "state.h"
#include "buffer.h"
#include "render.h"
#include "util.h"
#include "wayland.h"

#include "ext-workspace-v1-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

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
state_set_default_values(struct labline_state *state)
{
	state->faces = (struct faces) {
		.primary = {
			.bg = {0.88f, 0.87f, 0.86f},
			.fg = {0.0f, 0.0f, 0.0f},
			.br = {1.0f, 1.0f, 1.0f}
		},
		.secondary = {
			.bg = {0.96f, 0.96f, 0.96f},
			.fg = {0.0f, 0.0f, 0.0f},
			.br = {0.0f, 0.0f, 0.0f}
		}
	};

	state->width = 0;
	state->stride = 0;
	state->font = "Monospace 10";
	state->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
		| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;

	state->needs_render = true;
	state->statusline[0] = '\0';
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
		warn("Wrong HEX color code: %s", hex_code);
		return color;
	}
	hex_code++;

	if (sscanf(hex_code, "%02x%02x%02x", &r, &g, &b) == 3) {
		color.r = r / 255.0f;
		color.g = g / 255.0f;
		color.b = b / 255.0f;
	} else {
		warn("Couldn't read colors properly");
	}

	return color;
}

static void
parse_args(struct labline_state *state, int argc, char **argv)
{
	const struct option long_options[] = {
		{"anchor", required_argument, NULL, 'a'},
		{"font", required_argument, NULL, 'f'},
		{"pbg", required_argument, NULL, 1},
		{"bfg", required_argument, NULL, 2},
		{"sbg", required_argument, NULL, 3},
		{"sfg", required_argument, NULL, 4},
		{0, 0, 0, 0}
	};

	int option;
	while ((option = getopt_long(argc, argv, "a:f:",
			long_options, NULL)) != -1) {
		switch(option) {
			case 'a': {
				uint32_t sides =
					ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
					| ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
				if (strcmp(optarg, "bottom") == 0) {
					state->anchor = sides
						| ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
				} else if (strcmp(optarg, "top") == 0) {
					state->anchor = sides
						| ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
				} else {
					warn("Incorrect anchor: %s", optarg);
				}
				break;
			}

			case 'f': {
				/* TODO: check if optarg is a valid font */
				state->font = optarg;
				break;
			}

			case 1: state->faces.primary.bg   = hex_to_rgb(optarg); break;
			case 2: state->faces.primary.fg   = hex_to_rgb(optarg); break;
			case 3: state->faces.secondary.bg = hex_to_rgb(optarg); break;
			case 4: state->faces.secondary.fg = hex_to_rgb(optarg); break;
			case '?': break;
		}
	}
}

struct labline_state *
state_init(int argc, char **argv)
{
	struct labline_state *state = calloc(1, sizeof(struct labline_state));

	state_set_default_values(state);
	parse_args(state, argc, argv);

	state->font_height = get_font_height(state->font);
	state->height = state->font_height + 10;

	buffer_init(state);
	wayland_init(state);

	return state;
}
