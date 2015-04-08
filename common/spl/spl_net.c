/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <spl.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

void spl_net_load_image(const char *device)
{
	int rv;

	env_init();
	env_relocate();
	setenv("autoload", "yes");
	load_addr = CONFIG_SYS_TEXT_BASE - sizeof(struct image_header);
	rv = eth_initialize();
	if (rv == 0) {
		printf("No Ethernet devices found\n");
		hang();
	}
	if (device)
		setenv("ethact", device);
	rv = net_loop(BOOTP);
	if (rv < 0) {
		printf("Problem booting with BOOTP\n");
		hang();
	}
	spl_parse_image_header((struct image_header *)load_addr);
}
