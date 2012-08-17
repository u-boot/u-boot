/*
 * (C) Copyright 2009
 * Magnus Lilja <lilja.magnus@gmail.com>
 *
 * (C) Copyright 2008
 * Maxim Artamonov, <scn1874 at yandex.ru>
 *
 * (C) Copyright 2006-2008
 * Stefan Roese, DENX Software Engineering, sr at denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <nand.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <fsl_nfc.h>

static struct fsl_nfc_regs *const nfc = (void *)NFC_BASE_ADDR;

static void nfc_wait_ready(void)
{
	uint32_t tmp;

	while (!(readw(&nfc->nand_flash_config2) & NFC_INT))
		;

	/* Reset interrupt flag */
	tmp = readw(&nfc->nand_flash_config2);
	tmp &= ~NFC_INT;
	writew(tmp, &nfc->nand_flash_config2);
}

void nfc_nand_init(void)
{
#if defined(MXC_NFC_V1_1)
	int ecc_per_page  = CONFIG_SYS_NAND_PAGE_SIZE / 512;
	int config1;

	writew(CONFIG_SYS_NAND_SPARE_SIZE / 2, &nfc->spare_area_size);

	/* unlocking RAM Buff */
	writew(0x2, &nfc->configuration);

	/* hardware ECC checking and correct */
	config1 = readw(&nfc->nand_flash_config1) | NFC_ECC_EN | 0x800;
	/*
	 * if spare size is larger that 16 bytes per 512 byte hunk
	 * then use 8 symbol correction instead of 4
	 */
	if ((CONFIG_SYS_NAND_SPARE_SIZE / ecc_per_page) > 16)
		config1 &= ~NFC_4_8N_ECC;
	else
		config1 |= NFC_4_8N_ECC;
	writew(config1, &nfc->nand_flash_config1);
#elif defined(MXC_NFC_V1)
	/* unlocking RAM Buff */
	writew(0x2, &nfc->configuration);

	/* hardware ECC checking and correct */
	writew(NFC_ECC_EN, &nfc->nand_flash_config1);
#endif
}

static void nfc_nand_command(unsigned short command)
{
	writew(command, &nfc->flash_cmd);
	writew(NFC_CMD, &nfc->nand_flash_config2);
	nfc_wait_ready();
}

static void nfc_nand_page_address(unsigned int page_address)
{
	unsigned int page_count;

	writew(0x00, &nfc->flash_add);
	writew(NFC_ADDR, &nfc->nand_flash_config2);
	nfc_wait_ready();

	/* code only for large page flash */
	if (CONFIG_SYS_NAND_PAGE_SIZE > 512) {
		writew(0x00, &nfc->flash_add);
		writew(NFC_ADDR, &nfc->nand_flash_config2);
		nfc_wait_ready();
	}

	page_count = CONFIG_SYS_NAND_SIZE / CONFIG_SYS_NAND_PAGE_SIZE;

	if (page_address <= page_count) {
		page_count--; /* transform 0x01000000 to 0x00ffffff */
		do {
			writew(page_address & 0xff, &nfc->flash_add);
			writew(NFC_ADDR, &nfc->nand_flash_config2);
			nfc_wait_ready();
			page_address = page_address >> 8;
			page_count = page_count >> 8;
		} while (page_count);
	}

	writew(0x00, &nfc->flash_add);
	writew(NFC_ADDR, &nfc->nand_flash_config2);
	nfc_wait_ready();
}

static void nfc_nand_data_output(void)
{
	int config1 = readw(&nfc->nand_flash_config1);
#ifdef NAND_MXC_2K_MULTI_CYCLE
	int i;
#endif

	config1 |= NFC_ECC_EN | NFC_INT_MSK;
	writew(config1, &nfc->nand_flash_config1);
	writew(0, &nfc->buffer_address);
	writew(NFC_OUTPUT, &nfc->nand_flash_config2);
	nfc_wait_ready();
#ifdef NAND_MXC_2K_MULTI_CYCLE
	/*
	 * This NAND controller requires multiple input commands
	 * for pages larger than 512 bytes.
	 */
	for (i = 1; i < (CONFIG_SYS_NAND_PAGE_SIZE / 512); i++) {
		config1 = readw(&nfc->nand_flash_config1);
		config1 |= NFC_ECC_EN | NFC_INT_MSK;
		writew(config1, &nfc->nand_flash_config1);
		writew(i, &nfc->buffer_address);
		writew(NFC_OUTPUT, &nfc->nand_flash_config2);
		nfc_wait_ready();
	}
#endif
}

static int nfc_nand_check_ecc(void)
{
	return readw(&nfc->ecc_status_result);
}

