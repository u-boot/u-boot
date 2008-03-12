/*
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

/*
 * Boot support
 */
#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <zlib.h>
#include <bzlib.h>
#include <environment.h>
#include <lmb.h>
#include <asm/byteorder.h>

#ifdef CFG_HUSH_PARSER
#include <hush.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern int gunzip (void *dst, int dstlen, unsigned char *src, unsigned long *lenp);
#ifndef CFG_BOOTM_LEN
#define CFG_BOOTM_LEN	0x800000	/* use 8MByte as default max gunzip size */
#endif

#ifdef CONFIG_BZIP2
extern void bz_internal_error(int);
#endif

#if defined(CONFIG_CMD_IMI)
static int image_info (unsigned long addr);
#endif

#if defined(CONFIG_CMD_IMLS)
#include <flash.h>
extern flash_info_t flash_info[]; /* info for FLASH chips */
static int do_imls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif

#ifdef CONFIG_SILENT_CONSOLE
static void fixup_silent_linux (void);
#endif

static image_header_t *image_get_kernel (ulong img_addr, int verify);
#if defined(CONFIG_FIT)
static int fit_check_kernel (const void *fit, int os_noffset, int verify);
#endif

static void *boot_get_kernel (cmd_tbl_t *cmdtp, int flag,int argc, char *argv[],
		bootm_headers_t *images, ulong *os_data, ulong *os_len);
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/*
 *  Continue booting an OS image; caller already has:
 *  - copied image header to global variable `header'
 *  - checked header magic number, checksums (both header & image),
 *  - verified image architecture (PPC) and type (KERNEL or MULTI),
 *  - loaded (first part of) image to header load address,
 *  - disabled interrupts.
 */
typedef void boot_os_fn (cmd_tbl_t *cmdtp, int flag,
			int argc, char *argv[],
			bootm_headers_t *images); /* pointers to os/initrd/fdt */

