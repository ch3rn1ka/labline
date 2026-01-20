#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
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
static struct ext_workspace_manager_v1 *workspace_manager;

static int exit_status = 0;

static void registry_handle_global(void* data,
                                   struct wl_registry *wl_registry,
                                   uint32_t name,
                                   const char *iface,
                                   uint32_t version)
{
     /* i'm okay with some code duplication here */
     uint32_t v; 
     if (strcmp(iface, wl_compositor_interface.name) == 0) {
          v = MIN(version, (uint32_t)wl_compositor_interface.version);
          compositor = wl_registry_bind(registry, name,
                                        &wl_compositor_interface, v);
     } else if (strcmp(iface, wl_shm_interface.name) == 0) {
          v = MIN(version, (uint32_t) wl_shm_interface.version);
          shm = wl_registry_bind(registry, name,
                                 &wl_shm_interface, v);
     } else if (strcmp(iface, zwlr_layer_shell_v1_interface.name) == 0) {
          v = MIN(version, (uint32_t) zwlr_layer_shell_v1_interface.version);
          layer_shell = wl_registry_bind(registry, name,
                                         &zwlr_layer_shell_v1_interface, v);
     } else if (strcmp(iface, ext_workspace_manager_v1_interface.name) == 0) {
          v = MIN(version, (uint32_t)ext_workspace_manager_v1_interface.version);
          workspace_manager = wl_registry_bind(registry, name,
                                               &ext_workspace_manager_v1_interface, v);
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
     .global_remove = &registry_handle_global_remove
};

static void workspace_manager_handle_workspace(void *data,
                                               struct ext_workspace_manager_v1 *mgr,
                                               struct ext_workspace_handle_v1 *id)
{
     printf("New Workspace handle received: %p\n", (void *)id);
}

static void workspace_manager_handle_group(void *data,
                                           struct ext_workspace_manager_v1 *mgr,
                                           struct ext_workspace_group_handle_v1 *id)
{
     printf("New Workspace group received: %p\n", (void *)id);
}

static void workspace_manager_handle_done()
{
}

static void workspace_manager_handle_finished()
{
}

static const struct ext_workspace_manager_v1_listener workspace_manager_listener = {
     .workspace = &workspace_manager_handle_workspace,
     .workspace_group = &workspace_manager_handle_group,
     .done = &workspace_manager_handle_done,
     .finished = &workspace_manager_handle_finished
};

int main() {     
     display = wl_display_connect(NULL);
     if (!display) {
          fprintf(stderr, "Couldn't connect to the display.");
          return 1;
     }
     
     registry = wl_display_get_registry(display);
     if (!registry) {
          exit_status = 1;
          fprintf(stderr, "Couldn't get the registry.");
          goto disconnect;
     }
     wl_registry_add_listener(registry, &registry_listener, NULL);

     wl_display_roundtrip(display);
     if (!workspace_manager) {
          exit_status = 1;
          fprintf(stderr, "Workspace manager not supported by the compositor.");
          goto destroy_registry;
     }
     ext_workspace_manager_v1_add_listener(workspace_manager,
                                           &workspace_manager_listener,
                                           NULL);
     wl_display_roundtrip(display);
     

     while (wl_display_dispatch(display) != -1) {
          /* do stuff */
     }

destroy_registry:
     wl_registry_destroy(registry);
          
disconnect:
     wl_display_disconnect(display);
     
     return exit_status;
}
