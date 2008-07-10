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

#ifdef CFG_ENV_IS_IN_SPI_FLASH

#include <environment.h>
#include <spi_flash.h>

#ifndef CFG_ENV_SPI_BUS
# define CFG_ENV_SPI_BUS	0
#endif
#ifndef CFG_ENV_SPI_CS
# define CFG_ENV_SPI_CS		0
#endif
#ifndef CFG_ENV_SPI_MAX_HZ
# define CFG_ENV_SPI_MAX_HZ	1000000
#endif
#ifndef CFG_ENV_SPI_MODE
# define CFG_ENV_SPI_MODE	SPI_MODE_3
#endif

DECLARE_GLOBAL_DATA_PTR;

/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

char * env_name_spec = "SPI Flash";
env_t *env_ptr;

static struct spi_flash *env_flash;

uchar env_get_char_spec(int index)
{
	return *((uchar *)(gd->env_addr + index));
}

int saveenv(void)
{
	if (!env_flash) {
		puts("Environment SPI flash not initialized\n");
		return 1;
	}

	puts("Erasing SPI flash...");
	if (spi_flash_erase(env_flash, CFG_ENV_OFFSET, CFG_ENV_SIZE))
		return 1;

	puts("Writing to SPI flash...");
	if (spi_flash_write(env_flash, CFG_ENV_OFFSET, CFG_ENV_SIZE, env_ptr))
		return 1;

	puts("done\n");
	return 0;
}

void env_relocate_spec(void)
{
	int ret;

	env_flash = spi_flash_probe(CFG_ENV_SPI_BUS, CFG_ENV_SPI_CS,
			CFG_ENV_SPI_MAX_HZ, CFG_ENV_SPI_MODE);
	if (!env_flash)
		goto err_probe;

	ret = spi_flash_read(env_flash, CFG_ENV_OFFSET, CFG_ENV_SIZE, env_ptr);
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

	if (default_environment_size > CFG_ENV_SIZE) {
		gd->env_valid = 0;
		puts("*** Error - default environment is too large\n\n");
		return;
	}

	memset(env_ptr, 0, sizeof(env_t));
	memcpy(env_ptr->data, default_environment, default_environment_size);
	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
	gd->env_valid = 1;
}

int env_init(void)
{
	/* SPI flash isn't usable before relocation */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#endif /* CFG_ENV_IS_IN_SPI_FLASH */
