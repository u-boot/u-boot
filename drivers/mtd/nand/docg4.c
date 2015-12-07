/*
 * drivers/mtd/nand/docg4.c
 *
 * Copyright (C) 2013 Mike Dunn <mikedunn@newsguy.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * mtd nand driver for M-Systems DiskOnChip G4
 *
 * Tested on the Palm Treo 680.  The G4 is also present on Toshiba Portege, Asus
 * P526, some HTC smartphones (Wizard, Prophet, ...), O2 XDA Zinc, maybe others.
 * Should work on these as well.  Let me know!
 *
 * TODO:
 *
 *  Mechanism for management of password-protected areas
 *
 *  Hamming ecc when reading oob only
 *
 *  According to the M-Sys documentation, this device is also available in a
 *  "dual-die" configuration having a 256MB capacity, but no mechanism for
 *  detecting this variant is documented.  Currently this driver assumes 128MB
 *  capacity.
 *
 *  Support for multiple cascaded devices ("floors").  Not sure which gadgets
 *  contain multiple G4s in a cascaded configuration, if any.
 */


#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/errno.h>
#include <malloc.h>
#include <nand.h>
#include <linux/bch.h>
#include <linux/bitrev.h>
#include <linux/mtd/docg4.h>

/*
 * The device has a nop register which M-Sys claims is for the purpose of
 * inserting precise delays.  But beware; at least some operations fail if the
 * nop writes are replaced with a generic delay!
 */
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

/*
 * This is a module parameter in the linux kernel version of this driver.  It is
 * hard-coded to 'off' for u-boot.  This driver uses oob to mark bad blocks.
 * This can be problematic when dealing with data not intended for the mtd/nand
 * subsystem.  For example, on boards that boot from the docg4 and use the IPL
 * to load an spl + u-boot image, the blocks containing the image will be
 * reported as "bad" because the oob of the first page of each block contains a
 * magic number that the IPL looks for, which causes the badblock scan to
 * erroneously add them to the bad block table.  To erase such a block, use
 * u-boot's 'nand scrub'.  scrub is safe for the docg4.  The device does have a
 * factory bad block table, but it is read-only, and is used in conjunction with
 * oob bad block markers that are written by mtd/nand when a block is deemed to
 * be bad.  To read data from "bad" blocks, use 'read.raw'.  Unfortunately,
 * read.raw does not use ecc, which would still work fine on such misidentified
 * bad blocks.  TODO: u-boot nand utilities need the ability to ignore bad
 * blocks.
 */
static const int ignore_badblocks; /* remains false */

struct docg4_priv {
	int status;
	struct {
		unsigned int command;
		int column;
		int page;
	} last_command;
	uint8_t oob_buf[16];
	uint8_t ecc_buf[7];
	int oob_page;
	struct bch_control *bch;
};
/*
 * Oob bytes 0 - 6 are available to the user.
 * Byte 7 is hamming ecc for first 7 bytes.  Bytes 8 - 14 are hw-generated ecc.
 * Byte 15 (the last) is used by the driver as a "page written" flag.
 */
static struct nand_ecclayout docg4_oobinfo = {
	.eccbytes = 9,
	.eccpos = {7, 8, 9, 10, 11, 12, 13, 14, 15},
	.oobavail = 7,
	.oobfree = { {0, 7} }
};

static void reset(void __iomem *docptr)
{
	/* full device reset */

	writew(DOC_ASICMODE_RESET | DOC_ASICMODE_MDWREN, docptr + DOC_ASICMODE);
	writew(~(DOC_ASICMODE_RESET | DOC_ASICMODE_MDWREN),
	       docptr + DOC_ASICMODECONFIRM);
	write_nop(docptr);

	writew(DOC_ASICMODE_NORMAL | DOC_ASICMODE_MDWREN,
	       docptr + DOC_ASICMODE);
	writew(~(DOC_ASICMODE_NORMAL | DOC_ASICMODE_MDWREN),
	       docptr + DOC_ASICMODECONFIRM);

	writew(DOC_ECCCONF1_ECC_ENABLE, docptr + DOC_ECCCONF1);

	poll_status(docptr);
}

static void docg4_select_chip(struct mtd_info *mtd, int chip)
{
	/*
	 * Select among multiple cascaded chips ("floors").  Multiple floors are
	 * not yet supported, so the only valid non-negative value is 0.
	 */
	void __iomem *docptr = CONFIG_SYS_NAND_BASE;

	if (chip < 0)
		return;		/* deselected */

	if (chip > 0)
		printf("multiple floors currently unsupported\n");

	writew(0, docptr + DOC_DEVICESELECT);
}

