/*
 * drivers/nand/nand_util.c
 *
 * Copyright (C) 2006 by Weiss-Electronic GmbH.
 * All rights reserved.
 *
 * @author:	Guido Classen <clagix@gmail.com>
 * @descr:	NAND Flash support
 * @references: borrowed heavily from Linux mtd-utils code:
 *		flash_eraseall.c by Arcom Control System Ltd
 *		nandwrite.c by Steven J. Hill (sjhill@realitydiluted.com)
 *			       and Thomas Gleixner (tglx@linutronix.de)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
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
 *
 */

#include <common.h>

#if defined(CONFIG_CMD_NAND) && !defined(CFG_NAND_LEGACY)

#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <div64.h>

#include <nand.h>
#include <jffs2/jffs2.h>

typedef struct erase_info erase_info_t;
typedef struct mtd_info	  mtd_info_t;

/* support only for native endian JFFS2 */
#define cpu_to_je16(x) (x)
#define cpu_to_je32(x) (x)

/*****************************************************************************/
static int nand_block_bad_scrub(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	return 0;
}

/**
 * nand_erase_opts: - erase NAND flash with support for various options
 *		      (jffs2 formating)
 *
 * @param meminfo	NAND device to erase
 * @param opts		options,  @see struct nand_erase_options
 * @return		0 in case of success
 *
 * This code is ported from flash_eraseall.c from Linux mtd utils by
 * Arcom Control System Ltd.
 */
int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts)
{
	struct jffs2_unknown_node cleanmarker;
	int clmpos = 0;
	int clmlen = 8;
	erase_info_t erase;
	ulong erase_length;
	int isNAND;
	int bbtest = 1;
	int result;
	int percent_complete = -1;
	int (*nand_block_bad_old)(struct mtd_info *, loff_t, int) = NULL;
	const char *mtd_device = meminfo->name;

	memset(&erase, 0, sizeof(erase));

	erase.mtd = meminfo;
	erase.len  = meminfo->erasesize;
	erase.addr = opts->offset;
	erase_length = opts->length;

	isNAND = meminfo->type == MTD_NANDFLASH ? 1 : 0;

	if (opts->jffs2) {
		cleanmarker.magic = cpu_to_je16 (JFFS2_MAGIC_BITMASK);
		cleanmarker.nodetype = cpu_to_je16 (JFFS2_NODETYPE_CLEANMARKER);
		if (isNAND) {
			struct nand_oobinfo *oobinfo = &meminfo->oobinfo;

			/* check for autoplacement */
			if (oobinfo->useecc == MTD_NANDECC_AUTOPLACE) {
				/* get the position of the free bytes */
				if (!oobinfo->oobfree[0][1]) {
					printf(" Eeep. Autoplacement selected "
					       "and no empty space in oob\n");
					return -1;
				}
				clmpos = oobinfo->oobfree[0][0];
				clmlen = oobinfo->oobfree[0][1];
				if (clmlen > 8)
					clmlen = 8;
			} else {
				/* legacy mode */
				switch (meminfo->oobsize) {
				case 8:
					clmpos = 6;
					clmlen = 2;
					break;
				case 16:
					clmpos = 8;
					clmlen = 8;
					break;
				case 64:
					clmpos = 16;
					clmlen = 8;
					break;
				}
			}

			cleanmarker.totlen = cpu_to_je32(8);
		} else {
			cleanmarker.totlen =
				cpu_to_je32(sizeof(struct jffs2_unknown_node));
		}
		cleanmarker.hdr_crc =  cpu_to_je32(
			crc32_no_comp(0, (unsigned char *) &cleanmarker,
				      sizeof(struct jffs2_unknown_node) - 4));
	}

	/* scrub option allows to erase badblock. To prevent internal
	 * check from erase() method, set block check method to dummy
	 * and disable bad block table while erasing.
	 */
	if (opts->scrub) {
		struct nand_chip *priv_nand = meminfo->priv;

		nand_block_bad_old = priv_nand->block_bad;
		priv_nand->block_bad = nand_block_bad_scrub;
		/* we don't need the bad block table anymore...
		 * after scrub, there are no bad blocks left!
		 */
		if (priv_nand->bbt) {
			kfree(priv_nand->bbt);
		}
		priv_nand->bbt = NULL;
	}

	for (;
	     erase.addr < opts->offset + erase_length;
	     erase.addr += meminfo->erasesize) {

		WATCHDOG_RESET ();

		if (!opts->scrub && bbtest) {
			int ret = meminfo->block_isbad(meminfo, erase.addr);
			if (ret > 0) {
				if (!opts->quiet)
					printf("\rSkipping bad block at  "
					       "0x%08x                   "
					       "                         \n",
					       erase.addr);
				continue;

			} else if (ret < 0) {
				printf("\n%s: MTD get bad block failed: %d\n",
				       mtd_device,
				       ret);
				return -1;
			}
		}

		result = meminfo->erase(meminfo, &erase);
		if (result != 0) {
			printf("\n%s: MTD Erase failure: %d\n",
			       mtd_device, result);
			continue;
		}

		/* format for JFFS2 ? */
		if (opts->jffs2) {

			/* write cleanmarker */
			if (isNAND) {
				size_t written;
				result = meminfo->write_oob(meminfo,
							    erase.addr + clmpos,
							    clmlen,
							    &written,
							    (unsigned char *)
							    &cleanmarker);
				if (result != 0) {
					printf("\n%s: MTD writeoob failure: %d\n",
					       mtd_device, result);
					continue;
				}
			} else {
				printf("\n%s: this erase routine only supports"
				       " NAND devices!\n",
				       mtd_device);
			}
		}

		if (!opts->quiet) {
                        unsigned long long n =(unsigned long long)
				 (erase.addr+meminfo->erasesize-opts->offset)
				 * 100;
			int percent = (int)do_div(n, erase_length);

			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				percent_complete = percent;

				printf("\rErasing at 0x%x -- %3d%% complete.",
				       erase.addr, percent);

				if (opts->jffs2 && result == 0)
					printf(" Cleanmarker written at 0x%x.",
					       erase.addr);
			}
		}
	}
	if (!opts->quiet)
		printf("\n");

	if (nand_block_bad_old) {
		struct nand_chip *priv_nand = meminfo->priv;

		priv_nand->block_bad = nand_block_bad_old;
		priv_nand->scan_bbt(meminfo);
	}

	return 0;
}

