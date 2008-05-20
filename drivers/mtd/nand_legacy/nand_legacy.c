/*
 * (C) 2006 Denx
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <watchdog.h>

#if defined(CONFIG_CMD_NAND) && defined(CFG_NAND_LEGACY)

#include <linux/mtd/nand_legacy.h>
#include <linux/mtd/nand_ids.h>
#include <jffs2/jffs2.h>

#ifdef CONFIG_OMAP1510
void archflashwp(void *archdata, int wp);
#endif

#define ROUND_DOWN(value,boundary)      ((value) & (~((boundary)-1)))

#undef	PSYCHO_DEBUG
#undef	NAND_DEBUG

/* ****************** WARNING *********************
 * When ALLOW_ERASE_BAD_DEBUG is non-zero the erase command will
 * erase (or at least attempt to erase) blocks that are marked
 * bad. This can be very handy if you are _sure_ that the block
 * is OK, say because you marked a good block bad to test bad
 * block handling and you are done testing, or if you have
 * accidentally marked blocks bad.
 *
 * Erasing factory marked bad blocks is a _bad_ idea. If the
 * erase succeeds there is no reliable way to find them again,
 * and attempting to program or erase bad blocks can affect
 * the data in _other_ (good) blocks.
 */
#define	 ALLOW_ERASE_BAD_DEBUG 0

#define CONFIG_MTD_NAND_ECC  /* enable ECC */
#define CONFIG_MTD_NAND_ECC_JFFS2

/* bits for nand_legacy_rw() `cmd'; or together as needed */
#define NANDRW_READ	0x01
#define NANDRW_WRITE	0x00
#define NANDRW_JFFS2	0x02
#define NANDRW_JFFS2_SKIP	0x04


/*
 * Exported variables etc.
 */

/* Definition of the out of band configuration structure */
struct nand_oob_config {
	/* position of ECC bytes inside oob */
	int ecc_pos[6];
	/* position of  bad blk flag inside oob -1 = inactive */
	int badblock_pos;
	/* position of ECC valid flag inside oob -1 = inactive */
	int eccvalid_pos;
} oob_config = { {0}, 0, 0};

struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE] = {{0}};

int curr_device = -1; /* Current NAND Device */


/*
 * Exported functionss
 */
int nand_legacy_erase(struct nand_chip* nand, size_t ofs,
		     size_t len, int clean);
int nand_legacy_rw(struct nand_chip* nand, int cmd,
		  size_t start, size_t len,
		  size_t * retlen, u_char * buf);
void nand_print(struct nand_chip *nand);
void nand_print_bad(struct nand_chip *nand);
int nand_read_oob(struct nand_chip* nand, size_t ofs, size_t len,
		 size_t * retlen, u_char * buf);
int nand_write_oob(struct nand_chip* nand, size_t ofs, size_t len,
		 size_t * retlen, const u_char * buf);

/*
 * Internals
 */
static int NanD_WaitReady(struct nand_chip *nand, int ale_wait);
static int nand_read_ecc(struct nand_chip *nand, size_t start, size_t len,
		 size_t * retlen, u_char *buf, u_char *ecc_code);
static int nand_write_ecc (struct nand_chip* nand, size_t to, size_t len,
			   size_t * retlen, const u_char * buf,
			   u_char * ecc_code);
#ifdef CONFIG_MTD_NAND_ECC
static int nand_correct_data (u_char *dat, u_char *read_ecc, u_char *calc_ecc);
static void nand_calculate_ecc (const u_char *dat, u_char *ecc_code);
#endif


/*
 *
 * Function definitions
 *
 */

/* returns 0 if block containing pos is OK:
 *		valid erase block and
 *		not marked bad, or no bad mark position is specified
 * returns 1 if marked bad or otherwise invalid
 */
static int check_block (struct nand_chip *nand, unsigned long pos)
{
	size_t retlen;
	uint8_t oob_data;
	uint16_t oob_data16[6];
	int page0 = pos & (-nand->erasesize);
	int page1 = page0 + nand->oobblock;
	int badpos = oob_config.badblock_pos;

	if (pos >= nand->totlen)
		return 1;

	if (badpos < 0)
		return 0;	/* no way to check, assume OK */

	if (nand->bus16) {
		if (nand_read_oob(nand, (page0 + 0), 12, &retlen, (uint8_t *)oob_data16)
		    || (oob_data16[2] & 0xff00) != 0xff00)
			return 1;
		if (nand_read_oob(nand, (page1 + 0), 12, &retlen, (uint8_t *)oob_data16)
		    || (oob_data16[2] & 0xff00) != 0xff00)
			return 1;
	} else {
		/* Note - bad block marker can be on first or second page */
		if (nand_read_oob(nand, page0 + badpos, 1, &retlen, (unsigned char *)&oob_data)
		    || oob_data != 0xff
		    || nand_read_oob (nand, page1 + badpos, 1, &retlen, (unsigned char *)&oob_data)
		    || oob_data != 0xff)
			return 1;
	}

	return 0;
}

/* print bad blocks in NAND flash */
void nand_print_bad(struct nand_chip* nand)
{
	unsigned long pos;

	for (pos = 0; pos < nand->totlen; pos += nand->erasesize) {
		if (check_block(nand, pos))
			printf(" 0x%8.8lx\n", pos);
	}
	puts("\n");
}

/* cmd: 0: NANDRW_WRITE			write, fail on bad block
 *	1: NANDRW_READ			read, fail on bad block
 *	2: NANDRW_WRITE | NANDRW_JFFS2	write, skip bad blocks
 *	3: NANDRW_READ | NANDRW_JFFS2	read, data all 0xff for bad blocks
 *      7: NANDRW_READ | NANDRW_JFFS2 | NANDRW_JFFS2_SKIP read, skip bad blocks
 */
