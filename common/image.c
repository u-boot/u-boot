/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#define DEBUG

#ifndef USE_HOSTCC
#include <common.h>
#include <watchdog.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
#include <status_led.h>
#endif

#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif

#ifdef CONFIG_LOGBUFFER
#include <logbuff.h>
#endif

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE)
#include <rtc.h>
#endif

#if defined(CONFIG_FIT)
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>
#endif

#ifdef CONFIG_CMD_BDI
extern int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif

DECLARE_GLOBAL_DATA_PTR;

static image_header_t* image_get_ramdisk (cmd_tbl_t *cmdtp, int flag,
		int argc, char *argv[],
		ulong rd_addr, uint8_t arch, int verify);
#else
#include "mkimage.h"
#endif /* USE_HOSTCC*/

#include <image.h>

unsigned long crc32 (unsigned long, const unsigned char *, unsigned int);

/*****************************************************************************/
/* Legacy format routines */
/*****************************************************************************/
int image_check_hcrc (image_header_t *hdr)
{
	ulong hcrc;
	ulong len = image_get_header_size ();
	image_header_t header;

	/* Copy header so we can blank CRC field for re-calculation */
	memmove (&header, (char *)hdr, image_get_header_size ());
	image_set_hcrc (&header, 0);

	hcrc = crc32 (0, (unsigned char *)&header, len);

	return (hcrc == image_get_hcrc (hdr));
}

int image_check_dcrc (image_header_t *hdr)
{
	ulong data = image_get_data (hdr);
	ulong len = image_get_data_size (hdr);
	ulong dcrc = crc32 (0, (unsigned char *)data, len);

	return (dcrc == image_get_dcrc (hdr));
}

#ifndef USE_HOSTCC
int image_check_dcrc_wd (image_header_t *hdr, ulong chunksz)
{
	ulong dcrc = 0;
	ulong len = image_get_data_size (hdr);
	ulong data = image_get_data (hdr);

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	ulong cdata = data;
	ulong edata = cdata + len;

	while (cdata < edata) {
		ulong chunk = edata - cdata;

		if (chunk > chunksz)
			chunk = chunksz;
		dcrc = crc32 (dcrc, (unsigned char *)cdata, chunk);
		cdata += chunk;

		WATCHDOG_RESET ();
	}
#else
	dcrc = crc32 (0, (unsigned char *)data, len);
#endif

	return (dcrc == image_get_dcrc (hdr));
}

/**
 * image_multi_count - get component (sub-image) count
 * @hdr: pointer to the header of the multi component image
 *
 * image_multi_count() returns number of components in a multi
 * component image.
 *
 * Note: no checking of the image type is done, caller must pass
 * a valid multi component image.
 *
 * returns:
 *     number of components
 */
ulong image_multi_count (image_header_t *hdr)
{
	ulong i, count = 0;
	ulong *size;

	/* get start of the image payload, which in case of multi
	 * component images that points to a table of component sizes */
	size = (ulong *)image_get_data (hdr);

	/* count non empty slots */
	for (i = 0; size[i]; ++i)
		count++;

	return count;
}

/**
 * image_multi_getimg - get component data address and size
 * @hdr: pointer to the header of the multi component image
 * @idx: index of the requested component
 * @data: pointer to a ulong variable, will hold component data address
 * @len: pointer to a ulong variable, will hold component size
 *
 * image_multi_getimg() returns size and data address for the requested
 * component in a multi component image.
 *
 * Note: no checking of the image type is done, caller must pass
 * a valid multi component image.
 *
 * returns:
 *     data address and size of the component, if idx is valid
 *     0 in data and len, if idx is out of range
 */
void image_multi_getimg (image_header_t *hdr, ulong idx,
			ulong *data, ulong *len)
{
	int i;
	ulong *size;
	ulong offset, tail, count, img_data;

	/* get number of component */
	count = image_multi_count (hdr);

	/* get start of the image payload, which in case of multi
	 * component images that points to a table of component sizes */
	size = (ulong *)image_get_data (hdr);

	/* get address of the proper component data start, which means
	 * skipping sizes table (add 1 for last, null entry) */
	img_data = image_get_data (hdr) + (count + 1) * sizeof (ulong);

	if (idx < count) {
		*len = size[idx];
		offset = 0;
		tail = 0;

		/* go over all indices preceding requested component idx */
		for (i = 0; i < idx; i++) {
			/* add up i-th component size */
			offset += size[i];

			/* add up alignment for i-th component */
			tail += (4 - size[i] % 4);
		}

		/* calculate idx-th component data address */
		*data = img_data + offset + tail;
	} else {
		*len = 0;
		*data = 0;
	}
}

