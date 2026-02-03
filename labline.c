#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "ext-workspace-v1-client-protocol.h"

#define PANEL_WIDTH 800
#define PANEL_HEIGHT 26

static struct wl_display *display;
static struct wl_registry *registry;
static struct wl_compositor *compositor;
static struct wl_shm *shm;
static struct zwlr_layer_shell_v1 *layer_shell;
static struct ext_workspace_manager_v1 *workspace_manager;

struct workspace
{
  struct ext_workspace_handle_v1 *handle;
  char *name;
  struct wl_list node;
};

struct wl_list workspaces;

struct shm_context
{
  struct wl_shm_pool *pool;
  void *data;
  size_t size;
};

static int
create_shm_file(size_t size)
{
  char template[] = "/tmp/wayland-shm-XXXXXX";
  int fd = mkstemp(template);
  if (fd < 0)
    {
      fprintf(stderr, "mkstemp");      
      return -1;
    }

  unlink(template);
  if (ftruncate(fd, size) < 0)
    {
      fprintf(stderr, "unlink");
      return -1;
    }

  return fd;
}

static void
registry_handle_global(void* data, struct wl_registry *wl_registry,
                       uint32_t name, const char *iface, uint32_t version)
{
  /* i'm okay with some code duplication here */
  uint32_t bind_version;
  if (strcmp(iface, wl_compositor_interface.name) == 0)
    {
      bind_version = MIN(version, (uint32_t)wl_compositor_interface.version);
      compositor = wl_registry_bind(registry, name, &wl_compositor_interface, bind_version);
    }
  else if (strcmp(iface, wl_shm_interface.name) == 0)
    {
      bind_version = MIN(version, (uint32_t)wl_shm_interface.version);
      shm = wl_registry_bind(registry, name, &wl_shm_interface, bind_version);
    }
  else if (strcmp(iface, zwlr_layer_shell_v1_interface.name) == 0)
    {
      bind_version = MIN(version, (uint32_t)zwlr_layer_shell_v1_interface.version);
      layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, bind_version);
    }
  else if (strcmp(iface, ext_workspace_manager_v1_interface.name) == 0)
    {
      bind_version = MIN(version, (uint32_t)ext_workspace_manager_v1_interface.version);
      workspace_manager = wl_registry_bind(registry, name, &ext_workspace_manager_v1_interface,
                                           bind_version);
    }
}

static void registry_handle_global_remove() {}

static const struct wl_registry_listener
registry_listener = {
  .global        = &registry_handle_global,
  .global_remove = &registry_handle_global_remove
};

static void
workspace_handle_name(void *data, struct ext_workspace_handle_v1 *handle,
                      const char *name)
{
  struct workspace *workspace = data;
  if (workspace->name)
    {
      free(workspace->name);
    }
  workspace->name = strdup(name);
}

static void
workspace_handle_state(void *data, struct ext_workspace_handle_v1 *ext_workspace_handle_v1,
                       uint32_t state)
{
  struct workspace *workspace = data;
  printf("State changed for %s: %d\n", workspace->name, state);
}

static void
workspace_handle_id() {}

static void
workspace_handle_coordinates() {}

static void
workspace_handle_capabilities() {}

static void
workspace_handle_removed() 
{
  /* TODO: implement remove logic */
}

static const struct ext_workspace_handle_v1_listener
workspace_listener = {
  .id           = &workspace_handle_id,
  .name         = &workspace_handle_name,
  .coordinates  = &workspace_handle_coordinates,
  .state        = &workspace_handle_state,
  .capabilities = &workspace_handle_capabilities,
  .removed      = &workspace_handle_removed,
};

static void
workspace_manager_handle_workspace(void *data, struct ext_workspace_manager_v1 *mgr,
                                   struct ext_workspace_handle_v1 *handle)
{
  struct wl_list *workspaces = data;
  struct workspace *new_workspace = calloc(1, sizeof(struct workspace));
  new_workspace->handle = handle;

  ext_workspace_handle_v1_add_listener(handle, &workspace_listener, new_workspace);
  wl_list_insert(workspaces, &new_workspace->node);
  /* printf("New workspace %p tracked!\n", (void *)wh); */
}