int nand_legacy_rw (struct nand_chip* nand, int cmd,
		   size_t start, size_t len,
		   size_t * retlen, u_char * buf)
{
	int ret = 0, n, total = 0;
	char eccbuf[6];
	/* eblk (once set) is the start of the erase block containing the
	 * data being processed.
	 */
	unsigned long eblk = ~0;	/* force mismatch on first pass */
	unsigned long erasesize = nand->erasesize;

	while (len) {
		if ((start & (-erasesize)) != eblk) {
			/* have crossed into new erase block, deal with
			 * it if it is sure marked bad.
			 */
			eblk = start & (-erasesize); /* start of block */
			if (check_block(nand, eblk)) {
				if (cmd == (NANDRW_READ | NANDRW_JFFS2)) {
					while (len > 0 &&
					       start - eblk < erasesize) {
						*(buf++) = 0xff;
						++start;
						++total;
						--len;
					}
					continue;
				} else if (cmd == (NANDRW_READ | NANDRW_JFFS2 | NANDRW_JFFS2_SKIP)) {
					start += erasesize;
					continue;
				} else if (cmd == (NANDRW_WRITE | NANDRW_JFFS2)) {
					/* skip bad block */
					start += erasesize;
					continue;
				} else {
					ret = 1;
					break;
				}
			}
		}
		/* The ECC will not be calculated correctly if
		   less than 512 is written or read */
		/* Is request at least 512 bytes AND it starts on a proper boundry */
		if((start != ROUND_DOWN(start, 0x200)) || (len < 0x200))
			printf("Warning block writes should be at least 512 bytes and start on a 512 byte boundry\n");

		if (cmd & NANDRW_READ) {
			ret = nand_read_ecc(nand, start,
					   min(len, eblk + erasesize - start),
					   (size_t *)&n, (u_char*)buf, (u_char *)eccbuf);
		} else {
			ret = nand_write_ecc(nand, start,
					    min(len, eblk + erasesize - start),
					    (size_t *)&n, (u_char*)buf, (u_char *)eccbuf);
		}

		if (ret)
			break;

		start  += n;
		buf   += n;
		total += n;
		len   -= n;
	}
	if (retlen)
		*retlen = total;

	return ret;
}

void nand_print(struct nand_chip *nand)
{
	if (nand->numchips > 1) {
		printf("%s at 0x%lx,\n"
		       "\t  %d chips %s, size %d MB, \n"
		       "\t  total size %ld MB, sector size %ld kB\n",
		       nand->name, nand->IO_ADDR, nand->numchips,
		       nand->chips_name, 1 << (nand->chipshift - 20),
		       nand->totlen >> 20, nand->erasesize >> 10);
	}
	else {
		printf("%s at 0x%lx (", nand->chips_name, nand->IO_ADDR);
		print_size(nand->totlen, ", ");
		print_size(nand->erasesize, " sector)\n");
	}
}

/* ------------------------------------------------------------------------- */

static int NanD_WaitReady(struct nand_chip *nand, int ale_wait)
{
	/* This is inline, to optimise the common case, where it's ready instantly */
	int ret = 0;

#ifdef NAND_NO_RB	/* in config file, shorter delays currently wrap accesses */
	if(ale_wait)
		NAND_WAIT_READY(nand);	/* do the worst case 25us wait */
	else
		udelay(10);
#else	/* has functional r/b signal */
	NAND_WAIT_READY(nand);
#endif
	return ret;
}

/* NanD_Command: Send a flash command to the flash chip */

static inline int NanD_Command(struct nand_chip *nand, unsigned char command)
{
	unsigned long nandptr = nand->IO_ADDR;

	/* Assert the CLE (Command Latch Enable) line to the flash chip */
	NAND_CTL_SETCLE(nandptr);

	/* Send the command */
	WRITE_NAND_COMMAND(command, nandptr);

	/* Lower the CLE line */
	NAND_CTL_CLRCLE(nandptr);

#ifdef NAND_NO_RB
	if(command == NAND_CMD_RESET){
		u_char ret_val;
		NanD_Command(nand, NAND_CMD_STATUS);
		do {
			ret_val = READ_NAND(nandptr);/* wait till ready */
		} while((ret_val & 0x40) != 0x40);
	}
#endif
	return NanD_WaitReady(nand, 0);
}

/* NanD_Address: Set the current address for the flash chip */

static int NanD_Address(struct nand_chip *nand, int numbytes, unsigned long ofs)
{
	unsigned long nandptr;
	int i;

	nandptr = nand->IO_ADDR;

	/* Assert the ALE (Address Latch Enable) line to the flash chip */
	NAND_CTL_SETALE(nandptr);

	/* Send the address */
	/* Devices with 256-byte page are addressed as:
	 * Column (bits 0-7), Page (bits 8-15, 16-23, 24-31)
	 * there is no device on the market with page256
	 * and more than 24 bits.
	 * Devices with 512-byte page are addressed as:
	 * Column (bits 0-7), Page (bits 9-16, 17-24, 25-31)
	 * 25-31 is sent only if the chip support it.
	 * bit 8 changes the read command to be sent
	 * (NAND_CMD_READ0 or NAND_CMD_READ1).
	 */

	if (numbytes == ADDR_COLUMN || numbytes == ADDR_COLUMN_PAGE)
		WRITE_NAND_ADDRESS(ofs, nandptr);

	ofs = ofs >> nand->page_shift;

	if (numbytes == ADDR_PAGE || numbytes == ADDR_COLUMN_PAGE) {
		for (i = 0; i < nand->pageadrlen; i++, ofs = ofs >> 8) {
			WRITE_NAND_ADDRESS(ofs, nandptr);
		}
	}

	/* Lower the ALE line */
	NAND_CTL_CLRALE(nandptr);

	/* Wait for the chip to respond */
	return NanD_WaitReady(nand, 1);
}

/* NanD_SelectChip: Select a given flash chip within the current floor */

static inline int NanD_SelectChip(struct nand_chip *nand, int chip)
{
	/* Wait for it to be ready */
	return NanD_WaitReady(nand, 0);
}

/* NanD_IdentChip: Identify a given NAND chip given {floor,chip} */

