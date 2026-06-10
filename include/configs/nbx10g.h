/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017-2018 Freebox SA
 * Copyright (C) 2026 Free Mobile, Vincent Jardin <vjardin@free.fr>
 *
 * Configuration for Freebox Nodebox 10G
 */

#ifndef _CONFIG_NBX10G_H
#define _CONFIG_NBX10G_H

#include "mvebu_armada-8k.h"

/* Override environment settings for NBX */
#undef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
	"hostname=nodebox10G\0" \
	"ethrotate=no\0" \
	"image_addr=0x7000000\0" \
	"image_name=Image.nodebox10G\0" \
	"fdt_addr=0x6f00000\0" \
	"fdt_name=nodebox10G.dtb\0" \
	"console=ttyS0,115200\0" \
	"tftpboot=setenv bootargs console=${console} bank=tftp; " \
		"dhcp ${image_addr} ${image_name}; " \
		"tftp ${fdt_addr} ${fdt_name}; " \
		"booti ${image_addr} - ${fdt_addr}\0"

#endif /* _CONFIG_NBX10G_H */
