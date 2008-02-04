/*
 * (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering, gj@denx.de.
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
#include <malloc.h>
#include <image.h>
#include <asm/byteorder.h>
#include <usb.h>

#ifdef CFG_HUSH_PARSER
#include <hush.h>
#endif

#ifdef CONFIG_AUTO_UPDATE

#ifndef CONFIG_USB_OHCI_NEW
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

/*
 * Check whether a USB memory stick is plugged in.
 * If one is found:
 *	1) if prepare.img ist found load it into memory. If it is
 *		valid then run it.
 *	2) if preinst.img is found load it into memory. If it is
 *		valid then run it. Update the EEPROM.
 *	3) if firmw_01.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	4) if kernl_01.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	5) if app.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	6) if disk.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	7) if postinst.img is found load it into memory. If it is
 *		valid then run it. Update the EEPROM.
 */

#undef AU_DEBUG

#undef debug
#ifdef	AU_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif	/* AU_DEBUG */

/* possible names of files on the USB stick. */
#define AU_PREPARE	"prepare.img"
#define AU_PREINST	"preinst.img"
#define AU_FIRMWARE	"firmw_01.img"
#define AU_KERNEL	"kernl_01.img"
#define AU_APP		"app.img"
#define AU_DISK		"disk.img"
#define AU_POSTINST	"postinst.img"

struct flash_layout
{
	long start;
	long end;
};

/* layout of the FLASH. ST = start address, ND = end address. */
#ifndef CONFIG_FLASH_8MB			/* 16 MB Flash, 32 MB RAM */
#define AU_FL_FIRMWARE_ST	0x00000000
#define AU_FL_FIRMWARE_ND	0x0009FFFF
#define AU_FL_VFD_ST		0x000A0000
#define AU_FL_VFD_ND		0x000BFFFF
#define AU_FL_KERNEL_ST		0x000C0000
#define AU_FL_KERNEL_ND		0x001BFFFF
#define AU_FL_APP_ST		0x001C0000
#define AU_FL_APP_ND		0x005BFFFF
#define AU_FL_DISK_ST		0x005C0000
#define AU_FL_DISK_ND		0x00FFFFFF
#else						/*  8 MB Flash, 32 MB RAM */
#define AU_FL_FIRMWARE_ST	0x00000000
#define AU_FL_FIRMWARE_ND	0x0005FFFF
#define AU_FL_KERNEL_ST		0x00060000
#define AU_FL_KERNEL_ND		0x0013FFFF
#define AU_FL_APP_ST		0x00140000
#define AU_FL_APP_ND		0x0067FFFF
#define AU_FL_DISK_ST		0x00680000
#define AU_FL_DISK_ND		0x007DFFFF
#define AU_FL_VFD_ST		0x007E0000
#define AU_FL_VFD_ND		0x007FFFFF
#endif	/* CONFIG_FLASH_8MB */

/* a structure with the offsets to values in the EEPROM */
struct eeprom_layout
{
	int time;
	int size;
	int dcrc;
};

/* layout of the EEPROM - offset from the start. All entries are 32 bit. */
#define AU_EEPROM_TIME_PREINST	64
#define AU_EEPROM_SIZE_PREINST	68
#define AU_EEPROM_DCRC_PREINST	72
#define AU_EEPROM_TIME_FIRMWARE	76
#define AU_EEPROM_SIZE_FIRMWARE	80
#define AU_EEPROM_DCRC_FIRMWARE	84
#define AU_EEPROM_TIME_KERNEL	88
#define AU_EEPROM_SIZE_KERNEL	92
#define AU_EEPROM_DCRC_KERNEL	96
#define AU_EEPROM_TIME_APP	100
#define AU_EEPROM_SIZE_APP	104
#define AU_EEPROM_DCRC_APP	108
#define AU_EEPROM_TIME_DISK	112
#define AU_EEPROM_SIZE_DISK	116
#define AU_EEPROM_DCRC_DISK	120
#define AU_EEPROM_TIME_POSTINST 124
#define AU_EEPROM_SIZE_POSTINST 128
#define AU_EEPROM_DCRC_POSTINST 132

static int au_usb_stor_curr_dev; /* current device */

