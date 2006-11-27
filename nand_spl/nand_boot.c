/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#define CFG_NAND_READ_DELAY \
	{ volatile int dummy; int i; for (i=0; i<10000; i++) dummy = i; }

extern void board_nand_init(struct nand_chip *nand);
extern void ndfc_hwcontrol(struct mtd_info *mtdinfo, int cmd);
extern void ndfc_write_byte(struct mtd_info *mtdinfo, u_char byte);
extern u_char ndfc_read_byte(struct mtd_info *mtdinfo);
extern int ndfc_dev_ready(struct mtd_info *mtdinfo);
extern int jump_to_ram(ulong delta);
extern int jump_to_uboot(ulong addr);

static int nand_is_bad_block(struct mtd_info *mtd, int block)
{
	struct nand_chip *this = mtd->priv;
	int page_addr = block * CFG_NAND_PAGE_COUNT;

	/* Begin command latch cycle */
	this->hwcontrol(mtd, NAND_CTL_SETCLE);
	this->write_byte(mtd, NAND_CMD_READOOB);
	/* Set ALE and clear CLE to start address cycle */
	this->hwcontrol(mtd, NAND_CTL_CLRCLE);
	this->hwcontrol(mtd, NAND_CTL_SETALE);
	/* Column address */
	this->write_byte(mtd, CFG_NAND_BAD_BLOCK_POS);			/* A[7:0] */
	this->write_byte(mtd, (uchar)(page_addr & 0xff));		/* A[16:9] */
	this->write_byte(mtd, (uchar)((page_addr >> 8) & 0xff));	/* A[24:17] */
#ifdef CFG_NAND_4_ADDR_CYCLE
	/* One more address cycle for devices > 32MiB */
	this->write_byte(mtd, (uchar)((page_addr >> 16) & 0x0f));	/* A[xx:25] */
#endif
	/* Latch in address */
	this->hwcontrol(mtd, NAND_CTL_CLRALE);

	/*
	 * Wait a while for the data to be ready
	 */
	if (this->dev_ready)
		this->dev_ready(mtd);
	else
		CFG_NAND_READ_DELAY;

	/*
	 * Read on byte
	 */
	if (this->read_byte(mtd) != 0xff)
		return 1;

	return 0;
}

static int nand_read_page(struct mtd_info *mtd, int block, int page, uchar *dst)
{
	struct nand_chip *this = mtd->priv;
	int page_addr = page + block * CFG_NAND_PAGE_COUNT;
	int i;

	/* Begin command latch cycle */
	this->hwcontrol(mtd, NAND_CTL_SETCLE);
	this->write_byte(mtd, NAND_CMD_READ0);
	/* Set ALE and clear CLE to start address cycle */
	this->hwcontrol(mtd, NAND_CTL_CLRCLE);
	this->hwcontrol(mtd, NAND_CTL_SETALE);
	/* Column address */
	this->write_byte(mtd, 0);					/* A[7:0] */
	this->write_byte(mtd, (uchar)(page_addr & 0xff));		/* A[16:9] */
	this->write_byte(mtd, (uchar)((page_addr >> 8) & 0xff));	/* A[24:17] */
#ifdef CFG_NAND_4_ADDR_CYCLE
	/* One more address cycle for devices > 32MiB */
	this->write_byte(mtd, (uchar)((page_addr >> 16) & 0x0f));	/* A[xx:25] */
#endif
	/* Latch in address */
	this->hwcontrol(mtd, NAND_CTL_CLRALE);

	/*
	 * Wait a while for the data to be ready
	 */
	if (this->dev_ready)
		this->dev_ready(mtd);
	else
		CFG_NAND_READ_DELAY;

	/*
	 * Read page into buffer
	 */
	for (i=0; i<CFG_NAND_PAGE_SIZE; i++)
		*dst++ = this->read_byte(mtd);

	return 0;
}

static int nand_load(struct mtd_info *mtd, int offs, int uboot_size, uchar *dst)
{
	int block;
	int blockcopy_count;
	int page;

	/*
	 * offs has to be aligned to a block address!
	 */
	block = offs / CFG_NAND_BLOCK_SIZE;
	blockcopy_count = 0;

	while (blockcopy_count < (uboot_size / CFG_NAND_BLOCK_SIZE)) {
		if (!nand_is_bad_block(mtd, block)) {
			/*
			 * Skip bad blocks
			 */
			for (page = 0; page < CFG_NAND_PAGE_COUNT; page++) {
				nand_read_page(mtd, block, page, dst);
				dst += CFG_NAND_PAGE_SIZE;
			}

			blockcopy_count++;
		}

		block++;
	}

	return 0;
}

void nand_boot(void)
{
	ulong mem_size;
	struct nand_chip nand_chip;
	nand_info_t nand_info;
	int ret;
	void (*uboot)(void);

	/*
	 * Init sdram, so we have access to memory
	 */
	mem_size = initdram(0);

	/*
	 * Init board specific nand support
	 */
	nand_info.priv = &nand_chip;
	nand_chip.IO_ADDR_R = nand_chip.IO_ADDR_W = (void  __iomem *)CFG_NAND_BASE;
	nand_chip.dev_ready = NULL;	/* preset to NULL */
	board_nand_init(&nand_chip);

	/*
	 * Load U-Boot image from NAND into RAM
	 */
	ret = nand_load(&nand_info, CFG_NAND_U_BOOT_OFFS, CFG_NAND_U_BOOT_SIZE,
			(uchar *)CFG_NAND_U_BOOT_DST);

	/*
	 * Jump to U-Boot image
	 */
	uboot = (void (*)(void))CFG_NAND_U_BOOT_START;
	(*uboot)();
}
