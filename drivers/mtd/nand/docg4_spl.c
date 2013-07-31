/*
 * SPL driver for Diskonchip G4 nand flash
 *
 * Copyright (C) 2013 Mike Dunn <mikedunn@newsguy.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * This driver basically mimics the load functionality of a typical IPL (initial
 * program loader) resident in the 2k NOR-like region of the docg4 that is
 * mapped to the reset vector.  It allows the u-boot SPL to continue loading if
 * the IPL loads a fixed number of flash blocks that is insufficient to contain
 * the entire u-boot image.  In this case, a concatenated spl + u-boot image is
 * written at the flash offset from which the IPL loads an image, and when the
 * IPL jumps to the SPL, the SPL resumes loading where the IPL left off.  See
 * the palmtreo680 for an example.
 *
 * This driver assumes that the data was written to the flash using the device's
 * "reliable" mode, and also assumes that each 512 byte page is stored
 * redundantly in the subsequent page.  This storage format is likely to be used
 * by all boards that boot from the docg4.  The format compensates for the lack
 * of ecc in the IPL.
 *
 * Reliable mode reduces the capacity of a block by half, and the redundant
 * pages reduce it by half again.  As a result, the normal 256k capacity of a
 * block is reduced to 64k for the purposes of the IPL/SPL.
 */

#include <asm/io.h>
#include <linux/mtd/docg4.h>

/* forward declarations */
static inline void write_nop(void __iomem *docptr);
static int poll_status(void __iomem *docptr);
static void write_addr(void __iomem *docptr, uint32_t docg4_addr);
static void address_sequence(unsigned int g4_page, unsigned int g4_index,
			     void __iomem *docptr);
static int docg4_load_block_reliable(uint32_t flash_offset, void *dest_addr);

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	void *load_addr = dst;
	uint32_t flash_offset = offs;
	const unsigned int block_count =
		(size + DOCG4_BLOCK_CAPACITY_SPL - 1)
		/ DOCG4_BLOCK_CAPACITY_SPL;
	int i;

	for (i = 0; i < block_count; i++) {
		int ret = docg4_load_block_reliable(flash_offset, load_addr);
		if (ret)
			return ret;
		load_addr += DOCG4_BLOCK_CAPACITY_SPL;
		flash_offset += DOCG4_BLOCK_SIZE;
	}
	return 0;
}

static inline void write_nop(void __iomem *docptr)
{
	writew(0, docptr + DOC_NOP);
}

static int poll_status(void __iomem *docptr)
{
	/*
	 * Busy-wait for the FLASHREADY bit to be set in the FLASHCONTROL
	 * register.  Operations known to take a long time (e.g., block erase)
	 * should sleep for a while before calling this.
	 */

	uint8_t flash_status;

	/* hardware quirk requires reading twice initially */
	flash_status = readb(docptr + DOC_FLASHCONTROL);

	do {
		flash_status = readb(docptr + DOC_FLASHCONTROL);
	} while (!(flash_status & DOC_CTRL_FLASHREADY));

	return 0;
}

static void write_addr(void __iomem *docptr, uint32_t docg4_addr)
{
	/* write the four address bytes packed in docg4_addr to the device */

	writeb(docg4_addr & 0xff, docptr + DOC_FLASHADDRESS);
	docg4_addr >>= 8;
	writeb(docg4_addr & 0xff, docptr + DOC_FLASHADDRESS);
	docg4_addr >>= 8;
	writeb(docg4_addr & 0xff, docptr + DOC_FLASHADDRESS);
	docg4_addr >>= 8;
	writeb(docg4_addr & 0xff, docptr + DOC_FLASHADDRESS);
}