static int NanD_IdentChip(struct nand_chip *nand, int floor, int chip)
{
	int mfr, id, i;

	NAND_ENABLE_CE(nand);  /* set pin low */
	/* Reset the chip */
	if (NanD_Command(nand, NAND_CMD_RESET)) {
#ifdef NAND_DEBUG
		printf("NanD_Command (reset) for %d,%d returned true\n",
		       floor, chip);
#endif
		NAND_DISABLE_CE(nand);  /* set pin high */
		return 0;
	}

	/* Read the NAND chip ID: 1. Send ReadID command */
	if (NanD_Command(nand, NAND_CMD_READID)) {
#ifdef NAND_DEBUG
		printf("NanD_Command (ReadID) for %d,%d returned true\n",
		       floor, chip);
#endif
		NAND_DISABLE_CE(nand);  /* set pin high */
		return 0;
	}

	/* Read the NAND chip ID: 2. Send address byte zero */
	NanD_Address(nand, ADDR_COLUMN, 0);

	/* Read the manufacturer and device id codes from the device */

	mfr = READ_NAND(nand->IO_ADDR);

	id = READ_NAND(nand->IO_ADDR);

	NAND_DISABLE_CE(nand);  /* set pin high */

#ifdef NAND_DEBUG
	printf("NanD_Command (ReadID) got %x %x\n", mfr, id);
#endif
	if (mfr == 0xff || mfr == 0) {
		/* No response - return failure */
		return 0;
	}

	/* Check it's the same as the first chip we identified.
	 * M-Systems say that any given nand_chip device should only
	 * contain _one_ type of flash part, although that's not a
	 * hardware restriction. */
	if (nand->mfr) {
		if (nand->mfr == mfr && nand->id == id) {
			return 1;	/* This is another the same the first */
		} else {
			printf("Flash chip at floor %d, chip %d is different:\n",
			       floor, chip);
		}
	}

	/* Print and store the manufacturer and ID codes. */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (mfr == nand_flash_ids[i].manufacture_id &&
		    id == nand_flash_ids[i].model_id) {
#ifdef NAND_DEBUG
			printf("Flash chip found:\n\t Manufacturer ID: 0x%2.2X, "
			       "Chip ID: 0x%2.2X (%s)\n", mfr, id,
			       nand_flash_ids[i].name);
#endif
			if (!nand->mfr) {
				nand->mfr = mfr;
				nand->id = id;
				nand->chipshift =
				    nand_flash_ids[i].chipshift;
				nand->page256 = nand_flash_ids[i].page256;
				nand->eccsize = 256;
				if (nand->page256) {
					nand->oobblock = 256;
					nand->oobsize = 8;
					nand->page_shift = 8;
				} else {
					nand->oobblock = 512;
					nand->oobsize = 16;
					nand->page_shift = 9;
				}
				nand->pageadrlen = nand_flash_ids[i].pageadrlen;
				nand->erasesize  = nand_flash_ids[i].erasesize;
				nand->chips_name = nand_flash_ids[i].name;
				nand->bus16	 = nand_flash_ids[i].bus16;
				return 1;
			}
			return 0;
		}
	}


#ifdef NAND_DEBUG
	/* We haven't fully identified the chip. Print as much as we know. */
	printf("Unknown flash chip found: %2.2X %2.2X\n",
	       id, mfr);
#endif

	return 0;
}

/* NanD_ScanChips: Find all NAND chips present in a nand_chip, and identify them */

static void NanD_ScanChips(struct nand_chip *nand)
{
	int floor, chip;
	int numchips[NAND_MAX_FLOORS];
	int maxchips = NAND_MAX_CHIPS;
	int ret = 1;

	nand->numchips = 0;
	nand->mfr = 0;
	nand->id = 0;


	/* For each floor, find the number of valid chips it contains */
	for (floor = 0; floor < NAND_MAX_FLOORS; floor++) {
		ret = 1;
		numchips[floor] = 0;
		for (chip = 0; chip < maxchips && ret != 0; chip++) {

			ret = NanD_IdentChip(nand, floor, chip);
			if (ret) {
				numchips[floor]++;
				nand->numchips++;
			}
		}
	}

	/* If there are none at all that we recognise, bail */
	if (!nand->numchips) {
#ifdef NAND_DEBUG
		puts ("No NAND flash chips recognised.\n");
#endif
		return;
	}

	/* Allocate an array to hold the information for each chip */
	nand->chips = malloc(sizeof(struct Nand) * nand->numchips);
	if (!nand->chips) {
		puts ("No memory for allocating chip info structures\n");
		return;
	}

	ret = 0;

	/* Fill out the chip array with {floor, chipno} for each
	 * detected chip in the device. */
	for (floor = 0; floor < NAND_MAX_FLOORS; floor++) {
		for (chip = 0; chip < numchips[floor]; chip++) {
			nand->chips[ret].floor = floor;
			nand->chips[ret].chip = chip;
			nand->chips[ret].curadr = 0;
			nand->chips[ret].curmode = 0x50;
			ret++;
		}
	}

	/* Calculate and print the total size of the device */
	nand->totlen = nand->numchips * (1 << nand->chipshift);

#ifdef NAND_DEBUG
	printf("%d flash chips found. Total nand_chip size: %ld MB\n",
	       nand->numchips, nand->totlen >> 20);
#endif
}

/* we need to be fast here, 1 us per read translates to 1 second per meg */
static void NanD_ReadBuf (struct nand_chip *nand, u_char * data_buf, int cntr)
{
	unsigned long nandptr = nand->IO_ADDR;

	NanD_Command (nand, NAND_CMD_READ0);

	if (nand->bus16) {
		u16 val;

		while (cntr >= 16) {
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			cntr -= 16;
		}

		while (cntr > 0) {
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			cntr -= 2;
		}
	} else {
		while (cntr >= 16) {
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			cntr -= 16;
		}

		while (cntr > 0) {
			*data_buf++ = READ_NAND (nandptr);
			cntr--;
		}
	}
}

/*
 * NAND read with ECC
 */