#ifndef USE_HOSTCC
static void image_print_type (image_header_t *hdr)
{
	const char *os, *arch, *type, *comp;

	os = genimg_get_os_name (image_get_os (hdr));
	arch = genimg_get_arch_name (image_get_arch (hdr));
	type = genimg_get_type_name (image_get_type (hdr));
	comp = genimg_get_comp_name (image_get_comp (hdr));

	printf ("%s %s %s (%s)", arch, os, type, comp);
}

void image_print_contents (image_header_t *hdr)
{
#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE)
	time_t timestamp = (time_t)image_get_time (hdr);
	struct rtc_time tm;
#endif

	printf ("   Image Name:   %.*s\n", IH_NMLEN, image_get_name (hdr));

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE)
	to_tm (timestamp, &tm);
	printf ("   Created:      %4d-%02d-%02d  %2d:%02d:%02d UTC\n",
		tm.tm_year, tm.tm_mon, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif
	puts ("   Image Type:   ");
	image_print_type (hdr);

	printf ("\n   Data Size:    %d Bytes = ", image_get_data_size (hdr));
	print_size (image_get_data_size (hdr), "\n");
	printf ("   Load Address: %08x\n"
		"   Entry Point:  %08x\n",
		 image_get_load (hdr), image_get_ep (hdr));

	if (image_check_type (hdr, IH_TYPE_MULTI)) {
		int i;
		ulong data, len;
		ulong count = image_multi_count (hdr);

		puts ("   Contents:\n");
		for (i = 0; i < count; i++) {
			image_multi_getimg (hdr, i, &data, &len);
			printf ("   Image %d: %8ld Bytes = ", i, len);
			print_size (len, "\n");
		}
	}
}

/**
 * image_get_ramdisk - get and verify ramdisk image
 * @cmdtp: command table pointer
 * @flag: command flag
 * @argc: command argument count
 * @argv: command argument list
 * @rd_addr: ramdisk image start address
 * @arch: expected ramdisk architecture
 * @verify: checksum verification flag
 *
 * image_get_ramdisk() returns a pointer to the verified ramdisk image
 * header. Routine receives image start address and expected architecture
 * flag. Verification done covers data and header integrity and os/type/arch
 * fields checking.
 *
 * If dataflash support is enabled routine checks for dataflash addresses
 * and handles required dataflash reads.
 *
 * returns:
 *     pointer to a ramdisk image header, if image was found and valid
 *     otherwise, return NULL
 */
static image_header_t* image_get_ramdisk (cmd_tbl_t *cmdtp, int flag,
		int argc, char *argv[],
		ulong rd_addr, uint8_t arch, int verify)
{
	image_header_t *rd_hdr;

	show_boot_progress (9);
	rd_hdr = (image_header_t *)rd_addr;

	if (!image_check_magic (rd_hdr)) {
		puts ("Bad Magic Number\n");
		show_boot_progress (-10);
		return NULL;
	}

	if (!image_check_hcrc (rd_hdr)) {
		puts ("Bad Header Checksum\n");
		show_boot_progress (-11);
		return NULL;
	}

	show_boot_progress (10);
	image_print_contents (rd_hdr);

	if (verify) {
		puts("   Verifying Checksum ... ");
		if (!image_check_dcrc_wd (rd_hdr, CHUNKSZ)) {
			puts ("Bad Data CRC\n");
			show_boot_progress (-12);
			return NULL;
		}
		puts("OK\n");
	}

	show_boot_progress (11);

	if (!image_check_os (rd_hdr, IH_OS_LINUX) ||
	    !image_check_arch (rd_hdr, arch) ||
	    !image_check_type (rd_hdr, IH_TYPE_RAMDISK)) {
		printf ("No Linux %s Ramdisk Image\n",
				genimg_get_arch_name(arch));
		show_boot_progress (-13);
		return NULL;
	}

	return rd_hdr;
}

