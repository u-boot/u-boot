#ifndef UBOOT_LIBFDT_H
#define UBOOT_LIBFDT_H
/*
 * SPDX-License-Identifier:     GPL-2.0+ BSD-2-Clause
 */

#include "../lib/libfdt/libfdt.h"

extern struct fdt_header *working_fdt;	/* Pointer to the working fdt */

/* adding a ramdisk needs 0x44 bytes in version 2008.10 */
#define FDT_RAMDISK_OVERHEAD	0x80

#endif /* UBOOT_LIBFDT_H */