extern boot_os_fn do_bootm_linux;
static boot_os_fn do_bootm_netbsd;
#if defined(CONFIG_LYNXKDI)
static boot_os_fn do_bootm_lynxkdi;
extern void lynxkdi_boot (image_header_t *);
#endif
static boot_os_fn do_bootm_rtems;
#if defined(CONFIG_CMD_ELF)
static boot_os_fn do_bootm_vxworks;
static boot_os_fn do_bootm_qnxelf;
int do_bootvx (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_bootelf (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif
#if defined(CONFIG_ARTOS) && defined(CONFIG_PPC)
extern uchar (*env_get_char)(int); /* Returns a character from the environment */
static boot_os_fn do_bootm_artos;
#endif

ulong load_addr = CFG_LOAD_ADDR;	/* Default Load Address */
static bootm_headers_t images;		/* pointers to os/initrd/fdt images */

void __board_lmb_reserve(struct lmb *lmb)
{
	/* please define platform specific board_lmb_reserve() */
}
void board_lmb_reserve(struct lmb *lmb) __attribute__((weak, alias("__board_lmb_reserve")));


/*******************************************************************/
/* bootm - boot application image from image in memory */
/*******************************************************************/
int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong		iflag;
	const char	*type_name;
	uint		unc_len = CFG_BOOTM_LEN;
	uint8_t		comp, type, os;

	void		*os_hdr;
	ulong		os_data, os_len;
	ulong		image_start, image_end;
	ulong		load_start, load_end;
	ulong		mem_start, mem_size;
#if defined(CONFIG_FIT)
	int		os_noffset;
#endif

	struct lmb lmb;

	memset ((void *)&images, 0, sizeof (images));
	images.verify = getenv_verify();
	images.autostart = getenv_autostart();
	images.lmb = &lmb;

	lmb_init(&lmb);

	mem_start = getenv_bootm_low();
	mem_size = getenv_bootm_size();

	lmb_add(&lmb, mem_start, mem_size);

	board_lmb_reserve(&lmb);

	/* get kernel image header, start address and length */
	os_hdr = boot_get_kernel (cmdtp, flag, argc, argv,
			&images, &os_data, &os_len);
	if (os_len == 0) {
		puts ("ERROR: can't get kernel image!\n");
		return 1;
	}

	show_boot_progress (6);

	/* get image parameters */
	switch (genimg_get_format (os_hdr)) {
	case IMAGE_FORMAT_LEGACY:
		type = image_get_type (os_hdr);
		comp = image_get_comp (os_hdr);
		os = image_get_os (os_hdr);

		image_end = image_get_image_end (os_hdr);
		load_start = image_get_load (os_hdr);
		break;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		os_noffset = fit_image_get_node (images.fit_hdr_os,
				images.fit_uname_os);
		if (os_noffset < 0) {
			printf ("Can't get image node for '%s'!\n",
					images.fit_uname_os);
			return 1;
		}

		if (fit_image_get_type (images.fit_hdr_os, os_noffset, &type)) {
			puts ("Can't get image type!\n");
			return 1;
		}

		if (fit_image_get_comp (images.fit_hdr_os, os_noffset, &comp)) {
			puts ("Can't get image compression!\n");
			return 1;
		}

		if (fit_image_get_os (images.fit_hdr_os, os_noffset, &os)) {
			puts ("Can't get image OS!\n");
			return 1;
		}

		image_end = fit_get_end (images.fit_hdr_os);

		if (fit_image_get_load (images.fit_hdr_os, os_noffset,
					&load_start)) {
			puts ("Can't get image load address!\n");
			return 1;
		}
		break;
#endif
	default:
		puts ("ERROR: unknown image format type!\n");
		return 1;
	}

	image_start = (ulong)os_hdr;
	load_end = 0;
	type_name = genimg_get_type_name (type);

	/*
	 * We have reached the point of no return: we are going to
	 * overwrite all exception vector code, so we cannot easily
	 * recover from any failures any more...
	 */
	iflag = disable_interrupts();

#ifdef CONFIG_AMIGAONEG3SE
	/*
	 * We've possible left the caches enabled during
	 * bios emulation, so turn them off again
	 */
	icache_disable();
	invalidate_l1_instruction_cache();
	flush_data_cache();
	dcache_disable();
#endif

	switch (comp) {
	case IH_COMP_NONE:
		if (load_start == (ulong)os_hdr) {
			printf ("   XIP %s ... ", type_name);
		} else {
			printf ("   Loading %s ... ", type_name);

			memmove_wd ((void *)load_start,
				   (void *)os_data, os_len, CHUNKSZ);

			load_end = load_start + os_len;
			puts("OK\n");
		}
		break;
	case IH_COMP_GZIP:
		printf ("   Uncompressing %s ... ", type_name);
		if (gunzip ((void *)load_start, unc_len,
					(uchar *)os_data, &os_len) != 0) {
			puts ("GUNZIP ERROR - must RESET board to recover\n");
			show_boot_progress (-6);
			do_reset (cmdtp, flag, argc, argv);
		}

		load_end = load_start + os_len;
		break;
#ifdef CONFIG_BZIP2
	case IH_COMP_BZIP2:
		printf ("   Uncompressing %s ... ", type_name);
		/*
		 * If we've got less than 4 MB of malloc() space,
		 * use slower decompression algorithm which requires
		 * at most 2300 KB of memory.
		 */
		int i = BZ2_bzBuffToBuffDecompress ((char*)load_start,
					&unc_len, (char *)os_data, os_len,
					CFG_MALLOC_LEN < (4096 * 1024), 0);
		if (i != BZ_OK) {
			printf ("BUNZIP2 ERROR %d - must RESET board to recover\n", i);
			show_boot_progress (-6);
			do_reset (cmdtp, flag, argc, argv);
		}

		load_end = load_start + unc_len;
		break;
#endif /* CONFIG_BZIP2 */
	default:
		if (iflag)
			enable_interrupts();
		printf ("Unimplemented compression type %d\n", comp);
		show_boot_progress (-7);
		return 1;
	}
	puts ("OK\n");
	debug ("   kernel loaded at 0x%08lx, end = 0x%08lx\n", load_start, load_end);
	show_boot_progress (7);

	if ((load_start < image_end) && (load_end > image_start)) {
		debug ("image_start = 0x%lX, image_end = 0x%lx\n", image_start, image_end);
		debug ("load_start = 0x%lx, load_end = 0x%lx\n", load_start, load_end);

		puts ("ERROR: image overwritten - must RESET the board to recover.\n");
		do_reset (cmdtp, flag, argc, argv);
	}

	show_boot_progress (8);

	lmb_reserve(&lmb, load_start, (load_end - load_start));

	switch (os) {
	default:			/* handled by (original) Linux case */
	case IH_OS_LINUX:
#ifdef CONFIG_SILENT_CONSOLE
	    fixup_silent_linux();
#endif
	    do_bootm_linux (cmdtp, flag, argc, argv, &images);
	    break;

	case IH_OS_NETBSD:
	    do_bootm_netbsd (cmdtp, flag, argc, argv, &images);
	    break;

#ifdef CONFIG_LYNXKDI
	case IH_OS_LYNXOS:
	    do_bootm_lynxkdi (cmdtp, flag, argc, argv, &images);
	    break;
#endif

	case IH_OS_RTEMS:
	    do_bootm_rtems (cmdtp, flag, argc, argv, &images);
	    break;

#if defined(CONFIG_CMD_ELF)
	case IH_OS_VXWORKS:
	    do_bootm_vxworks (cmdtp, flag, argc, argv, &images);
	    break;

	case IH_OS_QNX:
	    do_bootm_qnxelf (cmdtp, flag, argc, argv, &images);
	    break;
#endif

#ifdef CONFIG_ARTOS
	case IH_OS_ARTOS:
	    do_bootm_artos (cmdtp, flag, argc, argv, &images);
	    break;
#endif
	}

	show_boot_progress (-9);
#ifdef DEBUG
	puts ("\n## Control returned to monitor - resetting...\n");
	do_reset (cmdtp, flag, argc, argv);
#endif
	return 1;
}

/**
 * image_get_kernel - verify legacy format kernel image
 * @img_addr: in RAM address of the legacy format image to be verified
 * @verify: data CRC verification flag
 *
 * image_get_kernel() verifies legacy image integrity and returns pointer to
 * legacy image header if image verification was completed successfully.
 *
 * returns:
 *     pointer to a legacy image header if valid image was found
 *     otherwise return NULL
 */
static image_header_t *image_get_kernel (ulong img_addr, int verify)
{
	image_header_t *hdr = (image_header_t *)img_addr;

	if (!image_check_magic(hdr)) {
		puts ("Bad Magic Number\n");
		show_boot_progress (-1);
		return NULL;
	}
	show_boot_progress (2);

	if (!image_check_hcrc (hdr)) {
		puts ("Bad Header Checksum\n");
		show_boot_progress (-2);
		return NULL;
	}

	show_boot_progress (3);
	image_print_contents (hdr);

	if (verify) {
		puts ("   Verifying Checksum ... ");
		if (!image_check_dcrc (hdr)) {
			printf ("Bad Data CRC\n");
			show_boot_progress (-3);
			return NULL;
		}
		puts ("OK\n");
	}
	show_boot_progress (4);

	if (!image_check_target_arch (hdr)) {
		printf ("Unsupported Architecture 0x%x\n", image_get_arch (hdr));
		show_boot_progress (-4);
		return NULL;
	}
	return hdr;
}

/**
 * fit_check_kernel - verify FIT format kernel subimage
 * @fit_hdr: pointer to the FIT image header
 * os_noffset: kernel subimage node offset within FIT image
 * @verify: data CRC verification flag
 *
 * fit_check_kernel() verifies integrity of the kernel subimage and from
 * specified FIT image.
 *
 * returns:
 *     1, on success
 *     0, on failure
 */
#if defined (CONFIG_FIT)
static int fit_check_kernel (const void *fit, int os_noffset, int verify)
{
	fit_image_print (fit, os_noffset, "   ");

	if (verify) {
		puts ("   Verifying Hash Integrity ... ");
		if (!fit_image_check_hashes (fit, os_noffset)) {
			puts ("Bad Data Hash\n");
			return 0;
		}
		puts ("OK\n");
	}

	if (!fit_image_check_target_arch (fit, os_noffset)) {
		puts ("Unsupported Architecture\n");
		return 0;
	}

	if (!fit_image_check_type (fit, os_noffset, IH_TYPE_KERNEL)) {
		puts ("Not a kernel image\n");
		return 0;
	}

	return 1;
}
#endif /* CONFIG_FIT */

/**
 * boot_get_kernel - find kernel image
 * @os_data: pointer to a ulong variable, will hold os data start address
 * @os_len: pointer to a ulong variable, will hold os data length
 *
 * boot_get_kernel() tries to find a kernel image, verifies its integrity
 * and locates kernel data.
 *
 * returns:
 *     pointer to image header if valid image was found, plus kernel start
 *     address and length, otherwise NULL
 */
static void *boot_get_kernel (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		bootm_headers_t *images, ulong *os_data, ulong *os_len)
{
	image_header_t	*hdr;
	ulong		img_addr;
#if defined(CONFIG_FIT)
	void		*fit_hdr;
	const char	*fit_uname_config = NULL;
	const char	*fit_uname_kernel = NULL;
	const void	*data;
	size_t		len;
	int		conf_noffset;
	int		os_noffset;
#endif

	/* find out kernel image address */
	if (argc < 2) {
		img_addr = load_addr;
		debug ("*  kernel: default image load address = 0x%08lx\n",
				load_addr);
#if defined(CONFIG_FIT)
	} else if (fit_parse_conf (argv[1], load_addr, &img_addr,
							&fit_uname_config)) {
		debug ("*  kernel: config '%s' from image at 0x%08lx\n",
				fit_uname_config, img_addr);
	} else if (fit_parse_subimage (argv[1], load_addr, &img_addr,
							&fit_uname_kernel)) {
		debug ("*  kernel: subimage '%s' from image at 0x%08lx\n",
				fit_uname_kernel, img_addr);
#endif
	} else {
		img_addr = simple_strtoul(argv[1], NULL, 16);
		debug ("*  kernel: cmdline image address = 0x%08lx\n", img_addr);
	}

	show_boot_progress (1);

	/* copy from dataflash if needed */
	img_addr = genimg_get_image (img_addr);

	/* check image type, for FIT images get FIT kernel node */
	*os_data = *os_len = 0;
	switch (genimg_get_format ((void *)img_addr)) {
	case IMAGE_FORMAT_LEGACY:
		printf ("## Booting kernel from Legacy Image at %08lx ...\n",
				img_addr);
		hdr = image_get_kernel (img_addr, images->verify);
		if (!hdr)
			return NULL;
		show_boot_progress (5);

		/* get os_data and os_len */
		switch (image_get_type (hdr)) {
		case IH_TYPE_KERNEL:
			*os_data = image_get_data (hdr);
			*os_len = image_get_data_size (hdr);
			break;
		case IH_TYPE_MULTI:
			image_multi_getimg (hdr, 0, os_data, os_len);
			break;
		default:
			printf ("Wrong Image Type for %s command\n", cmdtp->name);
			show_boot_progress (-5);
			return NULL;
		}
		images->legacy_hdr_os = hdr;
		images->legacy_hdr_valid = 1;

		break;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (void *)img_addr;
		printf ("## Booting kernel from FIT Image at %08lx ...\n",
				img_addr);

		if (!fit_check_format (fit_hdr)) {
			puts ("Bad FIT kernel image format!\n");
			return NULL;
		}

		if (!fit_uname_kernel) {
			/*
			 * no kernel image node unit name, try to get config
			 * node first. If config unit node name is NULL
			 * fit_conf_get_node() will try to find default config node
			 */
			conf_noffset = fit_conf_get_node (fit_hdr, fit_uname_config);
			if (conf_noffset < 0)
				return NULL;

			os_noffset = fit_conf_get_kernel_node (fit_hdr, conf_noffset);
			fit_uname_kernel = fit_get_name (fit_hdr, os_noffset, NULL);
		} else {
			/* get kernel component image node offset */
			os_noffset = fit_image_get_node (fit_hdr, fit_uname_kernel);
		}
		if (os_noffset < 0)
			return NULL;

		printf ("   Trying '%s' kernel subimage\n", fit_uname_kernel);

		if (!fit_check_kernel (fit_hdr, os_noffset, images->verify))
			return NULL;

		/* get kernel image data address and length */
		if (fit_image_get_data (fit_hdr, os_noffset, &data, &len)) {
			puts ("Could not find kernel subimage data!\n");
			return NULL;
		}

		*os_len = len;
		*os_data = (ulong)data;
		images->fit_hdr_os = fit_hdr;
		images->fit_uname_os = fit_uname_kernel;
		break;
#endif
	default:
		printf ("Wrong Image Format for %s command\n", cmdtp->name);
		return NULL;
	}

	debug ("   kernel data at 0x%08lx, len = 0x%08lx (%d)\n",
			*os_data, *os_len, *os_len);

	return (void *)img_addr;
}

U_BOOT_CMD(
	bootm,	CFG_MAXARGS,	1,	do_bootm,
	"bootm   - boot application image from memory\n",
	"[addr [arg ...]]\n    - boot application image stored in memory\n"
	"\tpassing arguments 'arg ...'; when booting a Linux kernel,\n"
	"\t'arg' can be the address of an initrd image\n"
#if defined(CONFIG_OF_LIBFDT)
	"\tWhen booting a Linux kernel which requires a flat device-tree\n"
	"\ta third argument is required which is the address of the\n"
	"\tdevice-tree blob. To boot that kernel without an initrd image,\n"
	"\tuse a '-' for the second argument. If you do not pass a third\n"
	"\ta bd_info struct will be passed instead\n"
#endif
#if defined(CONFIG_FIT)
	"\t\nFor the new multi component uImage format (FIT) addresses\n"
	"\tmust be extened to include component or configuration unit name:\n"
	"\taddr:<subimg_uname> - direct component image specification\n"
	"\taddr#<conf_uname>   - configuration specification\n"
	"\tUse iminfo command to get the list of existing component\n"
	"\timages and configurations.\n"
#endif
);

/*******************************************************************/
/* bootd - boot default image */
/*******************************************************************/
#if defined(CONFIG_CMD_BOOTD)
int do_bootd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode = 0;

#ifndef CFG_HUSH_PARSER
	if (run_command (getenv ("bootcmd"), flag) < 0)
		rcode = 1;
#else
	if (parse_string_outer (getenv ("bootcmd"),
			FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP) != 0)
		rcode = 1;
#endif
	return rcode;
}

