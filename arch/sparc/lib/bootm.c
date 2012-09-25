/* SPARC code for booting linux 2.6
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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
#include <asm/byteorder.h>
#include <asm/prom.h>
#include <asm/cache.h>
#include <image.h>

#define PRINT_KERNEL_HEADER

extern image_header_t header;
extern void srmmu_init_cpu(unsigned int entry);
extern void prepare_bootargs(char *bootargs);

#ifdef CONFIG_USB_UHCI
extern int usb_lowlevel_stop(int index);
#endif

/* sparc kernel argument (the ROM vector) */
struct linux_romvec *kernel_arg_promvec;

/* page szie is 4k */
#define PAGE_SIZE 0x1000
#define RAMDISK_IMAGE_START_MASK	0x07FF
#define RAMDISK_PROMPT_FLAG		0x8000
#define RAMDISK_LOAD_FLAG		0x4000
struct __attribute__ ((packed)) {
	char traptable[PAGE_SIZE];
	char swapper_pg_dir[PAGE_SIZE];
	char pg0[PAGE_SIZE];
	char pg1[PAGE_SIZE];
	char pg2[PAGE_SIZE];
	char pg3[PAGE_SIZE];
	char empty_bad_page[PAGE_SIZE];
	char empty_bad_page_table[PAGE_SIZE];
	char empty_zero_page[PAGE_SIZE];
	unsigned char hdr[4];	/* ascii "HdrS" */
	/* 00.02.06.0b is for Linux kernel 2.6.11 */
	unsigned char linuxver_mega_major;
	unsigned char linuxver_major;
	unsigned char linuxver_minor;
	unsigned char linuxver_revision;
	/* header version 0x0203 */
	unsigned short hdr_ver;
	union __attribute__ ((packed)) {
		struct __attribute__ ((packed)) {
			unsigned short root_flags;
			unsigned short root_dev;
			unsigned short ram_flags;
			unsigned int sparc_ramdisk_image;
			unsigned int sparc_ramdisk_size;
			unsigned int reboot_command;
			unsigned int resv[3];
			unsigned int end;
		} ver_0203;
	} hdr_input;
} *linux_hdr;

/* temporary initrd image holder */
image_header_t ihdr;

void arch_lmb_reserve(struct lmb *lmb)
{
	/* Reserve the space used by PROM and stack. This is done
	 * to avoid that the RAM image is copied over stack or
	 * PROM.
	 */
	lmb_reserve(lmb, CONFIG_SYS_RELOC_MONITOR_BASE, CONFIG_SYS_RAM_END);
}

/* boot the linux kernel */
int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t * images)
{
	char *bootargs;
	ulong initrd_start, initrd_end;
	ulong rd_len;
	void (*kernel) (struct linux_romvec *, void *);
	struct lmb *lmb = &images->lmb;
	int ret;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* Get virtual address of kernel start */
	linux_hdr = (void *)images->os.load;

	/* */
	kernel = (void (*)(struct linux_romvec *, void *))images->ep;

	/* check for a SPARC kernel */
	if ((linux_hdr->hdr[0] != 'H') ||
	    (linux_hdr->hdr[1] != 'd') ||
	    (linux_hdr->hdr[2] != 'r') || (linux_hdr->hdr[3] != 'S')) {
		puts("Error reading header of SPARC Linux kernel, aborting\n");
		goto error;
	}
#ifdef PRINT_KERNEL_HEADER
	printf("## Found SPARC Linux kernel %d.%d.%d ...\n",
	       linux_hdr->linuxver_major,
	       linux_hdr->linuxver_minor, linux_hdr->linuxver_revision);
#endif

#ifdef CONFIG_USB_UHCI
	usb_lowlevel_stop();
#endif

	/* set basic boot params in kernel header now that it has been
	 * extracted and is writeable.
	 */

	/* Calc length of RAM disk, if zero no ramdisk available */
	rd_len = images->rd_end - images->rd_start;

	if (rd_len) {
		ret = boot_ramdisk_high(lmb, images->rd_start, rd_len,
					&initrd_start, &initrd_end);
		if (ret) {
			puts("### Failed to relocate RAM disk\n");
			goto error;
		}

		/* Update SPARC kernel header so that Linux knows
		 * what is going on and where to find RAM disk.
		 *
		 * Set INITRD Image address relative to RAM Start
		 */
		linux_hdr->hdr_input.ver_0203.sparc_ramdisk_image =
		    initrd_start - CONFIG_SYS_RAM_BASE;
		linux_hdr->hdr_input.ver_0203.sparc_ramdisk_size = rd_len;
		/* Clear READ ONLY flag if set to non-zero */
		linux_hdr->hdr_input.ver_0203.root_flags = 1;
		/* Set root device to: Root_RAM0 */
		linux_hdr->hdr_input.ver_0203.root_dev = 0x100;
		linux_hdr->hdr_input.ver_0203.ram_flags = 0;
	} else {
		/* NOT using RAMDISK image, overwriting kernel defaults */
		linux_hdr->hdr_input.ver_0203.sparc_ramdisk_image = 0;
		linux_hdr->hdr_input.ver_0203.sparc_ramdisk_size = 0;
		/* Leave to kernel defaults
		   linux_hdr->hdr_input.ver_0203.root_flags = 1;
		   linux_hdr->hdr_input.ver_0203.root_dev = 0;
		   linux_hdr->hdr_input.ver_0203.ram_flags = 0;
		 */
	}

	/* Copy bootargs from bootargs variable to kernel readable area */
	bootargs = getenv("bootargs");
	prepare_bootargs(bootargs);

	/* turn on mmu & setup context table & page table for process 0 (kernel) */
	srmmu_init_cpu((unsigned int)kernel);

	/* Enter SPARC Linux kernel
	 * From now on the only code in u-boot that will be
	 * executed is the PROM code.
	 */
	kernel(kernel_arg_promvec, (void *)images->ep);

	/* It will never come to this... */
	while (1) ;

      error:
	return 1;
}
