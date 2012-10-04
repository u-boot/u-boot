/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
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
	rv = eth_initialize(gd->bd);
	if (rv == 0) {
		printf("No Ethernet devices found\n");
		hang();
	}
	if (device)
		setenv("ethact", device);
	rv = NetLoop(BOOTP);
	if (rv < 0) {
		printf("Problem booting with BOOTP\n");
		hang();
	}
	spl_parse_image_header((struct image_header *)load_addr);
}
