/*
 * (C) Copyright 2003-2004
 * Gary Jennejohn, DENX Software Engineering, gj@denx.de.
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>
#include <linux/mtd/nand.h>
#include <fat.h>

#include "auto_update.h"

#ifdef CONFIG_AUTO_UPDATE

#if !(CONFIG_COMMANDS & CFG_CMD_FAT)
#error "must define CFG_CMD_FAT"
#endif

extern au_image_t au_image[];
extern int N_AU_IMAGES;

#define AU_DEBUG
#undef AU_DEBUG

#undef debug
#ifdef	AU_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif	/* AU_DEBUG */


#define LOAD_ADDR ((unsigned char *)0x100000)   /* where to load files into memory */
#define MAX_LOADSZ 0x1e00000

/* externals */
extern int fat_register_device(block_dev_desc_t *, int);
extern int file_fat_detectfs(void);
extern long file_fat_read(const char *, void *, unsigned long);
long do_fat_read (const char *filename, void *buffer, unsigned long maxsize, int dols);
#ifdef CONFIG_VFD
extern int trab_vfd (ulong);
extern int transfer_pic(unsigned char, unsigned char *, int, int);
#endif
extern int flash_sect_erase(ulong, ulong);
extern int flash_sect_protect (int, ulong, ulong);
extern int flash_write (char *, ulong, ulong);
/* change char* to void* to shutup the compiler */
extern block_dev_desc_t *get_dev (char*, int);

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
/* references to names in cmd_nand.c */
#define NANDRW_READ	0x01
#define NANDRW_WRITE	0x00
#define NANDRW_JFFS2	0x02
#define NANDRW_JFFS2_SKIP	0x04
extern struct nand_chip nand_dev_desc[];
extern int nand_rw(struct nand_chip* nand, int cmd, size_t start, size_t len,
		   size_t * retlen, u_char * buf);
extern int nand_erase(struct nand_chip* nand, size_t ofs, size_t len, int clean);
#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */

extern block_dev_desc_t ide_dev_desc[CFG_IDE_MAXDEVICE];


int au_check_cksum_valid(int i, long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;

	hdr = (image_header_t *)LOAD_ADDR;

	if ((au_image[i].type == AU_FIRMWARE) && (au_image[i].size != ntohl(hdr->ih_size))) {
		printf ("Image %s has wrong size\n", au_image[i].name);
		return -1;
	}

	if (nbytes != (sizeof(*hdr) + ntohl(hdr->ih_size))) {
		printf ("Image %s bad total SIZE\n", au_image[i].name);
		return -1;
	}
	/* check the data CRC */
	checksum = ntohl(hdr->ih_dcrc);

	if (crc32 (0, (uchar *)(LOAD_ADDR + sizeof(*hdr)), ntohl(hdr->ih_size))
		!= checksum) {
		printf ("Image %s bad data checksum\n", au_image[i].name);
		return -1;
	}
	return 0;
}


