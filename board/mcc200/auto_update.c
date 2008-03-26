/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <malloc.h>
#include <image.h>
#include <asm/byteorder.h>
#include <usb.h>
#include <part.h>

#ifdef CFG_HUSH_PARSER
#include <hush.h>
#endif


#ifdef CONFIG_AUTO_UPDATE

#ifndef CONFIG_USB_OHCI
#error "must define CONFIG_USB_OHCI"
#endif

#ifndef CONFIG_USB_STORAGE
#error "must define CONFIG_USB_STORAGE"
#endif

#ifndef CFG_HUSH_PARSER
#error "must define CFG_HUSH_PARSER"
#endif

#if !defined(CONFIG_CMD_FAT)
#error "must define CONFIG_CMD_FAT"
#endif

#undef AU_DEBUG

#undef debug
#ifdef	AU_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif	/* AU_DEBUG */

/* possible names of files on the USB stick. */
#define AU_FIRMWARE	"u-boot.img"
#define AU_KERNEL	"kernel.img"
#define AU_ROOTFS	"rootfs.img"

struct flash_layout {
	long start;
	long end;
};

/* layout of the FLASH. ST = start address, ND = end address. */
#define AU_FL_FIRMWARE_ST	0xfC000000
#define AU_FL_FIRMWARE_ND	0xfC03FFFF
#define AU_FL_KERNEL_ST		0xfC0C0000
#define AU_FL_KERNEL_ND		0xfC1BFFFF
#define AU_FL_ROOTFS_ST		0xFC1C0000
#define AU_FL_ROOTFS_ND		0xFCFBFFFF

static int au_usb_stor_curr_dev; /* current device */

/* index of each file in the following arrays */
#define IDX_FIRMWARE	0
#define IDX_KERNEL	1
#define IDX_ROOTFS	2

/* max. number of files which could interest us */
#define AU_MAXFILES 3

/* pointers to file names */
char *aufile[AU_MAXFILES] = {
	AU_FIRMWARE,
	AU_KERNEL,
	AU_ROOTFS
};

/* sizes of flash areas for each file */
long ausize[AU_MAXFILES] = {
	(AU_FL_FIRMWARE_ND + 1) - AU_FL_FIRMWARE_ST,
	(AU_FL_KERNEL_ND   + 1) - AU_FL_KERNEL_ST,
	(AU_FL_ROOTFS_ND   + 1) - AU_FL_ROOTFS_ST,
};

/* array of flash areas start and end addresses */
struct flash_layout aufl_layout[AU_MAXFILES] = {
	{ AU_FL_FIRMWARE_ST,	AU_FL_FIRMWARE_ND, },
	{ AU_FL_KERNEL_ST,	AU_FL_KERNEL_ND,   },
	{ AU_FL_ROOTFS_ST,	AU_FL_ROOTFS_ND,   },
};

ulong totsize;

/* where to load files into memory */
#define LOAD_ADDR ((unsigned char *)0x00200000)

/* the root file system is the largest image */
#define MAX_LOADSZ ausize[IDX_ROOTFS]

/*i2c address of the keypad status*/
#define I2C_PSOC_KEYPAD_ADDR	0x53

/* keypad mask */
#define KEYPAD_ROW	2
#define KEYPAD_COL	2
#define KEYPAD_MASK_LO	((1<<(KEYPAD_COL-1+(KEYPAD_ROW*3-3)))&0xFF)
#define KEYPAD_MASK_HI	((1<<(KEYPAD_COL-1+(KEYPAD_ROW*3-3)))>>8)

/* externals */
extern int fat_register_device(block_dev_desc_t *, int);
extern int file_fat_detectfs(void);
extern long file_fat_read(const char *, void *, unsigned long);
extern int i2c_read (unsigned char, unsigned int, int , unsigned char* , int);
extern int flash_sect_erase(ulong, ulong);
extern int flash_sect_protect (int, ulong, ulong);
extern int flash_write (char *, ulong, ulong);
extern int u_boot_hush_start(void);
#ifdef CONFIG_PROGRESSBAR
extern void show_progress(int, int);
extern void lcd_puts (char *);
extern void lcd_enable(void);
#endif

