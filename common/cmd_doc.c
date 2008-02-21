/*
 * Driver for Disk-On-Chip 2000 and Millennium
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * $Id: doc2000.c,v 1.46 2001/10/02 15:05:13 dwmw2 Exp $
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/mtd/nftl.h>
#include <linux/mtd/doc2000.h>

#ifdef CFG_DOC_SUPPORT_2000
#define DoC_is_2000(doc) (doc->ChipID == DOC_ChipID_Doc2k)
#else
#define DoC_is_2000(doc) (0)
#endif

#ifdef CFG_DOC_SUPPORT_MILLENNIUM
#define DoC_is_Millennium(doc) (doc->ChipID == DOC_ChipID_DocMil)
#else
#define DoC_is_Millennium(doc) (0)
#endif

/* CFG_DOC_PASSIVE_PROBE:
   In order to ensure that the BIOS checksum is correct at boot time, and
   hence that the onboard BIOS extension gets executed, the DiskOnChip
   goes into reset mode when it is read sequentially: all registers
   return 0xff until the chip is woken up again by writing to the
   DOCControl register.

   Unfortunately, this means that the probe for the DiskOnChip is unsafe,
   because one of the first things it does is write to where it thinks
   the DOCControl register should be - which may well be shared memory
   for another device. I've had machines which lock up when this is
   attempted. Hence the possibility to do a passive probe, which will fail
   to detect a chip in reset mode, but is at least guaranteed not to lock
   the machine.

   If you have this problem, uncomment the following line:
#define CFG_DOC_PASSIVE_PROBE
*/

#undef	DOC_DEBUG
#undef	ECC_DEBUG
#undef	PSYCHO_DEBUG
#undef	NFTL_DEBUG

static struct DiskOnChip doc_dev_desc[CFG_MAX_DOC_DEVICE];

/* Current DOC Device	*/
static int curr_device = -1;

/* Supported NAND flash devices */
static struct nand_flash_dev nand_flash_ids[] = {
	{"Toshiba TC5816BDC",     NAND_MFR_TOSHIBA, 0x64, 21, 1, 2, 0x1000, 0},
	{"Toshiba TC5832DC",      NAND_MFR_TOSHIBA, 0x6b, 22, 0, 2, 0x2000, 0},
	{"Toshiba TH58V128DC",    NAND_MFR_TOSHIBA, 0x73, 24, 0, 2, 0x4000, 0},
	{"Toshiba TC58256FT/DC",  NAND_MFR_TOSHIBA, 0x75, 25, 0, 2, 0x4000, 0},
	{"Toshiba TH58512FT",     NAND_MFR_TOSHIBA, 0x76, 26, 0, 3, 0x4000, 0},
	{"Toshiba TC58V32DC",     NAND_MFR_TOSHIBA, 0xe5, 22, 0, 2, 0x2000, 0},
	{"Toshiba TC58V64AFT/DC", NAND_MFR_TOSHIBA, 0xe6, 23, 0, 2, 0x2000, 0},
	{"Toshiba TC58V16BDC",    NAND_MFR_TOSHIBA, 0xea, 21, 1, 2, 0x1000, 0},
	{"Toshiba TH58100FT",     NAND_MFR_TOSHIBA, 0x79, 27, 0, 3, 0x4000, 0},
	{"Samsung KM29N16000",    NAND_MFR_SAMSUNG, 0x64, 21, 1, 2, 0x1000, 0},
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0x6b, 22, 0, 2, 0x2000, 0},
	{"Samsung KM29U128T",     NAND_MFR_SAMSUNG, 0x73, 24, 0, 2, 0x4000, 0},
	{"Samsung KM29U256T",     NAND_MFR_SAMSUNG, 0x75, 25, 0, 2, 0x4000, 0},
	{"Samsung unknown 64Mb",  NAND_MFR_SAMSUNG, 0x76, 26, 0, 3, 0x4000, 0},
	{"Samsung KM29W32000",    NAND_MFR_SAMSUNG, 0xe3, 22, 0, 2, 0x2000, 0},
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0xe5, 22, 0, 2, 0x2000, 0},
	{"Samsung KM29U64000",    NAND_MFR_SAMSUNG, 0xe6, 23, 0, 2, 0x2000, 0},
	{"Samsung KM29W16000",    NAND_MFR_SAMSUNG, 0xea, 21, 1, 2, 0x1000, 0},
	{"Samsung K9F5616Q0C",    NAND_MFR_SAMSUNG, 0x45, 25, 0, 2, 0x4000, 1},
	{"Samsung K9K1216Q0C",    NAND_MFR_SAMSUNG, 0x46, 26, 0, 3, 0x4000, 1},
	{"Samsung K9F1G08U0M",    NAND_MFR_SAMSUNG, 0xf1, 27, 0, 2, 0, 0},
	{NULL,}
};

/* ------------------------------------------------------------------------- */

int do_doc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int rcode = 0;

    switch (argc) {
    case 0:
    case 1:
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
    case 2:
	if (strcmp(argv[1],"info") == 0) {
		int i;

		putc ('\n');

		for (i=0; i<CFG_MAX_DOC_DEVICE; ++i) {
			if(doc_dev_desc[i].ChipID == DOC_ChipID_UNKNOWN)
				continue; /* list only known devices */
			printf ("Device %d: ", i);
			doc_print(&doc_dev_desc[i]);
		}
		return 0;

	} else if (strcmp(argv[1],"device") == 0) {
		if ((curr_device < 0) || (curr_device >= CFG_MAX_DOC_DEVICE)) {
			puts ("\nno devices available\n");
			return 1;
		}
		printf ("\nDevice %d: ", curr_device);
		doc_print(&doc_dev_desc[curr_device]);
		return 0;
	}
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
    case 3:
	if (strcmp(argv[1],"device") == 0) {
		int dev = (int)simple_strtoul(argv[2], NULL, 10);

		printf ("\nDevice %d: ", dev);
		if (dev >= CFG_MAX_DOC_DEVICE) {
			puts ("unknown device\n");
			return 1;
		}
		doc_print(&doc_dev_desc[dev]);
		/*doc_print (dev);*/

		if (doc_dev_desc[dev].ChipID == DOC_ChipID_UNKNOWN) {
			return 1;
		}

		curr_device = dev;

		puts ("... is now current device\n");

		return 0;
	}

	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
    default:
	/* at least 4 args */

	if (strcmp(argv[1],"read") == 0 || strcmp(argv[1],"write") == 0) {
		ulong addr = simple_strtoul(argv[2], NULL, 16);
		ulong off  = simple_strtoul(argv[3], NULL, 16);
		ulong size = simple_strtoul(argv[4], NULL, 16);
		int cmd    = (strcmp(argv[1],"read") == 0);
		int ret, total;

		printf ("\nDOC %s: device %d offset %ld, size %ld ... ",
			cmd ? "read" : "write", curr_device, off, size);

		ret = doc_rw(doc_dev_desc + curr_device, cmd, off, size,
			     (size_t *)&total, (u_char*)addr);

		printf ("%d bytes %s: %s\n", total, cmd ? "read" : "write",
			ret ? "ERROR" : "OK");

		return ret;
	} else if (strcmp(argv[1],"erase") == 0) {
		ulong off = simple_strtoul(argv[2], NULL, 16);
		ulong size = simple_strtoul(argv[3], NULL, 16);
		int ret;

		printf ("\nDOC erase: device %d offset %ld, size %ld ... ",
			curr_device, off, size);

		ret = doc_erase (doc_dev_desc + curr_device, off, size);

		printf("%s\n", ret ? "ERROR" : "OK");

		return ret;
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		rcode = 1;
	}

	return rcode;
    }
}
U_BOOT_CMD(
	doc,	5,	1,	do_doc,
	"doc     - Disk-On-Chip sub-system\n",
	"info  - show available DOC devices\n"
	"doc device [dev] - show or set current device\n"
	"doc read  addr off size\n"
	"doc write addr off size - read/write `size'"
	" bytes starting at offset `off'\n"
	"    to/from memory address `addr'\n"
	"doc erase off size - erase `size' bytes of DOC from offset `off'\n"
);