int au_check_header_valid(int i, long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;

	hdr = (image_header_t *)LOAD_ADDR;
	/* check the easy ones first */
#undef CHECK_VALID_DEBUG
#ifdef CHECK_VALID_DEBUG
	printf("magic %#x %#x ", ntohl(hdr->ih_magic), IH_MAGIC);
	printf("arch %#x %#x ", hdr->ih_arch, IH_CPU_PPC);
	printf("size %#x %#lx ", ntohl(hdr->ih_size), nbytes);
	printf("type %#x %#x ", hdr->ih_type, IH_TYPE_KERNEL);
#endif
	if (nbytes < sizeof(*hdr))
	{
		printf ("Image %s bad header SIZE\n", au_image[i].name);
		return -1;
	}
	if (ntohl(hdr->ih_magic) != IH_MAGIC || hdr->ih_arch != IH_CPU_PPC)
	{
		printf ("Image %s bad MAGIC or ARCH\n", au_image[i].name);
		return -1;
	}
	/* check the hdr CRC */
	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;

	if (crc32 (0, (uchar *)hdr, sizeof(*hdr)) != checksum) {
		printf ("Image %s bad header checksum\n", au_image[i].name);
		return -1;
	}
	hdr->ih_hcrc = htonl(checksum);

	/* check the type - could do this all in one gigantic if() */
	if ((au_image[i].type == AU_FIRMWARE) && (hdr->ih_type != IH_TYPE_FIRMWARE)) {
		printf ("Image %s wrong type\n", au_image[i].name);
		return -1;
	}
	if ((au_image[i].type == AU_SCRIPT) && (hdr->ih_type != IH_TYPE_SCRIPT)) {
		printf ("Image %s wrong type\n", au_image[i].name);
		return -1;
	}

	/* recycle checksum */
	checksum = ntohl(hdr->ih_size);

#if 0 /* test-only */
	/* for kernel and app the image header must also fit into flash */
	if (idx != IDX_DISK)
		checksum += sizeof(*hdr);
	/* check the size does not exceed space in flash. HUSH scripts */
	/* all have ausize[] set to 0 */
	if ((ausize[idx] != 0) && (ausize[idx] < checksum)) {
		printf ("Image %s is bigger than FLASH\n", au_image[i].name);
		return -1;
	}
#endif

	return 0;
}


int au_do_update(int i, long sz)
{
	image_header_t *hdr;
	char *addr;
	long start, end;
	int off, rc;
	uint nbytes;
	int k;
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
	int total;
#endif

	hdr = (image_header_t *)LOAD_ADDR;

	switch (au_image[i].type) {
	case AU_SCRIPT:
		printf("Executing script %s\n", au_image[i].name);

		/* execute a script */
		if (hdr->ih_type == IH_TYPE_SCRIPT) {
			addr = (char *)((char *)hdr + sizeof(*hdr));
			/* stick a NULL at the end of the script, otherwise */
			/* parse_string_outer() runs off the end. */
			addr[ntohl(hdr->ih_size)] = 0;
			addr += 8;

			/*
			 * Replace cr/lf with ;
			 */
			k = 0;
			while (addr[k] != 0) {
				if ((addr[k] == 10) || (addr[k] == 13)) {
					addr[k] = ';';
				}
				k++;
			}

			run_command(addr, 0);
			return 0;
		}

		break;

	case AU_FIRMWARE:
	case AU_NOR:
	case AU_NAND:
		start = au_image[i].start;
		end = au_image[i].start + au_image[i].size - 1;

		/*
		 * do not update firmware when image is already in flash.
		 */
		if (au_image[i].type == AU_FIRMWARE) {
			char *orig = (char*)start;
			char *new  = (char *)((char *)hdr + sizeof(*hdr));
			nbytes = ntohl(hdr->ih_size);

			while(--nbytes) {
				if (*orig++ != *new++) {
					break;
				}
			}
			if (!nbytes) {
				printf("Skipping firmware update - images are identical\n");
				break;
			}
		}

		/* unprotect the address range */
		/* this assumes that ONLY the firmware is protected! */
		if (au_image[i].type == AU_FIRMWARE) {
			flash_sect_protect(0, start, end);
		}

		/*
		 * erase the address range.
		 */
		if (au_image[i].type != AU_NAND) {
			printf("Updating NOR FLASH with image %s\n", au_image[i].name);
			debug ("flash_sect_erase(%lx, %lx);\n", start, end);
			flash_sect_erase(start, end);
		} else {
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
			printf("Updating NAND FLASH with image %s\n", au_image[i].name);
			debug ("nand_erase(%lx, %lx);\n", start, end);
			rc = nand_erase (nand_dev_desc, start, end - start + 1, 0);
			debug ("nand_erase returned %x\n", rc);
#endif
		}

		udelay(10000);

		/* strip the header - except for the kernel and ramdisk */
		if (au_image[i].type != AU_FIRMWARE) {
			addr = (char *)hdr;
			off = sizeof(*hdr);
			nbytes = sizeof(*hdr) + ntohl(hdr->ih_size);
		} else {
			addr = (char *)((char *)hdr + sizeof(*hdr));
			off = 0;
			nbytes = ntohl(hdr->ih_size);
		}

		/*
		 * copy the data from RAM to FLASH
		 */
		if (au_image[i].type != AU_NAND) {
			debug ("flash_write(%p, %lx %x)\n", addr, start, nbytes);
			rc = flash_write((uchar *)addr, start, nbytes);
		} else {
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
			debug ("nand_rw(%p, %lx %x)\n", addr, start, nbytes);
			rc = nand_rw(nand_dev_desc, NANDRW_WRITE | NANDRW_JFFS2,
				     start, nbytes, (size_t *)&total, (uchar *)addr);
			debug ("nand_rw: ret=%x total=%d nbytes=%d\n", rc, total, nbytes);
#endif
		}
		if (rc != 0) {
			printf("Flashing failed due to error %d\n", rc);
			return -1;
		}

		/*
		 * check the dcrc of the copy
		 */
		if (au_image[i].type != AU_NAND) {
			rc = crc32 (0, (uchar *)(start + off), ntohl(hdr->ih_size));
		} else {
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
			rc = nand_rw(nand_dev_desc, NANDRW_READ | NANDRW_JFFS2 | NANDRW_JFFS2_SKIP,
				     start, nbytes, (size_t *)&total, (uchar *)addr);
			rc = crc32 (0, (uchar *)(addr + off), ntohl(hdr->ih_size));
#endif
		}
		if (rc != ntohl(hdr->ih_dcrc)) {
			printf ("Image %s Bad Data Checksum After COPY\n", au_image[i].name);
			return -1;
		}

		/* protect the address range */
		/* this assumes that ONLY the firmware is protected! */
		if (au_image[i].type == AU_FIRMWARE) {
			flash_sect_protect(1, start, end);
		}

		break;

	default:
		printf("Wrong image type selected!\n");
	}

	return 0;
}


