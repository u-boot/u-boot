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

#include <image.h>

#if defined(CONFIG_FIT)
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <sha1.h>
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
#include <time.h>
#include <image.h>
#endif /* !USE_HOSTCC*/

typedef struct table_entry {
	int	id;		/* as defined in image.h	*/
	char	*sname;		/* short (input) name		*/
	char	*lname;		/* long (output) name		*/
} table_entry_t;

static table_entry_t uimage_arch[] = {
	{	IH_ARCH_INVALID,	NULL,		"Invalid ARCH",	},
	{	IH_ARCH_ALPHA,		"alpha",	"Alpha",	},
	{	IH_ARCH_ARM,		"arm",		"ARM",		},
	{	IH_ARCH_I386,		"x86",		"Intel x86",	},
	{	IH_ARCH_IA64,		"ia64",		"IA64",		},
	{	IH_ARCH_M68K,		"m68k",		"M68K",		},
	{	IH_ARCH_MICROBLAZE,	"microblaze",	"MicroBlaze",	},
	{	IH_ARCH_MIPS,		"mips",		"MIPS",		},
	{	IH_ARCH_MIPS64,		"mips64",	"MIPS 64 Bit",	},
	{	IH_ARCH_NIOS,		"nios",		"NIOS",		},
	{	IH_ARCH_NIOS2,		"nios2",	"NIOS II",	},
	{	IH_ARCH_PPC,		"ppc",		"PowerPC",	},
	{	IH_ARCH_S390,		"s390",		"IBM S390",	},
	{	IH_ARCH_SH,		"sh",		"SuperH",	},
	{	IH_ARCH_SPARC,		"sparc",	"SPARC",	},
	{	IH_ARCH_SPARC64,	"sparc64",	"SPARC 64 Bit",	},
	{	IH_ARCH_BLACKFIN,	"blackfin",	"Blackfin",	},
	{	IH_ARCH_AVR32,		"avr32",	"AVR32",	},
	{	-1,			"",		"",		},
};

static table_entry_t uimage_os[] = {
	{	IH_OS_INVALID,	NULL,		"Invalid OS",		},
#if defined(CONFIG_ARTOS) || defined(USE_HOSTCC)
	{	IH_OS_ARTOS,	"artos",	"ARTOS",		},
#endif
	{	IH_OS_LINUX,	"linux",	"Linux",		},
#if defined(CONFIG_LYNXKDI) || defined(USE_HOSTCC)
	{	IH_OS_LYNXOS,	"lynxos",	"LynxOS",		},
#endif
	{	IH_OS_NETBSD,	"netbsd",	"NetBSD",		},
	{	IH_OS_RTEMS,	"rtems",	"RTEMS",		},
	{	IH_OS_U_BOOT,	"u-boot",	"U-Boot",		},
#if defined(CONFIG_CMD_ELF) || defined(USE_HOSTCC)
	{	IH_OS_QNX,	"qnx",		"QNX",			},
	{	IH_OS_VXWORKS,	"vxworks",	"VxWorks",		},
#endif
#ifdef USE_HOSTCC
	{	IH_OS_4_4BSD,	"4_4bsd",	"4_4BSD",		},
	{	IH_OS_DELL,	"dell",		"Dell",			},
	{	IH_OS_ESIX,	"esix",		"Esix",			},
	{	IH_OS_FREEBSD,	"freebsd",	"FreeBSD",		},
	{	IH_OS_IRIX,	"irix",		"Irix",			},
	{	IH_OS_NCR,	"ncr",		"NCR",			},
	{	IH_OS_OPENBSD,	"openbsd",	"OpenBSD",		},
	{	IH_OS_PSOS,	"psos",		"pSOS",			},
	{	IH_OS_SCO,	"sco",		"SCO",			},
	{	IH_OS_SOLARIS,	"solaris",	"Solaris",		},
	{	IH_OS_SVR4,	"svr4",		"SVR4",			},
#endif
	{	-1,		"",		"",			},
};

static table_entry_t uimage_type[] = {
	{	IH_TYPE_INVALID,    NULL,	  "Invalid Image",	},
	{	IH_TYPE_FILESYSTEM, "filesystem", "Filesystem Image",	},
	{	IH_TYPE_FIRMWARE,   "firmware",	  "Firmware",		},
	{	IH_TYPE_KERNEL,	    "kernel",	  "Kernel Image",	},
	{	IH_TYPE_MULTI,	    "multi",	  "Multi-File Image",	},
	{	IH_TYPE_RAMDISK,    "ramdisk",	  "RAMDisk Image",	},
	{	IH_TYPE_SCRIPT,     "script",	  "Script",		},
	{	IH_TYPE_STANDALONE, "standalone", "Standalone Program", },
	{	IH_TYPE_FLATDT,     "flat_dt",    "Flat Device Tree",	},
	{	-1,		    "",		  "",			},
};

static table_entry_t uimage_comp[] = {
	{	IH_COMP_NONE,	"none",		"uncompressed",		},
	{	IH_COMP_BZIP2,	"bzip2",	"bzip2 compressed",	},
	{	IH_COMP_GZIP,	"gzip",		"gzip compressed",	},
	{	-1,		"",		"",			},
};

