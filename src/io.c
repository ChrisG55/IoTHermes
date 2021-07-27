#include "config.h"
#include "io.h"

#if HAVE_UNISTD_H
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_UNISTD_H && (_XOPEN_VERSION >= 4 || defined(_XOPEN_XPG4) || defined(_XOPEN_XPG3) || defined(_XOPEN_XPG2))
#define MAX_RETRY 8

int gdiot_fprintf(FILE *restrict stream, const char *restrict format, ...)
{
	int rc;
	int retry = 0;
	va_list ap;

	va_start(ap, format);
	do {
		rc = vfprintf(stream, format, ap);
	} while (rc == EOF &&
		 ferror(stream) &&
		 (errno == EAGAIN || errno == EINTR) &&
		 retry++ < MAX_RETRY);
	va_end(ap);

	return rc;
}

int gdiot_printf(const char *restrict format, ...) {
	int rc;
	int retry = 0;
	va_list ap;

	va_start(ap, format);
	do {
		rc = vprintf(format, ap);
	} while (rc == EOF &&
		 ferror(stdout) &&
		 (errno == EAGAIN || errno == EINTR) &&
		 retry++ < MAX_RETRY);
	va_end(ap);

	return rc;
}

int gdiot_puts(const char *s)
{
	int rc;
	int retry = 0;

	do {
		rc = puts(s);
	} while (rc == EOF &&
		 ferror(stdout) &&
		 (errno == EAGAIN || errno == EINTR) &&
		 retry++ < MAX_RETRY);

	return rc;
}
#endif /* _XOPEN_VERSION || _XOPEN_XPG4 || _XOPEN_XPG3 || _XOPEN_XPG2 */