#define MAX_PAGE_SIZE	2048
#define MAX_OOB_SIZE	64

/*
 * buffer array used for writing data
 */
static unsigned char data_buf[MAX_PAGE_SIZE];
static unsigned char oob_buf[MAX_OOB_SIZE];

/* OOB layouts to pass into the kernel as default */
static struct nand_oobinfo none_oobinfo = {
	.useecc = MTD_NANDECC_OFF,
};

static struct nand_oobinfo jffs2_oobinfo = {
	.useecc = MTD_NANDECC_PLACE,
	.eccbytes = 6,
	.eccpos = { 0, 1, 2, 3, 6, 7 }
};

static struct nand_oobinfo yaffs_oobinfo = {
	.useecc = MTD_NANDECC_PLACE,
	.eccbytes = 6,
	.eccpos = { 8, 9, 10, 13, 14, 15}
};

static struct nand_oobinfo autoplace_oobinfo = {
	.useecc = MTD_NANDECC_AUTOPLACE
};

/**
 * nand_write_opts: - write image to NAND flash with support for various options
 *
 * @param meminfo	NAND device to erase
 * @param opts		write options (@see nand_write_options)
 * @return		0 in case of success
 *
 * This code is ported from nandwrite.c from Linux mtd utils by
 * Steven J. Hill and Thomas Gleixner.
 */
