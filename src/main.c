#include <stdio.h>
#include <poll.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "render.h"
#include "shm.h"
#include "wayland.h"

int
main(int argc, char **argv)
{
	struct state *state = state_init();

	struct pollfd fds[2];
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	fds[1].fd = wl_display_get_fd(state->display);
	fds[1].events = POLLIN;

	while (true) {
		while (wl_display_prepare_read(state->display) != 0) {
			wl_display_dispatch_pending(state->display);
		}
		wl_display_flush(state->display);

		if (poll(fds, 2, -1) <= 0) {
			wl_display_cancel_read(state->display);
			break;
		}

		if (fds[1].revents & POLLIN) {
			wl_display_read_events(state->display);
			/* wl_display_dispatch_pending(state->display); */
		} else {
			wl_display_cancel_read(state->display);
		}

		if (fds[0].revents & POLLIN) {
			if (fgets(state->statusline, BUFSIZ, stdin)) {
				int length = strlen(state->statusline);
				if (state->statusline[length - 1] == '\n') {
					state->statusline[length - 1] = '\0';
				}
				render(state);
			}
		}
	}

	return 0;
}
