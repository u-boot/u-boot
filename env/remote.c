/*
 * (C) Copyright 2011-2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

static int env_remote_init(void)
{
	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr = (ulong)&(env_ptr->data);
		gd->env_valid = ENV_VALID;
		return 0;
	}

	gd->env_addr = (ulong)default_environment;
	gd->env_valid = 0;
	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
static int env_remote_save(void)
{
#ifdef CONFIG_SRIO_PCIE_BOOT_SLAVE
	printf("Can not support the 'saveenv' when boot from SRIO or PCIE!\n");
	return 1;
#else
	return 0;
#endif
}
#endif /* CONFIG_CMD_SAVEENV */

static void env_remote_load(void)
{
#ifndef ENV_IS_EMBEDDED
	env_import((char *)env_ptr, 1);
#endif
}

U_BOOT_ENV_LOCATION(remote) = {
	.location	= ENVL_REMOTE,
	.load		= env_remote_load,
	.save		= env_save_ptr(env_remote_save),
	.init		= env_remote_init,
};
