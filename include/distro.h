/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __distro_h
#define __distro_h

#define DISTRO_FNAME	"extlinux/extlinux.conf"

/**
 * struct distro_info - useful information for distro_getfile()
 *
 * @dev: bootmethod device being used to boot
 * @bflow: bootflow being booted
 */
struct distro_info {
	struct udevice *dev;
	struct bootflow *bflow;
	struct cmd_tbl *cmdtp;
};

#endif