U_BOOT_CMD(
	boot,	1,	1,	do_bootd,
	"boot    - boot default, i.e., run 'bootcmd'\n",
	NULL
);

/* keep old command name "bootd" for backward compatibility */
U_BOOT_CMD(
	bootd, 1,	1,	do_bootd,
	"bootd   - boot default, i.e., run 'bootcmd'\n",
	NULL
);

#endif


/*******************************************************************/
/* iminfo - print header info for a requested image */
/*******************************************************************/
#if defined(CONFIG_CMD_IMI)
int do_iminfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int	arg;
	ulong	addr;
	int	rcode = 0;

	if (argc < 2) {
		return image_info (load_addr);
	}

	for (arg = 1; arg < argc; ++arg) {
		addr = simple_strtoul (argv[arg], NULL, 16);
		if (image_info (addr) != 0)
			rcode = 1;
	}
	return rcode;
}

static int image_info (ulong addr)
{
	void *hdr = (void *)addr;

	printf ("\n## Checking Image at %08lx ...\n", addr);

	switch (genimg_get_format (hdr)) {
	case IMAGE_FORMAT_LEGACY:
		puts ("   Legacy image found\n");
		if (!image_check_magic (hdr)) {
			puts ("   Bad Magic Number\n");
			return 1;
		}

		if (!image_check_hcrc (hdr)) {
			puts ("   Bad Header Checksum\n");
			return 1;
		}

		image_print_contents (hdr);

		puts ("   Verifying Checksum ... ");
		if (!image_check_dcrc (hdr)) {
			puts ("   Bad Data CRC\n");
			return 1;
		}
		puts ("OK\n");
		return 0;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		puts ("   FIT image found\n");

		if (!fit_check_format (hdr)) {
			puts ("Bad FIT image format!\n");
			return 1;
		}

		fit_print_contents (hdr);
		return 0;
#endif
	default:
		puts ("Unknown image format!\n");
		break;
	}

	return 1;
}