static void read_hw_ecc(void __iomem *docptr, uint8_t *ecc_buf)
{
	/* read the 7 hw-generated ecc bytes */

	int i;
	for (i = 0; i < 7; i++) { /* hw quirk; read twice */
		ecc_buf[i] = readb(docptr + DOC_BCH_SYNDROM(i));
		ecc_buf[i] = readb(docptr + DOC_BCH_SYNDROM(i));
	}
}

static int correct_data(struct mtd_info *mtd, uint8_t *buf, int page)
{
	/*
	 * Called after a page read when hardware reports bitflips.
	 * Up to four bitflips can be corrected.
	 */

	struct nand_chip *nand = mtd->priv;
	struct docg4_priv *doc = nand->priv;
	void __iomem *docptr = CONFIG_SYS_NAND_BASE;
	int i, numerrs;
	unsigned int errpos[4];
	const uint8_t blank_read_hwecc[8] = {
		0xcf, 0x72, 0xfc, 0x1b, 0xa9, 0xc7, 0xb9, 0 };

	read_hw_ecc(docptr, doc->ecc_buf); /* read 7 hw-generated ecc bytes */

	/* check if read error is due to a blank page */
	if (!memcmp(doc->ecc_buf, blank_read_hwecc, 7))
		return 0;	/* yes */

	/* skip additional check of "written flag" if ignore_badblocks */
	if (!ignore_badblocks) {
		/*
		 * If the hw ecc bytes are not those of a blank page, there's
		 * still a chance that the page is blank, but was read with
		 * errors.  Check the "written flag" in last oob byte, which
		 * is set to zero when a page is written.  If more than half
		 * the bits are set, assume a blank page.  Unfortunately, the
		 * bit flips(s) are not reported in stats.
		 */

		if (doc->oob_buf[15]) {
			int bit, numsetbits = 0;
			unsigned long written_flag = doc->oob_buf[15];

			for (bit = 0; bit < 8; bit++) {
				if (written_flag & 0x01)
					numsetbits++;
				written_flag >>= 1;
			}
			if (numsetbits > 4) { /* assume blank */
				printf("errors in blank page at offset %08x\n",
				       page * DOCG4_PAGE_SIZE);
				return 0;
			}
		}
	}

	/*
	 * The hardware ecc unit produces oob_ecc ^ calc_ecc.  The kernel's bch
	 * algorithm is used to decode this.  However the hw operates on page
	 * data in a bit order that is the reverse of that of the bch alg,
	 * requiring that the bits be reversed on the result.  Thanks to Ivan
	 * Djelic for his analysis!
	 */
	for (i = 0; i < 7; i++)
		doc->ecc_buf[i] = bitrev8(doc->ecc_buf[i]);

	numerrs = decode_bch(doc->bch, NULL, DOCG4_USERDATA_LEN, NULL,
			     doc->ecc_buf, NULL, errpos);

	if (numerrs == -EBADMSG) {
		printf("uncorrectable errors at offset %08x\n",
		       page * DOCG4_PAGE_SIZE);
		return -EBADMSG;
	}

	BUG_ON(numerrs < 0);	/* -EINVAL, or anything other than -EBADMSG */

	/* undo last step in BCH alg (modulo mirroring not needed) */
	for (i = 0; i < numerrs; i++)
		errpos[i] = (errpos[i] & ~7)|(7-(errpos[i] & 7));

	/* fix the errors */
	for (i = 0; i < numerrs; i++) {
		/* ignore if error within oob ecc bytes */
		if (errpos[i] > DOCG4_USERDATA_LEN * 8)
			continue;

		/* if error within oob area preceeding ecc bytes... */
		if (errpos[i] > DOCG4_PAGE_SIZE * 8)
			__change_bit(errpos[i] - DOCG4_PAGE_SIZE * 8,
				     (unsigned long *)doc->oob_buf);

		else    /* error in page data */
			__change_bit(errpos[i], (unsigned long *)buf);
	}

	printf("%d error(s) corrected at offset %08x\n",
	       numerrs, page * DOCG4_PAGE_SIZE);

	return numerrs;
}

