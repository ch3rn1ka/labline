#ifndef UTIL_H
#define UTIL_H

void _die(char *filename, int line_num, char *fmt, ...);

void _warn(char *filename, int line_num, char *fmt, ...);

#define die(...) _die(__FILE__, __LINE__, __VA_ARGS__)

#define warn(...) _warn(__FILE__, __LINE__, __VA_ARGS__)

#endif	/* UTIL_H */