/* index of each file in the following arrays */
#define IDX_PREPARE	0
#define IDX_PREINST	1
#define IDX_FIRMWARE	2
#define IDX_KERNEL	3
#define IDX_APP		4
#define IDX_DISK	5
#define IDX_POSTINST	6
/* max. number of files which could interest us */
#define AU_MAXFILES 7
/* pointers to file names */
char *aufile[AU_MAXFILES];
/* sizes of flash areas for each file */
long ausize[AU_MAXFILES];
/* offsets into the EEEPROM */
struct eeprom_layout auee_off[AU_MAXFILES] = { \
	{0}, \
	{AU_EEPROM_TIME_PREINST, AU_EEPROM_SIZE_PREINST, AU_EEPROM_DCRC_PREINST,}, \
	{AU_EEPROM_TIME_FIRMWARE, AU_EEPROM_SIZE_FIRMWARE, AU_EEPROM_DCRC_FIRMWARE,}, \
	{AU_EEPROM_TIME_KERNEL, AU_EEPROM_SIZE_KERNEL, AU_EEPROM_DCRC_KERNEL,}, \
	{AU_EEPROM_TIME_APP, AU_EEPROM_SIZE_APP, AU_EEPROM_DCRC_APP,}, \
	{AU_EEPROM_TIME_DISK, AU_EEPROM_SIZE_DISK, AU_EEPROM_DCRC_DISK,}, \
	{AU_EEPROM_TIME_POSTINST, AU_EEPROM_SIZE_POSTINST, AU_EEPROM_DCRC_POSTINST,} \
	};
/* array of flash areas start and end addresses */
struct flash_layout aufl_layout[AU_MAXFILES - 3] = { \
	{AU_FL_FIRMWARE_ST, AU_FL_FIRMWARE_ND,}, \
	{AU_FL_KERNEL_ST, AU_FL_KERNEL_ND,}, \
	{AU_FL_APP_ST, AU_FL_APP_ND,}, \
	{AU_FL_DISK_ST, AU_FL_DISK_ND,}, \
};
/* convert the index into aufile[] to an index into aufl_layout[] */
#define FIDX_TO_LIDX(idx) ((idx) - 2)

/* where to load files into memory */
#define LOAD_ADDR ((unsigned char *)0x0C100000)
/* the app is the largest image */
#define MAX_LOADSZ ausize[IDX_APP]

/* externals */
extern int fat_register_device(block_dev_desc_t *, int);
extern int file_fat_detectfs(void);
extern long file_fat_read(const char *, void *, unsigned long);
extern int i2c_read (unsigned char, unsigned int, int , unsigned char* , int);
extern int i2c_write (uchar, uint, int , uchar* , int);
#ifdef CONFIG_VFD
extern int trab_vfd (ulong);
extern int transfer_pic(unsigned char, unsigned char *, int, int);
#endif
extern int flash_sect_erase(ulong, ulong);
extern int flash_sect_protect (int, ulong, ulong);
extern int flash_write (char *, ulong, ulong);
/* change char* to void* to shutup the compiler */
extern int i2c_write_multiple (uchar, uint, int, void *, int);
extern int i2c_read_multiple (uchar, uint, int, void *, int);
extern int u_boot_hush_start(void);