static int read_progstatus(struct docg4_priv *doc, void __iomem *docptr)
{
	/*
	 * This apparently checks the status of programming.  Done after an
	 * erasure, and after page data is written.  On error, the status is
	 * saved, to be later retrieved by the nand infrastructure code.
	 */

	/* status is read from the I/O reg */
	uint16_t status1 = readw(docptr + DOC_IOSPACE_DATA);
	uint16_t status2 = readw(docptr + DOC_IOSPACE_DATA);
	uint16_t status3 = readw(docptr + DOCG4_MYSTERY_REG);

	MTDDEBUG(MTD_DEBUG_LEVEL3, "docg4: %s: %02x %02x %02x\n",
	    __func__, status1, status2, status3);

	if (status1 != DOCG4_PROGSTATUS_GOOD ||
	    status2 != DOCG4_PROGSTATUS_GOOD_2 ||
	    status3 != DOCG4_PROGSTATUS_GOOD_2) {
		doc->status = NAND_STATUS_FAIL;
		printf("read_progstatus failed: %02x, %02x, %02x\n",
		       status1, status2, status3);
		return -EIO;
	}
	return 0;
}

static int pageprog(struct mtd_info *mtd)
{
	/*
	 * Final step in writing a page.  Writes the contents of its
	 * internal buffer out to the flash array, or some such.
	 */

	struct nand_chip *nand = mtd->priv;
	struct docg4_priv *doc = nand->priv;
	void __iomem *docptr = CONFIG_SYS_NAND_BASE;
	int retval = 0;

	MTDDEBUG(MTD_DEBUG_LEVEL3, "docg4: %s\n", __func__);

	writew(DOCG4_SEQ_PAGEPROG, docptr + DOC_FLASHSEQUENCE);
	writew(DOC_CMD_PROG_CYCLE2, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_nop(docptr);

	/* Just busy-wait; usleep_range() slows things down noticeably. */
	poll_status(docptr);

	writew(DOCG4_SEQ_FLUSH, docptr + DOC_FLASHSEQUENCE);
	writew(DOCG4_CMD_FLUSH, docptr + DOC_FLASHCOMMAND);
	writew(DOC_ECCCONF0_READ_MODE | 4, docptr + DOC_ECCCONF0);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);

	retval = read_progstatus(doc, docptr);
	writew(0, docptr + DOC_DATAEND);
	write_nop(docptr);
	poll_status(docptr);
	write_nop(docptr);

	return retval;
}

