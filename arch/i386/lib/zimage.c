/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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

/*
 * Linux i386 zImage and bzImage loading
 *
 * based on the procdure described in
 * linux/Documentation/i386/boot.txt
 */

#include <common.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <asm/zimage.h>
#include <asm/realmode.h>
#include <asm/byteorder.h>
#include <asm/bootparam.h>
#include <asm/ic/sc520.h>

/*
 * Memory lay-out:
 *
 * relative to setup_base (which is 0x90000 currently)
 *
 *	0x0000-0x7FFF	Real mode kernel
 *	0x8000-0x8FFF	Stack and heap
 *	0x9000-0x90FF	Kernel command line
 */
#define DEFAULT_SETUP_BASE  0x90000
#define COMMAND_LINE_OFFSET 0x9000
#define HEAP_END_OFFSET     0x8e00

#define COMMAND_LINE_SIZE   2048

static void build_command_line(char *command_line, int auto_boot)
{
	char *env_command_line;

	command_line[0] = '\0';

	env_command_line =  getenv("bootargs");

	/* set console= argument if we use a serial console */
	if (NULL == strstr(env_command_line, "console=")) {
		if (0==strcmp(getenv("stdout"), "serial")) {

			/* We seem to use serial console */
			sprintf(command_line, "console=ttyS0,%s ",
				 getenv("baudrate"));
		}
	}

	if (auto_boot) {
		strcat(command_line, "auto ");
	}

	if (NULL != env_command_line) {
		strcat(command_line, env_command_line);
	}


	printf("Kernel command line: \"%s\"\n", command_line);
}