unsigned long crc32 (unsigned long, const unsigned char *, unsigned int);
static void genimg_print_size (uint32_t size);
#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || defined(USE_HOSTCC)
static void genimg_print_time (time_t timestamp);
#endif

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
#endif /* !USE_HOSTCC */

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
	uint32_t *size;

	/* get start of the image payload, which in case of multi
	 * component images that points to a table of component sizes */
	size = (uint32_t *)image_get_data (hdr);

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
	uint32_t *size;
	ulong offset, tail, count, img_data;

	/* get number of component */
	count = image_multi_count (hdr);

	/* get start of the image payload, which in case of multi
	 * component images that points to a table of component sizes */
	size = (uint32_t *)image_get_data (hdr);

	/* get address of the proper component data start, which means
	 * skipping sizes table (add 1 for last, null entry) */
	img_data = image_get_data (hdr) + (count + 1) * sizeof (uint32_t);

	if (idx < count) {
		*len = uimage_to_cpu (size[idx]);
		offset = 0;
		tail = 0;

		/* go over all indices preceding requested component idx */
		for (i = 0; i < idx; i++) {
			/* add up i-th component size */
			offset += uimage_to_cpu (size[i]);

			/* add up alignment for i-th component */
			tail += (4 - uimage_to_cpu (size[i]) % 4);
		}

		/* calculate idx-th component data address */
		*data = img_data + offset + tail;
	} else {
		*len = 0;
		*data = 0;
	}
}

static void image_print_type (image_header_t *hdr)
{
	const char *os, *arch, *type, *comp;

	os = genimg_get_os_name (image_get_os (hdr));
	arch = genimg_get_arch_name (image_get_arch (hdr));
	type = genimg_get_type_name (image_get_type (hdr));
	comp = genimg_get_comp_name (image_get_comp (hdr));

	printf ("%s %s %s (%s)\n", arch, os, type, comp);
}

/**
 * __image_print_contents - prints out the contents of the legacy format image
 * @hdr: pointer to the legacy format image header
 * @p: pointer to prefix string
 *
 * __image_print_contents() formats a multi line legacy image contents description.
 * The routine prints out all header fields followed by the size/offset data
 * for MULTI/SCRIPT images.
 *
 * returns:
 *     no returned results
 */
static void __image_print_contents (image_header_t *hdr, const char *p)
{
	printf ("%sImage Name:   %.*s\n", p, IH_NMLEN, image_get_name (hdr));
#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || defined(USE_HOSTCC)
	printf ("%sCreated:      ", p);
	genimg_print_time ((time_t)image_get_time (hdr));
#endif
	printf ("%sImage Type:   ", p);
	image_print_type (hdr);
	printf ("%sData Size:    ", p);
	genimg_print_size (image_get_data_size (hdr));
	printf ("%sLoad Address: %08x\n", p, image_get_load (hdr));
	printf ("%sEntry Point:  %08x\n", p, image_get_ep (hdr));

	if (image_check_type (hdr, IH_TYPE_MULTI) ||
			image_check_type (hdr, IH_TYPE_SCRIPT)) {
		int i;
		ulong data, len;
		ulong count = image_multi_count (hdr);

		printf ("%sContents:\n", p);
		for (i = 0; i < count; i++) {
			image_multi_getimg (hdr, i, &data, &len);

			printf ("%s   Image %d: ", p, i);
			genimg_print_size (len);

			if (image_check_type (hdr, IH_TYPE_SCRIPT) && i > 0) {
				/*
				 * the user may need to know offsets
				 * if planning to do something with
				 * multiple files
				 */
				printf ("%s    Offset = 0x%08lx\n", p, data);
			}
		}
	}
}

inline void image_print_contents (image_header_t *hdr)
{
	__image_print_contents (hdr, "   ");
}

inline void image_print_contents_noindent (image_header_t *hdr)
{
	__image_print_contents (hdr, "");
}

#ifndef USE_HOSTCC
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
#endif /* !USE_HOSTCC */

/*****************************************************************************/
/* Shared dual-format routines */
/*****************************************************************************/
#ifndef USE_HOSTCC
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
#endif /* !USE_HOSTCC */

