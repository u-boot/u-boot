/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <spi_flash.h>
#include <malloc.h>

#define ESPI_BOOT_IMAGE_SIZE	0x48
#define ESPI_BOOT_IMAGE_ADDR	0x50
#define CONFIG_CFG_DATA_SECTOR	0

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
void spi_boot(void)
{
	void (*uboot)(void) __noreturn;
	u32 offset, code_len;
	unsigned char *buf = NULL;
	struct spi_flash *flash;

	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (flash == NULL) {
		puts("\nspi_flash_probe failed");
		hang();
	}

	/*
	* Load U-Boot image from SPI flash into RAM
	*/
	buf = malloc(flash->page_size);
	if (buf == NULL) {
		puts("\nmalloc failed");
		hang();
	}
	memset(buf, 0, flash->page_size);

	spi_flash_read(flash, CONFIG_CFG_DATA_SECTOR,
		       flash->page_size, (void *)buf);
	offset = *(u32 *)(buf + ESPI_BOOT_IMAGE_ADDR);
	/* Skip spl code */
	offset += CONFIG_SYS_SPI_FLASH_U_BOOT_OFFS;
	/* Get the code size from offset 0x48 */
	code_len = *(u32 *)(buf + ESPI_BOOT_IMAGE_SIZE);
	/* Skip spl code */
	code_len = code_len - CONFIG_SPL_MAX_SIZE;
	/* copy code to DDR */
	spi_flash_read(flash, offset, code_len,
		       (void *)CONFIG_SYS_SPI_FLASH_U_BOOT_DST);
	/*
	* Jump to U-Boot image
	*/
	flush_cache(CONFIG_SYS_SPI_FLASH_U_BOOT_DST, code_len);
	uboot = (void *)CONFIG_SYS_SPI_FLASH_U_BOOT_START;
	(*uboot)();
}
