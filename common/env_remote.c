/*
 * (C) Copyright 2011-2012 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>

char *env_name_spec = "Remote";

#ifdef ENV_IS_EMBEDDED
env_t *env_ptr = &environment;
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = (env_t *)CONFIG_ENV_ADDR;
#endif /* ENV_IS_EMBEDDED */

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

int env_init(void)
{
	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr = (ulong)&(env_ptr->data);
		gd->env_valid = 1;
		return 0;
	}

	gd->env_addr = (ulong)default_environment;
	gd->env_valid = 0;
	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
int saveenv(void)
{
#ifdef CONFIG_SRIO_PCIE_BOOT_SLAVE
	printf("Can not support the 'saveenv' when boot from SRIO or PCIE!\n");
	return 1;
#else
	return 0;
#endif
}
#endif /* CONFIG_CMD_SAVEENV */

void env_relocate_spec(void)
{
#ifndef ENV_IS_EMBEDDED
	env_import((char *)env_ptr, 1);
#endif
}
