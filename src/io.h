#ifndef IO_H
#define IO_H

#include "config.h"

#include <stdio.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_UNISTD_H && (_XOPEN_VERSION >= 4 || defined(_XOPEN_XPG4) || defined(_XOPEN_XPG3) || defined(_XOPEN_XPG2))
int gdiot_fprintf(FILE *restrict stream, const char *restrict format, ...);
int gdiot_printf(const char *restrict format, ...);
int gdiot_puts(const char *s);
#else
#define gdiot_fprintf(stream, fmt, ...) fprintf(stream, fmt, __VA_ARGS__)
#define gdiot_printf(fmt, ...) printf(fmt, __VA_ARGS__)
#define gdiot_puts(s) puts(s)
#endif /* _XOPEN_VERSION || _XOPEN_XPG4 || _XOPEN_XPG3 || _XOPEN_XPG2 */

#endif /* IO_H */