U_BOOT_CMD(
	iminfo,	CFG_MAXARGS,	1,	do_iminfo,
	"iminfo  - print header information for application image\n",
	"addr [addr ...]\n"
	"    - print header information for application image starting at\n"
	"      address 'addr' in memory; this includes verification of the\n"
	"      image contents (magic number, header and payload checksums)\n"
);
#endif


/*******************************************************************/
/* imls - list all images found in flash */
/*******************************************************************/
#if defined(CONFIG_CMD_IMLS)
int do_imls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	flash_info_t *info;
	int i, j;
	void *hdr;

	for (i = 0, info = &flash_info[0];
		i < CFG_MAX_FLASH_BANKS; ++i, ++info) {

		if (info->flash_id == FLASH_UNKNOWN)
			goto next_bank;
		for (j = 0; j < info->sector_count; ++j) {

			hdr = (void *)info->start[j];
			if (!hdr)
				goto next_sector;

			switch (genimg_get_format (hdr)) {
			case IMAGE_FORMAT_LEGACY:
				if (!image_check_hcrc (hdr))
					goto next_sector;

				printf ("Legacy Image at %08lX:\n", (ulong)hdr);
				image_print_contents (hdr);

				puts ("   Verifying Checksum ... ");
				if (!image_check_dcrc (hdr)) {
					puts ("Bad Data CRC\n");
				} else {
					puts ("OK\n");
				}
				break;
#if defined(CONFIG_FIT)
			case IMAGE_FORMAT_FIT:
				if (!fit_check_format (hdr))
					goto next_sector;

				printf ("FIT Image at %08lX:\n", (ulong)hdr);
				fit_print_contents (hdr);
				break;
#endif
			default:
				goto next_sector;
			}

next_sector:		;
		}
next_bank:	;
	}

	return (0);
}