int au_check_cksum_valid(int idx, long nbytes)
{
	image_header_t *hdr;

	hdr = (image_header_t *)LOAD_ADDR;
#if defined(CONFIG_FIT)
	if (genimg_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	if (nbytes != image_get_image_size (hdr)) {
		printf ("Image %s bad total SIZE\n", aufile[idx]);
		return -1;
	}
	/* check the data CRC */
	if (!image_check_dcrc (hdr)) {
		printf ("Image %s bad data checksum\n", aufile[idx]);
		return -1;
	}
	return 0;
}

int au_check_header_valid(int idx, long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum, fsize;

	hdr = (image_header_t *)LOAD_ADDR;
#if defined(CONFIG_FIT)
	if (genimg_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	/* check the easy ones first */
#undef CHECK_VALID_DEBUG
#ifdef CHECK_VALID_DEBUG
	printf("magic %#x %#x ", image_get_magic (hdr), IH_MAGIC);
	printf("arch %#x %#x ", image_get_arch (hdr), IH_ARCH_ARM);
	printf("size %#x %#lx ", image_get_data_size (hdr), nbytes);
	printf("type %#x %#x ", image_get_type (hdr), IH_TYPE_KERNEL);
#endif
	if (nbytes < image_get_header_size ()) {
		printf ("Image %s bad header SIZE\n", aufile[idx]);
		ausize[idx] = 0;
		return -1;
	}
	if (!image_check_magic (hdr) || !image_check_arch (hdr, IH_ARCH_PPC)) {
		printf ("Image %s bad MAGIC or ARCH\n", aufile[idx]);
		ausize[idx] = 0;
		return -1;
	}
	/* check the hdr CRC */
	if (!image_check_hcrc (hdr)) {
		printf ("Image %s bad header checksum\n", aufile[idx]);
		ausize[idx] = 0;
		return -1;
	}
	/* check the type - could do this all in one gigantic if() */
	if ((idx == IDX_FIRMWARE) && !image_check_type (hdr, IH_TYPE_FIRMWARE)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		ausize[idx] = 0;
		return -1;
	}
	if ((idx == IDX_KERNEL) && !image_check_type (hdr, IH_TYPE_KERNEL)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		ausize[idx] = 0;
		return -1;
	}
	if ((idx == IDX_ROOTFS) &&
			(!image_check_type (hdr, IH_TYPE_RAMDISK) &&
			!image_check_type (hdr, IH_TYPE_FILESYSTEM))) {
		printf ("Image %s wrong type\n", aufile[idx]);
		ausize[idx] = 0;
		return -1;
	}
	/* recycle checksum */
	checksum = image_get_data_size (hdr);

	fsize = checksum + image_get_header_size ();
	/* for kernel and ramdisk the image header must also fit into flash */
	if (idx == IDX_KERNEL || image_check_type (hdr, IH_TYPE_RAMDISK))
		checksum += image_get_header_size ();

	/* check the size does not exceed space in flash. HUSH scripts */
	if ((ausize[idx] != 0) && (ausize[idx] < checksum)) {
		printf ("Image %s is bigger than FLASH\n", aufile[idx]);
		ausize[idx] = 0;
		return -1;
	}
	/* Update with the real filesize */
	ausize[idx] = fsize;

	return checksum; /* return size to be written to flash */
}

int au_do_update(int idx, long sz)
{
	image_header_t *hdr;
	char *addr;
	long start, end;
	int off, rc;
	uint nbytes;

	hdr = (image_header_t *)LOAD_ADDR;
#if defined(CONFIG_FIT)
	if (genimg_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	/* execute a script */
	if (image_check_type (hdr, IH_TYPE_SCRIPT)) {
		addr = (char *)((char *)hdr + image_get_header_size ());
		/* stick a NULL at the end of the script, otherwise */
		/* parse_string_outer() runs off the end. */
		addr[image_get_data_size (hdr)] = 0;
		addr += 8;
		parse_string_outer(addr, FLAG_PARSE_SEMICOLON);
		return 0;
	}

	start = aufl_layout[idx].start;
	end = aufl_layout[idx].end;

	/* unprotect the address range */
	/* this assumes that ONLY the firmware is protected! */
	if (idx == IDX_FIRMWARE) {
#undef AU_UPDATE_TEST
#ifdef AU_UPDATE_TEST
		/* erase it where Linux goes */
		start = aufl_layout[1].start;
		end = aufl_layout[1].end;
#endif
		flash_sect_protect(0, start, end);
	}

	/*
	 * erase the address range.
	 */
	debug ("flash_sect_erase(%lx, %lx);\n", start, end);
	flash_sect_erase(start, end);
	wait_ms(100);
#ifdef CONFIG_PROGRESSBAR
	show_progress(end - start, totsize);
#endif

	/* strip the header - except for the kernel and ramdisk */
	if (image_check_type (hdr, IH_TYPE_KERNEL) ||
			image_check_type (hdr, IH_TYPE_RAMDISK)) {
		addr = (char *)hdr;
		off = image_get_header_size ();
		nbytes = image_get_image_size (hdr);
	} else {
		addr = (char *)((char *)hdr + image_get_header_size ());
#ifdef AU_UPDATE_TEST
		/* copy it to where Linux goes */
		if (idx == IDX_FIRMWARE)
			start = aufl_layout[1].start;
#endif
		off = 0;
		nbytes = image_get_data_size (hdr);
	}

	/* copy the data from RAM to FLASH */
	debug ("flash_write(%p, %lx %x)\n", addr, start, nbytes);
	rc = flash_write(addr, start, nbytes);
	if (rc != 0) {
		printf("Flashing failed due to error %d\n", rc);
		return -1;
	}

#ifdef CONFIG_PROGRESSBAR
	show_progress(nbytes, totsize);
#endif

	/* check the data CRC of the copy */
	if (crc32 (0, (uchar *)(start + off), image_get_data_size (hdr)) !=
	    image_get_dcrc (hdr)) {
		printf ("Image %s Bad Data Checksum after COPY\n", aufile[idx]);
		return -1;
	}

	/* protect the address range */
	/* this assumes that ONLY the firmware is protected! */
	if (idx == IDX_FIRMWARE)
		flash_sect_protect(1, start, end);
	return 0;
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
	int i, res = 0, bitmap_first, cnt, old_ctrlc, got_ctrlc;
	char *env;
	long start, end;

#if 0 /* disable key-press detection to speed up boot-up time */
	uchar keypad_status1[2] = {0,0}, keypad_status2[2] = {0,0};

	/*
	 * Read keypad status
	 */
	i2c_read(I2C_PSOC_KEYPAD_ADDR, 0, 0, keypad_status1, 2);
	wait_ms(500);
	i2c_read(I2C_PSOC_KEYPAD_ADDR, 0, 0, keypad_status2, 2);

	/*
	 * Check keypad
	 */
	if ( !(keypad_status1[1] & KEYPAD_MASK_LO) ||
	      (keypad_status1[1] != keypad_status2[1])) {
		return 0;
	}

#endif
	au_usb_stor_curr_dev = -1;
	/* start USB */
	if (usb_stop() < 0) {
		debug ("usb_stop failed\n");
		return -1;
	}
	if (usb_init() < 0) {
		debug ("usb_init failed\n");
		return -1;
	}
	/*
	 * check whether a storage device is attached (assume that it's
	 * a USB memory stick, since nothing else should be attached).
	 */
	au_usb_stor_curr_dev = usb_stor_scan(0);
	if (au_usb_stor_curr_dev == -1) {
		debug ("No device found. Not initialized?\n");
		res = -1;
		goto xit;
	}
	/* check whether it has a partition table */
	stor_dev = get_dev("usb", 0);
	if (stor_dev == NULL) {
		debug ("uknown device type\n");
		res = -1;
		goto xit;
	}
	if (fat_register_device(stor_dev, 1) != 0) {
		debug ("Unable to use USB %d:%d for fatls\n",
			au_usb_stor_curr_dev, 1);
		res = -1;
		goto xit;
	}
	if (file_fat_detectfs() != 0) {
		debug ("file_fat_detectfs failed\n");
	}

	/*
	 * now check whether start and end are defined using environment
	 * variables.
	 */
	start = -1;
	end = 0;
	env = getenv("firmware_st");
	if (env != NULL)
		start = simple_strtoul(env, NULL, 16);
	env = getenv("firmware_nd");
	if (env != NULL)
		end = simple_strtoul(env, NULL, 16);
	if (start >= 0 && end && end > start) {
		ausize[IDX_FIRMWARE] = (end + 1) - start;
		aufl_layout[IDX_FIRMWARE].start = start;
		aufl_layout[IDX_FIRMWARE].end = end;
	}
	start = -1;
	end = 0;
	env = getenv("kernel_st");
	if (env != NULL)
		start = simple_strtoul(env, NULL, 16);
	env = getenv("kernel_nd");
	if (env != NULL)
		end = simple_strtoul(env, NULL, 16);
	if (start >= 0 && end && end > start) {
		ausize[IDX_KERNEL] = (end + 1) - start;
		aufl_layout[IDX_KERNEL].start = start;
		aufl_layout[IDX_KERNEL].end = end;
	}
	start = -1;
	end = 0;
	env = getenv("rootfs_st");
	if (env != NULL)
		start = simple_strtoul(env, NULL, 16);
	env = getenv("rootfs_nd");
	if (env != NULL)
		end = simple_strtoul(env, NULL, 16);
	if (start >= 0 && end && end > start) {
		ausize[IDX_ROOTFS] = (end + 1) - start;
		aufl_layout[IDX_ROOTFS].start = start;
		aufl_layout[IDX_ROOTFS].end = end;
	}

	/* make certain that HUSH is runnable */
	u_boot_hush_start();
	/* make sure that we see CTRL-C and save the old state */
	old_ctrlc = disable_ctrlc(0);

	bitmap_first = 0;

	/* validate the images first */
	for (i = 0; i < AU_MAXFILES; i++) {
		ulong imsize;
		/* just read the header */
		sz = file_fat_read(aufile[i], LOAD_ADDR, image_get_header_size ());
		debug ("read %s sz %ld hdr %d\n",
			aufile[i], sz, image_get_header_size ());
		if (sz <= 0 || sz < image_get_header_size ()) {
			debug ("%s not found\n", aufile[i]);
			ausize[i] = 0;
			continue;
		}
		/* au_check_header_valid() updates ausize[] */
		if ((imsize = au_check_header_valid(i, sz)) < 0) {
			debug ("%s header not valid\n", aufile[i]);
			continue;
		}
		/* totsize accounts for image size and flash erase size */
		totsize += (imsize + (aufl_layout[i].end - aufl_layout[i].start));
	}

#ifdef CONFIG_PROGRESSBAR
	if (totsize) {
		lcd_puts(" Update in progress\n");
		lcd_enable();
	}
#endif

	/* just loop thru all the possible files */
	for (i = 0; i < AU_MAXFILES && totsize; i++) {
		if (!ausize[i]) {
			continue;
		}
		sz = file_fat_read(aufile[i], LOAD_ADDR, ausize[i]);

		debug ("read %s sz %ld hdr %d\n",
			aufile[i], sz, image_get_header_size ());

		if (sz != ausize[i]) {
			printf ("%s: size %d read %d?\n", aufile[i], ausize[i], sz);
			continue;
		}

		if (sz <= 0 || sz <= image_get_header_size ()) {
			debug ("%s not found\n", aufile[i]);
			continue;
		}
		if (au_check_cksum_valid(i, sz) < 0) {
			debug ("%s checksum not valid\n", aufile[i]);
			continue;
		}
		/* this is really not a good idea, but it's what the */
		/* customer wants. */
		cnt = 0;
		got_ctrlc = 0;
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
#ifdef AU_TEST_ONLY
		} while (res < 0 && cnt < (AU_MAXFILES + 1));
		if (cnt < (AU_MAXFILES + 1))
#else
		} while (res < 0);
#endif
	}

	/* restore the old state */
	disable_ctrlc(old_ctrlc);
#ifdef CONFIG_PROGRESSBAR
	if (totsize) {
		if (!res) {
			lcd_puts("\n  Update completed\n");
		} else {
			lcd_puts("\n   Update error\n");
		}
		lcd_enable();
	}
#endif
 xit:
	usb_stop();
	return res;
}
#endif /* CONFIG_AUTO_UPDATE */