int nand_write_opts(nand_info_t *meminfo, const nand_write_options_t *opts)
{
	int imglen = 0;
	int pagelen;
	int baderaseblock;
	int blockstart = -1;
	loff_t offs;
	int readlen;
	int oobinfochanged = 0;
	int percent_complete = -1;
	struct nand_oobinfo old_oobinfo;
	ulong mtdoffset = opts->offset;
	ulong erasesize_blockalign;
	u_char *buffer = opts->buffer;
	size_t written;
	int result;

	if (opts->pad && opts->writeoob) {
		printf("Can't pad when oob data is present.\n");
		return -1;
	}

	/* set erasesize to specified number of blocks - to match
	 * jffs2 (virtual) block size */
	if (opts->blockalign == 0) {
		erasesize_blockalign = meminfo->erasesize;
	} else {
		erasesize_blockalign = meminfo->erasesize * opts->blockalign;
	}

	/* make sure device page sizes are valid */
	if (!(meminfo->oobsize == 16 && meminfo->oobblock == 512)
	    && !(meminfo->oobsize == 8 && meminfo->oobblock == 256)
	    && !(meminfo->oobsize == 64 && meminfo->oobblock == 2048)) {
		printf("Unknown flash (not normal NAND)\n");
		return -1;
	}

	/* read the current oob info */
	memcpy(&old_oobinfo, &meminfo->oobinfo, sizeof(old_oobinfo));

	/* write without ecc? */
	if (opts->noecc) {
		memcpy(&meminfo->oobinfo, &none_oobinfo,
		       sizeof(meminfo->oobinfo));
		oobinfochanged = 1;
	}

	/* autoplace ECC? */
	if (opts->autoplace && (old_oobinfo.useecc != MTD_NANDECC_AUTOPLACE)) {

		memcpy(&meminfo->oobinfo, &autoplace_oobinfo,
		       sizeof(meminfo->oobinfo));
		oobinfochanged = 1;
	}

	/* force OOB layout for jffs2 or yaffs? */
	if (opts->forcejffs2 || opts->forceyaffs) {
		struct nand_oobinfo *oobsel =
			opts->forcejffs2 ? &jffs2_oobinfo : &yaffs_oobinfo;

		if (meminfo->oobsize == 8) {
			if (opts->forceyaffs) {
				printf("YAFSS cannot operate on "
				       "256 Byte page size\n");
				goto restoreoob;
			}
			/* Adjust number of ecc bytes */
			jffs2_oobinfo.eccbytes = 3;
		}

		memcpy(&meminfo->oobinfo, oobsel, sizeof(meminfo->oobinfo));
	}

	/* get image length */
	imglen = opts->length;
	pagelen = meminfo->oobblock
		+ ((opts->writeoob != 0) ? meminfo->oobsize : 0);

	/* check, if file is pagealigned */
	if ((!opts->pad) && ((imglen % pagelen) != 0)) {
		printf("Input block length is not page aligned\n");
		goto restoreoob;
	}

	/* check, if length fits into device */
	if (((imglen / pagelen) * meminfo->oobblock)
	     > (meminfo->size - opts->offset)) {
		printf("Image %d bytes, NAND page %d bytes, "
		       "OOB area %u bytes, device size %u bytes\n",
		       imglen, pagelen, meminfo->oobblock, meminfo->size);
		printf("Input block does not fit into device\n");
		goto restoreoob;
	}

	if (!opts->quiet)
		printf("\n");

	/* get data from input and write to the device */
	while (imglen && (mtdoffset < meminfo->size)) {

		WATCHDOG_RESET ();

		/*
		 * new eraseblock, check for bad block(s). Stay in the
		 * loop to be sure if the offset changes because of
		 * a bad block, that the next block that will be
		 * written to is also checked. Thus avoiding errors if
		 * the block(s) after the skipped block(s) is also bad
		 * (number of blocks depending on the blockalign
		 */
		while (blockstart != (mtdoffset & (~erasesize_blockalign+1))) {
			blockstart = mtdoffset & (~erasesize_blockalign+1);
			offs = blockstart;
			baderaseblock = 0;

			/* check all the blocks in an erase block for
			 * bad blocks */
			do {
				int ret = meminfo->block_isbad(meminfo, offs);

				if (ret < 0) {
					printf("Bad block check failed\n");
					goto restoreoob;
				}
				if (ret == 1) {
					baderaseblock = 1;
					if (!opts->quiet)
						printf("\rBad block at 0x%lx "
						       "in erase block from "
						       "0x%x will be skipped\n",
						       (long) offs,
						       blockstart);
				}

				if (baderaseblock) {
					mtdoffset = blockstart
						+ erasesize_blockalign;
				}
				offs +=	 erasesize_blockalign
					/ opts->blockalign;
			} while (offs < blockstart + erasesize_blockalign);
		}

		readlen = meminfo->oobblock;
		if (opts->pad && (imglen < readlen)) {
			readlen = imglen;
			memset(data_buf + readlen, 0xff,
			       meminfo->oobblock - readlen);
		}

		/* read page data from input memory buffer */
		memcpy(data_buf, buffer, readlen);
		buffer += readlen;

		if (opts->writeoob) {
			/* read OOB data from input memory block, exit
			 * on failure */
			memcpy(oob_buf, buffer, meminfo->oobsize);
			buffer += meminfo->oobsize;

			/* write OOB data first, as ecc will be placed
			 * in there*/
			result = meminfo->write_oob(meminfo,
						    mtdoffset,
						    meminfo->oobsize,
						    &written,
						    (unsigned char *)
						    &oob_buf);

			if (result != 0) {
				printf("\nMTD writeoob failure: %d\n",
				       result);
				goto restoreoob;
			}
			imglen -= meminfo->oobsize;
		}

		/* write out the page data */
		result = meminfo->write(meminfo,
					mtdoffset,
					meminfo->oobblock,
					&written,
					(unsigned char *) &data_buf);

		if (result != 0) {
			printf("writing NAND page at offset 0x%lx failed\n",
			       mtdoffset);
			goto restoreoob;
		}
		imglen -= readlen;

		if (!opts->quiet) {
                        unsigned long long n = (unsigned long long)
			         (opts->length-imglen) * 100;
			int percent = (int)do_div(n, opts->length);
			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				printf("\rWriting data at 0x%x "
				       "-- %3d%% complete.",
				       mtdoffset, percent);
				percent_complete = percent;
			}
		}

		mtdoffset += meminfo->oobblock;
	}

	if (!opts->quiet)
		printf("\n");

