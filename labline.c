#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "ext-workspace-v1-client-protocol.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

static struct wl_display *display;
static struct wl_registry *registry;
static struct wl_compositor *compositor;
static struct wl_shm *shm;
static struct zwlr_layer_shell_v1 *layer_shell;
static struct ext_workspace_manager_v1 *workspace_mgr;

static void registry_handle_global(void* data, struct wl_registry *wl_registry,
                                   uint32_t name, const char *iface,
                                   uint32_t version)
{
     /* i'm okay with some code duplication here */
     if (strcmp(iface, wl_compositor_interface.name) == 0) {
          uint32_t version_to_bind = MIN(version, (uint32_t)wl_compositor_interface.version);
          compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version_to_bind);
     } else if (strcmp(iface, wl_shm_interface.name) == 0) {
          uint32_t version_to_bind = MIN(version, (uint32_t) wl_shm_interface.version);
          shm = wl_registry_bind(registry, name, &wl_shm_interface, version_to_bind);
     } else if (strcmp(iface, zwlr_layer_shell_v1_interface.name) == 0) {
          uint32_t version_to_bind = MIN(version, (uint32_t) zwlr_layer_shell_v1_interface.version);
          layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface,
                                         version_to_bind);
     } else if (strcmp(iface, ext_workspace_manager_v1_interface.name) == 0) {
          uint32_t version_to_bind = MIN(version,
                                         (uint32_t)ext_workspace_manager_v1_interface.version);
          workspace_mgr = wl_registry_bind(registry, name, &ext_workspace_manager_v1_interface,
                                           version_to_bind);
     }
}

static void registry_handle_global_remove(void *data,
                                          struct wl_registry *wl_registry,
                                          uint32_t name)
{
     /* do stuff */
}

static const struct wl_registry_listener registry_listener = {
     .global = &registry_handle_global,
     .global_remove = &registry_handle_global_remove,
};

int main() {
     display = wl_display_connect(NULL);
     registry = wl_display_get_registry(display);
     wl_registry_add_listener(registry, &registry_listener, NULL);
     wl_display_roundtrip(display);

     if (!display) {
          fprintf(stderr, "Failed to connect to Wayland display\n");
          return 1;
     }

     while (wl_display_dispatch(display) != -1) {
          /* do stuff */
     }
     return 0;
}