static void process_macros (const char *input, char *output)
{
	char c, prev;
	const char *varname_start = NULL;
	int inputcnt  = strlen (input);
	int outputcnt = CFG_CBSIZE;
	int state = 0;	/* 0 = waiting for '$'	*/
			/* 1 = waiting for '(' or '{' */
			/* 2 = waiting for ')' or '}' */
			/* 3 = waiting for '''  */
#ifdef DEBUG_PARSER
	char *output_start = output;

	printf ("[PROCESS_MACROS] INPUT len %d: \"%s\"\n", strlen(input), input);
#endif

	prev = '\0';			/* previous character	*/

	while (inputcnt && outputcnt) {
	    c = *input++;
	    inputcnt--;

	    if (state!=3) {
	    /* remove one level of escape characters */
	    if ((c == '\\') && (prev != '\\')) {
		if (inputcnt-- == 0)
			break;
		prev = c;
		c = *input++;
	    }
	    }

	    switch (state) {
	    case 0:			/* Waiting for (unescaped) $	*/
		if ((c == '\'') && (prev != '\\')) {
			state = 3;
			break;
		}
		if ((c == '$') && (prev != '\\')) {
			state++;
		} else {
			*(output++) = c;
			outputcnt--;
		}
		break;
	    case 1:			/* Waiting for (	*/
		if (c == '(' || c == '{') {
			state++;
			varname_start = input;
		} else {
			state = 0;
			*(output++) = '$';
			outputcnt--;

			if (outputcnt) {
				*(output++) = c;
				outputcnt--;
			}
		}
		break;
	    case 2:			/* Waiting for )	*/
		if (c == ')' || c == '}') {
			int i;
			char envname[CFG_CBSIZE], *envval;
			int envcnt = input-varname_start-1; /* Varname # of chars */

			/* Get the varname */
			for (i = 0; i < envcnt; i++) {
				envname[i] = varname_start[i];
			}
			envname[i] = 0;

			/* Get its value */
			envval = getenv (envname);

			/* Copy into the line if it exists */
			if (envval != NULL)
				while ((*envval) && outputcnt) {
					*(output++) = *(envval++);
					outputcnt--;
				}
			/* Look for another '$' */
			state = 0;
		}
		break;
	    case 3:			/* Waiting for '	*/
		if ((c == '\'') && (prev != '\\')) {
			state = 0;
		} else {
			*(output++) = c;
			outputcnt--;
		}
		break;
	    }
	    prev = c;
	}

	if (outputcnt)
		*output = 0;

#ifdef DEBUG_PARSER
	printf ("[PROCESS_MACROS] OUTPUT len %d: \"%s\"\n",
		strlen(output_start), output_start);
#endif
}