restoreoob:
	if (oobinfochanged) {
		memcpy(&meminfo->oobinfo, &old_oobinfo,
		       sizeof(meminfo->oobinfo));
	}

	if (imglen > 0) {
		printf("Data did not fit into device, due to bad blocks\n");
		return -1;
	}

	/* return happy */
	return 0;
}

/**
 * nand_read_opts: - read image from NAND flash with support for various options
 *
 * @param meminfo	NAND device to erase
 * @param opts		read options (@see struct nand_read_options)
 * @return		0 in case of success
 *
 */
int nand_read_opts(nand_info_t *meminfo, const nand_read_options_t *opts)
{
	int imglen = opts->length;
	int pagelen;
	int baderaseblock;
	int blockstart = -1;
	int percent_complete = -1;
	loff_t offs;
	size_t readlen;
	ulong mtdoffset = opts->offset;
	u_char *buffer = opts->buffer;
	int result;

	/* make sure device page sizes are valid */
	if (!(meminfo->oobsize == 16 && meminfo->oobblock == 512)
	    && !(meminfo->oobsize == 8 && meminfo->oobblock == 256)
	    && !(meminfo->oobsize == 64 && meminfo->oobblock == 2048)) {
		printf("Unknown flash (not normal NAND)\n");
		return -1;
	}

	pagelen = meminfo->oobblock
		+ ((opts->readoob != 0) ? meminfo->oobsize : 0);

	/* check, if length is not larger than device */
	if (((imglen / pagelen) * meminfo->oobblock)
	     > (meminfo->size - opts->offset)) {
		printf("Image %d bytes, NAND page %d bytes, "
		       "OOB area %u bytes, device size %u bytes\n",
		       imglen, pagelen, meminfo->oobblock, meminfo->size);
		printf("Input block is larger than device\n");
		return -1;
	}

	if (!opts->quiet)
		printf("\n");

	/* get data from input and write to the device */
	while (imglen && (mtdoffset < meminfo->size)) {

		WATCHDOG_RESET ();

		/*
		 * new eraseblock, check for bad block(s). Stay in the
		 * loop to be sure if the offset changes because of
		 * a bad block, that the next block that will be
		 * written to is also checked. Thus avoiding errors if
		 * the block(s) after the skipped block(s) is also bad
		 * (number of blocks depending on the blockalign
		 */
		while (blockstart != (mtdoffset & (~meminfo->erasesize+1))) {
			blockstart = mtdoffset & (~meminfo->erasesize+1);
			offs = blockstart;
			baderaseblock = 0;

			/* check all the blocks in an erase block for
			 * bad blocks */
			do {
				int ret = meminfo->block_isbad(meminfo, offs);

				if (ret < 0) {
					printf("Bad block check failed\n");
					return -1;
				}
				if (ret == 1) {
					baderaseblock = 1;
					if (!opts->quiet)
						printf("\rBad block at 0x%lx "
						       "in erase block from "
						       "0x%x will be skipped\n",
						       (long) offs,
						       blockstart);
				}

				if (baderaseblock) {
					mtdoffset = blockstart
						+ meminfo->erasesize;
				}
				offs +=	 meminfo->erasesize;

			} while (offs < blockstart + meminfo->erasesize);
		}


		/* read page data to memory buffer */
		result = meminfo->read(meminfo,
				       mtdoffset,
				       meminfo->oobblock,
				       &readlen,
				       (unsigned char *) &data_buf);

		if (result != 0) {
			printf("reading NAND page at offset 0x%lx failed\n",
			       mtdoffset);
			return -1;
		}

		if (imglen < readlen) {
			readlen = imglen;
		}

		memcpy(buffer, data_buf, readlen);
		buffer += readlen;
		imglen -= readlen;

		if (opts->readoob) {
			result = meminfo->read_oob(meminfo,
						   mtdoffset,
						   meminfo->oobsize,
						   &readlen,
						   (unsigned char *)
						   &oob_buf);

			if (result != 0) {
				printf("\nMTD readoob failure: %d\n",
				       result);
				return -1;
			}


			if (imglen < readlen) {
				readlen = imglen;
			}

			memcpy(buffer, oob_buf, readlen);

			buffer += readlen;
			imglen -= readlen;
		}

		if (!opts->quiet) {
                        unsigned long long n = (unsigned long long)
			         (opts->length-imglen) * 100;
			int percent = (int)do_div(n ,opts->length);
			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
			if (!opts->quiet)
				printf("\rReading data from 0x%x "
				       "-- %3d%% complete.",
				       mtdoffset, percent);
				percent_complete = percent;
			}
		}

		mtdoffset += meminfo->oobblock;
	}

	if (!opts->quiet)
		printf("\n");

	if (imglen > 0) {
		printf("Could not read entire image due to bad blocks\n");
		return -1;
	}

	/* return happy */
	return 0;
}