static void sequence_reset(void __iomem *docptr)
{
	/* common starting sequence for all operations */

	writew(DOC_CTRL_UNKNOWN | DOC_CTRL_CE, docptr + DOC_FLASHCONTROL);
	writew(DOC_SEQ_RESET, docptr + DOC_FLASHSEQUENCE);
	writew(DOC_CMD_RESET, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_nop(docptr);
	poll_status(docptr);
	write_nop(docptr);
}

static void read_page_prologue(void __iomem *docptr, uint32_t docg4_addr)
{
	/* first step in reading a page */

	sequence_reset(docptr);

	writew(DOCG4_SEQ_PAGE_READ, docptr + DOC_FLASHSEQUENCE);
	writew(DOCG4_CMD_PAGE_READ, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);

	write_addr(docptr, docg4_addr);

	write_nop(docptr);
	writew(DOCG4_CMD_READ2, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_nop(docptr);

	poll_status(docptr);
}

static void write_page_prologue(void __iomem *docptr, uint32_t docg4_addr)
{
	/* first step in writing a page */

	sequence_reset(docptr);
	writew(DOCG4_SEQ_PAGEWRITE, docptr + DOC_FLASHSEQUENCE);
	writew(DOCG4_CMD_PAGEWRITE, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_addr(docptr, docg4_addr);
	write_nop(docptr);
	write_nop(docptr);
	poll_status(docptr);
}

static uint32_t mtd_to_docg4_address(int page, int column)
{
	/*
	 * Convert mtd address to format used by the device, 32 bit packed.
	 *
	 * Some notes on G4 addressing... The M-Sys documentation on this device
	 * claims that pages are 2K in length, and indeed, the format of the
	 * address used by the device reflects that.  But within each page are
	 * four 512 byte "sub-pages", each with its own oob data that is
	 * read/written immediately after the 512 bytes of page data.  This oob
	 * data contains the ecc bytes for the preceeding 512 bytes.
	 *
	 * Rather than tell the mtd nand infrastructure that page size is 2k,
	 * with four sub-pages each, we engage in a little subterfuge and tell
	 * the infrastructure code that pages are 512 bytes in size.  This is
	 * done because during the course of reverse-engineering the device, I
	 * never observed an instance where an entire 2K "page" was read or
	 * written as a unit.  Each "sub-page" is always addressed individually,
	 * its data read/written, and ecc handled before the next "sub-page" is
	 * addressed.
	 *
	 * This requires us to convert addresses passed by the mtd nand
	 * infrastructure code to those used by the device.
	 *
	 * The address that is written to the device consists of four bytes: the
	 * first two are the 2k page number, and the second is the index into
	 * the page.  The index is in terms of 16-bit half-words and includes
	 * the preceeding oob data, so e.g., the index into the second
	 * "sub-page" is 0x108, and the full device address of the start of mtd
	 * page 0x201 is 0x00800108.
	 */
	int g4_page = page / 4;	                      /* device's 2K page */
	int g4_index = (page % 4) * 0x108 + column/2; /* offset into page */
	return (g4_page << 16) | g4_index;	      /* pack */
}

static void docg4_command(struct mtd_info *mtd, unsigned command, int column,
			  int page_addr)
{
	/* handle standard nand commands */

	struct nand_chip *nand = mtd->priv;
	struct docg4_priv *doc = nand->priv;
	uint32_t g4_addr = mtd_to_docg4_address(page_addr, column);

	MTDDEBUG(MTD_DEBUG_LEVEL3, "%s %x, page_addr=%x, column=%x\n",
	    __func__, command, page_addr, column);

	/*
	 * Save the command and its arguments.  This enables emulation of
	 * standard flash devices, and also some optimizations.
	 */
	doc->last_command.command = command;
	doc->last_command.column = column;
	doc->last_command.page = page_addr;

	switch (command) {
	case NAND_CMD_RESET:
		reset(CONFIG_SYS_NAND_BASE);
		break;

	case NAND_CMD_READ0:
		read_page_prologue(CONFIG_SYS_NAND_BASE, g4_addr);
		break;

	case NAND_CMD_STATUS:
		/* next call to read_byte() will expect a status */
		break;

	case NAND_CMD_SEQIN:
		write_page_prologue(CONFIG_SYS_NAND_BASE, g4_addr);

		/* hack for deferred write of oob bytes */
		if (doc->oob_page == page_addr)
			memcpy(nand->oob_poi, doc->oob_buf, 16);
		break;

	case NAND_CMD_PAGEPROG:
		pageprog(mtd);
		break;

	/* we don't expect these, based on review of nand_base.c */
	case NAND_CMD_READOOB:
	case NAND_CMD_READID:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
		printf("docg4_command: unexpected nand command 0x%x\n",
		       command);
		break;
	}
}

static void docg4_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	struct nand_chip *nand = mtd->priv;
	uint16_t *p = (uint16_t *)buf;
	len >>= 1;

	for (i = 0; i < len; i++)
		p[i] = readw(nand->IO_ADDR_R);
}

static int docg4_read_oob(struct mtd_info *mtd, struct nand_chip *nand,
			  int page)
{
	struct docg4_priv *doc = nand->priv;
	void __iomem *docptr = CONFIG_SYS_NAND_BASE;
	uint16_t status;

	MTDDEBUG(MTD_DEBUG_LEVEL3, "%s: page %x\n", __func__, page);

	/*
	 * Oob bytes are read as part of a normal page read.  If the previous
	 * nand command was a read of the page whose oob is now being read, just
	 * copy the oob bytes that we saved in a local buffer and avoid a
	 * separate oob read.
	 */
	if (doc->last_command.command == NAND_CMD_READ0 &&
	    doc->last_command.page == page) {
		memcpy(nand->oob_poi, doc->oob_buf, 16);
		return 0;
	}

	/*
	 * Separate read of oob data only.
	 */
	docg4_command(mtd, NAND_CMD_READ0, nand->ecc.size, page);

	writew(DOC_ECCCONF0_READ_MODE | DOCG4_OOB_SIZE, docptr + DOC_ECCCONF0);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);

	/* the 1st byte from the I/O reg is a status; the rest is oob data */
	status = readw(docptr + DOC_IOSPACE_DATA);
	if (status & DOCG4_READ_ERROR) {
		printf("docg4_read_oob failed: status = 0x%02x\n", status);
		return -EIO;
	}

	MTDDEBUG(MTD_DEBUG_LEVEL3, "%s: status = 0x%x\n", __func__, status);

	docg4_read_buf(mtd, nand->oob_poi, 16);

	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	writew(0, docptr + DOC_DATAEND);
	write_nop(docptr);

	return 0;
}

static int docg4_write_oob(struct mtd_info *mtd, struct nand_chip *nand,
			   int page)
{
	/*
	 * Writing oob-only is not really supported, because MLC nand must write
	 * oob bytes at the same time as page data.  Nonetheless, we save the
	 * oob buffer contents here, and then write it along with the page data
	 * if the same page is subsequently written.  This allows user space
	 * utilities that write the oob data prior to the page data to work
	 * (e.g., nandwrite).  The disdvantage is that, if the intention was to
	 * write oob only, the operation is quietly ignored.  Also, oob can get
	 * corrupted if two concurrent processes are running nandwrite.
	 */

	/* note that bytes 7..14 are hw generated hamming/ecc and overwritten */
	struct docg4_priv *doc = nand->priv;
	doc->oob_page = page;
	memcpy(doc->oob_buf, nand->oob_poi, 16);
	return 0;
}

static int docg4_block_neverbad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	/* only called when module_param ignore_badblocks is set */
	return 0;
}