int
au_check_cksum_valid(int idx, long nbytes)
{
	image_header_t *hdr;

	hdr = (image_header_t *)LOAD_ADDR;
#if defined(CONFIG_FIT)
	if (gen_image_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	if (nbytes != image_get_image_size (hdr))
	{
		printf ("Image %s bad total SIZE\n", aufile[idx]);
		return -1;
	}
	/* check the data CRC */
	if (!image_check_dcrc (hdr)) {
	{
		printf ("Image %s bad data checksum\n", aufile[idx]);
		return -1;
	}
	return 0;
}

int
au_check_header_valid(int idx, long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;
	unsigned char buf[4];

	hdr = (image_header_t *)LOAD_ADDR;
#if defined(CONFIG_FIT)
	if (gen_image_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
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
	if (nbytes < image_get_header_size ())
	{
		printf ("Image %s bad header SIZE\n", aufile[idx]);
		return -1;
	}
	if (!image_check_magic (hdr) || !image_check_arch (hdr, IH_ARCH_ARM))
	{
		printf ("Image %s bad MAGIC or ARCH\n", aufile[idx]);
		return -1;
	}
	/* check the hdr CRC */
	if (!image_check_hcrc (hdr)) {
		printf ("Image %s bad header checksum\n", aufile[idx]);
		return -1;
	}
	/* check the type - could do this all in one gigantic if() */
	if ((idx == IDX_FIRMWARE) && !image_check_type (hdr, IH_TYPE_FIRMWARE)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		return -1;
	}
	if ((idx == IDX_KERNEL) && !image_check_type (hdr, IH_TYPE_KERNEL)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		return -1;
	}
	if ((idx == IDX_DISK) && !image_check_type (hdr, IH_TYPE_FILESYSTEM)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		return -1;
	}
	if ((idx == IDX_APP) && !image_check_type (hdr, IH_TYPE_RAMDISK)
	    && !image_check_type (hdr, FILESYSTEM)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		return -1;
	}
	if ((idx == IDX_PREPARE || idx == IDX_PREINST || idx == IDX_POSTINST)
		&& !image_check_type (hdr, IH_TYPE_SCRIPT))
	{
		printf ("Image %s wrong type\n", aufile[idx]);
		return -1;
	}
	/* special case for prepare.img */
	if (idx == IDX_PREPARE)
		return 0;
	/* recycle checksum */
	checksum = image_get_data_size (hdr);
	/* for kernel and app the image header must also fit into flash */
	if ((idx != IDX_DISK) && (idx != IDX_FIRMWARE))
		checksum += image_get_header_size ();
	/* check the size does not exceed space in flash. HUSH scripts */
	/* all have ausize[] set to 0 */
	if ((ausize[idx] != 0) && (ausize[idx] < checksum)) {
		printf ("Image %s is bigger than FLASH\n", aufile[idx]);
		return -1;
	}
	/* check the time stamp from the EEPROM */
	/* read it in */
	i2c_read_multiple(0x54, auee_off[idx].time, 1, buf, sizeof(buf));
#ifdef CHECK_VALID_DEBUG
	printf ("buf[0] %#x buf[1] %#x buf[2] %#x buf[3] %#x "
		"as int %#x time %#x\n",
		buf[0], buf[1], buf[2], buf[3],
		*((unsigned int *)buf), image_get_time (hdr));
#endif
	/* check it */
	if (*((unsigned int *)buf) >= image_get_time (hdr)) {
		printf ("Image %s is too old\n", aufile[idx]);
		return -1;
	}

	return 0;
}

/* power control defines */
#define CPLD_VFD_BK ((volatile char *)0x04038002)
#define POWER_OFF (1 << 1)

int
au_do_update(int idx, long sz)
{
	image_header_t *hdr;
	char *addr;
	long start, end;
	int off, rc;
	uint nbytes;

	hdr = (image_header_t *)LOAD_ADDR;
#if defined(CONFIG_FIT)
	if (gen_image_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	/* disable the power switch */
	*CPLD_VFD_BK |= POWER_OFF;

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

	start = aufl_layout[FIDX_TO_LIDX(idx)].start;
	end = aufl_layout[FIDX_TO_LIDX(idx)].end;

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

	/* check the dcrc of the copy */
	if (crc32 (0, (uchar *)(start + off), image_get_data_size (hdr)) !=
	    image_get_dcrc (hdr)) {
		printf ("Image %s Bad Data Checksum After COPY\n", aufile[idx]);
		return -1;
	}

	/* protect the address range */
	/* this assumes that ONLY the firmware is protected! */
	if (idx == IDX_FIRMWARE)
		flash_sect_protect(1, start, end);
	return 0;
}

int
au_update_eeprom(int idx)
{
	image_header_t *hdr;
	int off;
	uint32_t val;

	/* special case for prepare.img */
	if (idx == IDX_PREPARE) {
		/* enable the power switch */
		*CPLD_VFD_BK &= ~POWER_OFF;
		return 0;
	}

	hdr = (image_header_t *)LOAD_ADDR;
#if defined(CONFIG_FIT)
	if (gen_image_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	/* write the time field into EEPROM */
	off = auee_off[idx].time;
	val = image_get_time (hdr);
	i2c_write_multiple(0x54, off, 1, &val, sizeof(val));
	/* write the size field into EEPROM */
	off = auee_off[idx].size;
	val = image_get_data_size (hdr);
	i2c_write_multiple(0x54, off, 1, &val, sizeof(val));
	/* write the dcrc field into EEPROM */
	off = auee_off[idx].dcrc;
	val = image_get_dcrc (hdr);
	i2c_write_multiple(0x54, off, 1, &val, sizeof(val));
	/* enable the power switch */
	*CPLD_VFD_BK &= ~POWER_OFF;
	return 0;
}

/*
 * this is called from board_init() after the hardware has been set up
 * and is usable. That seems like a good time to do this.
 * Right now the return value is ignored.
 */
int
do_auto_update(void)
{
	block_dev_desc_t *stor_dev;
	long sz;
	int i, res = 0, bitmap_first, cnt, old_ctrlc, got_ctrlc;
	char *env;
	long start, end;

#undef ERASE_EEPROM
#ifdef ERASE_EEPROM
	int arr[18];
	memset(arr, 0, sizeof(arr));
	i2c_write_multiple(0x54, 64, 1, arr, sizeof(arr));
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

	/* initialize the array of file names */
	memset(aufile, 0, sizeof(aufile));
	aufile[IDX_PREPARE] = AU_PREPARE;
	aufile[IDX_PREINST] = AU_PREINST;
	aufile[IDX_FIRMWARE] = AU_FIRMWARE;
	aufile[IDX_KERNEL] = AU_KERNEL;
	aufile[IDX_APP] = AU_APP;
	aufile[IDX_DISK] = AU_DISK;
	aufile[IDX_POSTINST] = AU_POSTINST;
	/* initialize the array of flash sizes */
	memset(ausize, 0, sizeof(ausize));
	ausize[IDX_FIRMWARE] = (AU_FL_FIRMWARE_ND + 1) - AU_FL_FIRMWARE_ST;
	ausize[IDX_KERNEL] = (AU_FL_KERNEL_ND + 1) - AU_FL_KERNEL_ST;
	ausize[IDX_APP] = (AU_FL_APP_ND + 1) - AU_FL_APP_ST;
	ausize[IDX_DISK] = (AU_FL_DISK_ND + 1) - AU_FL_DISK_ST;
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
		aufl_layout[0].start = start;
		aufl_layout[0].end = end;
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
		aufl_layout[1].start = start;
		aufl_layout[1].end = end;
	}
	start = -1;
	end = 0;
	env = getenv("app_st");
	if (env != NULL)
		start = simple_strtoul(env, NULL, 16);
	env = getenv("app_nd");
	if (env != NULL)
		end = simple_strtoul(env, NULL, 16);
	if (start >= 0 && end && end > start) {
		ausize[IDX_APP] = (end + 1) - start;
		aufl_layout[2].start = start;
		aufl_layout[2].end = end;
	}
	start = -1;
	end = 0;
	env = getenv("disk_st");
	if (env != NULL)
		start = simple_strtoul(env, NULL, 16);
	env = getenv("disk_nd");
	if (env != NULL)
		end = simple_strtoul(env, NULL, 16);
	if (start >= 0 && end && end > start) {
		ausize[IDX_DISK] = (end + 1) - start;
		aufl_layout[3].start = start;
		aufl_layout[3].end = end;
	}
	/* make certain that HUSH is runnable */
	u_boot_hush_start();
	/* make sure that we see CTRL-C and save the old state */
	old_ctrlc = disable_ctrlc(0);

	bitmap_first = 0;
	/* just loop thru all the possible files */
	for (i = 0; i < AU_MAXFILES; i++) {
		/* just read the header */
		sz = file_fat_read(aufile[i], LOAD_ADDR, image_get_header_size ());
		debug ("read %s sz %ld hdr %d\n",
			aufile[i], sz, image_get_header_size ());
		if (sz <= 0 || sz < image_get_header_size ()) {
			debug ("%s not found\n", aufile[i]);
			continue;
		}
		if (au_check_header_valid(i, sz) < 0) {
			debug ("%s header not valid\n", aufile[i]);
			continue;
		}
		sz = file_fat_read(aufile[i], LOAD_ADDR, MAX_LOADSZ);
		debug ("read %s sz %ld hdr %d\n",
			aufile[i], sz, image_get_header_size ());
		if (sz <= 0 || sz <= image_get_header_size ()) {
			debug ("%s not found\n", aufile[i]);
			continue;
		}
		if (au_check_cksum_valid(i, sz) < 0) {
			debug ("%s checksum not valid\n", aufile[i]);
			continue;
		}
#ifdef CONFIG_VFD
		/* now that we have a valid file we can display the */
		/* bitmap. */
		if (bitmap_first == 0) {
			env = getenv("bitmap2");
			if (env == NULL) {
				trab_vfd(0);
			} else {
				/* not so simple - bitmap2 is supposed to */
				/* contain the address of the bitmap */
				env = (char *)simple_strtoul(env, NULL, 16);
/* NOTE: these are taken from vfd_logo.h. If that file changes then */
/* these defines MUST also be updated! These may be wrong for bitmap2. */
#define VFD_LOGO_WIDTH 112
#define VFD_LOGO_HEIGHT 72
				/* must call transfer_pic directly */
				transfer_pic(3, (unsigned char *)env,
					     VFD_LOGO_HEIGHT, VFD_LOGO_WIDTH);
			}
			bitmap_first = 1;
		}
#endif
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
		} while (res < 0 && cnt < 3);
		if (cnt < 3)
#else
		} while (res < 0);
#endif
		/*
		 * it doesn't make sense to update the EEPROM if the
		 * update was interrupted by the user due to errors.
		 */
		if (got_ctrlc == 0)
			au_update_eeprom(i);
		else
			/* enable the power switch */
			*CPLD_VFD_BK &= ~POWER_OFF;
	}
	/* restore the old state */
	disable_ctrlc(old_ctrlc);
xit:
	usb_stop();
	return res;
}
#endif /* CONFIG_AUTO_UPDATE */