static void genimg_print_size (uint32_t size)
{
#ifndef USE_HOSTCC
	printf ("%d Bytes = ", size);
	print_size (size, "\n");
#else
	printf ("%d Bytes = %.2f kB = %.2f MB\n",
			size, (double)size / 1.024e3,
			(double)size / 1.048576e6);
#endif
}

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || defined(USE_HOSTCC)
static void genimg_print_time (time_t timestamp)
{
#ifndef USE_HOSTCC
	struct rtc_time tm;

	to_tm (timestamp, &tm);
	printf ("%4d-%02d-%02d  %2d:%02d:%02d UTC\n",
			tm.tm_year, tm.tm_mon, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
#else
	printf ("%s", ctime(&timestamp));
#endif
}
#endif /* CONFIG_TIMESTAMP || CONFIG_CMD_DATE || USE_HOSTCC */

/**
 * get_table_entry_name - translate entry id to long name
 * @table: pointer to a translation table for entries of a specific type
 * @msg: message to be returned when translation fails
 * @id: entry id to be translated
 *
 * get_table_entry_name() will go over translation table trying to find
 * entry that matches given id. If matching entry is found, its long
 * name is returned to the caller.
 *
 * returns:
 *     long entry name if translation succeeds
 *     msg otherwise
 */
static char *get_table_entry_name (table_entry_t *table, char *msg, int id)
{
	for (; table->id >= 0; ++table) {
		if (table->id == id)
			return (table->lname);
	}
	return (msg);
}

const char *genimg_get_os_name (uint8_t os)
{
	return (get_table_entry_name (uimage_os, "Unknown OS", os));
}

const char *genimg_get_arch_name (uint8_t arch)
{
	return (get_table_entry_name (uimage_arch, "Unknown Architecture", arch));
}

const char *genimg_get_type_name (uint8_t type)
{
	return (get_table_entry_name (uimage_type, "Unknown Image", type));
}

const char *genimg_get_comp_name (uint8_t comp)
{
	return (get_table_entry_name (uimage_comp, "Unknown Compression", comp));
}

/**
 * get_table_entry_id - translate short entry name to id
 * @table: pointer to a translation table for entries of a specific type
 * @table_name: to be used in case of error
 * @name: entry short name to be translated
 *
 * get_table_entry_id() will go over translation table trying to find
 * entry that matches given short name. If matching entry is found,
 * its id returned to the caller.
 *
 * returns:
 *     entry id if translation succeeds
 *     -1 otherwise
 */
static int get_table_entry_id (table_entry_t *table,
		const char *table_name, const char *name)
{
	table_entry_t *t;
#ifdef USE_HOSTCC
	int first = 1;

	for (t = table; t->id >= 0; ++t) {
		if (t->sname && strcasecmp(t->sname, name) == 0)
			return (t->id);
	}

	fprintf (stderr, "\nInvalid %s Type - valid names are", table_name);
	for (t = table; t->id >= 0; ++t) {
		if (t->sname == NULL)
			continue;
		fprintf (stderr, "%c %s", (first) ? ':' : ',', t->sname);
		first = 0;
	}
	fprintf (stderr, "\n");
#else
	for (t = table; t->id >= 0; ++t) {
		if (t->sname && strcmp(t->sname, name) == 0)
			return (t->id);
	}
	debug ("Invalid %s Type: %s\n", table_name, name);
#endif /* USE_HOSTCC */
	return (-1);
}

int genimg_get_os_id (const char *name)
{
	return (get_table_entry_id (uimage_os, "OS", name));
}

int genimg_get_arch_id (const char *name)
{
	return (get_table_entry_id (uimage_arch, "CPU", name));
}

int genimg_get_type_id (const char *name)
{
	return (get_table_entry_id (uimage_type, "Image", name));
}

int genimg_get_comp_id (const char *name)
{
	return (get_table_entry_id (uimage_comp, "Compression", name));
}

#ifndef USE_HOSTCC
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
			d_size = fit_get_size ((const void *)ram_addr) - h_size;
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
#endif /* !USE_HOSTCC */

#if defined(CONFIG_FIT)
/*****************************************************************************/
/* New uImage format routines */
/*****************************************************************************/
#ifndef USE_HOSTCC
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
#endif /* !USE_HOSTCC */

static void fit_get_debug (const void *fit, int noffset,
		char *prop_name, int err)
{
	debug ("Can't get '%s' property from FIT 0x%08lx, "
		"node: offset %d, name %s (%s)\n",
		prop_name, (ulong)fit, noffset,
		fit_get_name (fit, noffset, NULL),
		fdt_strerror (err));
}

/**
 * __fit_print_contents - prints out the contents of the FIT format image
 * @fit: pointer to the FIT format image header
 * @p: pointer to prefix string
 *
 * __fit_print_contents() formats a multi line FIT image contents description.
 * The routine prints out FIT image properties (root node level) follwed by
 * the details of each component image.
 *
 * returns:
 *     no returned results
 */
static void __fit_print_contents (const void *fit, const char *p)
{
	char *desc;
	char *uname;
	int images_noffset;
	int confs_noffset;
	int noffset;
	int ndepth;
	int count = 0;
	int ret;
#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || defined(USE_HOSTCC)
	time_t timestamp;
#endif

	/* Root node properties */
	ret = fit_get_desc (fit, 0, &desc);
	printf ("%sFIT description: ", p);
	if (ret)
		printf ("unavailable\n");
	else
		printf ("%s\n", desc);

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || defined(USE_HOSTCC)
	ret = fit_get_timestamp (fit, 0, &timestamp);
	printf ("%sCreated:         ", p);
	if (ret)
		printf ("unavailable\n");
	else
		genimg_print_time (timestamp);
#endif

	/* Find images parent node offset */
	images_noffset = fdt_path_offset (fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf ("Can't find images parent node '%s' (%s)\n",
			FIT_IMAGES_PATH, fdt_strerror (images_noffset));
		return;
	}

	/* Process its subnodes, print out component images details */
	for (ndepth = 0, count = 0, noffset = fdt_next_node (fit, images_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node (fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			printf ("%s Image %u (%s)\n", p, count++,
					fit_get_name(fit, noffset, NULL));

			fit_image_print (fit, noffset, p);
		}
	}

	/* Find configurations parent node offset */
	confs_noffset = fdt_path_offset (fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		debug ("Can't get configurations parent node '%s' (%s)\n",
			FIT_CONFS_PATH, fdt_strerror (confs_noffset));
		return;
	}

	/* get default configuration unit name from default property */
	uname = (char *)fdt_getprop (fit, noffset, FIT_DEFAULT_PROP, NULL);
	if (uname)
		printf ("%s Default Configuration: '%s'\n", p, uname);

	/* Process its subnodes, print out configurations details */
	for (ndepth = 0, count = 0, noffset = fdt_next_node (fit, confs_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node (fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the configurations parent node,
			 * i.e. configuration node.
			 */
			printf ("%s Configuration %u (%s)\n", p, count++,
					fit_get_name(fit, noffset, NULL));

			fit_conf_print (fit, noffset, p);
		}
	}
}

inline void fit_print_contents (const void *fit)
{
	__fit_print_contents (fit, "   ");
}

inline void fit_print_contents_noindent (const void *fit)
{
	__fit_print_contents (fit, "");
}

/**
 * fit_image_print - prints out the FIT component image details
 * @fit: pointer to the FIT format image header
 * @image_noffset: offset of the component image node
 * @p: pointer to prefix string
 *
 * fit_image_print() lists all mandatory properies for the processed component
 * image. If present, hash nodes are printed out as well.
 *
 * returns:
 *     no returned results
 */
void fit_image_print (const void *fit, int image_noffset, const char *p)
{
	char *desc;
	uint8_t type, arch, os, comp;
	size_t size;
	ulong load, entry;
	const void *data;
	int noffset;
	int ndepth;
	int ret;

	/* Mandatory properties */
	ret = fit_get_desc (fit, image_noffset, &desc);
	printf ("%s  Description:  ", p);
	if (ret)
		printf ("unavailable\n");
	else
		printf ("%s\n", desc);

	fit_image_get_type (fit, image_noffset, &type);
	printf ("%s  Type:         %s\n", p, genimg_get_type_name (type));

	fit_image_get_comp (fit, image_noffset, &comp);
	printf ("%s  Compression:  %s\n", p, genimg_get_comp_name (comp));

	ret = fit_image_get_data (fit, image_noffset, &data, &size);

#ifndef USE_HOSTCC
	printf ("%s  Data Start:   ", p);
	if (ret)
		printf ("unavailable\n");
	else
		printf ("0x%08lx\n", (ulong)data);
#endif

	printf ("%s  Data Size:    ", p);
	if (ret)
		printf ("unavailable\n");
	else
		genimg_print_size (size);

	/* Remaining, type dependent properties */
	if ((type == IH_TYPE_KERNEL) || (type == IH_TYPE_STANDALONE) ||
	    (type == IH_TYPE_RAMDISK) || (type == IH_TYPE_FIRMWARE) ||
	    (type == IH_TYPE_FLATDT)) {
		fit_image_get_arch (fit, image_noffset, &arch);
		printf ("%s  Architecture: %s\n", p, genimg_get_arch_name (arch));
	}

	if (type == IH_TYPE_KERNEL) {
		fit_image_get_os (fit, image_noffset, &os);
		printf ("%s  OS:           %s\n", p, genimg_get_os_name (os));
	}

	if ((type == IH_TYPE_KERNEL) || (type == IH_TYPE_STANDALONE)) {
		ret = fit_image_get_load (fit, image_noffset, &load);
		printf ("%s  Load Address: ", p);
		if (ret)
			printf ("unavailable\n");
		else
			printf ("0x%08lx\n", load);

		fit_image_get_entry (fit, image_noffset, &entry);
		printf ("%s  Entry Point:  ", p);
		if (ret)
			printf ("unavailable\n");
		else
			printf ("0x%08lx\n", entry);
	}

	/* Process all hash subnodes of the component image node */
	for (ndepth = 0, noffset = fdt_next_node (fit, image_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node (fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/* Direct child node of the component image node */
			fit_image_print_hash (fit, noffset, p);
		}
	}
}

/**
 * fit_image_print_hash - prints out the hash node details
 * @fit: pointer to the FIT format image header
 * @noffset: offset of the hash node
 * @p: pointer to prefix string
 *
 * fit_image_print_hash() lists properies for the processed hash node
 *
 * returns:
 *     no returned results
 */
void fit_image_print_hash (const void *fit, int noffset, const char *p)
{
	char *algo;
	uint8_t *value;
	int value_len;
	int i, ret;

	/*
	 * Check subnode name, must be equal to "hash".
	 * Multiple hash nodes require unique unit node
	 * names, e.g. hash@1, hash@2, etc.
	 */
	if (strncmp (fit_get_name(fit, noffset, NULL),
			FIT_HASH_NODENAME,
			strlen(FIT_HASH_NODENAME)) != 0)
		return;

	debug ("%s  Hash node:    '%s'\n", p,
			fit_get_name (fit, noffset, NULL));

	printf ("%s  Hash algo:    ", p);
	if (fit_image_hash_get_algo (fit, noffset, &algo)) {
		printf ("invalid/unsupported\n");
		return;
	}
	printf ("%s\n", algo);

	ret = fit_image_hash_get_value (fit, noffset, &value,
					&value_len);
	printf ("%s  Hash value:   ", p);
	if (ret) {
		printf ("unavailable\n");
	} else {
		for (i = 0; i < value_len; i++)
			printf ("%02x", value[i]);
		printf ("\n");
	}

	debug  ("%s  Hash len:     %d\n", p, value_len);
}

/**
 * fit_get_desc - get node description property
 * @fit: pointer to the FIT format image header
 * @noffset: node offset
 * @desc: double pointer to the char, will hold pointer to the descrption
 *
 * fit_get_desc() reads description property from a given node, if
 * description is found pointer to it is returened in third call argument.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_get_desc (const void *fit, int noffset, char **desc)
{
	int len;

	*desc = (char *)fdt_getprop (fit, noffset, FIT_DESC_PROP, &len);
	if (*desc == NULL) {
		fit_get_debug (fit, noffset, FIT_DESC_PROP, len);
		return -1;
	}

	return 0;
}

/**
 * fit_get_timestamp - get node timestamp property
 * @fit: pointer to the FIT format image header
 * @noffset: node offset
 * @timestamp: pointer to the time_t, will hold read timestamp
 *
 * fit_get_timestamp() reads timestamp poperty from given node, if timestamp
 * is found and has a correct size its value is retured in third call
 * argument.
 *
 * returns:
 *     0, on success
 *     -1, on property read failure
 *     -2, on wrong timestamp size
 */
int fit_get_timestamp (const void *fit, int noffset, time_t *timestamp)
{
	int len;
	const void *data;

	data = fdt_getprop (fit, noffset, FIT_TIMESTAMP_PROP, &len);
	if (data == NULL) {
		fit_get_debug (fit, noffset, FIT_TIMESTAMP_PROP, len);
		return -1;
	}
	if (len != sizeof (uint32_t)) {
		debug ("FIT timestamp with incorrect size of (%u)\n", len);
		return -2;
	}

	*timestamp = uimage_to_cpu (*((uint32_t *)data));
	return 0;
}

/**
 * fit_image_get_node - get node offset for component image of a given unit name
 * @fit: pointer to the FIT format image header
 * @image_uname: component image node unit name
 *
 * fit_image_get_node() finds a component image (withing the '/images'
 * node) of a provided unit name. If image is found its node offset is
 * returned to the caller.
 *
 * returns:
 *     image node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_image_get_node (const void *fit, const char *image_uname)
{
	int noffset, images_noffset;

	images_noffset = fdt_path_offset (fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		debug ("Can't find images parent node '%s' (%s)\n",
			FIT_IMAGES_PATH, fdt_strerror (images_noffset));
		return images_noffset;
	}

	noffset = fdt_subnode_offset (fit, images_noffset, image_uname);
	if (noffset < 0) {
		debug ("Can't get node offset for image unit name: '%s' (%s)\n",
			image_uname, fdt_strerror (noffset));
	}

	return noffset;
}

/**
 * fit_image_get_os - get os id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @os: pointer to the uint8_t, will hold os numeric id
 *
 * fit_image_get_os() finds os property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_os (const void *fit, int noffset, uint8_t *os)
{
	int len;
	const void *data;

	/* Get OS name from property data */
	data = fdt_getprop (fit, noffset, FIT_OS_PROP, &len);
	if (data == NULL) {
		fit_get_debug (fit, noffset, FIT_OS_PROP, len);
		*os = -1;
		return -1;
	}

	/* Translate OS name to id */
	*os = genimg_get_os_id (data);
	return 0;
}

/**
 * fit_image_get_arch - get arch id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @arch: pointer to the uint8_t, will hold arch numeric id
 *
 * fit_image_get_arch() finds arch property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_arch (const void *fit, int noffset, uint8_t *arch)
{
	int len;
	const void *data;

	/* Get architecture name from property data */
	data = fdt_getprop (fit, noffset, FIT_ARCH_PROP, &len);
	if (data == NULL) {
		fit_get_debug (fit, noffset, FIT_ARCH_PROP, len);
		*arch = -1;
		return -1;
	}

	/* Translate architecture name to id */
	*arch = genimg_get_arch_id (data);
	return 0;
}

/**
 * fit_image_get_type - get type id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @type: pointer to the uint8_t, will hold type numeric id
 *
 * fit_image_get_type() finds type property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_type (const void *fit, int noffset, uint8_t *type)
{
	int len;
	const void *data;

	/* Get image type name from property data */
	data = fdt_getprop (fit, noffset, FIT_TYPE_PROP, &len);
	if (data == NULL) {
		fit_get_debug (fit, noffset, FIT_TYPE_PROP, len);
		*type = -1;
		return -1;
	}

	/* Translate image type name to id */
	*type = genimg_get_type_id (data);
	return 0;
}

/**
 * fit_image_get_comp - get comp id for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @comp: pointer to the uint8_t, will hold comp numeric id
 *
 * fit_image_get_comp() finds comp property in a given component image node.
 * If the property is found, its (string) value is translated to the numeric
 * id which is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_comp (const void *fit, int noffset, uint8_t *comp)
{
	int len;
	const void *data;

	/* Get compression name from property data */
	data = fdt_getprop (fit, noffset, FIT_COMP_PROP, &len);
	if (data == NULL) {
		fit_get_debug (fit, noffset, FIT_COMP_PROP, len);
		*comp = -1;
		return -1;
	}

	/* Translate compression name to id */
	*comp = genimg_get_comp_id (data);
	return 0;
}