static void docg4_write_buf16(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int i;
	struct nand_chip *nand = mtd->priv;
	uint16_t *p = (uint16_t *)buf;
	len >>= 1;

	for (i = 0; i < len; i++)
		writew(p[i], nand->IO_ADDR_W);
}

static int write_page(struct mtd_info *mtd, struct nand_chip *nand,
		       const uint8_t *buf, int use_ecc)
{
	void __iomem *docptr = CONFIG_SYS_NAND_BASE;
	uint8_t ecc_buf[8];

	writew(DOC_ECCCONF0_ECC_ENABLE |
	       DOC_ECCCONF0_UNKNOWN |
	       DOCG4_BCH_SIZE,
	       docptr + DOC_ECCCONF0);
	write_nop(docptr);

	/* write the page data */
	docg4_write_buf16(mtd, buf, DOCG4_PAGE_SIZE);

	/* oob bytes 0 through 5 are written to I/O reg */
	docg4_write_buf16(mtd, nand->oob_poi, 6);

	/* oob byte 6 written to a separate reg */
	writew(nand->oob_poi[6], docptr + DOCG4_OOB_6_7);

	write_nop(docptr);
	write_nop(docptr);

	/* write hw-generated ecc bytes to oob */
	if (likely(use_ecc)) {
		/* oob byte 7 is hamming code */
		uint8_t hamming = readb(docptr + DOC_HAMMINGPARITY);
		hamming = readb(docptr + DOC_HAMMINGPARITY); /* 2nd read */
		writew(hamming, docptr + DOCG4_OOB_6_7);
		write_nop(docptr);

		/* read the 7 bch bytes from ecc regs */
		read_hw_ecc(docptr, ecc_buf);
		ecc_buf[7] = 0;         /* clear the "page written" flag */
	}

	/* write user-supplied bytes to oob */
	else {
		writew(nand->oob_poi[7], docptr + DOCG4_OOB_6_7);
		write_nop(docptr);
		memcpy(ecc_buf, &nand->oob_poi[8], 8);
	}

	docg4_write_buf16(mtd, ecc_buf, 8);
	write_nop(docptr);
	write_nop(docptr);
	writew(0, docptr + DOC_DATAEND);
	write_nop(docptr);

	return 0;
}

static int docg4_write_page_raw(struct mtd_info *mtd, struct nand_chip *nand,
				 const uint8_t *buf, int oob_required)
{
	return write_page(mtd, nand, buf, 0);
}

static int docg4_write_page(struct mtd_info *mtd, struct nand_chip *nand,
			     const uint8_t *buf, int oob_required)
{
	return write_page(mtd, nand, buf, 1);
}