/*****************************************************************************/
/* Shared dual-format routines */
/*****************************************************************************/
int getenv_verify (void)
{
	char *s = getenv ("verify");
	return (s && (*s == 'n')) ? 0 : 1;
}

int getenv_autostart (void)
{
	char *s = getenv ("autostart");
	return (s && (*s == 'n')) ? 0 : 1;
}

ulong getenv_bootm_low(void)
{
	char *s = getenv ("bootm_low");
	if (s) {
		ulong tmp = simple_strtoul (s, NULL, 16);
		return tmp;
	}

#ifdef CFG_SDRAM_BASE
	return CFG_SDRAM_BASE;
#else
	return 0;
#endif
}

ulong getenv_bootm_size(void)
{
	char *s = getenv ("bootm_size");
	if (s) {
		ulong tmp = simple_strtoul (s, NULL, 16);
		return tmp;
	}

	return gd->bd->bi_memsize;
}

void memmove_wd (void *to, void *from, size_t len, ulong chunksz)
{
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	while (len > 0) {
		size_t tail = (len > chunksz) ? chunksz : len;
		WATCHDOG_RESET ();
		memmove (to, from, tail);
		to += tail;
		from += tail;
		len -= tail;
	}
#else	/* !(CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG) */
	memmove (to, from, len);
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */
}
#endif /* USE_HOSTCC */

const char* genimg_get_os_name (uint8_t os)
{
	const char *name;

	switch (os) {
	case IH_OS_INVALID:	name = "Invalid OS";		break;
	case IH_OS_NETBSD:	name = "NetBSD";		break;
	case IH_OS_LINUX:	name = "Linux";			break;
	case IH_OS_VXWORKS:	name = "VxWorks";		break;
	case IH_OS_QNX:		name = "QNX";			break;
	case IH_OS_U_BOOT:	name = "U-Boot";		break;
	case IH_OS_RTEMS:	name = "RTEMS";			break;
#ifdef CONFIG_ARTOS
	case IH_OS_ARTOS:	name = "ARTOS";			break;
#endif
#ifdef CONFIG_LYNXKDI
	case IH_OS_LYNXOS:	name = "LynxOS";		break;
#endif
	default:		name = "Unknown OS";		break;
	}

	return name;
}

const char* genimg_get_arch_name (uint8_t arch)
{
	const char *name;

	switch (arch) {
	case IH_ARCH_INVALID:	name = "Invalid Architecture";	break;
	case IH_ARCH_ALPHA:	name = "Alpha";			break;
	case IH_ARCH_ARM:	name = "ARM";			break;
	case IH_ARCH_AVR32:	name = "AVR32";			break;
	case IH_ARCH_BLACKFIN:	name = "Blackfin";		break;
	case IH_ARCH_I386:	name = "Intel x86";		break;
	case IH_ARCH_IA64:	name = "IA64";			break;
	case IH_ARCH_M68K:	name = "M68K"; 			break;
	case IH_ARCH_MICROBLAZE:name = "Microblaze"; 		break;
	case IH_ARCH_MIPS64:	name = "MIPS 64 Bit";		break;
	case IH_ARCH_MIPS:	name = "MIPS";			break;
	case IH_ARCH_NIOS2:	name = "Nios-II";		break;
	case IH_ARCH_NIOS:	name = "Nios";			break;
	case IH_ARCH_PPC:	name = "PowerPC";		break;
	case IH_ARCH_S390:	name = "IBM S390";		break;
	case IH_ARCH_SH:	name = "SuperH";		break;
	case IH_ARCH_SPARC64:	name = "SPARC 64 Bit";		break;
	case IH_ARCH_SPARC:	name = "SPARC";			break;
	default:		name = "Unknown Architecture";	break;
	}

	return name;
}

const char* genimg_get_type_name (uint8_t type)
{
	const char *name;

	switch (type) {
	case IH_TYPE_INVALID:	name = "Invalid Image";		break;
	case IH_TYPE_STANDALONE:name = "Standalone Program";	break;
	case IH_TYPE_KERNEL:	name = "Kernel Image";		break;
	case IH_TYPE_RAMDISK:	name = "RAMDisk Image";		break;
	case IH_TYPE_MULTI:	name = "Multi-File Image";	break;
	case IH_TYPE_FIRMWARE:	name = "Firmware";		break;
	case IH_TYPE_SCRIPT:	name = "Script";		break;
	case IH_TYPE_FLATDT:	name = "Flat Device Tree";	break;
	default:		name = "Unknown Image";		break;
	}

	return name;
}

