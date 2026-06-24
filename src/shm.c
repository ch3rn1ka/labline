#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shm.h"
#include "util.h"

int
create_shm_file(size_t size)
{
	char template[] = "/tmp/wayland-shm-XXXXXX";
	int fd = mkstemp(template);
	if (fd < 0) {
		die("Failed to mkstemp()");
	}

	unlink(template);
	if (ftruncate(fd, size) < 0) {
		die("Failed to ftruncate()");
	}

	return fd;
}