static int read_page(struct mtd_info *mtd, struct nand_chip *nand,
		     uint8_t *buf, int page, int use_ecc)
{
	struct docg4_priv *doc = nand->priv;
	void __iomem *docptr = CONFIG_SYS_NAND_BASE;
	uint16_t status, edc_err, *buf16;

	writew(DOC_ECCCONF0_READ_MODE |
	       DOC_ECCCONF0_ECC_ENABLE |
	       DOC_ECCCONF0_UNKNOWN |
	       DOCG4_BCH_SIZE,
	       docptr + DOC_ECCCONF0);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);

	/* the 1st byte from the I/O reg is a status; the rest is page data */
	status = readw(docptr + DOC_IOSPACE_DATA);
	if (status & DOCG4_READ_ERROR) {
		printf("docg4_read_page: bad status: 0x%02x\n", status);
		writew(0, docptr + DOC_DATAEND);
		return -EIO;
	}

	docg4_read_buf(mtd, buf, DOCG4_PAGE_SIZE); /* read the page data */

	/* first 14 oob bytes read from I/O reg */
	docg4_read_buf(mtd, nand->oob_poi, 14);

	/* last 2 read from another reg */
	buf16 = (uint16_t *)(nand->oob_poi + 14);
	*buf16 = readw(docptr + DOCG4_MYSTERY_REG);

	/*
	 * Diskonchips read oob immediately after a page read.  Mtd
	 * infrastructure issues a separate command for reading oob after the
	 * page is read.  So we save the oob bytes in a local buffer and just
	 * copy it if the next command reads oob from the same page.
	 */
	memcpy(doc->oob_buf, nand->oob_poi, 16);

	write_nop(docptr);

	if (likely(use_ecc)) {
		/* read the register that tells us if bitflip(s) detected  */
		edc_err = readw(docptr + DOC_ECCCONF1);
		edc_err = readw(docptr + DOC_ECCCONF1);

		/* If bitflips are reported, attempt to correct with ecc */
		if (edc_err & DOC_ECCCONF1_BCH_SYNDROM_ERR) {
			int bits_corrected = correct_data(mtd, buf, page);
			if (bits_corrected == -EBADMSG)
				mtd->ecc_stats.failed++;
			else
				mtd->ecc_stats.corrected += bits_corrected;
		}
	}

	writew(0, docptr + DOC_DATAEND);
	return 0;
}


static int docg4_read_page_raw(struct mtd_info *mtd, struct nand_chip *nand,
			       uint8_t *buf, int oob_required, int page)
{
	return read_page(mtd, nand, buf, page, 0);
}

static int docg4_read_page(struct mtd_info *mtd, struct nand_chip *nand,
			   uint8_t *buf, int oob_required, int page)
{
	return read_page(mtd, nand, buf, page, 1);
}

static int docg4_erase_block(struct mtd_info *mtd, int page)
{
	struct nand_chip *nand = mtd->priv;
	struct docg4_priv *doc = nand->priv;
	void __iomem *docptr = CONFIG_SYS_NAND_BASE;
	uint16_t g4_page;

	MTDDEBUG(MTD_DEBUG_LEVEL3, "%s: page %04x\n", __func__, page);

	sequence_reset(docptr);

	writew(DOCG4_SEQ_BLOCKERASE, docptr + DOC_FLASHSEQUENCE);
	writew(DOC_CMD_PROG_BLOCK_ADDR, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);

	/* only 2 bytes of address are written to specify erase block */
	g4_page = (uint16_t)(page / 4);  /* to g4's 2k page addressing */
	writeb(g4_page & 0xff, docptr + DOC_FLASHADDRESS);
	g4_page >>= 8;
	writeb(g4_page & 0xff, docptr + DOC_FLASHADDRESS);
	write_nop(docptr);

	/* start the erasure */
	writew(DOC_CMD_ERASECYCLE2, docptr + DOC_FLASHCOMMAND);
	write_nop(docptr);
	write_nop(docptr);

	poll_status(docptr);
	writew(DOCG4_SEQ_FLUSH, docptr + DOC_FLASHSEQUENCE);
	writew(DOCG4_CMD_FLUSH, docptr + DOC_FLASHCOMMAND);
	writew(DOC_ECCCONF0_READ_MODE | 4, docptr + DOC_ECCCONF0);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);
	write_nop(docptr);

	read_progstatus(doc, docptr);

	writew(0, docptr + DOC_DATAEND);
	write_nop(docptr);
	poll_status(docptr);
	write_nop(docptr);

	return nand->waitfunc(mtd, nand);
}

