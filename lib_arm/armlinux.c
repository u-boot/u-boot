/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_VFD) || \
    defined (CONFIG_LCD)
static void setup_start_tag (bd_t *bd);

# ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags (bd_t *bd);
# endif
static void setup_commandline_tag (bd_t *bd, char *commandline);

#if 0
static void setup_ramdisk_tag (bd_t *bd);
#endif
# ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag (bd_t *bd, ulong initrd_start,
			      ulong initrd_end);
# endif
static void setup_end_tag (bd_t *bd);

# if defined (CONFIG_VFD) || defined (CONFIG_LCD)
static void setup_videolfb_tag (gd_t *gd);
# endif


static struct tag *params;
#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || CONFIG_INITRD_TAG */

extern image_header_t header;	/* from cmd_bootm.c */


void do_bootm_linux (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		     ulong addr, ulong *len_ptr, int verify)
{
	ulong len = 0, checksum;
	ulong initrd_start, initrd_end;
	ulong data;
	void (*theKernel)(int zero, int arch, uint params);
	image_header_t *hdr = &header;
	bd_t *bd = gd->bd;
	int machid = bd->bi_arch_number;
	char *s;

#ifdef CONFIG_CMDLINE_TAG
	char *commandline = getenv ("bootargs");
#endif

	theKernel = (void (*)(int, int, uint))ntohl(hdr->ih_ep);

	s = getenv ("machid");
	if (s) {
		machid = simple_strtoul (s, NULL, 16);
		printf ("Using machid 0x%x from environment\n", machid);
	}

	/*
	 * Check if there is an initrd image
	 */
	if (argc >= 3) {
		show_boot_progress (9);

		addr = simple_strtoul (argv[2], NULL, 16);

		printf ("## Loading Ramdisk Image at %08lx ...\n", addr);

		/* Copy header so we can blank CRC field for re-calculation */
#ifdef CONFIG_HAS_DATAFLASH
		if (addr_dataflash (addr)) {
			read_dataflash (addr, sizeof (image_header_t),
					(char *) &header);
		} else
#endif
			memcpy (&header, (char *) addr,
				sizeof (image_header_t));

		if (ntohl (hdr->ih_magic) != IH_MAGIC) {
			printf ("Bad Magic Number\n");
			show_boot_progress (-10);
			do_reset (cmdtp, flag, argc, argv);
		}

		data = (ulong) & header;
		len = sizeof (image_header_t);

		checksum = ntohl (hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (crc32 (0, (unsigned char *) data, len) != checksum) {
			printf ("Bad Header Checksum\n");
			show_boot_progress (-11);
			do_reset (cmdtp, flag, argc, argv);
		}

		show_boot_progress (10);

		print_image_hdr (hdr);

		data = addr + sizeof (image_header_t);
		len = ntohl (hdr->ih_size);

#ifdef CONFIG_HAS_DATAFLASH
		if (addr_dataflash (addr)) {
			read_dataflash (data, len, (char *) CFG_LOAD_ADDR);
			data = CFG_LOAD_ADDR;
		}
#endif

		if (verify) {
			ulong csum = 0;

			printf ("   Verifying Checksum ... ");
			csum = crc32 (0, (unsigned char *) data, len);
			if (csum != ntohl (hdr->ih_dcrc)) {
				printf ("Bad Data CRC\n");
				show_boot_progress (-12);
				do_reset (cmdtp, flag, argc, argv);
			}
			printf ("OK\n");
		}

		show_boot_progress (11);

		if ((hdr->ih_os != IH_OS_LINUX) ||
		    (hdr->ih_arch != IH_CPU_ARM) ||
		    (hdr->ih_type != IH_TYPE_RAMDISK)) {
			printf ("No Linux ARM Ramdisk Image\n");
			show_boot_progress (-13);
			do_reset (cmdtp, flag, argc, argv);
		}

#if defined(CONFIG_B2) || defined(CONFIG_EVB4510) || defined(CONFIG_ARMADILLO)
		/*
		 *we need to copy the ramdisk to SRAM to let Linux boot
		 */
		memmove ((void *) ntohl(hdr->ih_load), (uchar *)data, len);
		data = ntohl(hdr->ih_load);
#endif /* CONFIG_B2 || CONFIG_EVB4510 */

		/*
		 * Now check if we have a multifile image
		 */
	} else if ((hdr->ih_type == IH_TYPE_MULTI) && (len_ptr[1])) {
		ulong tail = ntohl (len_ptr[0]) % 4;
		int i;

		show_boot_progress (13);

		/* skip kernel length and terminator */
		data = (ulong) (&len_ptr[2]);
		/* skip any additional image length fields */
		for (i = 1; len_ptr[i]; ++i)
			data += 4;
		/* add kernel length, and align */
		data += ntohl (len_ptr[0]);
		if (tail) {
			data += 4 - tail;
		}

		len = ntohl (len_ptr[1]);

	} else {
		/*
		 * no initrd image
		 */
		show_boot_progress (14);

		len = data = 0;
	}

#ifdef	DEBUG
	if (!data) {
		printf ("No initrd\n");
	}
#endif

	if (data) {
		initrd_start = data;
		initrd_end = initrd_start + len;
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	show_boot_progress (15);

	debug ("## Transferring control to Linux (at address %08lx) ...\n",
	       (ulong) theKernel);

#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_LCD) || \
    defined (CONFIG_VFD)
	setup_start_tag (bd);
#ifdef CONFIG_SERIAL_TAG
	setup_serial_tag (&params);
#endif
#ifdef CONFIG_REVISION_TAG
	setup_revision_tag (&params);
#endif
#ifdef CONFIG_SETUP_MEMORY_TAGS
	setup_memory_tags (bd);
#endif
#ifdef CONFIG_CMDLINE_TAG
	setup_commandline_tag (bd, commandline);
#endif
#ifdef CONFIG_INITRD_TAG
	if (initrd_start && initrd_end)
		setup_initrd_tag (bd, initrd_start, initrd_end);
#endif
#if defined (CONFIG_VFD) || defined (CONFIG_LCD)
	setup_videolfb_tag ((gd_t *) gd);
#endif
	setup_end_tag (bd);
#endif

	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");

#ifdef CONFIG_USB_DEVICE
	{
		extern void udc_disconnect (void);
		udc_disconnect ();
	}
#endif

	cleanup_before_linux ();

	theKernel (0, machid, bd->bi_boot_params);
}


#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_LCD) || \
    defined (CONFIG_VFD)
static void setup_start_tag (bd_t *bd)
{
	params = (struct tag *) bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}


#ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags (bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		params = tag_next (params);
	}
}
#endif /* CONFIG_SETUP_MEMORY_TAGS */