const char* genimg_get_comp_name (uint8_t comp)
{
	const char *name;

	switch (comp) {
	case IH_COMP_NONE:	name = "uncompressed";		break;
	case IH_COMP_GZIP:	name = "gzip compressed";	break;
	case IH_COMP_BZIP2:	name = "bzip2 compressed";	break;
	default:		name = "unknown compression";	break;
	}

	return name;
}

/**
 * genimg_get_format - get image format type
 * @img_addr: image start address
 *
 * genimg_get_format() checks whether provided address points to a valid
 * legacy or FIT image.
 *
 * New uImage format and FDT blob are based on a libfdt. FDT blob
 * may be passed directly or embedded in a FIT image. In both situations
 * genimg_get_format() must be able to dectect libfdt header.
 *
 * returns:
 *     image format type or IMAGE_FORMAT_INVALID if no image is present
 */
int genimg_get_format (void *img_addr)
{
	ulong		format = IMAGE_FORMAT_INVALID;
	image_header_t	*hdr;
#if defined(CONFIG_FIT) || defined(CONFIG_OF_LIBFDT)
	char		*fit_hdr;
#endif

	hdr = (image_header_t *)img_addr;
	if (image_check_magic(hdr))
		format = IMAGE_FORMAT_LEGACY;
#if defined(CONFIG_FIT) || defined(CONFIG_OF_LIBFDT)
	else {
		fit_hdr = (char *)img_addr;
		if (fdt_check_header (fit_hdr) == 0)
			format = IMAGE_FORMAT_FIT;
	}
#endif

	return format;
}

/**
 * genimg_get_image - get image from special storage (if necessary)
 * @img_addr: image start address
 *
 * genimg_get_image() checks if provided image start adddress is located
 * in a dataflash storage. If so, image is moved to a system RAM memory.
 *
 * returns:
 *     image start address after possible relocation from special storage
 */
ulong genimg_get_image (ulong img_addr)
{
	ulong ram_addr = img_addr;

#ifdef CONFIG_HAS_DATAFLASH
	ulong h_size, d_size;

	if (addr_dataflash (img_addr)){
		/* ger RAM address */
		ram_addr = CFG_LOAD_ADDR;

		/* get header size */
		h_size = image_get_header_size ();
#if defined(CONFIG_FIT)
		if (sizeof(struct fdt_header) > h_size)
			h_size = sizeof(struct fdt_header);
#endif

		/* read in header */
		debug ("   Reading image header from dataflash address "
			"%08lx to RAM address %08lx\n", img_addr, ram_addr);

		read_dataflash (img_addr, h_size, (char *)ram_addr);

		/* get data size */
		switch (genimg_get_format ((void *)ram_addr)) {
		case IMAGE_FORMAT_LEGACY:
			d_size = image_get_data_size ((image_header_t *)ram_addr);
			debug ("   Legacy format image found at 0x%08lx, size 0x%08lx\n",
					ram_addr, d_size);
			break;
#if defined(CONFIG_FIT)
		case IMAGE_FORMAT_FIT:
			d_size = fdt_totalsize((void *)ram_addr) - h_size;
			debug ("   FIT/FDT format image found at 0x%08lx, size 0x%08lx\n",
					ram_addr, d_size);
			break;
#endif
		default:
			printf ("   No valid image found at 0x%08lx\n", img_addr);
			return ram_addr;
		}

		/* read in image data */
		debug ("   Reading image remaining data from dataflash address "
			"%08lx to RAM address %08lx\n", img_addr + h_size,
			ram_addr + h_size);

		read_dataflash (img_addr + h_size, d_size,
				(char *)(ram_addr + h_size));

	}
#endif /* CONFIG_HAS_DATAFLASH */

	return ram_addr;
}

