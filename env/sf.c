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
#include <dm.h>
#include <environment.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>
#include <dm/device-internal.h>

#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS	CONFIG_SF_DEFAULT_BUS
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS	CONFIG_SF_DEFAULT_CS
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED
#endif
#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE	CONFIG_SF_DEFAULT_MODE
#endif

#ifndef CONFIG_SPL_BUILD
#define CMD_SAVEENV
#endif

#ifdef CONFIG_ENV_OFFSET_REDUND
#ifdef CMD_SAVEENV
static ulong env_offset		= CONFIG_ENV_OFFSET;
static ulong env_new_offset	= CONFIG_ENV_OFFSET_REDUND;
#endif

#define ACTIVE_FLAG	1
#define OBSOLETE_FLAG	0
#endif /* CONFIG_ENV_OFFSET_REDUND */

DECLARE_GLOBAL_DATA_PTR;

static struct spi_flash *env_flash;

static int setup_flash_device(void)
{
#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *new;
	int	ret;

	/* speed and mode will be read from DT */
	ret = spi_flash_probe_bus_cs(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     0, 0, &new);
	if (ret) {
		set_default_env("!spi_flash_probe_bus_cs() failed");
		return ret;
	}

	env_flash = dev_get_uclass_priv(new);
#else

	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash) {
			set_default_env("!spi_flash_probe() failed");
			return -EIO;
		}
	}
#endif
	return 0;
}

#if defined(CONFIG_ENV_OFFSET_REDUND)
#ifdef CMD_SAVEENV
static int env_sf_save(void)
{
	env_t	env_new;
	char	*saved_buffer = NULL, flag = OBSOLETE_FLAG;
	u32	saved_size, saved_offset, sector;
	int	ret;

	ret = setup_flash_device();
	if (ret)
		return ret;

	ret = env_export(&env_new);
	if (ret)
		return -EIO;
	env_new.flags	= ACTIVE_FLAG;

	if (gd->env_valid == ENV_VALID) {
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
		saved_buffer = memalign(ARCH_DMA_MINALIGN, saved_size);
		if (!saved_buffer) {
			ret = -ENOMEM;
			goto done;
		}
		ret = spi_flash_read(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	sector = DIV_ROUND_UP(CONFIG_ENV_SIZE, CONFIG_ENV_SECT_SIZE);

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

	gd->env_valid = gd->env_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;

	printf("Valid environment: %d\n", (int)gd->env_valid);

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}
#endif /* CMD_SAVEENV */

static int env_sf_load(void)
{
	int ret;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1 = NULL;
	env_t *tmp_env2 = NULL;
	env_t *ep = NULL;

	tmp_env1 = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_ENV_SIZE);
	tmp_env2 = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_ENV_SIZE);
	if (!tmp_env1 || !tmp_env2) {
		set_default_env("!malloc() failed");
		ret = -EIO;
		goto out;
	}

	ret = setup_flash_device();
	if (ret)
		goto out;

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
		ret = -EIO;
		goto err_read;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = ENV_VALID;
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = ENV_REDUND;
	} else if (tmp_env1->flags == ACTIVE_FLAG &&
		   tmp_env2->flags == OBSOLETE_FLAG) {
		gd->env_valid = ENV_VALID;
	} else if (tmp_env1->flags == OBSOLETE_FLAG &&
		   tmp_env2->flags == ACTIVE_FLAG) {
		gd->env_valid = ENV_REDUND;
	} else if (tmp_env1->flags == tmp_env2->flags) {
		gd->env_valid = ENV_VALID;
	} else if (tmp_env1->flags == 0xFF) {
		gd->env_valid = ENV_VALID;
	} else if (tmp_env2->flags == 0xFF) {
		gd->env_valid = ENV_REDUND;
	} else {
		/*
		 * this differs from code in env_flash.c, but I think a sane
		 * default path is desirable.
		 */
		gd->env_valid = ENV_VALID;
	}

	if (gd->env_valid == ENV_VALID)
		ep = tmp_env1;
	else
		ep = tmp_env2;

	ret = env_import((char *)ep, 0);
	if (!ret) {
		pr_err("Cannot import environment: errno = %d\n", errno);
		set_default_env("!env_import failed");
	}

err_read:
	spi_flash_free(env_flash);
	env_flash = NULL;
out:
	free(tmp_env1);
	free(tmp_env2);

	return ret;
}
#else
#ifdef CMD_SAVEENV
static int env_sf_save(void)
{
	u32	saved_size, saved_offset, sector;
	char	*saved_buffer = NULL;
	int	ret = 1;
	env_t	env_new;

	ret = setup_flash_device();
	if (ret)
		return ret;

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

	ret = env_export(&env_new);
	if (ret)
		goto done;

	sector = DIV_ROUND_UP(CONFIG_ENV_SIZE, CONFIG_ENV_SECT_SIZE);

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
#endif /* CMD_SAVEENV */

static int env_sf_load(void)
{
	int ret;
	char *buf = NULL;

	buf = (char *)memalign(ARCH_DMA_MINALIGN, CONFIG_ENV_SIZE);
	if (!buf) {
		set_default_env("!malloc() failed");
		return -EIO;
	}

	ret = setup_flash_device();
	if (ret)
		goto out;

	ret = spi_flash_read(env_flash,
		CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, buf);
	if (ret) {
		set_default_env("!spi_flash_read() failed");
		goto err_read;
	}

	ret = env_import(buf, 1);
	if (ret)
		gd->env_valid = ENV_VALID;

err_read:
	spi_flash_free(env_flash);
	env_flash = NULL;
out:
	free(buf);

	return ret;
}
#endif

U_BOOT_ENV_LOCATION(sf) = {
	.location	= ENVL_SPI_FLASH,
	ENV_NAME("SPI Flash")
	.load		= env_sf_load,
#ifdef CMD_SAVEENV
	.save		= env_save_ptr(env_sf_save),
#endif
};
