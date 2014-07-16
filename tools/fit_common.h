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

/**
 * Map an FDT into memory, optionally increasing its size
 *
 * @cmdname:	Tool name (for displaying with error messages)
 * @fname:	Filename containing FDT
 * @size_inc:	Amount to increase size by (0 = leave it alone)
 * @blobp:	Returns pointer to FDT blob
 * @sbuf:	File status information is stored here
 * @delete_on_error:	true to delete the file if we get an error
 * @return 0 if OK, -1 on error.
 */
int mmap_fdt(const char *cmdname, const char *fname, size_t size_inc,
	     void **blobp, struct stat *sbuf, bool delete_on_error);

#endif /* _FIT_COMMON_H_ */
