#include <stdio.h>
#include <poll.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "render.h"
#include "shm.h"
#include "wayland.h"

int main()
{
  struct state *state = init_state();

  struct pollfd fds[2];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = wl_display_get_fd(state->display);
  fds[1].events = POLLIN;

  while (true)
    {
      while (wl_display_prepare_read(state->display) != 0)
        {
          wl_display_dispatch_pending(state->display);
        }
      wl_display_flush(state->display);

      if (poll(fds, 2, -1) <= 0)
        {
          wl_display_cancel_read(state->display);
          break;
        }

      if (fds[1].revents & POLLIN)
        {
          wl_display_read_events(state->display);
        }
      else
        {
          wl_display_cancel_read(state->display);
        }

      if (fds[0].revents & POLLIN)
        {
          if (fgets(state->text, BUFSIZ, stdin))
            {
              int length = strlen(state->text);
              if (state->text[length - 1] == '\n')
                {
                  state->text[length - 1] = '\0';
                }

              /*
               * TODO: separate the logic for finding a free buffer
               * into its own function (it repeats in wayland.c)
               */
              for (int i = 0; i < 2; ++i)
                {
                  struct buffer_context *buf_ctx = state->buffers[i];
                  if (!buf_ctx->busy)
                    {
                      prepare_buffer(buf_ctx, state);
                      render(buf_ctx, state);
                      wl_surface_attach(state->surface, buf_ctx->buf, 0, 0);
                      wl_surface_damage_buffer(state->surface, 0, 0,
                                               state->width, state->height);
                      wl_surface_commit(state->surface);
                      buf_ctx->busy = true;
                      break;
                    }
                }
            }
        }
    }

  return 0;
}
