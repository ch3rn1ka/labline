#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
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