static int nand_read_ecc(struct nand_chip *nand, size_t start, size_t len,
		 size_t * retlen, u_char *buf, u_char *ecc_code)
{
	int col, page;
	int ecc_status = 0;
#ifdef CONFIG_MTD_NAND_ECC
	int j;
	int ecc_failed = 0;
	u_char *data_poi;
	u_char ecc_calc[6];
#endif

	/* Do not allow reads past end of device */
	if ((start + len) > nand->totlen) {
		printf ("%s: Attempt read beyond end of device %x %x %x\n",
			__FUNCTION__, (uint) start, (uint) len, (uint) nand->totlen);
		*retlen = 0;
		return -1;
	}

	/* First we calculate the starting page */
	/*page = shr(start, nand->page_shift);*/
	page = start >> nand->page_shift;

	/* Get raw starting column */
	col = start & (nand->oobblock - 1);

	/* Initialize return value */
	*retlen = 0;

	/* Select the NAND device */
	NAND_ENABLE_CE(nand);  /* set pin low */

	/* Loop until all data read */
	while (*retlen < len) {

#ifdef CONFIG_MTD_NAND_ECC
		/* Do we have this page in cache ? */
		if (nand->cache_page == page)
			goto readdata;
		/* Send the read command */
		NanD_Command(nand, NAND_CMD_READ0);
		if (nand->bus16) {
			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + (col >> 1));
		} else {
			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + col);
		}

		/* Read in a page + oob data */
		NanD_ReadBuf(nand, nand->data_buf, nand->oobblock + nand->oobsize);

		/* copy data into cache, for read out of cache and if ecc fails */
		if (nand->data_cache) {
			memcpy (nand->data_cache, nand->data_buf,
				nand->oobblock + nand->oobsize);
		}

		/* Pick the ECC bytes out of the oob data */
		for (j = 0; j < 6; j++) {
			ecc_code[j] = nand->data_buf[(nand->oobblock + oob_config.ecc_pos[j])];
		}

		/* Calculate the ECC and verify it */
		/* If block was not written with ECC, skip ECC */
		if (oob_config.eccvalid_pos != -1 &&
		    (nand->data_buf[nand->oobblock + oob_config.eccvalid_pos] & 0x0f) != 0x0f) {

			nand_calculate_ecc (&nand->data_buf[0], &ecc_calc[0]);
			switch (nand_correct_data (&nand->data_buf[0], &ecc_code[0], &ecc_calc[0])) {
			case -1:
				printf ("%s: Failed ECC read, page 0x%08x\n", __FUNCTION__, page);
				ecc_failed++;
				break;
			case 1:
			case 2:	/* transfer ECC corrected data to cache */
				if (nand->data_cache)
					memcpy (nand->data_cache, nand->data_buf, 256);
				break;
			}
		}

		if (oob_config.eccvalid_pos != -1 &&
		    nand->oobblock == 512 && (nand->data_buf[nand->oobblock + oob_config.eccvalid_pos] & 0xf0) != 0xf0) {

			nand_calculate_ecc (&nand->data_buf[256], &ecc_calc[3]);
			switch (nand_correct_data (&nand->data_buf[256], &ecc_code[3], &ecc_calc[3])) {
			case -1:
				printf ("%s: Failed ECC read, page 0x%08x\n", __FUNCTION__, page);
				ecc_failed++;
				break;
			case 1:
			case 2:	/* transfer ECC corrected data to cache */
				if (nand->data_cache)
					memcpy (&nand->data_cache[256], &nand->data_buf[256], 256);
				break;
			}
		}
readdata:
		/* Read the data from ECC data buffer into return buffer */
		data_poi = (nand->data_cache) ? nand->data_cache : nand->data_buf;
		data_poi += col;
		if ((*retlen + (nand->oobblock - col)) >= len) {
			memcpy (buf + *retlen, data_poi, len - *retlen);
			*retlen = len;
		} else {
			memcpy (buf + *retlen, data_poi,  nand->oobblock - col);
			*retlen += nand->oobblock - col;
		}
		/* Set cache page address, invalidate, if ecc_failed */
		nand->cache_page = (nand->data_cache && !ecc_failed) ? page : -1;

		ecc_status += ecc_failed;
		ecc_failed = 0;

#else
		/* Send the read command */
		NanD_Command(nand, NAND_CMD_READ0);
		if (nand->bus16) {
			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + (col >> 1));
		} else {
			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + col);
		}

		/* Read the data directly into the return buffer */
		if ((*retlen + (nand->oobblock - col)) >= len) {
			NanD_ReadBuf(nand, buf + *retlen, len - *retlen);
			*retlen = len;
			/* We're done */
			continue;
		} else {
			NanD_ReadBuf(nand, buf + *retlen, nand->oobblock - col);
			*retlen += nand->oobblock - col;
			}
#endif
		/* For subsequent reads align to page boundary. */
		col = 0;
		/* Increment page address */
		page++;
	}

	/* De-select the NAND device */
	NAND_DISABLE_CE(nand);  /* set pin high */

	/*
	 * Return success, if no ECC failures, else -EIO
	 * fs driver will take care of that, because
	 * retlen == desired len and result == -EIO
	 */
	return ecc_status ? -1 : 0;
}

/*
 *	Nand_page_program function is used for write and writev !
 */