U_BOOT_CMD(
	imls,	1,		1,	do_imls,
	"imls    - list all images found in flash\n",
	"\n"
	"    - Prints information about all images found at sector\n"
	"      boundaries in flash.\n"
);
#endif

/*******************************************************************/
/* helper routines */
/*******************************************************************/
#ifdef CONFIG_SILENT_CONSOLE
static void fixup_silent_linux ()
{
	char buf[256], *start, *end;
	char *cmdline = getenv ("bootargs");

	/* Only fix cmdline when requested */
	if (!(gd->flags & GD_FLG_SILENT))
		return;

	debug ("before silent fix-up: %s\n", cmdline);
	if (cmdline) {
		if ((start = strstr (cmdline, "console=")) != NULL) {
			end = strchr (start, ' ');
			strncpy (buf, cmdline, (start - cmdline + 8));
			if (end)
				strcpy (buf + (start - cmdline + 8), end);
			else
				buf[start - cmdline + 8] = '\0';
		} else {
			strcpy (buf, cmdline);
			strcat (buf, " console=");
		}
	} else {
		strcpy (buf, "console=");
	}

	setenv ("bootargs", buf);
	debug ("after silent fix-up: %s\n", buf);
}
#endif /* CONFIG_SILENT_CONSOLE */


/*******************************************************************/
/* OS booting routines */
/*******************************************************************/

