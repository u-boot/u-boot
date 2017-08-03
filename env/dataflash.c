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

static int env_dataflash_get_char(int index)
{
	uchar c;

	read_dataflash(CONFIG_ENV_ADDR + index + offsetof(env_t, data),
			1, (char *)&c);
	return c;
}

static int env_dataflash_load(void)
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

	return 0;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
#error No support for redundant environment on dataflash yet!
#endif

static int env_dataflash_save(void)
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

U_BOOT_ENV_LOCATION(dataflash) = {
	.location	= ENVL_DATAFLASH,
	ENV_NAME("dataflash")
	.get_char	= env_dataflash_get_char,
	.load		= env_dataflash_load,
	.save		= env_save_ptr(env_dataflash_save),
};