/**
 * fit_image_get_load - get load address property for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @load: pointer to the uint32_t, will hold load address
 *
 * fit_image_get_load() finds load address property in a given component image node.
 * If the property is found, its value is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_load (const void *fit, int noffset, ulong *load)
{
	int len;
	const uint32_t *data;

	data = fdt_getprop (fit, noffset, FIT_LOAD_PROP, &len);
	if (data == NULL) {
		fit_get_debug (fit, noffset, FIT_LOAD_PROP, len);
		return -1;
	}

	*load = uimage_to_cpu (*data);
	return 0;
}

/**
 * fit_image_get_entry - get entry point address property for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @entry: pointer to the uint32_t, will hold entry point address
 *
 * fit_image_get_entry() finds entry point address property in a given component image node.
 * If the property is found, its value is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_entry (const void *fit, int noffset, ulong *entry)
{
	int len;
	const uint32_t *data;

	data = fdt_getprop (fit, noffset, FIT_ENTRY_PROP, &len);
	if (data == NULL) {
		fit_get_debug (fit, noffset, FIT_ENTRY_PROP, len);
		return -1;
	}

	*entry = uimage_to_cpu (*data);
	return 0;
}

/**
 * fit_image_get_data - get data property and its size for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @data: double pointer to void, will hold data property's data address
 * @size: pointer to size_t, will hold data property's data size
 *
 * fit_image_get_data() finds data property in a given component image node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_data (const void *fit, int noffset,
		const void **data, size_t *size)
{
	int len;

	*data = fdt_getprop (fit, noffset, FIT_DATA_PROP, &len);
	if (*data == NULL) {
		fit_get_debug (fit, noffset, FIT_DATA_PROP, len);
		*size = 0;
		return -1;
	}

	*size = len;
	return 0;
}

/**
 * fit_image_hash_get_algo - get hash algorithm name
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @algo: double pointer to char, will hold pointer to the algorithm name
 *
 * fit_image_hash_get_algo() finds hash algorithm property in a given hash node.
 * If the property is found its data start address is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_algo (const void *fit, int noffset, char **algo)
{
	int len;

	*algo = (char *)fdt_getprop (fit, noffset, FIT_ALGO_PROP, &len);
	if (*algo == NULL) {
		fit_get_debug (fit, noffset, FIT_ALGO_PROP, len);
		return -1;
	}

	return 0;
}

/**
 * fit_image_hash_get_value - get hash value and length
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: double pointer to uint8_t, will hold address of a hash value data
 * @value_len: pointer to an int, will hold hash data length
 *
 * fit_image_hash_get_value() finds hash value property in a given hash node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_value (const void *fit, int noffset, uint8_t **value,
				int *value_len)
{
	int len;

	*value = (uint8_t *)fdt_getprop (fit, noffset, FIT_VALUE_PROP, &len);
	if (*value == NULL) {
		fit_get_debug (fit, noffset, FIT_VALUE_PROP, len);
		*value_len = 0;
		return -1;
	}

	*value_len = len;
	return 0;
}

/**
 * fit_set_timestamp - set node timestamp property
 * @fit: pointer to the FIT format image header
 * @noffset: node offset
 * @timestamp: timestamp value to be set
 *
 * fit_set_timestamp() attempts to set timestamp property in the requested
 * node and returns operation status to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on property read failure
 */