static void do_bootm_netbsd (cmd_tbl_t *cmdtp, int flag,
			    int argc, char *argv[],
			    bootm_headers_t *images)
{
	void (*loader)(bd_t *, image_header_t *, char *, char *);
	image_header_t *os_hdr, *hdr;
	ulong kernel_data, kernel_len;
	char *consdev;
	char *cmdline;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset ("NetBSD");
		do_reset (cmdtp, flag, argc, argv);
	}
#endif
	hdr = images->legacy_hdr_os;

	/*
	 * Booting a (NetBSD) kernel image
	 *
	 * This process is pretty similar to a standalone application:
	 * The (first part of an multi-) image must be a stage-2 loader,
	 * which in turn is responsible for loading & invoking the actual
	 * kernel.  The only differences are the parameters being passed:
	 * besides the board info strucure, the loader expects a command
	 * line, the name of the console device, and (optionally) the
	 * address of the original image header.
	 */
	os_hdr = NULL;
	if (image_check_type (hdr, IH_TYPE_MULTI)) {
		image_multi_getimg (hdr, 1, &kernel_data, &kernel_len);
		if (kernel_len)
			os_hdr = hdr;
	}

	consdev = "";
#if   defined (CONFIG_8xx_CONS_SMC1)
	consdev = "smc1";