static int read_factory_bbt(struct mtd_info *mtd)
{
	/*
	 * The device contains a read-only factory bad block table.  Read it and
	 * update the memory-based bbt accordingly.
	 */

	struct nand_chip *nand = mtd->priv;
	uint32_t g4_addr = mtd_to_docg4_address(DOCG4_FACTORY_BBT_PAGE, 0);
	uint8_t *buf;
	int i, block, status;

	buf = kzalloc(DOCG4_PAGE_SIZE, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	read_page_prologue(CONFIG_SYS_NAND_BASE, g4_addr);
	status = docg4_read_page(mtd, nand, buf, 0, DOCG4_FACTORY_BBT_PAGE);
	if (status)
		goto exit;

	/*
	 * If no memory-based bbt was created, exit.  This will happen if module
	 * parameter ignore_badblocks is set.  Then why even call this function?
	 * For an unknown reason, block erase always fails if it's the first
	 * operation after device power-up.  The above read ensures it never is.
	 * Ugly, I know.
	 */
	if (nand->bbt == NULL)  /* no memory-based bbt */
		goto exit;

	/*
	 * Parse factory bbt and update memory-based bbt.  Factory bbt format is
	 * simple: one bit per block, block numbers increase left to right (msb
	 * to lsb).  Bit clear means bad block.
	 */
	for (i = block = 0; block < DOCG4_NUMBLOCKS; block += 8, i++) {
		int bitnum;
		uint8_t mask;
		for (bitnum = 0, mask = 0x80;
		     bitnum < 8; bitnum++, mask >>= 1) {
			if (!(buf[i] & mask)) {
				int badblock = block + bitnum;
				nand->bbt[badblock / 4] |=
					0x03 << ((badblock % 4) * 2);
				mtd->ecc_stats.badblocks++;
				printf("factory-marked bad block: %d\n",
				       badblock);
			}
		}
	}
 exit:
	kfree(buf);
	return status;
}

static int docg4_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	/*
	 * Mark a block as bad.  Bad blocks are marked in the oob area of the
	 * first page of the block.  The default scan_bbt() in the nand
	 * infrastructure code works fine for building the memory-based bbt
	 * during initialization, as does the nand infrastructure function that
	 * checks if a block is bad by reading the bbt.  This function replaces
	 * the nand default because writes to oob-only are not supported.
	 */

	int ret, i;
	uint8_t *buf;
	struct nand_chip *nand = mtd->priv;
	struct nand_bbt_descr *bbtd = nand->badblock_pattern;
	int block = (int)(ofs >> nand->bbt_erase_shift);
	int page = (int)(ofs >> nand->page_shift);
	uint32_t g4_addr = mtd_to_docg4_address(page, 0);

	MTDDEBUG(MTD_DEBUG_LEVEL3, "%s: %08llx\n", __func__, ofs);

	if (unlikely(ofs & (DOCG4_BLOCK_SIZE - 1)))
		printf("%s: ofs %llx not start of block!\n",
		       __func__, ofs);

	/* allocate blank buffer for page data */
	buf = kzalloc(DOCG4_PAGE_SIZE, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	/* update bbt in memory */
	nand->bbt[block / 4] |= 0x01 << ((block & 0x03) * 2);

	/* write bit-wise negation of pattern to oob buffer */
	memset(nand->oob_poi, 0xff, mtd->oobsize);
	for (i = 0; i < bbtd->len; i++)
		nand->oob_poi[bbtd->offs + i] = ~bbtd->pattern[i];

	/* write first page of block */
	write_page_prologue(CONFIG_SYS_NAND_BASE, g4_addr);
	docg4_write_page(mtd, nand, buf, 1);
	ret = pageprog(mtd);
	if (!ret)
		mtd->ecc_stats.badblocks++;

	kfree(buf);

	return ret;
}

static uint8_t docg4_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd->priv;
	struct docg4_priv *doc = nand->priv;

	MTDDEBUG(MTD_DEBUG_LEVEL3, "%s\n", __func__);

	if (doc->last_command.command == NAND_CMD_STATUS) {
		int status;

		/*
		 * Previous nand command was status request, so nand
		 * infrastructure code expects to read the status here.  If an
		 * error occurred in a previous operation, report it.
		 */
		doc->last_command.command = 0;

		if (doc->status) {
			status = doc->status;
			doc->status = 0;
		}

		/* why is NAND_STATUS_WP inverse logic?? */
		else
			status = NAND_STATUS_WP | NAND_STATUS_READY;

		return status;
	}

	printf("unexpectd call to read_byte()\n");

	return 0;
}

static int docg4_wait(struct mtd_info *mtd, struct nand_chip *nand)
{
	struct docg4_priv *doc = nand->priv;
	int status = NAND_STATUS_WP;       /* inverse logic?? */
	MTDDEBUG(MTD_DEBUG_LEVEL3, "%s...\n", __func__);

	/* report any previously unreported error */
	if (doc->status) {
		status |= doc->status;
		doc->status = 0;
		return status;
	}

	status |= poll_status(CONFIG_SYS_NAND_BASE);
	return status;
}