int do_docboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *boot_device = NULL;
	char *ep;
	int dev;
	ulong cnt;
	ulong addr;
	ulong offset = 0;
	image_header_t *hdr;
	int rcode = 0;

	show_boot_progress (34);
	switch (argc) {
	case 1:
		addr = CFG_LOAD_ADDR;
		boot_device = getenv ("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv ("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		offset = simple_strtoul(argv[3], NULL, 16);
		break;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		show_boot_progress (-35);
		return 1;
	}

	show_boot_progress (35);
	if (!boot_device) {
		puts ("\n** No boot device **\n");
		show_boot_progress (-36);
		return 1;
	}
	show_boot_progress (36);

	dev = simple_strtoul(boot_device, &ep, 16);

	if ((dev >= CFG_MAX_DOC_DEVICE) ||
	    (doc_dev_desc[dev].ChipID == DOC_ChipID_UNKNOWN)) {
		printf ("\n** Device %d not available\n", dev);
		show_boot_progress (-37);
		return 1;
	}
	show_boot_progress (37);

	printf ("\nLoading from device %d: %s at 0x%lX (offset 0x%lX)\n",
		dev, doc_dev_desc[dev].name, doc_dev_desc[dev].physadr,
		offset);

	if (doc_rw (doc_dev_desc + dev, 1, offset,
		    SECTORSIZE, NULL, (u_char *)addr)) {
		printf ("** Read error on %d\n", dev);
		show_boot_progress (-38);
		return 1;
	}
	show_boot_progress (38);

	hdr = (image_header_t *)addr;

	if (image_check_magic (hdr)) {

		image_print_contents (hdr);

		cnt = image_get_image_size (hdr);
		cnt -= SECTORSIZE;
	} else {
		puts ("\n** Bad Magic Number **\n");
		show_boot_progress (-39);
		return 1;
	}
	show_boot_progress (39);

	if (doc_rw (doc_dev_desc + dev, 1, offset + SECTORSIZE, cnt,
		    NULL, (u_char *)(addr+SECTORSIZE))) {
		printf ("** Read error on %d\n", dev);
		show_boot_progress (-40);
		return 1;
	}
	show_boot_progress (40);

	/* Loading ok, update default load address */

	load_addr = addr;

	/* Check if we should attempt an auto-start */
	if (((ep = getenv("autostart")) != NULL) && (strcmp(ep,"yes") == 0)) {
		char *local_args[2];
		extern int do_bootm (cmd_tbl_t *, int, int, char *[]);

		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n", addr);

		do_bootm (cmdtp, 0, 1, local_args);
		rcode = 1;
	}
	return rcode;
}

U_BOOT_CMD(
	docboot,	4,	1,	do_docboot,
	"docboot - boot from DOC device\n",
	"loadAddr dev\n"
);

int doc_rw (struct DiskOnChip* this, int cmd,
	    loff_t from, size_t len,
	    size_t * retlen, u_char * buf)
{
	int noecc, ret = 0, n, total = 0;
	char eccbuf[6];

	while(len) {
		/* The ECC will not be calculated correctly if
		   less than 512 is written or read */
		noecc = (from != (from | 0x1ff) + 1) ||	(len < 0x200);

		if (cmd)
			ret = doc_read_ecc(this, from, len,
					   (size_t *)&n, (u_char*)buf,
					   noecc ? (uchar *)NULL : (uchar *)eccbuf);
		else
			ret = doc_write_ecc(this, from, len,
					    (size_t *)&n, (u_char*)buf,
					    noecc ? (uchar *)NULL : (uchar *)eccbuf);

		if (ret)
			break;

		from  += n;
		buf   += n;
		total += n;
		len   -= n;
	}

	if (retlen)
		*retlen = total;

	return ret;
}

void doc_print(struct DiskOnChip *this) {
	printf("%s at 0x%lX,\n"
	       "\t  %d chip%s %s, size %d MB, \n"
	       "\t  total size %ld MB, sector size %ld kB\n",
	       this->name, this->physadr, this->numchips,
	       this->numchips>1 ? "s" : "", this->chips_name,
	       1 << (this->chipshift - 20),
	       this->totlen >> 20, this->erasesize >> 10);

	if (this->nftl_found) {
		struct NFTLrecord *nftl = &this->nftl;
		unsigned long bin_size, flash_size;

		bin_size = nftl->nb_boot_blocks * this->erasesize;
		flash_size = (nftl->nb_blocks - nftl->nb_boot_blocks) * this->erasesize;

		printf("\t  NFTL boot record:\n"
		       "\t    Binary partition: size %ld%s\n"
		       "\t    Flash disk partition: size %ld%s, offset 0x%lx\n",
		       bin_size > (1 << 20) ? bin_size >> 20 : bin_size >> 10,
		       bin_size > (1 << 20) ? "MB" : "kB",
		       flash_size > (1 << 20) ? flash_size >> 20 : flash_size >> 10,
		       flash_size > (1 << 20) ? "MB" : "kB", bin_size);
	} else {
		puts ("\t  No NFTL boot record found.\n");
	}
}

/* ------------------------------------------------------------------------- */

/* This function is needed to avoid calls of the __ashrdi3 function. */
static int shr(int val, int shift) {
	return val >> shift;
}

/* Perform the required delay cycles by reading from the appropriate register */
static void DoC_Delay(struct DiskOnChip *doc, unsigned short cycles)
{
	volatile char dummy;
	int i;

	for (i = 0; i < cycles; i++) {
		if (DoC_is_Millennium(doc))
			dummy = ReadDOC(doc->virtadr, NOP);
		else
			dummy = ReadDOC(doc->virtadr, DOCStatus);
	}

}