#elif defined (CONFIG_8xx_CONS_SMC2)
	consdev = "smc2";
#elif defined (CONFIG_8xx_CONS_SCC2)
	consdev = "scc2";
#elif defined (CONFIG_8xx_CONS_SCC3)
	consdev = "scc3";
#endif

	if (argc > 2) {
		ulong len;
		int   i;

		for (i = 2, len = 0; i < argc; i += 1)
			len += strlen (argv[i]) + 1;
		cmdline = malloc (len);

		for (i = 2, len = 0; i < argc; i += 1) {
			if (i > 2)
				cmdline[len++] = ' ';
			strcpy (&cmdline[len], argv[i]);
			len += strlen (argv[i]);
		}
	} else if ((cmdline = getenv ("bootargs")) == NULL) {
		cmdline = "";
	}

	loader = (void (*)(bd_t *, image_header_t *, char *, char *))image_get_ep (hdr);

	printf ("## Transferring control to NetBSD stage-2 loader (at address %08lx) ...\n",
		(ulong)loader);

	show_boot_progress (15);

	/*
	 * NetBSD Stage-2 Loader Parameters:
	 *   r3: ptr to board info data
	 *   r4: image address
	 *   r5: console device
	 *   r6: boot args string
	 */
	(*loader) (gd->bd, os_hdr, consdev, cmdline);
}

#ifdef CONFIG_LYNXKDI
static void do_bootm_lynxkdi (cmd_tbl_t *cmdtp, int flag,
			     int argc, char *argv[],
			     bootm_headers_t *images)
{
	image_header_t *hdr = images->legacy_hdr_os;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset ("Lynx");
		do_reset (cmdtp, flag, argc, argv);
	}
#endif

	lynxkdi_boot ((image_header_t *)hdr);
}
#endif /* CONFIG_LYNXKDI */

static void do_bootm_rtems (cmd_tbl_t *cmdtp, int flag,
			   int argc, char *argv[],
			   bootm_headers_t *images)
{
	image_header_t *hdr = images->legacy_hdr_os;
	void (*entry_point)(bd_t *);

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset ("RTEMS");
		do_reset (cmdtp, flag, argc, argv);
	}
#endif

	entry_point = (void (*)(bd_t *))image_get_ep (hdr);

	printf ("## Transferring control to RTEMS (at address %08lx) ...\n",
		(ulong)entry_point);

	show_boot_progress (15);

	/*
	 * RTEMS Parameters:
	 *   r3: ptr to board info data
	 */
	(*entry_point)(gd->bd);
}

#if defined(CONFIG_CMD_ELF)
static void do_bootm_vxworks (cmd_tbl_t *cmdtp, int flag,
			     int argc, char *argv[],
			     bootm_headers_t *images)
{
	char str[80];
	image_header_t *hdr = images->legacy_hdr_os;

#if defined(CONFIG_FIT)
	if (hdr == NULL) {
		fit_unsupported_reset ("VxWorks");
		do_reset (cmdtp, flag, argc, argv);
	}
#endif

	sprintf(str, "%x", image_get_ep (hdr)); /* write entry-point into string */
	setenv("loadaddr", str);
	do_bootvx(cmdtp, 0, 0, NULL);
}