static int nand_write_page (struct nand_chip *nand,
			    int page, int col, int last, u_char * ecc_code)
{

	int i;
	unsigned long nandptr = nand->IO_ADDR;

#ifdef CONFIG_MTD_NAND_ECC
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	int ecc_bytes = (nand->oobblock == 512) ? 6 : 3;
#endif
#endif
	/* pad oob area */
	for (i = nand->oobblock; i < nand->oobblock + nand->oobsize; i++)
		nand->data_buf[i] = 0xff;

#ifdef CONFIG_MTD_NAND_ECC
	/* Zero out the ECC array */
	for (i = 0; i < 6; i++)
		ecc_code[i] = 0x00;

	/* Read back previous written data, if col > 0 */
	if (col) {
		NanD_Command (nand, NAND_CMD_READ0);
		if (nand->bus16) {
			NanD_Address (nand, ADDR_COLUMN_PAGE,
				      (page << nand->page_shift) + (col >> 1));
		} else {
			NanD_Address (nand, ADDR_COLUMN_PAGE,
				      (page << nand->page_shift) + col);
		}

		if (nand->bus16) {
			u16 val;

			for (i = 0; i < col; i += 2) {
				val = READ_NAND (nandptr);
				nand->data_buf[i] = val & 0xff;
				nand->data_buf[i + 1] = val >> 8;
			}
		} else {
			for (i = 0; i < col; i++)
				nand->data_buf[i] = READ_NAND (nandptr);
		}
	}

	/* Calculate and write the ECC if we have enough data */
	if ((col < nand->eccsize) && (last >= nand->eccsize)) {
		nand_calculate_ecc (&nand->data_buf[0], &(ecc_code[0]));
		for (i = 0; i < 3; i++) {
			nand->data_buf[(nand->oobblock +
					oob_config.ecc_pos[i])] = ecc_code[i];
		}
		if (oob_config.eccvalid_pos != -1) {
			nand->data_buf[nand->oobblock +
				       oob_config.eccvalid_pos] = 0xf0;
		}
	}

	/* Calculate and write the second ECC if we have enough data */
	if ((nand->oobblock == 512) && (last == nand->oobblock)) {
		nand_calculate_ecc (&nand->data_buf[256], &(ecc_code[3]));
		for (i = 3; i < 6; i++) {
			nand->data_buf[(nand->oobblock +
					oob_config.ecc_pos[i])] = ecc_code[i];
		}
		if (oob_config.eccvalid_pos != -1) {
			nand->data_buf[nand->oobblock +
				       oob_config.eccvalid_pos] &= 0x0f;
		}
	}
#endif
	/* Prepad for partial page programming !!! */
	for (i = 0; i < col; i++)
		nand->data_buf[i] = 0xff;

	/* Postpad for partial page programming !!! oob is already padded */
	for (i = last; i < nand->oobblock; i++)
		nand->data_buf[i] = 0xff;

	/* Send command to begin auto page programming */
	NanD_Command (nand, NAND_CMD_READ0);
	NanD_Command (nand, NAND_CMD_SEQIN);
	if (nand->bus16) {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + (col >> 1));
	} else {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + col);
	}

	/* Write out complete page of data */
	if (nand->bus16) {
		for (i = 0; i < (nand->oobblock + nand->oobsize); i += 2) {
			WRITE_NAND (nand->data_buf[i] +
				    (nand->data_buf[i + 1] << 8),
				    nand->IO_ADDR);
		}
	} else {
		for (i = 0; i < (nand->oobblock + nand->oobsize); i++)
			WRITE_NAND (nand->data_buf[i], nand->IO_ADDR);
	}

	/* Send command to actually program the data */
	NanD_Command (nand, NAND_CMD_PAGEPROG);
	NanD_Command (nand, NAND_CMD_STATUS);
#ifdef NAND_NO_RB
	{
		u_char ret_val;

		do {
			ret_val = READ_NAND (nandptr);	/* wait till ready */
		} while ((ret_val & 0x40) != 0x40);
	}
#endif
	/* See if device thinks it succeeded */
	if (READ_NAND (nand->IO_ADDR) & 0x01) {
		printf ("%s: Failed write, page 0x%08x, ", __FUNCTION__,
			page);
		return -1;
	}
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/*
	 * The NAND device assumes that it is always writing to
	 * a cleanly erased page. Hence, it performs its internal
	 * write verification only on bits that transitioned from
	 * 1 to 0. The device does NOT verify the whole page on a
	 * byte by byte basis. It is possible that the page was
	 * not completely erased or the page is becoming unusable
	 * due to wear. The read with ECC would catch the error
	 * later when the ECC page check fails, but we would rather
	 * catch it early in the page write stage. Better to write
	 * no data than invalid data.
	 */

	/* Send command to read back the page */
	if (col < nand->eccsize)
		NanD_Command (nand, NAND_CMD_READ0);
	else
		NanD_Command (nand, NAND_CMD_READ1);
	if (nand->bus16) {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + (col >> 1));
	} else {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + col);
	}

	/* Loop through and verify the data */
	if (nand->bus16) {
		for (i = col; i < last; i = +2) {
			if ((nand->data_buf[i] +
			     (nand->data_buf[i + 1] << 8)) != READ_NAND (nand->IO_ADDR)) {
				printf ("%s: Failed write verify, page 0x%08x ",
					__FUNCTION__, page);
				return -1;
			}
		}
	} else {
		for (i = col; i < last; i++) {
			if (nand->data_buf[i] != READ_NAND (nand->IO_ADDR)) {
				printf ("%s: Failed write verify, page 0x%08x ",
					__FUNCTION__, page);
				return -1;
			}
		}
	}

#ifdef CONFIG_MTD_NAND_ECC
	/*
	 * We also want to check that the ECC bytes wrote
	 * correctly for the same reasons stated above.
	 */
	NanD_Command (nand, NAND_CMD_READOOB);
	if (nand->bus16) {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + (col >> 1));
	} else {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + col);
	}
	if (nand->bus16) {
		for (i = 0; i < nand->oobsize; i += 2) {
			u16 val;

			val = READ_NAND (nand->IO_ADDR);
			nand->data_buf[i] = val & 0xff;
			nand->data_buf[i + 1] = val >> 8;
		}
	} else {
		for (i = 0; i < nand->oobsize; i++) {
			nand->data_buf[i] = READ_NAND (nand->IO_ADDR);
		}
	}
	for (i = 0; i < ecc_bytes; i++) {
		if ((nand->data_buf[(oob_config.ecc_pos[i])] != ecc_code[i]) && ecc_code[i]) {
			printf ("%s: Failed ECC write "
				"verify, page 0x%08x, "
				"%6i bytes were succesful\n",
				__FUNCTION__, page, i);
			return -1;
		}
	}
#endif	/* CONFIG_MTD_NAND_ECC */
#endif	/* CONFIG_MTD_NAND_VERIFY_WRITE */
	return 0;
}