/**
 * boot_get_ramdisk - main ramdisk handling routine
 * @cmdtp: command table pointer
 * @flag: command flag
 * @argc: command argument count
 * @argv: command argument list
 * @images: pointer to the bootm images structure
 * @arch: expected ramdisk architecture
 * @rd_start: pointer to a ulong variable, will hold ramdisk start address
 * @rd_end: pointer to a ulong variable, will hold ramdisk end
 *
 * boot_get_ramdisk() is responsible for finding a valid ramdisk image.
 * Curently supported are the following ramdisk sources:
 *      - multicomponent kernel/ramdisk image,
 *      - commandline provided address of decicated ramdisk image.
 *
 * returns:
 *     rd_start and rd_end are set to ramdisk start/end addresses if
 *     ramdisk image is found and valid
 *     rd_start and rd_end are set to 0 if no ramdisk exists
 *     return 1 if ramdisk image is found but corrupted
 */
int boot_get_ramdisk (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		bootm_headers_t *images, uint8_t arch,
		ulong *rd_start, ulong *rd_end)
{
	ulong rd_addr, rd_load;
	ulong rd_data, rd_len;
	image_header_t *rd_hdr;
#if defined(CONFIG_FIT)
	void		*fit_hdr;
	const char	*fit_uname_config = NULL;
	const char	*fit_uname_ramdisk = NULL;
	ulong		default_addr;
#endif

	/*
	 * Look for a '-' which indicates to ignore the
	 * ramdisk argument
	 */
	if ((argc >= 3) && (strcmp(argv[2], "-") ==  0)) {
		debug ("## Skipping init Ramdisk\n");
		rd_len = rd_data = 0;
	} else if (argc >= 3) {
#if defined(CONFIG_FIT)
		/*
		 * If the init ramdisk comes from the FIT image and the FIT image
		 * address is omitted in the command line argument, try to use
		 * os FIT image address or default load address.
		 */
		if (images->fit_uname_os)
			default_addr = (ulong)images->fit_hdr_os;
		else
			default_addr = load_addr;

		if (fit_parse_conf (argv[2], default_addr,
					&rd_addr, &fit_uname_config)) {
			debug ("*  ramdisk: config '%s' from image at 0x%08lx\n",
					fit_uname_config, rd_addr);
		} else if (fit_parse_subimage (argv[2], default_addr,
					&rd_addr, &fit_uname_ramdisk)) {
			debug ("*  ramdisk: subimage '%s' from image at 0x%08lx\n",
					fit_uname_ramdisk, rd_addr);
		} else
#endif
		{
			rd_addr = simple_strtoul(argv[2], NULL, 16);
			debug ("*  ramdisk: cmdline image address = 0x%08lx\n",
					rd_addr);
		}

		/* copy from dataflash if needed */
		printf ("## Loading init Ramdisk Image at %08lx ...\n",
				rd_addr);
		rd_addr = genimg_get_image (rd_addr);

		/*
		 * Check if there is an initrd image at the
		 * address provided in the second bootm argument
		 * check image type, for FIT images get FIT node.
		 */
		switch (genimg_get_format ((void *)rd_addr)) {
		case IMAGE_FORMAT_LEGACY:

			debug ("*  ramdisk: legacy format image\n");

			rd_hdr = image_get_ramdisk (cmdtp, flag, argc, argv,
						rd_addr, arch, images->verify);

			if (rd_hdr == NULL) {
				*rd_start = 0;
				*rd_end = 0;
				return 1;
			}

			rd_data = image_get_data (rd_hdr);
			rd_len = image_get_data_size (rd_hdr);
			rd_load = image_get_load (rd_hdr);
			break;
#if defined(CONFIG_FIT)
		case IMAGE_FORMAT_FIT:
			fit_hdr = (void *)rd_addr;
			debug ("*  ramdisk: FIT format image\n");
			fit_unsupported_reset ("ramdisk");
			return 1;
#endif
		default:
			printf ("Wrong Image Format for %s command\n",
					cmdtp->name);
			rd_data = rd_len = 0;
		}

#if defined(CONFIG_B2) || defined(CONFIG_EVB4510) || defined(CONFIG_ARMADILLO)
		/*
		 * We need to copy the ramdisk to SRAM to let Linux boot
		 */
		if (rd_data) {
			memmove ((void *)rd_load, (uchar *)rd_data, rd_len);
			rd_data = rd_load;
		}
#endif /* CONFIG_B2 || CONFIG_EVB4510 || CONFIG_ARMADILLO */

	} else if (images->legacy_hdr_valid &&
			image_check_type (images->legacy_hdr_os, IH_TYPE_MULTI)) {
		/*
		 * Now check if we have a legacy mult-component image,
		 * get second entry data start address and len.
		 */
		show_boot_progress (13);
		printf ("## Loading init Ramdisk from multi component "
				"Image at %08lx ...\n",
				(ulong)images->legacy_hdr_os);

		image_multi_getimg (images->legacy_hdr_os, 1, &rd_data, &rd_len);
	} else {
		/*
		 * no initrd image
		 */
		show_boot_progress (14);
		rd_len = rd_data = 0;
	}

	if (!rd_data) {
		debug ("## No init Ramdisk\n");
		*rd_start = 0;
		*rd_end = 0;
	} else {
		*rd_start = rd_data;
		*rd_end = rd_data + rd_len;
	}
	debug ("   ramdisk start = 0x%08lx, ramdisk end = 0x%08lx\n",
			*rd_start, *rd_end);

	return 0;
}