static void
workspace_manager_handle_group() {}

static void
workspace_manager_handle_done() {}

static void
workspace_manager_handle_finished() {}

static const struct ext_workspace_manager_v1_listener
workspace_manager_listener = {
  .workspace       = &workspace_manager_handle_workspace,
  .workspace_group = &workspace_manager_handle_group,
  .done            = &workspace_manager_handle_done,
  .finished        = &workspace_manager_handle_finished
};

static void
surface_listener_handle_configure(void *data,
                                  struct zwlr_layer_surface_v1 *layer_surface,
                                  uint32_t serial, uint32_t width, uint32_t height)
{
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

static void
surface_listener_handle_closed(void *data,
                               struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1)
{

}

static const struct zwlr_layer_surface_v1_listener surface_listener = {
  .configure = &surface_listener_handle_configure,
  .closed    = &surface_listener_handle_closed
};

int main() {
  int exit_status = EXIT_SUCCESS;

  wl_list_init(&workspaces);

  display = wl_display_connect(NULL);
  if (!display)
    {
      fprintf(stderr, "Couldn't connect to the display.\n");
      return 1;
    }

  registry = wl_display_get_registry(display);
  if (!registry)
    {
      exit_status = EXIT_FAILURE;
      fprintf(stderr, "Couldn't get the registry.\n");
      goto disconnect;
    }
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display);

  if (!workspace_manager)
    {
      exit_status = EXIT_FAILURE;
      fprintf(stderr, "Workspace manager not supported by the compositor.\n");
      goto destroy_registry;
    }
  ext_workspace_manager_v1_add_listener(workspace_manager, &workspace_manager_listener,
                                        &workspaces);
  wl_display_roundtrip(display);
  
  int shm_size = PANEL_WIDTH * PANEL_HEIGHT * 4;
  int shm_fd = create_shm_file(shm_size);
  if (shm_fd < 0)
    {
      fprintf(stderr, "create_shm_file");
      exit_status = EXIT_FAILURE;
      goto destroy_registry;
    }
  struct shm_context shm_ctx;
  shm_ctx.pool = wl_shm_create_pool(shm, shm_fd, shm_size);
  shm_ctx.size = shm_size;
  shm_ctx.data = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ctx.data == MAP_FAILED)
    {
      fprintf(stderr, "mmap");
      goto destroy_pool;
    }

  struct wl_buffer *buffer = wl_shm_pool_create_buffer(shm_ctx.pool, 0,
                                                       PANEL_WIDTH, PANEL_HEIGHT,
                                                       PANEL_WIDTH * 4,
                                                       WL_SHM_FORMAT_ARGB8888);
    
  struct wl_surface *wl_surface = wl_compositor_create_surface(compositor);
  struct zwlr_layer_surface_v1 *layer_surface =
    zwlr_layer_shell_v1_get_layer_surface(layer_shell, wl_surface, NULL,
                                          ZWLR_LAYER_SHELL_V1_LAYER_TOP,
                                          "panel");
  zwlr_layer_surface_v1_set_size(layer_surface, 0, PANEL_HEIGHT);
  zwlr_layer_surface_v1_set_anchor(layer_surface,
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, PANEL_HEIGHT);
  zwlr_layer_surface_v1_add_listener(layer_surface, &surface_listener, NULL);
  wl_surface_commit(wl_surface);

  /* TODO: actually draw the pixels on the surface with cairo */
  while (wl_display_dispatch(display) != -1) {
    wl_surface_attach(wl_surface, buffer, 0, 0);
    wl_surface_damage_buffer(wl_surface, 0, 0, PANEL_WIDTH, PANEL_HEIGHT);
    wl_surface_commit(wl_surface);
  }

 destroy_pool:
  close(shm_fd);
  wl_shm_pool_destroy(shm_ctx.pool);
 destroy_registry:
  wl_registry_destroy(registry);
 disconnect:
  wl_display_disconnect(display);

  /* TODO: free the workspaces list */
  return exit_status;
}
