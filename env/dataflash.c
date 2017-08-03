/*
 * LowLevel function for DataFlash environment support
 * Author : Gilles Gastaldi (Atmel)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <dataflash.h>
#include <search.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr;

char *env_name_spec = "dataflash";

uchar env_get_char_spec(int index)
{
	uchar c;

	read_dataflash(CONFIG_ENV_ADDR + index + offsetof(env_t, data),
			1, (char *)&c);
	return c;
}

void env_relocate_spec(void)
{
	ulong crc, new = 0;
	unsigned off;
	char buf[CONFIG_ENV_SIZE];

	/* Read old CRC */
	read_dataflash(CONFIG_ENV_ADDR + offsetof(env_t, crc),
		       sizeof(ulong), (char *)&crc);

	/* Read whole environment */
	read_dataflash(CONFIG_ENV_ADDR, CONFIG_ENV_SIZE, buf);

	/* Calculate the CRC */
	off = offsetof(env_t, data);
	new = crc32(new, (unsigned char *)(buf + off), ENV_SIZE);

	if (crc == new)
		env_import(buf, 1);
	else
		set_default_env("!bad CRC");
}

#ifdef CONFIG_ENV_OFFSET_REDUND
#error No support for redundant environment on dataflash yet!
#endif

int saveenv(void)
{
	env_t env_new;
	int ret;

	ret = env_export(&env_new);
	if (ret)
		return ret;

	return write_dataflash(CONFIG_ENV_ADDR,
				(unsigned long)&env_new,
				CONFIG_ENV_SIZE);
}

/*
 * Initialize environment use
 *
 * We are still running from ROM, so data use is limited.
 * Use a (moderately small) buffer on the stack
 */
int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}
