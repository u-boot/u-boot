/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>

#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS	0
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS	0
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ	1000000
#endif
#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE	SPI_MODE_3
#endif

#ifdef CONFIG_ENV_OFFSET_REDUND
static ulong env_offset		= CONFIG_ENV_OFFSET;
static ulong env_new_offset	= CONFIG_ENV_OFFSET_REDUND;

#define ACTIVE_FLAG	1
#define OBSOLETE_FLAG	0
#endif /* CONFIG_ENV_OFFSET_REDUND */

DECLARE_GLOBAL_DATA_PTR;

char *env_name_spec = "SPI Flash";

static struct spi_flash *env_flash;

#if defined(CONFIG_ENV_OFFSET_REDUND)
int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res, *saved_buffer = NULL, flag = OBSOLETE_FLAG;
	u32	saved_size, saved_offset, sector = 1;
	int	ret;

	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash) {
			set_default_env("!spi_flash_probe() failed");
			return 1;
		}
	}

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc	= crc32(0, env_new.data, ENV_SIZE);
	env_new.flags	= ACTIVE_FLAG;

	if (gd->env_valid == 1) {
		env_new_offset = CONFIG_ENV_OFFSET_REDUND;
		env_offset = CONFIG_ENV_OFFSET;
	} else {
		env_new_offset = CONFIG_ENV_OFFSET;
		env_offset = CONFIG_ENV_OFFSET_REDUND;
	}

	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = env_new_offset + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer) {
			ret = 1;
			goto done;
		}
		ret = spi_flash_read(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash, env_new_offset,
				sector * CONFIG_ENV_SECT_SIZE);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");

	ret = spi_flash_write(env_flash, env_new_offset,
		CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = spi_flash_write(env_flash, env_offset + offsetof(env_t, flags),
				sizeof(env_new.flags), &flag);
	if (ret)
		goto done;

	puts("done\n");

	gd->env_valid = gd->env_valid == 2 ? 1 : 2;

	printf("Valid environment: %d\n", (int)gd->env_valid);

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

void env_relocate_spec(void)
{
	int ret;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1 = NULL;
	env_t *tmp_env2 = NULL;
	env_t *ep = NULL;

	tmp_env1 = (env_t *)malloc(CONFIG_ENV_SIZE);
	tmp_env2 = (env_t *)malloc(CONFIG_ENV_SIZE);

	if (!tmp_env1 || !tmp_env2) {
		set_default_env("!malloc() failed");
		goto out;
	}

	env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!env_flash) {
		set_default_env("!spi_flash_probe() failed");
		goto out;
	}

	ret = spi_flash_read(env_flash, CONFIG_ENV_OFFSET,
				CONFIG_ENV_SIZE, tmp_env1);
	if (ret) {
		set_default_env("!spi_flash_read() failed");
		goto err_read;
	}

	if (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc)
		crc1_ok = 1;

	ret = spi_flash_read(env_flash, CONFIG_ENV_OFFSET_REDUND,
				CONFIG_ENV_SIZE, tmp_env2);
	if (!ret) {
		if (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc)
			crc2_ok = 1;
	}

	if (!crc1_ok && !crc2_ok) {
		set_default_env("!bad CRC");
		goto err_read;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = 1;
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = 2;
	} else if (tmp_env1->flags == ACTIVE_FLAG &&
		   tmp_env2->flags == OBSOLETE_FLAG) {
		gd->env_valid = 1;
	} else if (tmp_env1->flags == OBSOLETE_FLAG &&
		   tmp_env2->flags == ACTIVE_FLAG) {
		gd->env_valid = 2;
	} else if (tmp_env1->flags == tmp_env2->flags) {
		gd->env_valid = 2;
	} else if (tmp_env1->flags == 0xFF) {
		gd->env_valid = 2;
	} else {
		/*
		 * this differs from code in env_flash.c, but I think a sane
		 * default path is desirable.
		 */
		gd->env_valid = 2;
	}

	if (gd->env_valid == 1)
		ep = tmp_env1;
	else
		ep = tmp_env2;

	ret = env_import((char *)ep, 0);
	if (!ret) {
		error("Cannot import environment: errno = %d\n", errno);
		set_default_env("env_import failed");
	}

err_read:
	spi_flash_free(env_flash);
	env_flash = NULL;
out:
	free(tmp_env1);
	free(tmp_env2);
}
#else
int saveenv(void)
{
	u32	saved_size, saved_offset, sector = 1;
	char	*res, *saved_buffer = NULL;
	int	ret = 1;
	env_t	env_new;
	ssize_t	len;

	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash) {
			set_default_env("!spi_flash_probe() failed");
			return 1;
		}
	}

	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer)
			goto done;

		ret = spi_flash_read(env_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		goto done;
	}
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash, CONFIG_ENV_OFFSET,
		sector * CONFIG_ENV_SECT_SIZE);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");
	ret = spi_flash_write(env_flash, CONFIG_ENV_OFFSET,
		CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = 0;
	puts("done\n");

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

void env_relocate_spec(void)
{
	int ret;
	char *buf = NULL;

	buf = (char *)malloc(CONFIG_ENV_SIZE);
	env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!env_flash) {
		set_default_env("!spi_flash_probe() failed");
		if (buf)
			free(buf);
		return;
	}

	ret = spi_flash_read(env_flash,
		CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, buf);
	if (ret) {
		set_default_env("!spi_flash_read() failed");
		goto out;
	}

	ret = env_import(buf, 1);
	if (ret)
		gd->env_valid = 1;
out:
	spi_flash_free(env_flash);
	if (buf)
		free(buf);
	env_flash = NULL;
}
#endif

int env_init(void)
{
	/* SPI flash isn't usable before relocation */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}