/* DOC_WaitReady: Wait for RDY line to be asserted by the flash chip */
static int _DoC_WaitReady(struct DiskOnChip *doc)
{
	unsigned long docptr = doc->virtadr;
	unsigned long start = get_timer(0);

#ifdef PSYCHO_DEBUG
	puts ("_DoC_WaitReady called for out-of-line wait\n");
#endif

	/* Out-of-line routine to wait for chip response */
	while (!(ReadDOC(docptr, CDSNControl) & CDSN_CTRL_FR_B)) {
#ifdef CFG_DOC_SHORT_TIMEOUT
		/* it seems that after a certain time the DoC deasserts
		 * the CDSN_CTRL_FR_B although it is not ready...
		 * using a short timout solve this (timer increments every ms) */
		if (get_timer(start) > 10) {
			return DOC_ETIMEOUT;
		}
#else
		if (get_timer(start) > 10 * 1000) {
			puts ("_DoC_WaitReady timed out.\n");
			return DOC_ETIMEOUT;
		}
#endif
		udelay(1);
	}

	return 0;
}

static int DoC_WaitReady(struct DiskOnChip *doc)
{
	unsigned long docptr = doc->virtadr;
	/* This is inline, to optimise the common case, where it's ready instantly */
	int ret = 0;

	/* 4 read form NOP register should be issued in prior to the read from CDSNControl
	   see Software Requirement 11.4 item 2. */
	DoC_Delay(doc, 4);

	if (!(ReadDOC(docptr, CDSNControl) & CDSN_CTRL_FR_B))
		/* Call the out-of-line routine to wait */
		ret = _DoC_WaitReady(doc);

	/* issue 2 read from NOP register after reading from CDSNControl register
	   see Software Requirement 11.4 item 2. */
	DoC_Delay(doc, 2);

	return ret;
}

/* DoC_Command: Send a flash command to the flash chip through the CDSN Slow IO register to
   bypass the internal pipeline. Each of 4 delay cycles (read from the NOP register) is
   required after writing to CDSN Control register, see Software Requirement 11.4 item 3. */

