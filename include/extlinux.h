/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __extlinux_h
#define __extlinux_h

#include <bootflow.h>
#include <linux/types.h>

struct pxe_context;

#define EXTLINUX_FNAME	"extlinux/extlinux.conf"

/**
 * struct extlinux_info - useful information for extlinux_getfile()
 *
 * @dev: bootmethod device being used to boot
 * @bflow: bootflow being booted
 * @cmdtp: command table entry to run the boot command with, or NULL
 */
struct extlinux_info {
	struct udevice *dev;
	struct bootflow *bflow;
	struct cmd_tbl *cmdtp;
};

/**
 * extlinux_getfile() - Read a file named by a pxelinux-style config
 *
 * This is the pxe_getfile_func used by every bootmeth that hands its files
 * to the pxelinux parser. It reads the file through the bootmeth uclass, so
 * @ctx->userdata must point at a struct extlinux_info.
 *
 * @ctx: PXE context
 * @file_path: Path of the file to read
 * @file_addr: Address to load the file to, as a hex string
 * @type: Image type used to record the loaded file in the bootflow
 * @sizep: Returns the file size in bytes
 * Return: 0 if OK, -ve on error
 */
int extlinux_getfile(struct pxe_context *ctx, const char *file_path,
		     char *file_addr, enum bootflow_img_t type, ulong *sizep);

#endif
