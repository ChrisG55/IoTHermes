/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "streamid.h"

int streamid_compare(const void *left_id, const void *right_id)
{
	unsigned int left, right;

	left = *(unsigned int *)left_id;
	right = *(unsigned int *)right_id;

	if (left < right)
		return -1;
	else if (left > right)
		return 1;
	return 0;
}
