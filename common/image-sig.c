/*
 * Copyright (c) 2013, Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#else
#include <common.h>
#endif /* !USE_HOSTCC*/
#include <errno.h>
#include <image.h>

struct image_sig_algo image_sig_algos[] = {
};

struct image_sig_algo *image_get_sig_algo(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(image_sig_algos); i++) {
		if (!strcmp(image_sig_algos[i].name, name))
			return &image_sig_algos[i];
	}

	return NULL;
}
