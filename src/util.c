/* SPDX-License-Identifier: GPL-2.0-only */
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
	fprintf(stderr, "WARNING: [%s:%d] ", filename, line_num);

	va_list args;
	va_start(args, fmt);

	vprintf(fmt, args);

	va_end(args);

	putchar('\n');
}

void
_debug(char *filename, int line_num, char *fmt, ...)
{
#ifndef NO_DEBUG
	fprintf(stderr, "DEBUG: [%s:%d] ", filename, line_num);

	va_list args;
	va_start(args, fmt);

	vprintf(fmt, args);

	va_end(args);

	putchar('\n');
#endif	/* NO_DEBUG */
}