#if defined(CONFIG_PPC) || defined(CONFIG_M68K)
/**
 * boot_ramdisk_high - relocate init ramdisk
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @rd_data: ramdisk data start address
 * @rd_len: ramdisk data length
 * @initrd_start: pointer to a ulong variable, will hold final init ramdisk
 *      start address (after possible relocation)
 * @initrd_end: pointer to a ulong variable, will hold final init ramdisk
 *      end address (after possible relocation)
 *
 * boot_ramdisk_high() takes a relocation hint from "initrd_high" environement
 * variable and if requested ramdisk data is moved to a specified location.
 *
 * Initrd_start and initrd_end are set to final (after relocation) ramdisk
 * start/end addresses if ramdisk image start and len were provided,
 * otherwise set initrd_start and initrd_end set to zeros.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_ramdisk_high (struct lmb *lmb, ulong rd_data, ulong rd_len,
		  ulong *initrd_start, ulong *initrd_end)
{
	char	*s;
	ulong	initrd_high;
	int	initrd_copy_to_ram = 1;

	if ((s = getenv ("initrd_high")) != NULL) {
		/* a value of "no" or a similar string will act like 0,
		 * turning the "load high" feature off. This is intentional.
		 */
		initrd_high = simple_strtoul (s, NULL, 16);
		if (initrd_high == ~0)
			initrd_copy_to_ram = 0;
	} else {
		/* not set, no restrictions to load high */
		initrd_high = ~0;
	}

	debug ("## initrd_high = 0x%08lx, copy_to_ram = %d\n",
			initrd_high, initrd_copy_to_ram);

	if (rd_data) {
		if (!initrd_copy_to_ram) {	/* zero-copy ramdisk support */
			debug ("   in-place initrd\n");
			*initrd_start = rd_data;
			*initrd_end = rd_data + rd_len;
			lmb_reserve(lmb, rd_data, rd_len);
		} else {
			if (initrd_high)
				*initrd_start = lmb_alloc_base (lmb, rd_len, 0x1000, initrd_high);
			else
				*initrd_start = lmb_alloc (lmb, rd_len, 0x1000);

			if (*initrd_start == 0) {
				puts ("ramdisk - allocation error\n");
				goto error;
			}
			show_boot_progress (12);

			*initrd_end = *initrd_start + rd_len;
			printf ("   Loading Ramdisk to %08lx, end %08lx ... ",
					*initrd_start, *initrd_end);

			memmove_wd ((void *)*initrd_start,
					(void *)rd_data, rd_len, CHUNKSZ);

			puts ("OK\n");
		}
	} else {
		*initrd_start = 0;
		*initrd_end = 0;
	}
	debug ("   ramdisk load start = 0x%08lx, ramdisk load end = 0x%08lx\n",
			*initrd_start, *initrd_end);

	return 0;

error:
	return -1;
}