int docg4_nand_init(struct mtd_info *mtd, struct nand_chip *nand, int devnum)
{
	uint16_t id1, id2;
	struct docg4_priv *docg4;
	int retval;

	docg4 = kzalloc(sizeof(*docg4), GFP_KERNEL);
	if (!docg4)
		return -1;

	mtd->priv = nand;
	nand->priv = docg4;

	/* These must be initialized here because the docg4 is non-standard
	 * and doesn't produce an id that the nand code can use to look up
	 * these values (nand_scan_ident() not called).
	 */
	mtd->size = DOCG4_CHIP_SIZE;
	mtd->name = "Msys_Diskonchip_G4";
	mtd->writesize = DOCG4_PAGE_SIZE;
	mtd->erasesize = DOCG4_BLOCK_SIZE;
	mtd->oobsize = DOCG4_OOB_SIZE;

	nand->IO_ADDR_R =
		(void __iomem *)CONFIG_SYS_NAND_BASE + DOC_IOSPACE_DATA;
	nand->IO_ADDR_W = nand->IO_ADDR_R;
	nand->chipsize = DOCG4_CHIP_SIZE;
	nand->chip_shift = DOCG4_CHIP_SHIFT;
	nand->bbt_erase_shift = DOCG4_ERASE_SHIFT;
	nand->phys_erase_shift = DOCG4_ERASE_SHIFT;
	nand->chip_delay = 20;
	nand->page_shift = DOCG4_PAGE_SHIFT;
	nand->pagemask = 0x3ffff;
	nand->badblockpos = NAND_LARGE_BADBLOCK_POS;
	nand->badblockbits = 8;
	nand->ecc.layout = &docg4_oobinfo;
	nand->ecc.mode = NAND_ECC_HW_SYNDROME;
	nand->ecc.size = DOCG4_PAGE_SIZE;
	nand->ecc.prepad = 8;
	nand->ecc.bytes	= 8;
	nand->ecc.strength = DOCG4_T;
	nand->options = NAND_BUSWIDTH_16 | NAND_NO_SUBPAGE_WRITE;
	nand->controller = &nand->hwcontrol;

	/* methods */
	nand->cmdfunc = docg4_command;
	nand->waitfunc = docg4_wait;
	nand->select_chip = docg4_select_chip;
	nand->read_byte = docg4_read_byte;
	nand->block_markbad = docg4_block_markbad;
	nand->read_buf = docg4_read_buf;
	nand->write_buf = docg4_write_buf16;
	nand->scan_bbt = nand_default_bbt;
	nand->erase = docg4_erase_block;
	nand->ecc.read_page = docg4_read_page;
	nand->ecc.write_page = docg4_write_page;
	nand->ecc.read_page_raw = docg4_read_page_raw;
	nand->ecc.write_page_raw = docg4_write_page_raw;
	nand->ecc.read_oob = docg4_read_oob;
	nand->ecc.write_oob = docg4_write_oob;

	/*
	 * The way the nand infrastructure code is written, a memory-based bbt
	 * is not created if NAND_SKIP_BBTSCAN is set.  With no memory bbt,
	 * nand->block_bad() is used.  So when ignoring bad blocks, we skip the
	 * scan and define a dummy block_bad() which always returns 0.
	 */
	if (ignore_badblocks) {
		nand->options |= NAND_SKIP_BBTSCAN;
		nand->block_bad	= docg4_block_neverbad;
	}

	reset(CONFIG_SYS_NAND_BASE);

	/* check for presence of g4 chip by reading id registers */
	id1 = readw(CONFIG_SYS_NAND_BASE + DOC_CHIPID);
	id1 = readw(CONFIG_SYS_NAND_BASE + DOCG4_MYSTERY_REG);
	id2 = readw(CONFIG_SYS_NAND_BASE + DOC_CHIPID_INV);
	id2 = readw(CONFIG_SYS_NAND_BASE + DOCG4_MYSTERY_REG);
	if (id1 != DOCG4_IDREG1_VALUE || id2 != DOCG4_IDREG2_VALUE)
		return -1;

	/* initialize bch algorithm */
	docg4->bch = init_bch(DOCG4_M, DOCG4_T, DOCG4_PRIMITIVE_POLY);
	if (docg4->bch == NULL)
		return -1;

	retval = nand_scan_tail(mtd);
	if (retval)
		return -1;

	/*
	 * Scan for bad blocks and create bbt here, then add the factory-marked
	 * bad blocks to the bbt.
	 */
	nand->scan_bbt(mtd);
	nand->options |= NAND_BBT_SCANNED;
	retval = read_factory_bbt(mtd);
	if (retval)
		return -1;

	retval = nand_register(devnum);
	if (retval)
		return -1;

	return 0;
}