int fit_set_timestamp (void *fit, int noffset, time_t timestamp)
{
	uint32_t t;
	int ret;

	t = cpu_to_uimage (timestamp);
	ret = fdt_setprop (fit, noffset, FIT_TIMESTAMP_PROP, &t,
				sizeof (uint32_t));
	if (ret) {
		printf ("Can't set '%s' property for '%s' node (%s)\n",
			FIT_TIMESTAMP_PROP, fit_get_name (fit, noffset, NULL),
			fdt_strerror (ret));
		return -1;
	}

	return 0;
}

/**
 * calculate_hash - calculate and return hash for provided input data
 * @data: pointer to the input data
 * @data_len: data length
 * @algo: requested hash algorithm
 * @value: pointer to the char, will hold hash value data (caller must
 * allocate enough free space)
 * value_len: length of the calculated hash
 *
 * calculate_hash() computes input data hash according to the requested algorithm.
 * Resulting hash value is placed in caller provided 'value' buffer, length
 * of the calculated hash is returned via value_len pointer argument.
 *
 * returns:
 *     0, on success
 *    -1, when algo is unsupported
 */
static int calculate_hash (const void *data, int data_len, const char *algo,
			uint8_t *value, int *value_len)
{
	if (strcmp (algo, "crc32") == 0 ) {
		*((uint32_t *)value) = crc32 (0, data, data_len);
		*((uint32_t *)value) = cpu_to_uimage (*((uint32_t *)value));
		*value_len = 4;
	} else if (strcmp (algo, "sha1") == 0 ) {
		sha1_csum ((unsigned char *) data, data_len,
				(unsigned char *) value);
		*value_len = 20;
	} else if (strcmp (algo, "md5") == 0 ) {
		printf ("MD5 not supported\n");
		*value_len = 0;
	} else {
		debug ("Unsupported hash alogrithm\n");
		return -1;
	}
	return 0;
}