static int nand_write_ecc (struct nand_chip* nand, size_t to, size_t len,
			   size_t * retlen, const u_char * buf, u_char * ecc_code)
{
	int i, page, col, cnt, ret = 0;

	/* Do not allow write past end of device */
	if ((to + len) > nand->totlen) {
		printf ("%s: Attempt to write past end of page\n", __FUNCTION__);
		return -1;
	}

	/* Shift to get page */
	page = ((int) to) >> nand->page_shift;

	/* Get the starting column */
	col = to & (nand->oobblock - 1);

	/* Initialize return length value */
	*retlen = 0;

	/* Select the NAND device */
#ifdef CONFIG_OMAP1510
	archflashwp(0,0);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_OFF();
#endif

	NAND_ENABLE_CE(nand);  /* set pin low */

	/* Check the WP bit */
	NanD_Command(nand, NAND_CMD_STATUS);
	if (!(READ_NAND(nand->IO_ADDR) & 0x80)) {
		printf ("%s: Device is write protected!!!\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	/* Loop until all data is written */
	while (*retlen < len) {
		/* Invalidate cache, if we write to this page */
		if (nand->cache_page == page)
			nand->cache_page = -1;

		/* Write data into buffer */
		if ((col + len) >= nand->oobblock) {
			for (i = col, cnt = 0; i < nand->oobblock; i++, cnt++) {
				nand->data_buf[i] = buf[(*retlen + cnt)];
			}
		} else {
			for (i = col, cnt = 0; cnt < (len - *retlen); i++, cnt++) {
				nand->data_buf[i] = buf[(*retlen + cnt)];
			}
		}
		/* We use the same function for write and writev !) */
		ret = nand_write_page (nand, page, col, i, ecc_code);
		if (ret)
			goto out;

		/* Next data start at page boundary */
		col = 0;

		/* Update written bytes count */
		*retlen += cnt;

		/* Increment page address */
		page++;
	}

	/* Return happy */
	*retlen = len;

out:
	/* De-select the NAND device */
	NAND_DISABLE_CE(nand);  /* set pin high */
#ifdef CONFIG_OMAP1510
	archflashwp(0,1);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_ON();
#endif

	return ret;
}

/* read from the 16 bytes of oob data that correspond to a 512 byte
 * page or 2 256-byte pages.
 */
int nand_read_oob(struct nand_chip* nand, size_t ofs, size_t len,
			 size_t * retlen, u_char * buf)
{
	int len256 = 0;
	struct Nand *mychip;
	int ret = 0;

	mychip = &nand->chips[ofs >> nand->chipshift];

	/* update address for 2M x 8bit devices. OOB starts on the second */
	/* page to maintain compatibility with nand_read_ecc. */
	if (nand->page256) {
		if (!(ofs & 0x8))
			ofs += 0x100;
		else
			ofs -= 0x8;
	}

	NAND_ENABLE_CE(nand);  /* set pin low */
	NanD_Command(nand, NAND_CMD_READOOB);
	if (nand->bus16) {
		NanD_Address(nand, ADDR_COLUMN_PAGE,
			     ((ofs >> nand->page_shift) << nand->page_shift) +
				((ofs & (nand->oobblock - 1)) >> 1));
	} else {
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs);
	}

	/* treat crossing 8-byte OOB data for 2M x 8bit devices */
	/* Note: datasheet says it should automaticaly wrap to the */
	/*       next OOB block, but it didn't work here. mf.      */
	if (nand->page256 && ofs + len > (ofs | 0x7) + 1) {
		len256 = (ofs | 0x7) + 1 - ofs;
		NanD_ReadBuf(nand, buf, len256);

		NanD_Command(nand, NAND_CMD_READOOB);
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs & (~0x1ff));
	}

	NanD_ReadBuf(nand, &buf[len256], len - len256);

	*retlen = len;
	/* Reading the full OOB data drops us off of the end of the page,
	 * causing the flash device to go into busy mode, so we need
	 * to wait until ready 11.4.1 and Toshiba TC58256FT nands */

	ret = NanD_WaitReady(nand, 1);
	NAND_DISABLE_CE(nand);  /* set pin high */

	return ret;

}

/* write to the 16 bytes of oob data that correspond to a 512 byte
 * page or 2 256-byte pages.
 */
int nand_write_oob(struct nand_chip* nand, size_t ofs, size_t len,
		  size_t * retlen, const u_char * buf)
{
	int len256 = 0;
	int i;
	unsigned long nandptr = nand->IO_ADDR;

#ifdef PSYCHO_DEBUG
	printf("nand_write_oob(%lx, %d): %2.2X %2.2X %2.2X %2.2X ... %2.2X %2.2X .. %2.2X %2.2X\n",
	       (long)ofs, len, buf[0], buf[1], buf[2], buf[3],
	       buf[8], buf[9], buf[14],buf[15]);
#endif

	NAND_ENABLE_CE(nand);  /* set pin low to enable chip */

	/* Reset the chip */
	NanD_Command(nand, NAND_CMD_RESET);

	/* issue the Read2 command to set the pointer to the Spare Data Area. */
	NanD_Command(nand, NAND_CMD_READOOB);
	if (nand->bus16) {
		NanD_Address(nand, ADDR_COLUMN_PAGE,
			     ((ofs >> nand->page_shift) << nand->page_shift) +
				((ofs & (nand->oobblock - 1)) >> 1));
	} else {
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs);
	}

	/* update address for 2M x 8bit devices. OOB starts on the second */
	/* page to maintain compatibility with nand_read_ecc. */
	if (nand->page256) {
		if (!(ofs & 0x8))
			ofs += 0x100;
		else
			ofs -= 0x8;
	}

	/* issue the Serial Data In command to initial the Page Program process */
	NanD_Command(nand, NAND_CMD_SEQIN);
	if (nand->bus16) {
		NanD_Address(nand, ADDR_COLUMN_PAGE,
			     ((ofs >> nand->page_shift) << nand->page_shift) +
				((ofs & (nand->oobblock - 1)) >> 1));
	} else {
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs);
	}

	/* treat crossing 8-byte OOB data for 2M x 8bit devices */
	/* Note: datasheet says it should automaticaly wrap to the */
	/*       next OOB block, but it didn't work here. mf.      */
	if (nand->page256 && ofs + len > (ofs | 0x7) + 1) {
		len256 = (ofs | 0x7) + 1 - ofs;
		for (i = 0; i < len256; i++)
			WRITE_NAND(buf[i], nandptr);

		NanD_Command(nand, NAND_CMD_PAGEPROG);
		NanD_Command(nand, NAND_CMD_STATUS);
#ifdef NAND_NO_RB
		{ u_char ret_val;
			do {
				ret_val = READ_NAND(nandptr); /* wait till ready */
			} while ((ret_val & 0x40) != 0x40);
		}
#endif
		if (READ_NAND(nandptr) & 1) {
			puts ("Error programming oob data\n");
			/* There was an error */
			NAND_DISABLE_CE(nand);  /* set pin high */
			*retlen = 0;
			return -1;
		}
		NanD_Command(nand, NAND_CMD_SEQIN);
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs & (~0x1ff));
	}

	if (nand->bus16) {
		for (i = len256; i < len; i += 2) {
			WRITE_NAND(buf[i] + (buf[i+1] << 8), nandptr);
		}
	} else {
		for (i = len256; i < len; i++)
			WRITE_NAND(buf[i], nandptr);
	}

	NanD_Command(nand, NAND_CMD_PAGEPROG);
	NanD_Command(nand, NAND_CMD_STATUS);
