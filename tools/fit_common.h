/*
 * (C) Copyright 2014
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _FIT_COMMON_H_
#define _FIT_COMMON_H_

#include "imagetool.h"
#include "mkimage.h"
#include <image.h>

int fit_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params);

int fit_check_image_types(uint8_t type);

int mmap_fdt(char *cmdname, const char *fname, void **blobp,
		struct stat *sbuf, int useunlink);

#endif /* _FIT_COMMON_H_ */