void *load_zimage(char *image, unsigned long kernel_size,
		  unsigned long initrd_addr, unsigned long initrd_size,
		  int auto_boot)
{
	void *setup_base;
	int setup_size;
	int bootproto;
	int big_image;
	void *load_address;

	struct setup_header *hdr = (struct setup_header *)(image + SETUP_SECTS_OFF);

	setup_base = (void*)DEFAULT_SETUP_BASE;	/* base address for real-mode segment */

	if (KERNEL_MAGIC != hdr->boot_flag) {
		printf("Error: Invalid Boot Flag (found 0x%04x, expected 0x%04x)\n",
				hdr->boot_flag, KERNEL_MAGIC);
		return 0;
	} else {
		printf("Valid Boot Flag\n");
	}

	/* determine boot protocol version */
	if (KERNEL_V2_MAGIC == hdr->header) {
		printf("Magic signature found\n");

		bootproto = hdr->version;
	} else {
		/* Very old kernel */
		printf("Magic signature not found\n");
		bootproto = 0x0100;
	}

	/* determine size of setup */
	if (0 == hdr->setup_sects) {
		printf("Setup Sectors = 0 (defaulting to 4)\n");
		setup_size = 5 * 512;
	} else {
		setup_size = (hdr->setup_sects + 1) * 512;
	}

	printf("Setup Size = 0x%8.8lx\n", (ulong)setup_size);

	if (setup_size > SETUP_MAX_SIZE) {
		printf("Error: Setup is too large (%d bytes)\n", setup_size);
	}

	/* Determine image type */
	big_image = (bootproto >= 0x0200) && (hdr->loadflags & BIG_KERNEL_FLAG);

	/* Determine load address */
	load_address = (void*)(big_image ? BZIMAGE_LOAD_ADDR : ZIMAGE_LOAD_ADDR);

	/* load setup */
	printf("Moving Real-Mode Code to 0x%8.8lx (%d bytes)\n", (ulong)setup_base, setup_size);
	memmove(setup_base, image, setup_size);

	printf("Using boot protocol version %x.%02x\n",
	       (bootproto & 0xff00) >> 8, bootproto & 0xff);

	if (bootproto == 0x0100) {

		*(u16*)(setup_base + CMD_LINE_MAGIC_OFF) = COMMAND_LINE_MAGIC;
		*(u16*)(setup_base + CMD_LINE_OFFSET_OFF) = COMMAND_LINE_OFFSET;

		/* A very old kernel MUST have its real-mode code
		 * loaded at 0x90000 */

		if ((u32)setup_base != 0x90000) {
			/* Copy the real-mode kernel */
			memmove((void*)0x90000, setup_base, setup_size);
			/* Copy the command line */
			memmove((void*)0x99000, setup_base+COMMAND_LINE_OFFSET,
			       COMMAND_LINE_SIZE);

			setup_base = (void*)0x90000;		 /* Relocated */
		}

		/* It is recommended to clear memory up to the 32K mark */
		memset((void*)0x90000 + setup_size, 0, SETUP_MAX_SIZE-setup_size);
	}

	/* We are now setting up the real-mode version of the header */
	hdr = (struct setup_header *)(setup_base + SETUP_SECTS_OFF);

	if (bootproto >= 0x0200) {
		hdr->type_of_loader = 8;

		if (hdr->setup_sects >= 15)
			printf("Linux kernel version %s\n", (char *)
					(setup_base + (hdr->kernel_version + 0x200)));
		else
			printf("Setup Sectors < 15 - Cannot print kernel version.\n");

		if (initrd_addr) {
			printf("Initial RAM disk at linear address 0x%08lx, size %ld bytes\n",
			       initrd_addr, initrd_size);

			hdr->ramdisk_image = initrd_addr;
			hdr->ramdisk_size = initrd_size;
		}
	}

	if (bootproto >= 0x0201) {
		hdr->heap_end_ptr = HEAP_END_OFFSET;
		hdr->loadflags |= HEAP_FLAG;
	}

	if (bootproto >= 0x0202) {
		hdr->cmd_line_ptr = (u32)setup_base + COMMAND_LINE_OFFSET;
	} else if (bootproto >= 0x0200) {

		*(u16*)(setup_base + CMD_LINE_MAGIC_OFF) = COMMAND_LINE_MAGIC;
		*(u16*)(setup_base + CMD_LINE_OFFSET_OFF) = COMMAND_LINE_OFFSET;

		hdr->setup_move_size = 0x9100;
	}

	if (bootproto >= 0x0204)
		kernel_size = hdr->syssize * 16;
	else
		kernel_size -= setup_size;


	if (big_image) {
		if ((kernel_size) > BZIMAGE_MAX_SIZE) {
			printf("Error: bzImage kernel too big! (size: %ld, max: %d)\n",
			       kernel_size, BZIMAGE_MAX_SIZE);
			return 0;
		}

	} else if ((kernel_size) > ZIMAGE_MAX_SIZE) {
		printf("Error: zImage kernel too big! (size: %ld, max: %d)\n",
		       kernel_size, ZIMAGE_MAX_SIZE);
		return 0;
	}

	/* build command line at COMMAND_LINE_OFFSET */
	build_command_line(setup_base + COMMAND_LINE_OFFSET, auto_boot);

	printf("Loading %czImage at address 0x%08x (%ld bytes)\n", big_image ? 'b' : ' ',
	       (u32)load_address, kernel_size);


	memmove(load_address, image + setup_size, kernel_size);

	/* ready for booting */
	return setup_base;
}

void boot_zimage(void *setup_base)
{
	struct pt_regs regs;

	memset(&regs, 0, sizeof(struct pt_regs));
	regs.xds = (u32)setup_base >> 4;
	regs.xes = regs.xds;
	regs.xss = regs.xds;
	regs.esp = 0x9000;
	regs.eflags = 0;
	enter_realmode(((u32)setup_base+SETUP_START_OFFSET)>>4, 0, &regs, &regs);
}

int do_zboot (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	void *base_ptr;
	void *bzImage_addr;
	ulong bzImage_size = 0;

	disable_interrupts();

	/* Setup board for maximum PC/AT Compatibility */
	setup_pcat_compatibility();

	/* argv[1] holds the address of the bzImage */
	bzImage_addr = (void *)simple_strtoul(argv[1], NULL, 16);

	if (argc == 3)
		bzImage_size = simple_strtoul(argv[2], NULL, 16);

	/* Lets look for*/
	base_ptr = load_zimage (bzImage_addr, bzImage_size, 0, 0, 0);

	if (NULL == base_ptr) {
		printf ("## Kernel loading failed ...\n");
	} else {
		printf ("## Transferring control to Linux (at address %08x) ...\n",
			(u32)base_ptr);

		/* we assume that the kernel is in place */
		printf("\nStarting kernel ...\n\n");

		boot_zimage(base_ptr);
		/* does not return */
	}

	return -1;
}

U_BOOT_CMD(
	zboot, 3, 0,	do_zboot,
	"Boot bzImage",
	""
);