static inline int DoC_Command(struct DiskOnChip *doc, unsigned char command,
			      unsigned char xtraflags)
{
	unsigned long docptr = doc->virtadr;

	if (DoC_is_2000(doc))
		xtraflags |= CDSN_CTRL_FLASH_IO;

	/* Assert the CLE (Command Latch Enable) line to the flash chip */
	WriteDOC(xtraflags | CDSN_CTRL_CLE | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(doc, 4);	/* Software requirement 11.4.3 for Millennium */

	if (DoC_is_Millennium(doc))
		WriteDOC(command, docptr, CDSNSlowIO);

	/* Send the command */
	WriteDOC_(command, docptr, doc->ioreg);

	/* Lower the CLE line */
	WriteDOC(xtraflags | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(doc, 4);	/* Software requirement 11.4.3 for Millennium */

	/* Wait for the chip to respond - Software requirement 11.4.1 (extended for any command) */
	return DoC_WaitReady(doc);
}

/* DoC_Address: Set the current address for the flash chip through the CDSN Slow IO register to
   bypass the internal pipeline. Each of 4 delay cycles (read from the NOP register) is
   required after writing to CDSN Control register, see Software Requirement 11.4 item 3. */

static int DoC_Address(struct DiskOnChip *doc, int numbytes, unsigned long ofs,
		       unsigned char xtraflags1, unsigned char xtraflags2)
{
	unsigned long docptr;
	int i;

	docptr = doc->virtadr;

	if (DoC_is_2000(doc))
		xtraflags1 |= CDSN_CTRL_FLASH_IO;

	/* Assert the ALE (Address Latch Enable) line to the flash chip */
	WriteDOC(xtraflags1 | CDSN_CTRL_ALE | CDSN_CTRL_CE, docptr, CDSNControl);

	DoC_Delay(doc, 4);	/* Software requirement 11.4.3 for Millennium */

	/* Send the address */
	/* Devices with 256-byte page are addressed as:
	   Column (bits 0-7), Page (bits 8-15, 16-23, 24-31)
	   * there is no device on the market with page256
	   and more than 24 bits.
	   Devices with 512-byte page are addressed as:
	   Column (bits 0-7), Page (bits 9-16, 17-24, 25-31)
	   * 25-31 is sent only if the chip support it.
	   * bit 8 changes the read command to be sent
	   (NAND_CMD_READ0 or NAND_CMD_READ1).
	 */

	if (numbytes == ADDR_COLUMN || numbytes == ADDR_COLUMN_PAGE) {
		if (DoC_is_Millennium(doc))
			WriteDOC(ofs & 0xff, docptr, CDSNSlowIO);
		WriteDOC_(ofs & 0xff, docptr, doc->ioreg);
	}

	if (doc->page256) {
		ofs = ofs >> 8;
	} else {
		ofs = ofs >> 9;
	}

	if (numbytes == ADDR_PAGE || numbytes == ADDR_COLUMN_PAGE) {
		for (i = 0; i < doc->pageadrlen; i++, ofs = ofs >> 8) {
			if (DoC_is_Millennium(doc))
				WriteDOC(ofs & 0xff, docptr, CDSNSlowIO);
			WriteDOC_(ofs & 0xff, docptr, doc->ioreg);
		}
	}

	DoC_Delay(doc, 2);	/* Needed for some slow flash chips. mf. */

	/* FIXME: The SlowIO's for millennium could be replaced by
	   a single WritePipeTerm here. mf. */

	/* Lower the ALE line */
	WriteDOC(xtraflags1 | xtraflags2 | CDSN_CTRL_CE, docptr,
		 CDSNControl);

	DoC_Delay(doc, 4);	/* Software requirement 11.4.3 for Millennium */

	/* Wait for the chip to respond - Software requirement 11.4.1 */
	return DoC_WaitReady(doc);
}

/* Read a buffer from DoC, taking care of Millennium oddities */
static void DoC_ReadBuf(struct DiskOnChip *doc, u_char * buf, int len)
{
	volatile int dummy;
	int modulus = 0xffff;
	unsigned long docptr;
	int i;

	docptr = doc->virtadr;

	if (len <= 0)
		return;

	if (DoC_is_Millennium(doc)) {
		/* Read the data via the internal pipeline through CDSN IO register,
		   see Pipelined Read Operations 11.3 */
		dummy = ReadDOC(docptr, ReadPipeInit);

		/* Millennium should use the LastDataRead register - Pipeline Reads */
		len--;

		/* This is needed for correctly ECC calculation */
		modulus = 0xff;
	}

	for (i = 0; i < len; i++)
		buf[i] = ReadDOC_(docptr, doc->ioreg + (i & modulus));

	if (DoC_is_Millennium(doc)) {
		buf[i] = ReadDOC(docptr, LastDataRead);
	}
}

/* Write a buffer to DoC, taking care of Millennium oddities */
static void DoC_WriteBuf(struct DiskOnChip *doc, const u_char * buf, int len)
{
	unsigned long docptr;
	int i;

	docptr = doc->virtadr;

	if (len <= 0)
		return;

	for (i = 0; i < len; i++)
		WriteDOC_(buf[i], docptr, doc->ioreg + i);

	if (DoC_is_Millennium(doc)) {
		WriteDOC(0x00, docptr, WritePipeTerm);
	}
}


/* DoC_SelectChip: Select a given flash chip within the current floor */

static inline int DoC_SelectChip(struct DiskOnChip *doc, int chip)
{
	unsigned long docptr = doc->virtadr;

	/* Software requirement 11.4.4 before writing DeviceSelect */
	/* Deassert the CE line to eliminate glitches on the FCE# outputs */
	WriteDOC(CDSN_CTRL_WP, docptr, CDSNControl);
	DoC_Delay(doc, 4);	/* Software requirement 11.4.3 for Millennium */

	/* Select the individual flash chip requested */
	WriteDOC(chip, docptr, CDSNDeviceSelect);
	DoC_Delay(doc, 4);

	/* Reassert the CE line */
	WriteDOC(CDSN_CTRL_CE | CDSN_CTRL_FLASH_IO | CDSN_CTRL_WP, docptr,
		 CDSNControl);
	DoC_Delay(doc, 4);	/* Software requirement 11.4.3 for Millennium */

	/* Wait for it to be ready */
	return DoC_WaitReady(doc);
}

/* DoC_SelectFloor: Select a given floor (bank of flash chips) */

static inline int DoC_SelectFloor(struct DiskOnChip *doc, int floor)
{
	unsigned long docptr = doc->virtadr;

	/* Select the floor (bank) of chips required */
	WriteDOC(floor, docptr, FloorSelect);

	/* Wait for the chip to be ready */
	return DoC_WaitReady(doc);
}

/* DoC_IdentChip: Identify a given NAND chip given {floor,chip} */

static int DoC_IdentChip(struct DiskOnChip *doc, int floor, int chip)
{
	int mfr, id, i;
	volatile char dummy;

	/* Page in the required floor/chip */
	DoC_SelectFloor(doc, floor);
	DoC_SelectChip(doc, chip);

	/* Reset the chip */
	if (DoC_Command(doc, NAND_CMD_RESET, CDSN_CTRL_WP)) {
#ifdef DOC_DEBUG
		printf("DoC_Command (reset) for %d,%d returned true\n",
		       floor, chip);
#endif
		return 0;
	}


	/* Read the NAND chip ID: 1. Send ReadID command */
	if (DoC_Command(doc, NAND_CMD_READID, CDSN_CTRL_WP)) {
#ifdef DOC_DEBUG
		printf("DoC_Command (ReadID) for %d,%d returned true\n",
		       floor, chip);
#endif
		return 0;
	}

	/* Read the NAND chip ID: 2. Send address byte zero */
	DoC_Address(doc, ADDR_COLUMN, 0, CDSN_CTRL_WP, 0);

	/* Read the manufacturer and device id codes from the device */

	/* CDSN Slow IO register see Software Requirement 11.4 item 5. */
	dummy = ReadDOC(doc->virtadr, CDSNSlowIO);
	DoC_Delay(doc, 2);
	mfr = ReadDOC_(doc->virtadr, doc->ioreg);

	/* CDSN Slow IO register see Software Requirement 11.4 item 5. */
	dummy = ReadDOC(doc->virtadr, CDSNSlowIO);
	DoC_Delay(doc, 2);
	id = ReadDOC_(doc->virtadr, doc->ioreg);

	/* No response - return failure */
	if (mfr == 0xff || mfr == 0)
		return 0;

	/* Check it's the same as the first chip we identified.
	 * M-Systems say that any given DiskOnChip device should only
	 * contain _one_ type of flash part, although that's not a
	 * hardware restriction. */
	if (doc->mfr) {
		if (doc->mfr == mfr && doc->id == id)
			return 1;	/* This is another the same the first */
		else
			printf("Flash chip at floor %d, chip %d is different:\n",
			       floor, chip);
	}

	/* Print and store the manufacturer and ID codes. */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (mfr == nand_flash_ids[i].manufacture_id &&
		    id == nand_flash_ids[i].model_id) {
#ifdef DOC_DEBUG
			printf("Flash chip found: Manufacturer ID: %2.2X, "
			       "Chip ID: %2.2X (%s)\n", mfr, id,
			       nand_flash_ids[i].name);
#endif
			if (!doc->mfr) {
				doc->mfr = mfr;
				doc->id = id;
				doc->chipshift =
				    nand_flash_ids[i].chipshift;
				doc->page256 = nand_flash_ids[i].page256;
				doc->pageadrlen =
				    nand_flash_ids[i].pageadrlen;
				doc->erasesize =
				    nand_flash_ids[i].erasesize;
				doc->chips_name =
				    nand_flash_ids[i].name;
				return 1;
			}
			return 0;
		}
	}


#ifdef DOC_DEBUG
	/* We haven't fully identified the chip. Print as much as we know. */
	printf("Unknown flash chip found: %2.2X %2.2X\n",
	       id, mfr);
#endif

	return 0;
}

/* DoC_ScanChips: Find all NAND chips present in a DiskOnChip, and identify them */

static void DoC_ScanChips(struct DiskOnChip *this)
{
	int floor, chip;
	int numchips[MAX_FLOORS];
	int maxchips = MAX_CHIPS;
	int ret = 1;

	this->numchips = 0;
	this->mfr = 0;
	this->id = 0;

	if (DoC_is_Millennium(this))
		maxchips = MAX_CHIPS_MIL;

	/* For each floor, find the number of valid chips it contains */
	for (floor = 0; floor < MAX_FLOORS; floor++) {
		ret = 1;
		numchips[floor] = 0;
		for (chip = 0; chip < maxchips && ret != 0; chip++) {

			ret = DoC_IdentChip(this, floor, chip);
			if (ret) {
				numchips[floor]++;
				this->numchips++;
			}
		}
	}

	/* If there are none at all that we recognise, bail */
	if (!this->numchips) {
		puts ("No flash chips recognised.\n");
		return;
	}

	/* Allocate an array to hold the information for each chip */
	this->chips = malloc(sizeof(struct Nand) * this->numchips);
	if (!this->chips) {
		puts ("No memory for allocating chip info structures\n");
		return;
	}

	ret = 0;

	/* Fill out the chip array with {floor, chipno} for each
	 * detected chip in the device. */
	for (floor = 0; floor < MAX_FLOORS; floor++) {
		for (chip = 0; chip < numchips[floor]; chip++) {
			this->chips[ret].floor = floor;
			this->chips[ret].chip = chip;
			this->chips[ret].curadr = 0;
			this->chips[ret].curmode = 0x50;
			ret++;
		}
	}

	/* Calculate and print the total size of the device */
	this->totlen = this->numchips * (1 << this->chipshift);

#ifdef DOC_DEBUG
	printf("%d flash chips found. Total DiskOnChip size: %ld MB\n",
	       this->numchips, this->totlen >> 20);
#endif
}

/* find_boot_record: Find the NFTL Media Header and its Spare copy which contains the
 *	various device information of the NFTL partition and Bad Unit Table. Update
 *	the ReplUnitTable[] table accroding to the Bad Unit Table. ReplUnitTable[]
 *	is used for management of Erase Unit in other routines in nftl.c and nftlmount.c
 */
static int find_boot_record(struct NFTLrecord *nftl)
{
	struct nftl_uci1 h1;
	struct nftl_oob oob;
	unsigned int block, boot_record_count = 0;
	int retlen;
	u8 buf[SECTORSIZE];
	struct NFTLMediaHeader *mh = &nftl->MediaHdr;
	unsigned int i;

	nftl->MediaUnit = BLOCK_NIL;
	nftl->SpareMediaUnit = BLOCK_NIL;

	/* search for a valid boot record */
	for (block = 0; block < nftl->nb_blocks; block++) {
		int ret;

		/* Check for ANAND header first. Then can whinge if it's found but later
		   checks fail */
		if ((ret = doc_read_ecc(nftl->mtd, block * nftl->EraseSize, SECTORSIZE,
					(size_t *)&retlen, buf, NULL))) {
			static int warncount = 5;

			if (warncount) {
				printf("Block read at 0x%x failed\n", block * nftl->EraseSize);
				if (!--warncount)
					puts ("Further failures for this block will not be printed\n");
			}
			continue;
		}

		if (retlen < 6 || memcmp(buf, "ANAND", 6)) {
			/* ANAND\0 not found. Continue */
#ifdef PSYCHO_DEBUG
			printf("ANAND header not found at 0x%x\n", block * nftl->EraseSize);
#endif
			continue;
		}

#ifdef NFTL_DEBUG
		printf("ANAND header found at 0x%x\n", block * nftl->EraseSize);
#endif

		/* To be safer with BIOS, also use erase mark as discriminant */
		if ((ret = doc_read_oob(nftl->mtd, block * nftl->EraseSize + SECTORSIZE + 8,
				8, (size_t *)&retlen, (uchar *)&h1) < 0)) {
#ifdef NFTL_DEBUG
			printf("ANAND header found at 0x%x, but OOB data read failed\n",
			       block * nftl->EraseSize);
#endif
			continue;
		}

		/* OK, we like it. */

		if (boot_record_count) {
			/* We've already processed one. So we just check if
			   this one is the same as the first one we found */
			if (memcmp(mh, buf, sizeof(struct NFTLMediaHeader))) {
#ifdef NFTL_DEBUG
				printf("NFTL Media Headers at 0x%x and 0x%x disagree.\n",
				       nftl->MediaUnit * nftl->EraseSize, block * nftl->EraseSize);
#endif
				/* if (debug) Print both side by side */
				return -1;
			}
			if (boot_record_count == 1)
				nftl->SpareMediaUnit = block;

			boot_record_count++;
			continue;
		}

		/* This is the first we've seen. Copy the media header structure into place */
		memcpy(mh, buf, sizeof(struct NFTLMediaHeader));

		/* Do some sanity checks on it */
		if (mh->UnitSizeFactor == 0) {
#ifdef NFTL_DEBUG
			puts ("UnitSizeFactor 0x00 detected.\n"
			      "This violates the spec but we think we know what it means...\n");
#endif
		} else if (mh->UnitSizeFactor != 0xff) {
			printf ("Sorry, we don't support UnitSizeFactor "
			      "of != 1 yet.\n");
			return -1;
		}

		nftl->nb_boot_blocks = le16_to_cpu(mh->FirstPhysicalEUN);
		if ((nftl->nb_boot_blocks + 2) >= nftl->nb_blocks) {
			printf ("NFTL Media Header sanity check failed:\n"
				"nb_boot_blocks (%d) + 2 > nb_blocks (%d)\n",
				nftl->nb_boot_blocks, nftl->nb_blocks);
			return -1;
		}

		nftl->numvunits = le32_to_cpu(mh->FormattedSize) / nftl->EraseSize;
		if (nftl->numvunits > (nftl->nb_blocks - nftl->nb_boot_blocks - 2)) {
			printf ("NFTL Media Header sanity check failed:\n"
				"numvunits (%d) > nb_blocks (%d) - nb_boot_blocks(%d) - 2\n",
				nftl->numvunits,
				nftl->nb_blocks,
				nftl->nb_boot_blocks);
			return -1;
		}

		nftl->nr_sects  = nftl->numvunits * (nftl->EraseSize / SECTORSIZE);

		/* If we're not using the last sectors in the device for some reason,
		   reduce nb_blocks accordingly so we forget they're there */
		nftl->nb_blocks = le16_to_cpu(mh->NumEraseUnits) + le16_to_cpu(mh->FirstPhysicalEUN);

		/* read the Bad Erase Unit Table and modify ReplUnitTable[] accordingly */
		for (i = 0; i < nftl->nb_blocks; i++) {
			if ((i & (SECTORSIZE - 1)) == 0) {
				/* read one sector for every SECTORSIZE of blocks */
				if ((ret = doc_read_ecc(nftl->mtd, block * nftl->EraseSize +
						       i + SECTORSIZE, SECTORSIZE,
						       (size_t *)&retlen, buf, (uchar *)&oob)) < 0) {
					puts ("Read of bad sector table failed\n");
					return -1;
				}
			}
			/* mark the Bad Erase Unit as RESERVED in ReplUnitTable */
			if (buf[i & (SECTORSIZE - 1)] != 0xff)
				nftl->ReplUnitTable[i] = BLOCK_RESERVED;
		}

		nftl->MediaUnit = block;
		boot_record_count++;

	} /* foreach (block) */

	return boot_record_count?0:-1;
}

/* This routine is made available to other mtd code via
 * inter_module_register.  It must only be accessed through
 * inter_module_get which will bump the use count of this module.  The
 * addresses passed back in mtd are valid as long as the use count of
 * this module is non-zero, i.e. between inter_module_get and
 * inter_module_put.  Keith Owens <kaos@ocs.com.au> 29 Oct 2000.
 */
static void DoC2k_init(struct DiskOnChip* this)
{
	struct NFTLrecord *nftl;

	switch (this->ChipID) {
	case DOC_ChipID_Doc2k:
		this->name = "DiskOnChip 2000";
		this->ioreg = DoC_2k_CDSN_IO;
		break;
	case DOC_ChipID_DocMil:
		this->name = "DiskOnChip Millennium";
		this->ioreg = DoC_Mil_CDSN_IO;
		break;
	}

#ifdef DOC_DEBUG
	printf("%s found at address 0x%lX\n", this->name,
	       this->physadr);
#endif

	this->totlen = 0;
	this->numchips = 0;

	this->curfloor = -1;
	this->curchip = -1;

	/* Ident all the chips present. */
	DoC_ScanChips(this);
	if ((!this->numchips) || (!this->chips))
		return;

	nftl = &this->nftl;

	/* Get physical parameters */
	nftl->EraseSize = this->erasesize;
	nftl->nb_blocks = this->totlen / this->erasesize;
	nftl->mtd = this;

	if (find_boot_record(nftl) != 0)
		this->nftl_found = 0;
	else
		this->nftl_found = 1;

	printf("%s @ 0x%lX, %ld MB\n", this->name, this->physadr, this->totlen >> 20);
}

int doc_read_ecc(struct DiskOnChip* this, loff_t from, size_t len,
		 size_t * retlen, u_char * buf, u_char * eccbuf)
{
	unsigned long docptr;
	struct Nand *mychip;
	unsigned char syndrome[6];
	volatile char dummy;
	int i, len256 = 0, ret=0;

	docptr = this->virtadr;

	/* Don't allow read past end of device */
	if (from >= this->totlen) {
		puts ("Out of flash\n");
		return DOC_EINVAL;
	}

	/* Don't allow a single read to cross a 512-byte block boundary */
	if (from + len > ((from | 0x1ff) + 1))
		len = ((from | 0x1ff) + 1) - from;

	/* The ECC will not be calculated correctly if less than 512 is read */
	if (len != 0x200 && eccbuf)
		printf("ECC needs a full sector read (adr: %lx size %lx)\n",
		       (long) from, (long) len);

#ifdef PSYCHO_DEBUG
	printf("DoC_Read (adr: %lx size %lx)\n", (long) from, (long) len);
#endif

	/* Find the chip which is to be used and select it */
	mychip = &this->chips[shr(from, this->chipshift)];

	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(this, mychip->floor);
		DoC_SelectChip(this, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(this, mychip->chip);
	}

	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	DoC_Command(this,
		    (!this->page256
		     && (from & 0x100)) ? NAND_CMD_READ1 : NAND_CMD_READ0,
		    CDSN_CTRL_WP);
	DoC_Address(this, ADDR_COLUMN_PAGE, from, CDSN_CTRL_WP,
		    CDSN_CTRL_ECC_IO);

	if (eccbuf) {
		/* Prime the ECC engine */
		WriteDOC(DOC_ECC_RESET, docptr, ECCConf);
		WriteDOC(DOC_ECC_EN, docptr, ECCConf);
	} else {
		/* disable the ECC engine */
		WriteDOC(DOC_ECC_RESET, docptr, ECCConf);
		WriteDOC(DOC_ECC_DIS, docptr, ECCConf);
	}

	/* treat crossing 256-byte sector for 2M x 8bits devices */
	if (this->page256 && from + len > (from | 0xff) + 1) {
		len256 = (from | 0xff) + 1 - from;
		DoC_ReadBuf(this, buf, len256);

		DoC_Command(this, NAND_CMD_READ0, CDSN_CTRL_WP);
		DoC_Address(this, ADDR_COLUMN_PAGE, from + len256,
			    CDSN_CTRL_WP, CDSN_CTRL_ECC_IO);
	}

	DoC_ReadBuf(this, &buf[len256], len - len256);

	/* Let the caller know we completed it */
	*retlen = len;

	if (eccbuf) {
		/* Read the ECC data through the DiskOnChip ECC logic */
		/* Note: this will work even with 2M x 8bit devices as   */
		/*       they have 8 bytes of OOB per 256 page. mf.      */
		DoC_ReadBuf(this, eccbuf, 6);

		/* Flush the pipeline */
		if (DoC_is_Millennium(this)) {
			dummy = ReadDOC(docptr, ECCConf);
			dummy = ReadDOC(docptr, ECCConf);
			i = ReadDOC(docptr, ECCConf);
		} else {
			dummy = ReadDOC(docptr, 2k_ECCStatus);
			dummy = ReadDOC(docptr, 2k_ECCStatus);
			i = ReadDOC(docptr, 2k_ECCStatus);
		}

		/* Check the ECC Status */
		if (i & 0x80) {
			int nb_errors;
			/* There was an ECC error */
#ifdef ECC_DEBUG
			printf("DiskOnChip ECC Error: Read at %lx\n", (long)from);
#endif
			/* Read the ECC syndrom through the DiskOnChip ECC logic.
			   These syndrome will be all ZERO when there is no error */
			for (i = 0; i < 6; i++) {
				syndrome[i] =
				    ReadDOC(docptr, ECCSyndrome0 + i);
			}
			nb_errors = doc_decode_ecc(buf, syndrome);

#ifdef ECC_DEBUG
			printf("Errors corrected: %x\n", nb_errors);
#endif
			if (nb_errors < 0) {
				/* We return error, but have actually done the read. Not that
				   this can be told to user-space, via sys_read(), but at least
				   MTD-aware stuff can know about it by checking *retlen */
				printf("ECC Errors at %lx\n", (long)from);
				ret = DOC_EECC;
			}
		}

#ifdef PSYCHO_DEBUG
		printf("ECC DATA at %lxB: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
			     (long)from, eccbuf[0], eccbuf[1], eccbuf[2],
			     eccbuf[3], eccbuf[4], eccbuf[5]);
#endif

		/* disable the ECC engine */
		WriteDOC(DOC_ECC_DIS, docptr , ECCConf);
	}

	/* according to 11.4.1, we need to wait for the busy line
	 * drop if we read to the end of the page.  */
	if(0 == ((from + *retlen) & 0x1ff))
	{
	    DoC_WaitReady(this);
	}

	return ret;
}

int doc_write_ecc(struct DiskOnChip* this, loff_t to, size_t len,
		  size_t * retlen, const u_char * buf,
		  u_char * eccbuf)
{
	int di; /* Yes, DI is a hangover from when I was disassembling the binary driver */
	unsigned long docptr;
	volatile char dummy;
	int len256 = 0;
	struct Nand *mychip;

	docptr = this->virtadr;

	/* Don't allow write past end of device */
	if (to >= this->totlen) {
		puts ("Out of flash\n");
		return DOC_EINVAL;
	}

	/* Don't allow a single write to cross a 512-byte block boundary */
	if (to + len > ((to | 0x1ff) + 1))
		len = ((to | 0x1ff) + 1) - to;

	/* The ECC will not be calculated correctly if less than 512 is written */
	if (len != 0x200 && eccbuf)
		printf("ECC needs a full sector write (adr: %lx size %lx)\n",
		       (long) to, (long) len);

	/* printf("DoC_Write (adr: %lx size %lx)\n", (long) to, (long) len); */

	/* Find the chip which is to be used and select it */
	mychip = &this->chips[shr(to, this->chipshift)];

	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(this, mychip->floor);
		DoC_SelectChip(this, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(this, mychip->chip);
	}

	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* Set device to main plane of flash */
	DoC_Command(this, NAND_CMD_RESET, CDSN_CTRL_WP);
	DoC_Command(this,
		    (!this->page256
		     && (to & 0x100)) ? NAND_CMD_READ1 : NAND_CMD_READ0,
		    CDSN_CTRL_WP);

	DoC_Command(this, NAND_CMD_SEQIN, 0);
	DoC_Address(this, ADDR_COLUMN_PAGE, to, 0, CDSN_CTRL_ECC_IO);

	if (eccbuf) {
		/* Prime the ECC engine */
		WriteDOC(DOC_ECC_RESET, docptr, ECCConf);
		WriteDOC(DOC_ECC_EN | DOC_ECC_RW, docptr, ECCConf);
	} else {
		/* disable the ECC engine */
		WriteDOC(DOC_ECC_RESET, docptr, ECCConf);
		WriteDOC(DOC_ECC_DIS, docptr, ECCConf);
	}

	/* treat crossing 256-byte sector for 2M x 8bits devices */
	if (this->page256 && to + len > (to | 0xff) + 1) {
		len256 = (to | 0xff) + 1 - to;
		DoC_WriteBuf(this, buf, len256);

		DoC_Command(this, NAND_CMD_PAGEPROG, 0);

		DoC_Command(this, NAND_CMD_STATUS, CDSN_CTRL_WP);
		/* There's an implicit DoC_WaitReady() in DoC_Command */

		dummy = ReadDOC(docptr, CDSNSlowIO);
		DoC_Delay(this, 2);

		if (ReadDOC_(docptr, this->ioreg) & 1) {
			puts ("Error programming flash\n");
			/* Error in programming */
			*retlen = 0;
			return DOC_EIO;
		}

		DoC_Command(this, NAND_CMD_SEQIN, 0);
		DoC_Address(this, ADDR_COLUMN_PAGE, to + len256, 0,
			    CDSN_CTRL_ECC_IO);
	}

	DoC_WriteBuf(this, &buf[len256], len - len256);

	if (eccbuf) {
		WriteDOC(CDSN_CTRL_ECC_IO | CDSN_CTRL_CE, docptr,
			 CDSNControl);

		if (DoC_is_Millennium(this)) {
			WriteDOC(0, docptr, NOP);
			WriteDOC(0, docptr, NOP);
			WriteDOC(0, docptr, NOP);
		} else {
			WriteDOC_(0, docptr, this->ioreg);
			WriteDOC_(0, docptr, this->ioreg);
			WriteDOC_(0, docptr, this->ioreg);
		}

		/* Read the ECC data through the DiskOnChip ECC logic */
		for (di = 0; di < 6; di++) {
			eccbuf[di] = ReadDOC(docptr, ECCSyndrome0 + di);
		}

		/* Reset the ECC engine */
		WriteDOC(DOC_ECC_DIS, docptr, ECCConf);

#ifdef PSYCHO_DEBUG
		printf
		    ("OOB data at %lx is %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
		     (long) to, eccbuf[0], eccbuf[1], eccbuf[2], eccbuf[3],
		     eccbuf[4], eccbuf[5]);
#endif
	}

	DoC_Command(this, NAND_CMD_PAGEPROG, 0);

	DoC_Command(this, NAND_CMD_STATUS, CDSN_CTRL_WP);
	/* There's an implicit DoC_WaitReady() in DoC_Command */

	dummy = ReadDOC(docptr, CDSNSlowIO);
	DoC_Delay(this, 2);

	if (ReadDOC_(docptr, this->ioreg) & 1) {
		puts ("Error programming flash\n");
		/* Error in programming */
		*retlen = 0;
		return DOC_EIO;
	}

	/* Let the caller know we completed it */
	*retlen = len;

	if (eccbuf) {
		unsigned char x[8];
		size_t dummy;
		int ret;

		/* Write the ECC data to flash */
		for (di=0; di<6; di++)
			x[di] = eccbuf[di];

		x[6]=0x55;
		x[7]=0x55;

		ret = doc_write_oob(this, to, 8, &dummy, x);
		return ret;
	}
	return 0;
}

int doc_read_oob(struct DiskOnChip* this, loff_t ofs, size_t len,
		 size_t * retlen, u_char * buf)
{
	int len256 = 0, ret;
	unsigned long docptr;
	struct Nand *mychip;

	docptr = this->virtadr;

	mychip = &this->chips[shr(ofs, this->chipshift)];

	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(this, mychip->floor);
		DoC_SelectChip(this, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(this, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* update address for 2M x 8bit devices. OOB starts on the second */
	/* page to maintain compatibility with doc_read_ecc. */
	if (this->page256) {
		if (!(ofs & 0x8))
			ofs += 0x100;
		else
			ofs -= 0x8;
	}

	DoC_Command(this, NAND_CMD_READOOB, CDSN_CTRL_WP);
	DoC_Address(this, ADDR_COLUMN_PAGE, ofs, CDSN_CTRL_WP, 0);

	/* treat crossing 8-byte OOB data for 2M x 8bit devices */
	/* Note: datasheet says it should automaticaly wrap to the */
	/*       next OOB block, but it didn't work here. mf.      */
	if (this->page256 && ofs + len > (ofs | 0x7) + 1) {
		len256 = (ofs | 0x7) + 1 - ofs;
		DoC_ReadBuf(this, buf, len256);

		DoC_Command(this, NAND_CMD_READOOB, CDSN_CTRL_WP);
		DoC_Address(this, ADDR_COLUMN_PAGE, ofs & (~0x1ff),
			    CDSN_CTRL_WP, 0);
	}

	DoC_ReadBuf(this, &buf[len256], len - len256);

	*retlen = len;
	/* Reading the full OOB data drops us off of the end of the page,
	 * causing the flash device to go into busy mode, so we need
	 * to wait until ready 11.4.1 and Toshiba TC58256FT docs */

	ret = DoC_WaitReady(this);

	return ret;

}

int doc_write_oob(struct DiskOnChip* this, loff_t ofs, size_t len,
		  size_t * retlen, const u_char * buf)
{
	int len256 = 0;
	unsigned long docptr = this->virtadr;
	struct Nand *mychip = &this->chips[shr(ofs, this->chipshift)];
	volatile int dummy;

#ifdef PSYCHO_DEBUG
	printf("doc_write_oob(%lx, %d): %2.2X %2.2X %2.2X %2.2X ... %2.2X %2.2X .. %2.2X %2.2X\n",
	       (long)ofs, len, buf[0], buf[1], buf[2], buf[3],
	       buf[8], buf[9], buf[14],buf[15]);
#endif

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(this, mychip->floor);
		DoC_SelectChip(this, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(this, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* disable the ECC engine */
	WriteDOC (DOC_ECC_RESET, docptr, ECCConf);
	WriteDOC (DOC_ECC_DIS, docptr, ECCConf);

	/* Reset the chip, see Software Requirement 11.4 item 1. */
	DoC_Command(this, NAND_CMD_RESET, CDSN_CTRL_WP);

	/* issue the Read2 command to set the pointer to the Spare Data Area. */
	DoC_Command(this, NAND_CMD_READOOB, CDSN_CTRL_WP);

	/* update address for 2M x 8bit devices. OOB starts on the second */
	/* page to maintain compatibility with doc_read_ecc. */
	if (this->page256) {
		if (!(ofs & 0x8))
			ofs += 0x100;
		else
			ofs -= 0x8;
	}

	/* issue the Serial Data In command to initial the Page Program process */
	DoC_Command(this, NAND_CMD_SEQIN, 0);
	DoC_Address(this, ADDR_COLUMN_PAGE, ofs, 0, 0);

	/* treat crossing 8-byte OOB data for 2M x 8bit devices */
	/* Note: datasheet says it should automaticaly wrap to the */
	/*       next OOB block, but it didn't work here. mf.      */
	if (this->page256 && ofs + len > (ofs | 0x7) + 1) {
		len256 = (ofs | 0x7) + 1 - ofs;
		DoC_WriteBuf(this, buf, len256);

		DoC_Command(this, NAND_CMD_PAGEPROG, 0);
		DoC_Command(this, NAND_CMD_STATUS, 0);
		/* DoC_WaitReady() is implicit in DoC_Command */

		dummy = ReadDOC(docptr, CDSNSlowIO);
		DoC_Delay(this, 2);

		if (ReadDOC_(docptr, this->ioreg) & 1) {
			puts ("Error programming oob data\n");
			/* There was an error */
			*retlen = 0;
			return DOC_EIO;
		}
		DoC_Command(this, NAND_CMD_SEQIN, 0);
		DoC_Address(this, ADDR_COLUMN_PAGE, ofs & (~0x1ff), 0, 0);
	}

	DoC_WriteBuf(this, &buf[len256], len - len256);

	DoC_Command(this, NAND_CMD_PAGEPROG, 0);
	DoC_Command(this, NAND_CMD_STATUS, 0);
	/* DoC_WaitReady() is implicit in DoC_Command */

	dummy = ReadDOC(docptr, CDSNSlowIO);
	DoC_Delay(this, 2);

	if (ReadDOC_(docptr, this->ioreg) & 1) {
		puts ("Error programming oob data\n");
		/* There was an error */
		*retlen = 0;
		return DOC_EIO;
	}

	*retlen = len;
	return 0;

}

int doc_erase(struct DiskOnChip* this, loff_t ofs, size_t len)
{
	volatile int dummy;
	unsigned long docptr;
	struct Nand *mychip;

	if (ofs & (this->erasesize-1) || len & (this->erasesize-1)) {
		puts ("Offset and size must be sector aligned\n");
		return DOC_EINVAL;
	}

	docptr = this->virtadr;

	/* FIXME: Do this in the background. Use timers or schedule_task() */
	while(len) {
		mychip = &this->chips[shr(ofs, this->chipshift)];

		if (this->curfloor != mychip->floor) {
			DoC_SelectFloor(this, mychip->floor);
			DoC_SelectChip(this, mychip->chip);
		} else if (this->curchip != mychip->chip) {
			DoC_SelectChip(this, mychip->chip);
		}
		this->curfloor = mychip->floor;
		this->curchip = mychip->chip;

		DoC_Command(this, NAND_CMD_ERASE1, 0);
		DoC_Address(this, ADDR_PAGE, ofs, 0, 0);
		DoC_Command(this, NAND_CMD_ERASE2, 0);

		DoC_Command(this, NAND_CMD_STATUS, CDSN_CTRL_WP);

		dummy = ReadDOC(docptr, CDSNSlowIO);
		DoC_Delay(this, 2);

		if (ReadDOC_(docptr, this->ioreg) & 1) {
			printf("Error erasing at 0x%lx\n", (long)ofs);
			/* There was an error */
			goto callback;
		}
		ofs += this->erasesize;
		len -= this->erasesize;
	}

 callback:
	return 0;
}

static inline int doccheck(unsigned long potential, unsigned long physadr)
{
	unsigned long window=potential;
	unsigned char tmp, ChipID;
#ifndef DOC_PASSIVE_PROBE
	unsigned char tmp2;
#endif

	/* Routine copied from the Linux DOC driver */

#ifdef CFG_DOCPROBE_55AA
	/* Check for 0x55 0xAA signature at beginning of window,
	   this is no longer true once we remove the IPL (for Millennium */
	if (ReadDOC(window, Sig1) != 0x55 || ReadDOC(window, Sig2) != 0xaa)
		return 0;
#endif /* CFG_DOCPROBE_55AA */

#ifndef DOC_PASSIVE_PROBE
	/* It's not possible to cleanly detect the DiskOnChip - the
	 * bootup procedure will put the device into reset mode, and
	 * it's not possible to talk to it without actually writing
	 * to the DOCControl register. So we store the current contents
	 * of the DOCControl register's location, in case we later decide
	 * that it's not a DiskOnChip, and want to put it back how we
	 * found it.
	 */
	tmp2 = ReadDOC(window, DOCControl);

	/* Reset the DiskOnChip ASIC */
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_RESET,
		 window, DOCControl);
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_RESET,
		 window, DOCControl);

	/* Enable the DiskOnChip ASIC */
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_NORMAL,
		 window, DOCControl);
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_NORMAL,
		 window, DOCControl);
#endif /* !DOC_PASSIVE_PROBE */

	ChipID = ReadDOC(window, ChipID);

	switch (ChipID) {
	case DOC_ChipID_Doc2k:
		/* Check the TOGGLE bit in the ECC register */
		tmp = ReadDOC(window, 2k_ECCStatus) & DOC_TOGGLE_BIT;
		if ((ReadDOC(window, 2k_ECCStatus) & DOC_TOGGLE_BIT) != tmp)
				return ChipID;
		break;

	case DOC_ChipID_DocMil:
		/* Check the TOGGLE bit in the ECC register */
		tmp = ReadDOC(window, ECCConf) & DOC_TOGGLE_BIT;
		if ((ReadDOC(window, ECCConf) & DOC_TOGGLE_BIT) != tmp)
				return ChipID;
		break;

	default:
#ifndef CFG_DOCPROBE_55AA
/*
 * if the ID isn't the DoC2000 or DoCMillenium ID, so we can assume
 * the DOC is missing
 */
# if 0
		printf("Possible DiskOnChip with unknown ChipID %2.2X found at 0x%lx\n",
		       ChipID, physadr);
# endif
#endif
#ifndef DOC_PASSIVE_PROBE
		/* Put back the contents of the DOCControl register, in case it's not
		 * actually a DiskOnChip.
		 */
		WriteDOC(tmp2, window, DOCControl);
#endif
		return 0;
	}

	puts ("DiskOnChip failed TOGGLE test, dropping.\n");

#ifndef DOC_PASSIVE_PROBE
	/* Put back the contents of the DOCControl register: it's not a DiskOnChip */
	WriteDOC(tmp2, window, DOCControl);
#endif
	return 0;
}

void doc_probe(unsigned long physadr)
{
	struct DiskOnChip *this = NULL;
	int i=0, ChipID;

	if ((ChipID = doccheck(physadr, physadr))) {

		for (i=0; i<CFG_MAX_DOC_DEVICE; i++) {
			if (doc_dev_desc[i].ChipID == DOC_ChipID_UNKNOWN) {
				this = doc_dev_desc + i;
				break;
			}
		}

		if (!this) {
			puts ("Cannot allocate memory for data structures.\n");
			return;
		}

		if (curr_device == -1)
			curr_device = i;

		memset((char *)this, 0, sizeof(struct DiskOnChip));

		this->virtadr = physadr;
		this->physadr = physadr;
		this->ChipID = ChipID;

		DoC2k_init(this);
	} else {
		puts ("No DiskOnChip found\n");
	}
}
