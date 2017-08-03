/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr;

void env_relocate_spec(void)
{
}

/*
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited
 */
int env_init(void)
{
	gd->env_addr	= (ulong)&default_environment[0];
	gd->env_valid	= 0;

	return 0;
}