/******************************************************************************
 * Support for locking / unlocking operations of some NAND devices
 *****************************************************************************/

#define NAND_CMD_LOCK		0x2a
#define NAND_CMD_LOCK_TIGHT	0x2c
#define NAND_CMD_UNLOCK1	0x23
#define NAND_CMD_UNLOCK2	0x24
#define NAND_CMD_LOCK_STATUS	0x7a

/**
 * nand_lock: Set all pages of NAND flash chip to the LOCK or LOCK-TIGHT
 *	      state
 *
 * @param meminfo	nand mtd instance
 * @param tight		bring device in lock tight mode
 *
 * @return		0 on success, -1 in case of error
 *
 * The lock / lock-tight command only applies to the whole chip. To get some
 * parts of the chip lock and others unlocked use the following sequence:
 *
 * - Lock all pages of the chip using nand_lock(mtd, 0) (or the lockpre pin)
 * - Call nand_unlock() once for each consecutive area to be unlocked
 * - If desired: Bring the chip to the lock-tight state using nand_lock(mtd, 1)
 *
 *   If the device is in lock-tight state software can't change the
 *   current active lock/unlock state of all pages. nand_lock() / nand_unlock()
 *   calls will fail. It is only posible to leave lock-tight state by
 *   an hardware signal (low pulse on _WP pin) or by power down.
 */
