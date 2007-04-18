/*
 * Copyright (C) 2004-2006 Atmel Corporation
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
#include <zlib.h>
#include <asm/byteorder.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/* CPU-specific hook to allow flushing of caches, etc. */
extern void prepare_to_boot(void);

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg) show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

extern image_header_t header;		/* from cmd_bootm.c */

static struct tag *setup_start_tag(struct tag *params)
{
	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size(tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 4096;
	params->u.core.rootdev = 0;

	return tag_next(params);
}

static struct tag *setup_memory_tags(struct tag *params)
{
	bd_t *bd = gd->bd;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size(tag_mem_range);

		params->u.mem_range.addr = bd->bi_dram[i].start;
		params->u.mem_range.size = bd->bi_dram[i].size;

		params = tag_next(params);
	}

	return params;
}

static struct tag *setup_commandline_tag(struct tag *params, char *cmdline)
{
	if (!cmdline)
		return params;

	/* eat leading white space */
	while (*cmdline == ' ') cmdline++;

	/*
	 * Don't include tags for empty command lines; let the kernel
	 * use its default command line.
	 */
	if (*cmdline == '\0')
		return params;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen(cmdline) + 1 + 3) >> 2;
	strcpy(params->u.cmdline.cmdline, cmdline);

	return tag_next(params);
}

static struct tag *setup_ramdisk_tag(struct tag *params,
				     unsigned long rd_start,
				     unsigned long rd_end)
{
	if (rd_start == rd_end)
		return params;

	params->hdr.tag = ATAG_RDIMG;
	params->hdr.size = tag_size(tag_mem_range);

	params->u.mem_range.addr = rd_start;
	params->u.mem_range.size = rd_end - rd_start;

	return tag_next(params);
}

static struct tag *setup_clock_tags(struct tag *params)
{
	params->hdr.tag = ATAG_CLOCK;
	params->hdr.size = tag_size(tag_clock);
	params->u.clock.clock_id = ACLOCK_BOOTCPU;
	params->u.clock.clock_flags = 0;
	params->u.clock.clock_hz = gd->cpu_hz;

#ifdef CONFIG_AT32AP7000
	/*
	 * New kernels don't need this, but we should be backwards
	 * compatible for a while...
	 */
	params = tag_next(params);

	params->hdr.tag = ATAG_CLOCK;
	params->hdr.size = tag_size(tag_clock);
	params->u.clock.clock_id = ACLOCK_HSB;
	params->u.clock.clock_flags = 0;
	params->u.clock.clock_hz = get_hsb_clk_rate();
#endif

	return tag_next(params);
}

static struct tag *setup_ethernet_tag(struct tag *params,
				      char *addr, int index)
{
	char *s, *e;
	int i;

	params->hdr.tag = ATAG_ETHERNET;
	params->hdr.size = tag_size(tag_ethernet);

	params->u.ethernet.mac_index = index;
	params->u.ethernet.mii_phy_addr = gd->bd->bi_phy_id[index];

	s = addr;
	for (i = 0; i < 6; i++) {
		params->u.ethernet.hw_address[i] = simple_strtoul(s, &e, 16);
		s = e + 1;
	}

	return tag_next(params);
}

static struct tag *setup_ethernet_tags(struct tag *params)
{
	char name[16] = "ethaddr";
	char *addr;
	int i = 0;

	do {
		addr = getenv(name);
		if (addr)
			params = setup_ethernet_tag(params, addr, i);
		sprintf(name, "eth%daddr", ++i);
	} while (i < 4);

	return params;
}

static void setup_end_tag(struct tag *params)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

void do_bootm_linux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		    unsigned long addr, unsigned long *len_ptr, int verify)
{
	unsigned long data, len = 0;
	unsigned long initrd_start, initrd_end;
	unsigned long image_start, image_end;
	unsigned long checksum;
	void (*theKernel)(int magic, void *tagtable);
	image_header_t *hdr;
	struct tag *params, *params_start;
	char *commandline = getenv("bootargs");

	hdr = (image_header_t *)addr;
	image_start = addr;
	image_end = addr + hdr->ih_size;

	theKernel = (void *)ntohl(hdr->ih_ep);

	/*
	 * Check if there is an initrd image
	 */
	if (argc >= 3) {
		SHOW_BOOT_PROGRESS(9);

		addr = simple_strtoul(argv[2], NULL, 16);

		printf("## Loading RAMDISK image at %08lx ...\n", addr);

		memcpy(&header, (char *)addr, sizeof(header));
		hdr = &header;

		if (ntohl(hdr->ih_magic) != IH_MAGIC) {
			puts("Bad Magic Number\n");
			SHOW_BOOT_PROGRESS(-10);
			do_reset(cmdtp, flag, argc, argv);
		}

		data = (unsigned long)hdr;
		len = sizeof(*hdr);
		checksum = ntohl(hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (crc32(0, (unsigned char *)data, len) != checksum) {
			puts("Bad Header Checksum\n");
			SHOW_BOOT_PROGRESS(-11);
			do_reset(cmdtp, flag, argc, argv);
		}

		SHOW_BOOT_PROGRESS(10);

		print_image_hdr(hdr);

		data = addr + sizeof(header);
		len = ntohl(hdr->ih_size);

		if (verify) {
			unsigned long csum = 0;

			puts("   Verifying Checksum ... ");
			csum = crc32(0, (unsigned char *)data, len);
			if (csum != ntohl(hdr->ih_dcrc)) {
				puts("Bad Data CRC\n");
				SHOW_BOOT_PROGRESS(-12);
				do_reset(cmdtp, flag, argc, argv);
			}
			puts("OK\n");
		}

		SHOW_BOOT_PROGRESS(11);

		if ((hdr->ih_os != IH_OS_LINUX) ||
		    (hdr->ih_arch != IH_CPU_AVR32) ||
		    (hdr->ih_type != IH_TYPE_RAMDISK)) {
			puts("Not a Linux/AVR32 RAMDISK image\n");
			SHOW_BOOT_PROGRESS(-13);
			do_reset(cmdtp, flag, argc, argv);
		}
	} else if ((hdr->ih_type == IH_TYPE_MULTI) && (len_ptr[1])) {
		ulong tail = ntohl (len_ptr[0]) % 4;
		int i;

		SHOW_BOOT_PROGRESS (13);

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
		/* no initrd image */
		SHOW_BOOT_PROGRESS(14);
		len = data = 0;
	}

	if (data) {
		initrd_start = data;
		initrd_end = initrd_start + len;
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	SHOW_BOOT_PROGRESS(15);

	params = params_start = (struct tag *)gd->bd->bi_boot_params;
	params = setup_start_tag(params);
	params = setup_memory_tags(params);
	if (initrd_start) {
		params = setup_ramdisk_tag(params,
					   PHYSADDR(initrd_start),
					   PHYSADDR(initrd_end));
	}
	params = setup_commandline_tag(params, commandline);
	params = setup_clock_tags(params);
	params = setup_ethernet_tags(params);
	setup_end_tag(params);

	printf("\nStarting kernel at %p (params at %p)...\n\n",
	       theKernel, params_start);

	prepare_to_boot();

	theKernel(ATAG_MAGIC, params_start);
}
