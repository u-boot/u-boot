/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#ifdef CONFIG_FSL_CORENET
	offset = CONFIG_SYS_SPI_FLASH_U_BOOT_OFFS;
	code_len = CONFIG_SYS_SPI_FLASH_U_BOOT_SIZE;
#else
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
#endif
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
