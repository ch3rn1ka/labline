/* SPDX-License-Identifier: GPL-2.0-only */
#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "render.h"
#include "shm.h"
#include "util.h"

int
main(int argc, char **argv)
{
	struct labline_state *state = state_init(argc, argv);

	struct pollfd fds[2];
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	fds[1].fd = wl_display_get_fd(state->display);
	fds[1].events = POLLIN;

	while (true) {
		if (state->needs_render) {
			render(state);
			state->needs_render = false;
		}

		while (wl_display_prepare_read(state->display) != 0) {
			if (wl_display_dispatch_pending(state->display) == -1) {
				die("Couldn't dispatch pending events");
			}
		}
		while (wl_display_flush(state->display) == -1) {
			if (errno != EAGAIN) {
				die("Wayland display disconnected or socket error");
			}
		}

		int ret = poll(fds, 2, -1);
		if (ret <= 0) {
			wl_display_cancel_read(state->display);
			if (ret == -1 && errno == EINTR) {
				continue;
			}
			break;
		}

		/* Events from the Wayland fd */
		if (fds[1].revents & (POLLHUP | POLLERR)) {
			wl_display_cancel_read(state->display);
			die("Failed to get events from Wayland fd");
		}

		if (fds[1].revents & POLLIN) {
			if (wl_display_read_events(state->display) == -1) {
				die("Couldn't read events from display");
			}
			if (wl_display_dispatch_pending(state->display) == -1) {
				die("Couldn't dispatch pending events");
			}
		} else {
			wl_display_cancel_read(state->display);
		}

		/* Input from stdin */
		if (fds[0].revents & POLLERR) {
			die("POLLERR on stdin");
		}

		if (fds[0].revents & (POLLIN | POLLHUP)) {
			if (fgets(state->statusline, BUFSIZ, stdin)) {
				int length = strlen(state->statusline);
				if (length > 0 && state->statusline[length - 1] == '\n') {
					state->statusline[length - 1] = '\0';
				}
				state->needs_render = true;
			} else if (feof(stdin)) {
				fds[0].fd = -1;
			} else {
				warn("Failed to read status from stdin");
			}
		}
	}

	return 0;
}