static void setup_commandline_tag (bd_t *bd, char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}


#ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag (bd_t *bd, ulong initrd_start, ulong initrd_end)
{
	/* an ATAG_INITRD node tells the kernel where the compressed
	 * ramdisk can be found. ATAG_RDIMG is a better name, actually.
	 */
	params->hdr.tag = ATAG_INITRD2;
	params->hdr.size = tag_size (tag_initrd);

	params->u.initrd.start = initrd_start;
	params->u.initrd.size = initrd_end - initrd_start;

	params = tag_next (params);
}
#endif /* CONFIG_INITRD_TAG */


#if defined (CONFIG_VFD) || defined (CONFIG_LCD)
extern ulong calc_fbsize (void);
static void setup_videolfb_tag (gd_t *gd)
{
	/* An ATAG_VIDEOLFB node tells the kernel where and how large
	 * the framebuffer for video was allocated (among other things).
	 * Note that a _physical_ address is passed !
	 *
	 * We only use it to pass the address and size, the other entries
	 * in the tag_videolfb are not of interest.
	 */
	params->hdr.tag = ATAG_VIDEOLFB;
	params->hdr.size = tag_size (tag_videolfb);

	params->u.videolfb.lfb_base = (u32) gd->fb_base;
	/* Fb size is calculated according to parameters for our panel
	 */
	params->u.videolfb.lfb_size = calc_fbsize();

	params = tag_next (params);
}
#endif /* CONFIG_VFD || CONFIG_LCD */

#ifdef CONFIG_SERIAL_TAG
void setup_serial_tag (struct tag **tmp)
{
	struct tag *params = *tmp;
	struct tag_serialnr serialnr;
	void get_board_serial(struct tag_serialnr *serialnr);

	get_board_serial(&serialnr);
	params->hdr.tag = ATAG_SERIAL;
	params->hdr.size = tag_size (tag_serialnr);
	params->u.serialnr.low = serialnr.low;
	params->u.serialnr.high= serialnr.high;
	params = tag_next (params);
	*tmp = params;
}
#endif

#ifdef CONFIG_REVISION_TAG
void setup_revision_tag(struct tag **in_params)
{
	u32 rev = 0;
	u32 get_board_rev(void);

	rev = get_board_rev();
	params->hdr.tag = ATAG_REVISION;
	params->hdr.size = tag_size (tag_revision);
	params->u.revision.rev = rev;
	params = tag_next (params);
}
#endif  /* CONFIG_REVISION_TAG */


static void setup_end_tag (bd_t *bd)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || CONFIG_INITRD_TAG */