static int nfc_read_page(unsigned int page_address, unsigned char *buf)
{
	int i;
	u32 *src;
	u32 *dst;

	writew(0, &nfc->buffer_address); /* read in first 0 buffer */
	nfc_nand_command(NAND_CMD_READ0);
	nfc_nand_page_address(page_address);

	if (CONFIG_SYS_NAND_PAGE_SIZE > 512)
		nfc_nand_command(NAND_CMD_READSTART);

	nfc_nand_data_output(); /* fill the main buffer 0 */

	if (nfc_nand_check_ecc())
		return -1;

	src = &nfc->main_area[0][0];
	dst = (u32 *)buf;

	/* main copy loop from NAND-buffer to SDRAM memory */
	for (i = 0; i < (CONFIG_SYS_NAND_PAGE_SIZE / 4); i++) {
		writel(readl(src), dst);
		src++;
		dst++;
	}

	return 0;
}

static int is_badblock(int pagenumber)
{
	int page = pagenumber;
	u32 badblock;
	u32 *src;

	/* Check the first two pages for bad block markers */
	for (page = pagenumber; page < pagenumber + 2; page++) {
		writew(0, &nfc->buffer_address); /* read in first 0 buffer */
		nfc_nand_command(NAND_CMD_READ0);
		nfc_nand_page_address(page);

		if (CONFIG_SYS_NAND_PAGE_SIZE > 512)
			nfc_nand_command(NAND_CMD_READSTART);

		nfc_nand_data_output(); /* fill the main buffer 0 */

		src = &nfc->spare_area[0][0];

		/*
		 * IMPORTANT NOTE: The nand flash controller uses a non-
		 * standard layout for large page devices. This can
		 * affect the position of the bad block marker.
		 */
		/* Get the bad block marker */
		badblock = readl(&src[CONFIG_SYS_NAND_BAD_BLOCK_POS / 4]);
		badblock >>= 8 * (CONFIG_SYS_NAND_BAD_BLOCK_POS % 4);
		badblock &= 0xff;

		/* bad block marker verify */
		if (badblock != 0xff)
			return 1; /* potential bad block */
	}

	return 0;
}

static int nand_load(unsigned int from, unsigned int size, unsigned char *buf)
{
	int i;
	unsigned int page;
	unsigned int maxpages = CONFIG_SYS_NAND_SIZE /
				CONFIG_SYS_NAND_PAGE_SIZE;

	nfc_nand_init();

	/* Convert to page number */
	page = from / CONFIG_SYS_NAND_PAGE_SIZE;
	i = 0;

	while (i < (size / CONFIG_SYS_NAND_PAGE_SIZE)) {
		if (nfc_read_page(page, buf) < 0)
			return -1;

		page++;
		i++;
		buf = buf + CONFIG_SYS_NAND_PAGE_SIZE;

		/*
		 * Check if we have crossed a block boundary, and if so
		 * check for bad block.
		 */
		if (!(page % CONFIG_SYS_NAND_PAGE_COUNT)) {
			/*
			 * Yes, new block. See if this block is good. If not,
			 * loop until we find a good block.
			 */
			while (is_badblock(page)) {
				page = page + CONFIG_SYS_NAND_PAGE_COUNT;
				/* Check i we've reached the end of flash. */
				if (page >= maxpages)
					return -1;
			}
		}
	}

	return 0;
}

#if defined(CONFIG_ARM)
void board_init_f (ulong bootflag)
{
	relocate_code (CONFIG_SYS_TEXT_BASE - TOTAL_MALLOC_LEN, NULL,
		       CONFIG_SYS_TEXT_BASE);
}
#endif

/*
 * The main entry for NAND booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from NAND into SDRAM and starts it from there.
 */
void nand_boot(void)
{
	__attribute__((noreturn)) void (*uboot)(void);

	/*
	 * CONFIG_SYS_NAND_U_BOOT_OFFS and CONFIG_SYS_NAND_U_BOOT_SIZE must
	 * be aligned to full pages
	 */
	if (!nand_load(CONFIG_SYS_NAND_U_BOOT_OFFS, CONFIG_SYS_NAND_U_BOOT_SIZE,
		       (uchar *)CONFIG_SYS_NAND_U_BOOT_DST)) {
		/* Copy from NAND successful, start U-boot */
		uboot = (void *)CONFIG_SYS_NAND_U_BOOT_START;
		uboot();
	} else {
		/* Unrecoverable error when copying from NAND */
		hang();
	}
}

/*
 * Called in case of an exception.
 */
void hang(void)
{
	/* Loop forever */
	while (1) ;
}