static void address_sequence(unsigned int g4_page, unsigned int g4_index,
			     void __iomem *docptr)
{
	writew(DOCG4_SEQ_PAGE_READ, docptr + DOC_FLASHSEQUENCE);
	writew(DOCG4_CMD_PAGE_READ, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_addr(docptr, ((uint32_t)g4_page << 16) | g4_index);
	write_nop(docptr);
}

static int docg4_load_block_reliable(uint32_t flash_offset, void *dest_addr)
{
	void __iomem *docptr = (void *)CONFIG_SYS_NAND_BASE;
	unsigned int g4_page = flash_offset >> 11; /* 2k page */
	const unsigned int last_g4_page = g4_page + 0x80; /* last in block */
	int g4_index = 0;
	uint16_t flash_status;
	uint16_t *buf;

	/* flash_offset must be aligned to the start of a block */
	if (flash_offset & 0x3ffff)
		return -1;

	writew(DOC_SEQ_RESET, docptr + DOC_FLASHSEQUENCE);
	writew(DOC_CMD_RESET, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_nop(docptr);
	poll_status(docptr);
	write_nop(docptr);
	writew(0x45, docptr + DOC_FLASHSEQUENCE);
	writew(0xa3, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	writew(0x22, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);

	/* read 1st 4 oob bytes of first subpage of block */
	address_sequence(g4_page, 0x0100, docptr); /* index at oob */
	write_nop(docptr);
	flash_status = readw(docptr + DOC_FLASHCONTROL);
	flash_status = readw(docptr + DOC_FLASHCONTROL);
	if (flash_status & 0x06) /* sequence or protection errors */
		return -1;
	writew(DOCG4_CMD_READ2, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_nop(docptr);
	poll_status(docptr);
	writew(DOC_ECCCONF0_READ_MODE | 4, docptr + DOC_ECCCONF0);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);

	/*
	 * Here we read the first four oob bytes of the first page of the block.
	 * The IPL on the palmtreo680 requires that this contain a 32 bit magic
	 * number, or the load aborts.  We'll ignore it.
	 */
	readw(docptr + 0x103c); /* hw quirk; 1st read discarded */
	readw(docptr + 0x103c);	/* lower 16 bits of magic number */
	readw(docptr + DOCG4_MYSTERY_REG); /* upper 16 bits of magic number */
	writew(0, docptr + DOC_DATAEND);
	write_nop(docptr);
	write_nop(docptr);

	/* load contents of block to memory */
	buf = (uint16_t *)dest_addr;
	do {
		int i;

		address_sequence(g4_page, g4_index, docptr);
		writew(DOCG4_CMD_READ2,
		       docptr + DOC_FLASHCOMMAND);
		write_nop(docptr);
		write_nop(docptr);
		poll_status(docptr);
		writew(DOC_ECCCONF0_READ_MODE |
		       DOC_ECCCONF0_ECC_ENABLE |
		       DOCG4_BCH_SIZE,
		       docptr + DOC_ECCCONF0);
		write_nop(docptr);
		write_nop(docptr);
		write_nop(docptr);
		write_nop(docptr);
		write_nop(docptr);

		/* read the 512 bytes of page data, 2 bytes at a time */
		readw(docptr + 0x103c); /* hw quirk */
		for (i = 0; i < 256; i++)
			*buf++ = readw(docptr + 0x103c);

		/* read oob, but discard it */
		for (i = 0; i < 7; i++)
			readw(docptr + 0x103c);
		readw(docptr + DOCG4_OOB_6_7);
		readw(docptr + DOCG4_OOB_6_7);

		writew(0, docptr + DOC_DATAEND);
		write_nop(docptr);
		write_nop(docptr);

		if (!(g4_index & 0x100)) {
			/* not redundant subpage read; check for ecc error */
			write_nop(docptr);
			flash_status = readw(docptr + DOC_ECCCONF1);
			flash_status = readw(docptr + DOC_ECCCONF1);
			if (flash_status & 0x80) { /* ecc error */
				g4_index += 0x108; /* read redundant subpage */
				buf -= 256;        /* back up ram ptr */
				continue;
			} else                       /* no ecc error */
				g4_index += 0x210; /* skip redundant subpage */
		} else  /* redundant page was just read; skip ecc error check */
			g4_index += 0x108;

		if (g4_index == 0x420) { /* finished with 2k page */
			g4_index = 0;
			g4_page += 2; /* odd-numbered 2k pages skipped */
		}

	} while (g4_page != last_g4_page); /* while still on same block */

	return 0;
}