/**
 * boot_get_cmdline - allocate and initialize kernel cmdline
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @cmd_start: pointer to a ulong variable, will hold cmdline start
 * @cmd_end: pointer to a ulong variable, will hold cmdline end
 * @bootmap_base: ulong variable, holds offset in physical memory to
 * base of bootmap
 *
 * boot_get_cmdline() allocates space for kernel command line below
 * BOOTMAPSZ + bootmap_base address. If "bootargs" U-boot environemnt
 * variable is present its contents is copied to allocated kernel
 * command line.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_get_cmdline (struct lmb *lmb, ulong *cmd_start, ulong *cmd_end,
			ulong bootmap_base)
{
	char *cmdline;
	char *s;

	cmdline = (char *)lmb_alloc_base(lmb, CFG_BARGSIZE, 0xf,
					 CFG_BOOTMAPSZ + bootmap_base);

	if (cmdline == NULL)
		return -1;

	if ((s = getenv("bootargs")) == NULL)
		s = "";

	strcpy(cmdline, s);

	*cmd_start = (ulong) & cmdline[0];
	*cmd_end = *cmd_start + strlen(cmdline);

	debug ("## cmdline at 0x%08lx ... 0x%08lx\n", *cmd_start, *cmd_end);

	return 0;
}

/**
 * boot_get_kbd - allocate and initialize kernel copy of board info
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @kbd: double pointer to board info data
 * @bootmap_base: ulong variable, holds offset in physical memory to
 * base of bootmap
 *
 * boot_get_kbd() allocates space for kernel copy of board info data below
 * BOOTMAPSZ + bootmap_base address and kernel board info is initialized with
 * the current u-boot board info data.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_get_kbd (struct lmb *lmb, bd_t **kbd, ulong bootmap_base)
{
	*kbd = (bd_t *)lmb_alloc_base(lmb, sizeof(bd_t), 0xf,
				      CFG_BOOTMAPSZ + bootmap_base);
	if (*kbd == NULL)
		return -1;

	**kbd = *(gd->bd);

	debug ("## kernel board info at 0x%08lx\n", (ulong)*kbd);

#if defined(DEBUG) && defined(CONFIG_CMD_BDI)
	do_bdinfo(NULL, 0, 0, NULL);
#endif

	return 0;
}
#endif /* CONFIG_PPC || CONFIG_M68K */

#if defined(CONFIG_FIT)
/*****************************************************************************/
/* New uImage format routines */
/*****************************************************************************/
static int fit_parse_spec (const char *spec, char sepc, ulong addr_curr,
		ulong *addr, const char **name)
{
	const char *sep;

	*addr = addr_curr;
	*name = NULL;

	sep = strchr (spec, sepc);
	if (sep) {
		if (sep - spec > 0)
			*addr = simple_strtoul (spec, NULL, 16);

		*name = sep + 1;
		return 1;
	}

	return 0;
}

/**
 * fit_parse_conf - parse FIT configuration spec
 * @spec: input string, containing configuration spec
 * @add_curr: current image address (to be used as a possible default)
 * @addr: pointer to a ulong variable, will hold FIT image address of a given
 * configuration
 * @conf_name double pointer to a char, will hold pointer to a configuration
 * unit name
 *
 * fit_parse_conf() expects configuration spec in the for of [<addr>]#<conf>,
 * where <addr> is a FIT image address that contains configuration
 * with a <conf> unit name.
 *
 * Address part is optional, and if omitted default add_curr will
 * be used instead.
 *
 * returns:
 *     1 if spec is a valid configuration string,
 *     addr and conf_name are set accordingly
 *     0 otherwise
 */
inline int fit_parse_conf (const char *spec, ulong addr_curr,
		ulong *addr, const char **conf_name)
{
	return fit_parse_spec (spec, '#', addr_curr, addr, conf_name);
}

/**
 * fit_parse_subimage - parse FIT subimage spec
 * @spec: input string, containing subimage spec
 * @add_curr: current image address (to be used as a possible default)
 * @addr: pointer to a ulong variable, will hold FIT image address of a given
 * subimage
 * @image_name: double pointer to a char, will hold pointer to a subimage name
 *
 * fit_parse_subimage() expects subimage spec in the for of
 * [<addr>]:<subimage>, where <addr> is a FIT image address that contains
 * subimage with a <subimg> unit name.
 *
 * Address part is optional, and if omitted default add_curr will
 * be used instead.
 *
 * returns:
 *     1 if spec is a valid subimage string,
 *     addr and image_name are set accordingly
 *     0 otherwise
 */
inline int fit_parse_subimage (const char *spec, ulong addr_curr,
		ulong *addr, const char **image_name)
{
	return fit_parse_spec (spec, ':', addr_curr, addr, image_name);
}

#endif /* CONFIG_FIT */

#endif /* USE_HOSTCC */