#ifdef NAND_NO_RB
	{	u_char ret_val;
		do {
			ret_val = READ_NAND(nandptr); /* wait till ready */
		} while ((ret_val & 0x40) != 0x40);
	}
#endif
	if (READ_NAND(nandptr) & 1) {
		puts ("Error programming oob data\n");
		/* There was an error */
		NAND_DISABLE_CE(nand);  /* set pin high */
		*retlen = 0;
		return -1;
	}

	NAND_DISABLE_CE(nand);  /* set pin high */
	*retlen = len;
	return 0;

}

int nand_legacy_erase(struct nand_chip* nand, size_t ofs, size_t len, int clean)
{
	/* This is defined as a structure so it will work on any system
	 * using native endian jffs2 (the default).
	 */
	static struct jffs2_unknown_node clean_marker = {
		JFFS2_MAGIC_BITMASK,
		JFFS2_NODETYPE_CLEANMARKER,
		8		/* 8 bytes in this node */
	};
	unsigned long nandptr;
	struct Nand *mychip;
	int ret = 0;

	if (ofs & (nand->erasesize-1) || len & (nand->erasesize-1)) {
		printf ("Offset and size must be sector aligned, erasesize = %d\n",
			(int) nand->erasesize);
		return -1;
	}

	nandptr = nand->IO_ADDR;

	/* Select the NAND device */
#ifdef CONFIG_OMAP1510
	archflashwp(0,0);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_OFF();
#endif
    NAND_ENABLE_CE(nand);  /* set pin low */

	/* Check the WP bit */
	NanD_Command(nand, NAND_CMD_STATUS);
	if (!(READ_NAND(nand->IO_ADDR) & 0x80)) {
		printf ("nand_write_ecc: Device is write protected!!!\n");
		ret = -1;
		goto out;
	}

	/* Check the WP bit */
	NanD_Command(nand, NAND_CMD_STATUS);
	if (!(READ_NAND(nand->IO_ADDR) & 0x80)) {
		printf ("%s: Device is write protected!!!\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	/* FIXME: Do nand in the background. Use timers or schedule_task() */
	while(len) {
		/*mychip = &nand->chips[shr(ofs, nand->chipshift)];*/
		mychip = &nand->chips[ofs >> nand->chipshift];

		/* always check for bad block first, genuine bad blocks
		 * should _never_  be erased.
		 */
		if (ALLOW_ERASE_BAD_DEBUG || !check_block(nand, ofs)) {
			/* Select the NAND device */
			NAND_ENABLE_CE(nand);  /* set pin low */

			NanD_Command(nand, NAND_CMD_ERASE1);
			NanD_Address(nand, ADDR_PAGE, ofs);
			NanD_Command(nand, NAND_CMD_ERASE2);

			NanD_Command(nand, NAND_CMD_STATUS);

#ifdef NAND_NO_RB
			{	u_char ret_val;
				do {
					ret_val = READ_NAND(nandptr); /* wait till ready */
				} while ((ret_val & 0x40) != 0x40);
			}
#endif
			if (READ_NAND(nandptr) & 1) {
				printf ("%s: Error erasing at 0x%lx\n",
					__FUNCTION__, (long)ofs);
				/* There was an error */
				ret = -1;
				goto out;
			}
			if (clean) {
				int n;	/* return value not used */
				int p, l;

				/* clean marker position and size depend
				 * on the page size, since 256 byte pages
				 * only have 8 bytes of oob data
				 */
				if (nand->page256) {
					p = NAND_JFFS2_OOB8_FSDAPOS;
					l = NAND_JFFS2_OOB8_FSDALEN;
				} else {
					p = NAND_JFFS2_OOB16_FSDAPOS;
					l = NAND_JFFS2_OOB16_FSDALEN;
				}

				ret = nand_write_oob(nand, ofs + p, l, (size_t *)&n,
						     (u_char *)&clean_marker);
				/* quit here if write failed */
				if (ret)
					goto out;
			}
		}
		ofs += nand->erasesize;
		len -= nand->erasesize;
	}

out:
	/* De-select the NAND device */
	NAND_DISABLE_CE(nand);  /* set pin high */
#ifdef CONFIG_OMAP1510
	archflashwp(0,1);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_ON();
#endif

	return ret;
}


static inline int nandcheck(unsigned long potential, unsigned long physadr)
{
	return 0;
}

unsigned long nand_probe(unsigned long physadr)
{
	struct nand_chip *nand = NULL;
	int i = 0, ChipID = 1;

#ifdef CONFIG_MTD_NAND_ECC_JFFS2
	oob_config.ecc_pos[0] = NAND_JFFS2_OOB_ECCPOS0;
	oob_config.ecc_pos[1] = NAND_JFFS2_OOB_ECCPOS1;
	oob_config.ecc_pos[2] = NAND_JFFS2_OOB_ECCPOS2;
	oob_config.ecc_pos[3] = NAND_JFFS2_OOB_ECCPOS3;
	oob_config.ecc_pos[4] = NAND_JFFS2_OOB_ECCPOS4;
	oob_config.ecc_pos[5] = NAND_JFFS2_OOB_ECCPOS5;
	oob_config.eccvalid_pos = 4;
#else
	oob_config.ecc_pos[0] = NAND_NOOB_ECCPOS0;
	oob_config.ecc_pos[1] = NAND_NOOB_ECCPOS1;
	oob_config.ecc_pos[2] = NAND_NOOB_ECCPOS2;
	oob_config.ecc_pos[3] = NAND_NOOB_ECCPOS3;
	oob_config.ecc_pos[4] = NAND_NOOB_ECCPOS4;
	oob_config.ecc_pos[5] = NAND_NOOB_ECCPOS5;
	oob_config.eccvalid_pos = NAND_NOOB_ECCVPOS;
#endif
	oob_config.badblock_pos = 5;

	for (i=0; i<CFG_MAX_NAND_DEVICE; i++) {
		if (nand_dev_desc[i].ChipID == NAND_ChipID_UNKNOWN) {
			nand = &nand_dev_desc[i];
			break;
		}
	}
	if (!nand)
		return (0);

	memset((char *)nand, 0, sizeof(struct nand_chip));

	nand->IO_ADDR = physadr;
	nand->cache_page = -1;  /* init the cache page */
	NanD_ScanChips(nand);

	if (nand->totlen == 0) {
		/* no chips found, clean up and quit */
		memset((char *)nand, 0, sizeof(struct nand_chip));
		nand->ChipID = NAND_ChipID_UNKNOWN;
		return (0);
	}

	nand->ChipID = ChipID;
	if (curr_device == -1)
		curr_device = i;

	nand->data_buf = malloc (nand->oobblock + nand->oobsize);
	if (!nand->data_buf) {
		puts ("Cannot allocate memory for data structures.\n");
		return (0);
	}

	return (nand->totlen);
}

#ifdef CONFIG_MTD_NAND_ECC
/*
 * Pre-calculated 256-way 1 byte column parity
 */
static const u_char nand_ecc_precalc_table[] = {
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a,
	0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f,
	0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c,
	0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59,
	0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33,
	0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56,
	0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55,
	0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30,
	0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30,
	0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55,
	0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56,
	0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33,
	0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59,
	0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c,
	0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f,
	0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a,
	0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};


/*
 * Creates non-inverted ECC code from line parity
 */
static void nand_trans_result(u_char reg2, u_char reg3,
	u_char *ecc_code)
{
	u_char a, b, i, tmp1, tmp2;

	/* Initialize variables */
	a = b = 0x80;
	tmp1 = tmp2 = 0;

	/* Calculate first ECC byte */
	for (i = 0; i < 4; i++) {
		if (reg3 & a)		/* LP15,13,11,9 --> ecc_code[0] */
			tmp1 |= b;
		b >>= 1;
		if (reg2 & a)		/* LP14,12,10,8 --> ecc_code[0] */
			tmp1 |= b;
		b >>= 1;
		a >>= 1;
	}

	/* Calculate second ECC byte */
	b = 0x80;
	for (i = 0; i < 4; i++) {
		if (reg3 & a)		/* LP7,5,3,1 --> ecc_code[1] */
			tmp2 |= b;
		b >>= 1;
		if (reg2 & a)		/* LP6,4,2,0 --> ecc_code[1] */
			tmp2 |= b;
		b >>= 1;
		a >>= 1;
	}

	/* Store two of the ECC bytes */
	ecc_code[0] = tmp1;
	ecc_code[1] = tmp2;
}

/*
 * Calculate 3 byte ECC code for 256 byte block
 */
static void nand_calculate_ecc (const u_char *dat, u_char *ecc_code)
{
	u_char idx, reg1, reg3;
	int j;

	/* Initialize variables */
	reg1 = reg3 = 0;
	ecc_code[0] = ecc_code[1] = ecc_code[2] = 0;

	/* Build up column parity */
	for(j = 0; j < 256; j++) {

		/* Get CP0 - CP5 from table */
		idx = nand_ecc_precalc_table[dat[j]];
		reg1 ^= idx;

		/* All bit XOR = 1 ? */
		if (idx & 0x40) {
			reg3 ^= (u_char) j;
		}
	}

	/* Create non-inverted ECC code from line parity */
	nand_trans_result((reg1 & 0x40) ? ~reg3 : reg3, reg3, ecc_code);

	/* Calculate final ECC code */
	ecc_code[0] = ~ecc_code[0];
	ecc_code[1] = ~ecc_code[1];
	ecc_code[2] = ((~reg1) << 2) | 0x03;
}

/*
 * Detect and correct a 1 bit error for 256 byte block
 */
static int nand_correct_data (u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	u_char a, b, c, d1, d2, d3, add, bit, i;

	/* Do error detection */
	d1 = calc_ecc[0] ^ read_ecc[0];
	d2 = calc_ecc[1] ^ read_ecc[1];
	d3 = calc_ecc[2] ^ read_ecc[2];

	if ((d1 | d2 | d3) == 0) {
		/* No errors */
		return 0;
	} else {
		a = (d1 ^ (d1 >> 1)) & 0x55;
		b = (d2 ^ (d2 >> 1)) & 0x55;
		c = (d3 ^ (d3 >> 1)) & 0x54;

		/* Found and will correct single bit error in the data */
		if ((a == 0x55) && (b == 0x55) && (c == 0x54)) {
			c = 0x80;
			add = 0;
			a = 0x80;
			for (i=0; i<4; i++) {
				if (d1 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			c = 0x80;
			for (i=0; i<4; i++) {
				if (d2 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			bit = 0;
			b = 0x04;
			c = 0x80;
			for (i=0; i<3; i++) {
				if (d3 & c)
					bit |= b;
				c >>= 2;
				b >>= 1;
			}
			b = 0x01;
			a = dat[add];
			a ^= (b << bit);
			dat[add] = a;
			return 1;
		}
		else {
			i = 0;
			while (d1) {
				if (d1 & 0x01)
					++i;
				d1 >>= 1;
			}
			while (d2) {
				if (d2 & 0x01)
					++i;
				d2 >>= 1;
			}
			while (d3) {
				if (d3 & 0x01)
					++i;
				d3 >>= 1;
			}
			if (i == 1) {
				/* ECC Code Error Correction */
				read_ecc[0] = calc_ecc[0];
				read_ecc[1] = calc_ecc[1];
				read_ecc[2] = calc_ecc[2];
				return 2;
			}
			else {
				/* Uncorrectable Error */
				return -1;
			}
		}
	}

	/* Should never happen */
	return -1;
}

#endif

#ifdef CONFIG_JFFS2_NAND
int read_jffs2_nand(size_t start, size_t len,
		size_t * retlen, u_char * buf, int nanddev)
{
	return nand_legacy_rw(nand_dev_desc + nanddev, NANDRW_READ | NANDRW_JFFS2,
			start, len, retlen, buf);
}
#endif /* CONFIG_JFFS2_NAND */

#endif