int nand_lock(nand_info_t *meminfo, int tight)
{
	int ret = 0;
	int status;
	struct nand_chip *this = meminfo->priv;

	/* select the NAND device */
	this->select_chip(meminfo, 0);

	this->cmdfunc(meminfo,
		      (tight ? NAND_CMD_LOCK_TIGHT : NAND_CMD_LOCK),
		      -1, -1);

	/* call wait ready function */
	status = this->waitfunc(meminfo, this, FL_WRITING);

	/* see if device thinks it succeeded */
	if (status & 0x01) {
		ret = -1;
	}

	/* de-select the NAND device */
	this->select_chip(meminfo, -1);
	return ret;
}

/**
 * nand_get_lock_status: - query current lock state from one page of NAND
 *			   flash
 *
 * @param meminfo	nand mtd instance
 * @param offset	page address to query (muss be page aligned!)
 *
 * @return		-1 in case of error
 *			>0 lock status:
 *			  bitfield with the following combinations:
 *			  NAND_LOCK_STATUS_TIGHT: page in tight state
 *			  NAND_LOCK_STATUS_LOCK:  page locked
 *			  NAND_LOCK_STATUS_UNLOCK: page unlocked
 *
 */
int nand_get_lock_status(nand_info_t *meminfo, ulong offset)
{
	int ret = 0;
	int chipnr;
	int page;
	struct nand_chip *this = meminfo->priv;

	/* select the NAND device */
	chipnr = (int)(offset >> this->chip_shift);
	this->select_chip(meminfo, chipnr);


	if ((offset & (meminfo->oobblock - 1)) != 0) {
		printf ("nand_get_lock_status: "
			"Start address must be beginning of "
			"nand page!\n");
		ret = -1;
		goto out;
	}

	/* check the Lock Status */
	page = (int)(offset >> this->page_shift);
	this->cmdfunc(meminfo, NAND_CMD_LOCK_STATUS, -1, page & this->pagemask);

	ret = this->read_byte(meminfo) & (NAND_LOCK_STATUS_TIGHT
					  | NAND_LOCK_STATUS_LOCK
					  | NAND_LOCK_STATUS_UNLOCK);

 out:
	/* de-select the NAND device */
	this->select_chip(meminfo, -1);
	return ret;
}

/**
 * nand_unlock: - Unlock area of NAND pages
 *		  only one consecutive area can be unlocked at one time!
 *
 * @param meminfo	nand mtd instance
 * @param start		start byte address
 * @param length	number of bytes to unlock (must be a multiple of
 *			page size nand->oobblock)
 *
 * @return		0 on success, -1 in case of error
 */
int nand_unlock(nand_info_t *meminfo, ulong start, ulong length)
{
	int ret = 0;
	int chipnr;
	int status;
	int page;
	struct nand_chip *this = meminfo->priv;
	printf ("nand_unlock: start: %08x, length: %d!\n",
		(int)start, (int)length);

	/* select the NAND device */
	chipnr = (int)(start >> this->chip_shift);
	this->select_chip(meminfo, chipnr);

	/* check the WP bit */
	this->cmdfunc(meminfo, NAND_CMD_STATUS, -1, -1);
	if ((this->read_byte(meminfo) & 0x80) == 0) {
		printf ("nand_unlock: Device is write protected!\n");
		ret = -1;
		goto out;
	}

	if ((start & (meminfo->oobblock - 1)) != 0) {
		printf ("nand_unlock: Start address must be beginning of "
			"nand page!\n");
		ret = -1;
		goto out;
	}

	if (length == 0 || (length & (meminfo->oobblock - 1)) != 0) {
		printf ("nand_unlock: Length must be a multiple of nand page "
			"size!\n");
		ret = -1;
		goto out;
	}

	/* submit address of first page to unlock */
	page = (int)(start >> this->page_shift);
	this->cmdfunc(meminfo, NAND_CMD_UNLOCK1, -1, page & this->pagemask);

	/* submit ADDRESS of LAST page to unlock */
	page += (int)(length >> this->page_shift) - 1;
	this->cmdfunc(meminfo, NAND_CMD_UNLOCK2, -1, page & this->pagemask);

	/* call wait ready function */
	status = this->waitfunc(meminfo, this, FL_WRITING);
	/* see if device thinks it succeeded */
	if (status & 0x01) {
		/* there was an error */
		ret = -1;
		goto out;
	}

 out:
	/* de-select the NAND device */
	this->select_chip(meminfo, -1);
	return ret;
}

#endif