/*
 * this is called from board_init() after the hardware has been set up
 * and is usable. That seems like a good time to do this.
 * Right now the return value is ignored.
 */
int do_auto_update(void)
{
	block_dev_desc_t *stor_dev;
	long sz;
	int i, res, cnt, old_ctrlc, got_ctrlc;
	char buffer[32];
	char str[80];

	/*
	 * Check whether a CompactFlash is inserted
	 */
	if (ide_dev_desc[0].type == DEV_TYPE_UNKNOWN) {
		return -1;       /* no disk detected! */
	}

	/* check whether it has a partition table */
	stor_dev = get_dev("ide", 0);
	if (stor_dev == NULL) {
		debug ("Uknown device type\n");
		return -1;
	}
	if (fat_register_device(stor_dev, 1) != 0) {
		debug ("Unable to register ide disk 0:1 for fatls\n");
		return -1;
	}

	/*
	 * Check if magic file is present
	 */
	if (do_fat_read(AU_MAGIC_FILE, buffer, sizeof(buffer), LS_NO) <= 0) {
		return -1;
	}

#ifdef CONFIG_AUTO_UPDATE_SHOW
	board_auto_update_show(1);
#endif
	puts("\nAutoUpdate Disk detected! Trying to update system...\n");

	/* make sure that we see CTRL-C and save the old state */
	old_ctrlc = disable_ctrlc(0);

	/* just loop thru all the possible files */
	for (i = 0; i < N_AU_IMAGES; i++) {
		/*
		 * Try to expand the environment var in the fname
		 */
		process_macros(au_image[i].name, str);
		strcpy(au_image[i].name, str);

		printf("Reading %s ...", au_image[i].name);
		/* just read the header */
		sz = do_fat_read(au_image[i].name, LOAD_ADDR, sizeof(image_header_t), LS_NO);
		debug ("read %s sz %ld hdr %d\n",
			au_image[i].name, sz, sizeof(image_header_t));
		if (sz <= 0 || sz < sizeof(image_header_t)) {
			puts(" not found\n");
			continue;
		}
		if (au_check_header_valid(i, sz) < 0) {
			puts(" header not valid\n");
			continue;
		}
		sz = do_fat_read(au_image[i].name, LOAD_ADDR, MAX_LOADSZ, LS_NO);
		debug ("read %s sz %ld hdr %d\n",
			au_image[i].name, sz, sizeof(image_header_t));
		if (sz <= 0 || sz <= sizeof(image_header_t)) {
			puts(" not found\n");
			continue;
		}
		if (au_check_cksum_valid(i, sz) < 0) {
			puts(" checksum not valid\n");
			continue;
		}
		puts(" done\n");

		do {
			res = au_do_update(i, sz);
			/* let the user break out of the loop */
			if (ctrlc() || had_ctrlc()) {
				clear_ctrlc();
				if (res < 0)
					got_ctrlc = 1;
				break;
			}
			cnt++;
		} while (res < 0);
	}

	/* restore the old state */
	disable_ctrlc(old_ctrlc);

	puts("AutoUpdate finished\n\n");
#ifdef CONFIG_AUTO_UPDATE_SHOW
	board_auto_update_show(0);
#endif

	return 0;
}


int auto_update(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	do_auto_update();

	return 0;
}
U_BOOT_CMD(
	autoupd,	1,	1,	auto_update,
	"autoupd - Automatically update images\n",
	NULL
);
#endif /* CONFIG_AUTO_UPDATE */
