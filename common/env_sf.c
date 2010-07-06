/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
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
#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi_flash.h>

#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS	0
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS		0
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ	1000000
#endif
#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE	SPI_MODE_3
#endif

#ifdef CONFIG_ENV_OFFSET_REDUND
static ulong env_offset = CONFIG_ENV_OFFSET;
static ulong env_new_offset = CONFIG_ENV_OFFSET_REDUND;

#define ACTIVE_FLAG   1
#define OBSOLETE_FLAG 0
#endif /* CONFIG_ENV_ADDR_REDUND */

DECLARE_GLOBAL_DATA_PTR;

/* references to names in env_common.c */
extern uchar default_environment[];

char * env_name_spec = "SPI Flash";
env_t *env_ptr;

static struct spi_flash *env_flash;

uchar env_get_char_spec(int index)
{
	return *((uchar *)(gd->env_addr + index));
}

#if defined(CONFIG_ENV_OFFSET_REDUND)
void swap_env(void)
{
	ulong tmp_offset = env_offset;

	env_offset = env_new_offset;
	env_new_offset = tmp_offset;
}

int saveenv(void)
{
	u32 saved_size, saved_offset;
	char *saved_buffer = NULL;
	u32 sector = 1;
	int ret;
	char flag = OBSOLETE_FLAG, new_flag = ACTIVE_FLAG;

	if (!env_flash) {
		puts("Environment SPI flash not initialized\n");
		return 1;
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
	ret = spi_flash_write(env_flash,
		env_new_offset + offsetof(env_t, data),
		sizeof(env_ptr->data), env_ptr->data);
	if (ret)
		goto done;

	ret = spi_flash_write(env_flash,
		env_new_offset + offsetof(env_t, crc),
		sizeof(env_ptr->crc), &env_ptr->crc);
	if (ret)
		goto done;

	ret = spi_flash_write(env_flash,
		env_offset + offsetof(env_t, flags),
		sizeof(env_ptr->flags), &flag);
	if (ret)
		goto done;

	ret = spi_flash_write(env_flash,
		env_new_offset + offsetof(env_t, flags),
		sizeof(env_ptr->flags), &new_flag);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	swap_env();

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
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1 = NULL;
	env_t *tmp_env2 = NULL;
	uchar flag1, flag2;
	/* current_env is set only in case both areas are valid! */
	int current_env = 0;

	tmp_env1 = (env_t *)malloc(CONFIG_ENV_SIZE);
	if (!tmp_env1) {
		puts("*** Warning: could not init environment,"
			" using defaults\n\n");
		goto out;
	}

	tmp_env2 = (env_t *)malloc(CONFIG_ENV_SIZE);
	if (!tmp_env2) {
		puts("*** Warning: could not init environment,"
			" using defaults\n\n");
		goto out;
	}

	env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!env_flash)
		goto err_probe;

	ret = spi_flash_read(env_flash, CONFIG_ENV_OFFSET,
				CONFIG_ENV_SIZE, tmp_env1);
	if (ret)
		goto err_read;

	if (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc)
		crc1_ok = 1;
	flag1 = tmp_env1->flags;

	ret = spi_flash_read(env_flash, CONFIG_ENV_OFFSET_REDUND,
				CONFIG_ENV_SIZE, tmp_env2);
	if (!ret) {
		if (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc)
			crc2_ok = 1;
		flag2 = tmp_env2->flags;
	}

	if (!crc1_ok && !crc2_ok)
		goto err_crc;
	else if (crc1_ok && !crc2_ok) {
		gd->env_valid = 1;
		memcpy(env_ptr, tmp_env1, CONFIG_ENV_SIZE);
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = 1;
		memcpy(env_ptr, tmp_env2, CONFIG_ENV_SIZE);
		swap_env();
	} else if (flag1 == ACTIVE_FLAG && flag2 == OBSOLETE_FLAG) {
		gd->env_valid = 1;
		memcpy(env_ptr, tmp_env1, CONFIG_ENV_SIZE);
	} else if (flag1 == OBSOLETE_FLAG && flag2 == ACTIVE_FLAG) {
		gd->env_valid = 1;
		memcpy(env_ptr, tmp_env2, CONFIG_ENV_SIZE);
		swap_env();
	} else if (flag1 == flag2) {
		gd->env_valid = 2;
		memcpy(env_ptr, tmp_env1, CONFIG_ENV_SIZE);
		current_env = 1;
	} else if (flag1 == 0xFF) {
		gd->env_valid = 2;
		memcpy(env_ptr, tmp_env1, CONFIG_ENV_SIZE);
		current_env = 1;
	} else {
		/*
		 * this differs from code in env_flash.c, but I think a sane
		 * default path is desirable.
		 */
		gd->env_valid = 2;
		memcpy(env_ptr, tmp_env2, CONFIG_ENV_SIZE);
		swap_env();
		current_env = 2;
	}
	if (current_env == 1) {
		if (flag2 != OBSOLETE_FLAG) {
			flag2 = OBSOLETE_FLAG;
			spi_flash_write(env_flash,
				env_new_offset + offsetof(env_t, flags),
				sizeof(env_ptr->flags), &flag2);
		}
		if (flag1 != ACTIVE_FLAG) {
			flag1 = ACTIVE_FLAG;
			spi_flash_write(env_flash,
				env_offset + offsetof(env_t, flags),
				sizeof(env_ptr->flags), &flag1);
		}
	} else if (current_env == 2) {
		if (flag1 != OBSOLETE_FLAG) {
			flag1 = OBSOLETE_FLAG;
			spi_flash_write(env_flash,
				env_new_offset + offsetof(env_t, flags),
				sizeof(env_ptr->flags), &flag1);
		}
		if (flag2 != ACTIVE_FLAG) {
			flag2 = ACTIVE_FLAG;
			spi_flash_write(env_flash,
				env_offset + offsetof(env_t, flags),
				sizeof(env_ptr->flags), &flag2);
		}
	}
	if (gd->env_valid == 2) {
		puts("*** Warning - some problems detected "
			"reading environment; recovered successfully\n\n");
	}
	if (tmp_env1)
		free(tmp_env1);
	if (tmp_env2)
		free(tmp_env2);
	return;

err_read:
	spi_flash_free(env_flash);
	env_flash = NULL;
err_probe:
err_crc:
	puts("*** Warning - bad CRC, using default environment\n\n");
out:
	if (tmp_env1)
		free(tmp_env1);
	if (tmp_env2)
		free(tmp_env2);
	set_default_env();
}
#else
int saveenv(void)
{
	u32 saved_size, saved_offset;
	char *saved_buffer = NULL;
	u32 sector = 1;
	int ret;

	if (!env_flash) {
		puts("Environment SPI flash not initialized\n");
		return 1;
	}

	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer) {
			ret = 1;
			goto done;
		}
		ret = spi_flash_read(env_flash, saved_offset, saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash, CONFIG_ENV_OFFSET, sector * CONFIG_ENV_SECT_SIZE);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");
	ret = spi_flash_write(env_flash, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, env_ptr);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset, saved_size, saved_buffer);
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

	env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!env_flash)
		goto err_probe;

	ret = spi_flash_read(env_flash, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, env_ptr);
	if (ret)
		goto err_read;

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		goto err_crc;

	gd->env_valid = 1;

	return;

err_read:
	spi_flash_free(env_flash);
	env_flash = NULL;
err_probe:
err_crc:
	puts("*** Warning - bad CRC, using default environment\n\n");

	set_default_env();
}
#endif

int env_init(void)
{
	/* SPI flash isn't usable before relocation */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}
