#include "config.h"
#include "exit.h"

#include <errno.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

int errno2exit(void)
{
	int rc;

	if (errno <= 0)
		return EXIT_SUCCESS;

	switch (errno) {
	case EDOM:
		rc = 3;
		break;
	case EILSEQ:
		rc = 4;
		break;
	case ERANGE:
		rc = 5;
		break;
#if HAVE_UNISTD_H && (_XOPEN_VERSION >= 4 || defined(_XOPEN_XPG4) || defined(_XOPEN_XPG3) || defined(_XOPEN_XPG2))
	case E2BIG:
		rc = 6;
		break;
	case EAGAIN:
		rc = 7;
		break;
	case EBADF:
		rc = 8;
		break;
	case EFAULT:
		rc = 9;
		break;
	case EFBIG:
		rc = 10;
		break;
	case EINTR:
		rc = 11;
		break;
	case EINVAL:
		rc = 12;
		break;
	case EIO:
		rc = 13;
		break;
	case ENOMEM:
		rc = 14;
		break;
	case ENOSPC:
		rc = 15;
		break;
	case ENXIO:
		rc = 16;
		break;
	case EPIPE:
		rc = 17;
		break;
#endif /* _XOPEN_VERSION || _XOPEN_XPG4 || _XOPEN_XPG3 || _XOPEN_XPG2 */
	default:
		rc = EXIT_FAILURE;
	}

	return rc;
}

void gdiot_exit(int status)
{
	if (status == 0)
		exit(EXIT_SUCCESS);

	exit(errno2exit());
}
