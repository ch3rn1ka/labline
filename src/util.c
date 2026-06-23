#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void
_die(char *filename, int line_num, char *fmt, ...)
{
	fprintf(stderr, "ERROR: [%s:%d] ", filename, line_num);

	va_list args;
	va_start(args, fmt);

	vprintf(fmt, args);

	va_end(args);

	putchar('\n');
	exit(EXIT_FAILURE);
}

void
_warn(char *filename, int line_num, char *fmt, ...)
{
	fprintf(stderr, "ERROR: [%s:%d] ", filename, line_num);

	va_list args;
	va_start(args, fmt);

	vprintf(fmt, args);

	va_end(args);

	putchar('\n');
	exit(EXIT_FAILURE);
}