static void do_bootm_qnxelf(cmd_tbl_t *cmdtp, int flag,
			    int argc, char *argv[],
			    bootm_headers_t *images)
{
	char *local_args[2];
	char str[16];
	image_header_t *hdr = images->legacy_hdr_os;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset ("QNX");
		do_reset (cmdtp, flag, argc, argv);
	}
#endif

	sprintf(str, "%x", image_get_ep (hdr)); /* write entry-point into string */
	local_args[0] = argv[0];
	local_args[1] = str;	/* and provide it via the arguments */
	do_bootelf(cmdtp, 0, 2, local_args);
}
#endif

#if defined(CONFIG_ARTOS) && defined(CONFIG_PPC)
static void do_bootm_artos (cmd_tbl_t *cmdtp, int flag,
			   int argc, char *argv[],
			   bootm_headers_t *images)
{
	ulong top;
	char *s, *cmdline;
	char **fwenv, **ss;
	int i, j, nxt, len, envno, envsz;
	bd_t *kbd;
	void (*entry)(bd_t *bd, char *cmdline, char **fwenv, ulong top);
	image_header_t *hdr = images->legacy_hdr_os;

#if defined(CONFIG_FIT)
	if (!images->legacy_hdr_valid) {
		fit_unsupported_reset ("ARTOS");
		do_reset (cmdtp, flag, argc, argv);
	}
#endif

	/*
	 * Booting an ARTOS kernel image + application
	 */

	/* this used to be the top of memory, but was wrong... */
#ifdef CONFIG_PPC
	/* get stack pointer */
	asm volatile ("mr %0,1" : "=r"(top) );
#endif
	debug ("## Current stack ends at 0x%08lX ", top);

	top -= 2048;		/* just to be sure */
	if (top > CFG_BOOTMAPSZ)
		top = CFG_BOOTMAPSZ;
	top &= ~0xF;

	debug ("=> set upper limit to 0x%08lX\n", top);

	/* first check the artos specific boot args, then the linux args*/
	if ((s = getenv( "abootargs")) == NULL && (s = getenv ("bootargs")) == NULL)
		s = "";

	/* get length of cmdline, and place it */
	len = strlen (s);
	top = (top - (len + 1)) & ~0xF;
	cmdline = (char *)top;
	debug ("## cmdline at 0x%08lX ", top);
	strcpy (cmdline, s);

	/* copy bdinfo */
	top = (top - sizeof (bd_t)) & ~0xF;
	debug ("## bd at 0x%08lX ", top);
	kbd = (bd_t *)top;
	memcpy (kbd, gd->bd, sizeof (bd_t));

	/* first find number of env entries, and their size */
	envno = 0;
	envsz = 0;
	for (i = 0; env_get_char (i) != '\0'; i = nxt + 1) {
		for (nxt = i; env_get_char (nxt) != '\0'; ++nxt)
			;
		envno++;
		envsz += (nxt - i) + 1;	/* plus trailing zero */
	}
	envno++;	/* plus the terminating zero */
	debug ("## %u envvars total size %u ", envno, envsz);

	top = (top - sizeof (char **) * envno) & ~0xF;
	fwenv = (char **)top;
	debug ("## fwenv at 0x%08lX ", top);

	top = (top - envsz) & ~0xF;
	s = (char *)top;
	ss = fwenv;

	/* now copy them */
	for (i = 0; env_get_char (i) != '\0'; i = nxt + 1) {
		for (nxt = i; env_get_char (nxt) != '\0'; ++nxt)
			;
		*ss++ = s;
		for (j = i; j < nxt; ++j)
			*s++ = env_get_char (j);
		*s++ = '\0';
	}
	*ss++ = NULL;	/* terminate */

	entry = (void (*)(bd_t *, char *, char **, ulong))image_get_ep (hdr);
	(*entry) (kbd, cmdline, fwenv, top);
}
#endif