#ifdef USE_HOSTCC
/**
 * fit_set_hashes - process FIT component image nodes and calculate hashes
 * @fit: pointer to the FIT format image header
 *
 * fit_set_hashes() adds hash values for all component images in the FIT blob.
 * Hashes are calculated for all component images which have hash subnodes
 * with algorithm property set to one of the supported hash algorithms.
 *
 * returns
 *     0, on success
 *     libfdt error code, on failure
 */
int fit_set_hashes (void *fit)
{
	int images_noffset;
	int noffset;
	int ndepth;
	int ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset (fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf ("Can't find images parent node '%s' (%s)\n",
			FIT_IMAGES_PATH, fdt_strerror (images_noffset));
		return images_noffset;
	}

	/* Process its subnodes, print out component images details */
	for (ndepth = 0, noffset = fdt_next_node (fit, images_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node (fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			ret = fit_image_set_hashes (fit, noffset);
			if (ret)
				return ret;
		}
	}

	return 0;
}

/**
 * fit_image_set_hashes - calculate/set hashes for given component image node
 * @fit: pointer to the FIT format image header
 * @image_noffset: requested component image node
 *
 * fit_image_set_hashes() adds hash values for an component image node. All
 * existing hash subnodes are checked, if algorithm property is set to one of
 * the supported hash algorithms, hash value is computed and corresponding
 * hash node property is set, for example:
 *
 * Input component image node structure:
 *
 * o image@1 (at image_noffset)
 *   | - data = [binary data]
 *   o hash@1
 *     |- algo = "sha1"
 *
 * Output component image node structure:
 *
 * o image@1 (at image_noffset)
 *   | - data = [binary data]
 *   o hash@1
 *     |- algo = "sha1"
 *     |- value = sha1(data)
 *
 * returns:
 *     0 on sucess
 *    <0 on failure
 */
int fit_image_set_hashes (void *fit, int image_noffset)
{
	const void *data;
	size_t size;
	char *algo;
	uint8_t value[FIT_MAX_HASH_LEN];
	int value_len;
	int noffset;
	int ndepth;

	/* Get image data and data length */
	if (fit_image_get_data (fit, image_noffset, &data, &size)) {
		printf ("Can't get image data/size\n");
		return -1;
	}

	/* Process all hash subnodes of the component image node */
	for (ndepth = 0, noffset = fdt_next_node (fit, image_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node (fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/* Direct child node of the component image node */

			/*
			 * Check subnode name, must be equal to "hash".
			 * Multiple hash nodes require unique unit node
			 * names, e.g. hash@1, hash@2, etc.
			 */
			if (strncmp (fit_get_name(fit, noffset, NULL),
						FIT_HASH_NODENAME,
						strlen(FIT_HASH_NODENAME)) != 0) {
				/* Not a hash subnode, skip it */
				continue;
			}

			if (fit_image_hash_get_algo (fit, noffset, &algo)) {
				printf ("Can't get hash algo property for "
					"'%s' hash node in '%s' image node\n",
					fit_get_name (fit, noffset, NULL),
					fit_get_name (fit, image_noffset, NULL));
				return -1;
			}

			if (calculate_hash (data, size, algo, value, &value_len)) {
				printf ("Unsupported hash algorithm (%s) for "
					"'%s' hash node in '%s' image node\n",
					algo, fit_get_name (fit, noffset, NULL),
					fit_get_name (fit, image_noffset, NULL));
				return -1;
			}

			if (fit_image_hash_set_value (fit, noffset, value,
							value_len)) {
				printf ("Can't set hash value for "
					"'%s' hash node in '%s' image node\n",
					fit_get_name (fit, noffset, NULL),
					fit_get_name (fit, image_noffset, NULL));
				return -1;
			}
		}
	}

	return 0;
}

/**
 * fit_image_hash_set_value - set hash value in requested has node
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: hash value to be set
 * @value_len: hash value length
 *
 * fit_image_hash_set_value() attempts to set hash value in a node at offset
 * given and returns operation status to the caller.
 *
 * returns
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_set_value (void *fit, int noffset, uint8_t *value,
				int value_len)
{
	int ret;

	ret = fdt_setprop (fit, noffset, FIT_VALUE_PROP, value, value_len);
	if (ret) {
		printf ("Can't set hash '%s' property for '%s' node (%s)\n",
			FIT_VALUE_PROP, fit_get_name (fit, noffset, NULL),
			fdt_strerror (ret));
		return -1;
	}

	return 0;
}
#endif /* USE_HOSTCC */

/**
 * fit_image_check_hashes - verify data intergity
 * @fit: pointer to the FIT format image header
 * @image_noffset: component image node offset
 *
 * fit_image_check_hashes() goes over component image hash nodes,
 * re-calculates each data hash and compares with the value stored in hash
 * node.
 *
 * returns:
 *     1, if all hashes are valid
 *     0, otherwise (or on error)
 */
int fit_image_check_hashes (const void *fit, int image_noffset)
{
	const void	*data;
	size_t		size;
	char		*algo;
	uint8_t		*fit_value;
	int		fit_value_len;
	uint8_t		value[FIT_MAX_HASH_LEN];
	int		value_len;
	int		noffset;
	int		ndepth;
	char		*err_msg = "";

	/* Get image data and data length */
	if (fit_image_get_data (fit, image_noffset, &data, &size)) {
		printf ("Can't get image data/size\n");
		return 0;
	}

	/* Process all hash subnodes of the component image node */
	for (ndepth = 0, noffset = fdt_next_node (fit, image_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node (fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/* Direct child node of the component image node */

			/*
			 * Check subnode name, must be equal to "hash".
			 * Multiple hash nodes require unique unit node
			 * names, e.g. hash@1, hash@2, etc.
			 */
			if (strncmp (fit_get_name(fit, noffset, NULL),
					FIT_HASH_NODENAME,
					strlen(FIT_HASH_NODENAME)) != 0)
				continue;

			if (fit_image_hash_get_algo (fit, noffset, &algo)) {
				err_msg = "Can't get hash algo property";
				goto error;
			}
			printf ("%s", algo);

			if (fit_image_hash_get_value (fit, noffset, &fit_value,
							&fit_value_len)) {
				err_msg = "Can't get hash value property";
				goto error;
			}

			if (calculate_hash (data, size, algo, value, &value_len)) {
				err_msg = "Unsupported hash algorithm";
				goto error;
			}

			if (value_len != fit_value_len) {
				err_msg = "Bad hash value len";
				goto error;
			} else if (memcmp (value, fit_value, value_len) != 0) {
				err_msg = "Bad hash value";
				goto error;
			}
			printf ("+ ");
		}
	}

	return 1;

error:
	printf ("%s for '%s' hash node in '%s' image node\n",
			err_msg, fit_get_name (fit, noffset, NULL),
			fit_get_name (fit, image_noffset, NULL));
	return 0;
}

/**
 * fit_image_check_os - check whether image node is of a given os type
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @os: requested image os
 *
 * fit_image_check_os() reads image os property and compares its numeric
 * id with the requested os. Comparison result is returned to the caller.
 *
 * returns:
 *     1 if image is of given os type
 *     0 otherwise (or on error)
 */
int fit_image_check_os (const void *fit, int noffset, uint8_t os)
{
	uint8_t image_os;

	if (fit_image_get_os (fit, noffset, &image_os))
		return 0;
	return (os == image_os);
}

/**
 * fit_image_check_arch - check whether image node is of a given arch
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @arch: requested imagearch
 *
 * fit_image_check_arch() reads image arch property and compares its numeric
 * id with the requested arch. Comparison result is returned to the caller.
 *
 * returns:
 *     1 if image is of given arch
 *     0 otherwise (or on error)
 */
int fit_image_check_arch (const void *fit, int noffset, uint8_t arch)
{
	uint8_t image_arch;

	if (fit_image_get_arch (fit, noffset, &image_arch))
		return 0;
	return (arch == image_arch);
}

/**
 * fit_image_check_type - check whether image node is of a given type
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @type: requested image type
 *
 * fit_image_check_type() reads image type property and compares its numeric
 * id with the requested type. Comparison result is returned to the caller.
 *
 * returns:
 *     1 if image is of given type
 *     0 otherwise (or on error)
 */
int fit_image_check_type (const void *fit, int noffset, uint8_t type)
{
	uint8_t image_type;

	if (fit_image_get_type (fit, noffset, &image_type))
		return 0;
	return (type == image_type);
}

/**
 * fit_image_check_comp - check whether image node uses given compression
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @comp: requested image compression type
 *
 * fit_image_check_comp() reads image compression property and compares its
 * numeric id with the requested compression type. Comparison result is
 * returned to the caller.
 *
 * returns:
 *     1 if image uses requested compression
 *     0 otherwise (or on error)
 */
int fit_image_check_comp (const void *fit, int noffset, uint8_t comp)
{
	uint8_t image_comp;

	if (fit_image_get_comp (fit, noffset, &image_comp))
		return 0;
	return (comp == image_comp);
}

/**
 * fit_check_format - sanity check FIT image format
 * @fit: pointer to the FIT format image header
 *
 * fit_check_format() runs a basic sanity FIT image verification.
 * Routine checks for mandatory properties, nodes, etc.
 *
 * returns:
 *     1, on success
 *     0, on failure
 */
int fit_check_format (const void *fit)
{
	/* mandatory / node 'description' property */
	if (fdt_getprop (fit, 0, FIT_DESC_PROP, NULL) == NULL) {
		debug ("Wrong FIT format: no description\n");
		return 0;
	}

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || defined(USE_HOSTCC)
	/* mandatory / node 'timestamp' property */
	if (fdt_getprop (fit, 0, FIT_TIMESTAMP_PROP, NULL) == NULL) {
		debug ("Wrong FIT format: no description\n");
		return 0;
	}
#endif

	/* mandatory subimages parent '/images' node */
	if (fdt_path_offset (fit, FIT_IMAGES_PATH) < 0) {
		debug ("Wrong FIT format: no images parent node\n");
		return 0;
	}

	return 1;
}

/**
 * fit_conf_get_node - get node offset for configuration of a given unit name
 * @fit: pointer to the FIT format image header
 * @conf_uname: configuration node unit name
 *
 * fit_conf_get_node() finds a configuration (withing the '/configurations'
 * parant node) of a provided unit name. If configuration is found its node offset
 * is returned to the caller.
 *
 * When NULL is provided in second argument fit_conf_get_node() will search
 * for a default configuration node instead. Default configuration node unit name
 * is retrived from FIT_DEFAULT_PROP property of the '/configurations' node.
 *
 * returns:
 *     configuration node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_conf_get_node (const void *fit, const char *conf_uname)
{
	int noffset, confs_noffset;
	int len;

	confs_noffset = fdt_path_offset (fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		debug ("Can't find configurations parent node '%s' (%s)\n",
			FIT_CONFS_PATH, fdt_strerror (confs_noffset));
		return confs_noffset;
	}

	if (conf_uname == NULL) {
		/* get configuration unit name from the default property */
		debug ("No configuration specified, trying default...\n");
		conf_uname = (char *)fdt_getprop (fit, confs_noffset, FIT_DEFAULT_PROP, &len);
		if (conf_uname == NULL) {
			fit_get_debug (fit, confs_noffset, FIT_DEFAULT_PROP, len);
			return len;
		}
		debug ("Found default configuration: '%s'\n", conf_uname);
	}

	noffset = fdt_subnode_offset (fit, confs_noffset, conf_uname);
	if (noffset < 0) {
		debug ("Can't get node offset for configuration unit name: '%s' (%s)\n",
			conf_uname, fdt_strerror (noffset));
	}

	return noffset;
}

static int __fit_conf_get_prop_node (const void *fit, int noffset,
		const char *prop_name)
{
	char *uname;
	int len;

	/* get kernel image unit name from configuration kernel property */
	uname = (char *)fdt_getprop (fit, noffset, prop_name, &len);
	if (uname == NULL)
		return len;

	return fit_image_get_node (fit, uname);
}

/**
 * fit_conf_get_kernel_node - get kernel image node offset that corresponds to
 * a given configuration
 * @fit: pointer to the FIT format image header
 * @noffset: configuration node offset
 *
 * fit_conf_get_kernel_node() retrives kernel image node unit name from
 * configuration FIT_KERNEL_PROP property and translates it to the node
 * offset.
 *
 * returns:
 *     image node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_conf_get_kernel_node (const void *fit, int noffset)
{
	return __fit_conf_get_prop_node (fit, noffset, FIT_KERNEL_PROP);
}

/**
 * fit_conf_get_ramdisk_node - get ramdisk image node offset that corresponds to
 * a given configuration
 * @fit: pointer to the FIT format image header
 * @noffset: configuration node offset
 *
 * fit_conf_get_ramdisk_node() retrives ramdisk image node unit name from
 * configuration FIT_KERNEL_PROP property and translates it to the node
 * offset.
 *
 * returns:
 *     image node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_conf_get_ramdisk_node (const void *fit, int noffset)
{
	return __fit_conf_get_prop_node (fit, noffset, FIT_RAMDISK_PROP);
}

/**
 * fit_conf_get_fdt_node - get fdt image node offset that corresponds to
 * a given configuration
 * @fit: pointer to the FIT format image header
 * @noffset: configuration node offset
 *
 * fit_conf_get_fdt_node() retrives fdt image node unit name from
 * configuration FIT_KERNEL_PROP property and translates it to the node
 * offset.
 *
 * returns:
 *     image node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_conf_get_fdt_node (const void *fit, int noffset)
{
	return __fit_conf_get_prop_node (fit, noffset, FIT_FDT_PROP);
}

/**
 * fit_conf_print - prints out the FIT configuration details
 * @fit: pointer to the FIT format image header
 * @conf_noffset: offset of the configuration node
 * @p: pointer to prefix string
 *
 * fit_conf_print() lists all mandatory properies for the processed
 * configuration node.
 *
 * returns:
 *     no returned results
 */
void fit_conf_print (const void *fit, int noffset, const char *p)
{
	char *desc;
	char *uname;
	int ret;

	/* Mandatory properties */
	ret = fit_get_desc (fit, noffset, &desc);
	printf ("%s  Description:  ", p);
	if (ret)
		printf ("unavailable\n");
	else
		printf ("%s\n", desc);

	uname = (char *)fdt_getprop (fit, noffset, FIT_KERNEL_PROP, NULL);
	printf ("%s  Kernel:       ", p);
	if (uname == NULL)
		printf ("unavailable\n");
	else
		printf ("%s\n", uname);

	/* Optional properties */
	uname = (char *)fdt_getprop (fit, noffset, FIT_RAMDISK_PROP, NULL);
	if (uname)
		printf ("%s  Init Ramdisk: %s\n", p, uname);

	uname = (char *)fdt_getprop (fit, noffset, FIT_FDT_PROP, NULL);
	if (uname)
		printf ("%s  FDT:          %s\n", p, uname);
}
#endif /* CONFIG_FIT */
