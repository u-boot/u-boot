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


	setup_base = (void*)DEFAULT_SETUP_BASE;	/* base address for real-mode segment */

	if (KERNEL_MAGIC != *(u16*)(image + BOOT_FLAG_OFF)) {
		printf("Error: Invalid kernel magic (found 0x%04x, expected 0xaa55)\n",
		       *(u16*)(image + BOOT_FLAG_OFF));
		return 0;
	}


	/* determine boot protocol version */
	if (KERNEL_V2_MAGIC == *(u32*)(image+HEADER_OFF)) {
		bootproto = *(u16*)(image+VERSION_OFF);
	} else {
		/* Very old kernel */
		bootproto = 0x0100;
	}

	/* determine size of setup */
	if (0 == *(u8*)(image + SETUP_SECTS_OFF)) {
		setup_size = 5 * 512;
	} else {
		setup_size = (*(u8*)(image + SETUP_SECTS_OFF) + 1) * 512;
	}

	if (setup_size > SETUP_MAX_SIZE) {
		printf("Error: Setup is too large (%d bytes)\n", setup_size);
	}

	/* Determine image type */
	big_image = (bootproto >= 0x0200) && (*(u8*)(image + LOADFLAGS_OFF) & BIG_KERNEL_FLAG);

	/* Derermine load address */
	load_address = (void*)(big_image ? BZIMAGE_LOAD_ADDR:ZIMAGE_LOAD_ADDR);

	/* load setup */
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

	if (bootproto >= 0x0200) {
		*(u8*)(setup_base + TYPE_OF_LOADER_OFF) = 0xff;
		printf("Linux kernel version %s\n",
		       (char*)(setup_base + SETUP_START_OFFSET +
			       *(u16*)(setup_base + START_SYS_OFF + 2)));

		if (initrd_addr) {
			printf("Initial RAM disk at linear address 0x%08lx, size %ld bytes\n",
			       initrd_addr, initrd_size);

			*(u32*)(setup_base + RAMDISK_IMAGE_OFF) = initrd_addr;
			*(u32*)(setup_base + RAMDISK_SIZE_OFF)=initrd_size;
		}
	}

	if (bootproto >= 0x0201) {
		*(u16*)(setup_base + HEAP_END_PTR_OFF) = HEAP_END_OFFSET;

		/* CAN_USE_HEAP */
		*(u8*)(setup_base + LOADFLAGS_OFF) =
			*(u8*)(setup_base + LOADFLAGS_OFF) | HEAP_FLAG;
	}

	if (bootproto >= 0x0202) {
		*(u32*)(setup_base + CMD_LINE_PTR_OFF) = (u32)setup_base + COMMAND_LINE_OFFSET;
	} else if (bootproto >= 0x0200) {
		*(u16*)(setup_base + CMD_LINE_MAGIC_OFF) = COMMAND_LINE_MAGIC;
		*(u16*)(setup_base + CMD_LINE_OFFSET_OFF) = COMMAND_LINE_OFFSET;
		*(u16*)(setup_base + SETUP_MOVE_SIZE_OFF) = 0x9100;
	}


	if (big_image) {
		if ((kernel_size - setup_size) > BZIMAGE_MAX_SIZE) {
			printf("Error: bzImage kernel too big! (size: %ld, max: %d)\n",
			       kernel_size - setup_size, BZIMAGE_MAX_SIZE);
			return 0;
		}

	} else if ((kernel_size - setup_size) > ZIMAGE_MAX_SIZE) {
		printf("Error: zImage kernel too big! (size: %ld, max: %d)\n",
		       kernel_size - setup_size, ZIMAGE_MAX_SIZE);
		return 0;
	}

	/* build command line at COMMAND_LINE_OFFSET */
	build_command_line(setup_base + COMMAND_LINE_OFFSET, auto_boot);

	printf("Loading %czImage at address 0x%08x (%ld bytes)\n", big_image ? 'b' : ' ',
	       (u32)load_address, kernel_size - setup_size);


	memmove(load_address, image + setup_size, kernel_size - setup_size);

	/* ready for booting */
	return setup_base;
}


void boot_zimage(void *setup_base)
{
	struct pt_regs regs;

	memset(&regs, 0, sizeof(struct pt_regs));
	regs.xds = (u32)setup_base >> 4;
	regs.xss = 0x9000;
	regs.esp = 0x9000;
	regs.eflags = 0;
	enter_realmode(((u32)setup_base+SETUP_START_OFFSET)>>4, 0, &regs, &regs);
}


image_header_t *fake_zimage_header(image_header_t *hdr, void *ptr, int size)
{
	/* There is no way to know the size of a zImage ... *
	 * so we assume that 2MB will be enough for now */
#define ZIMAGE_SIZE 0x200000

	/* load a 1MB, the loaded will have to be moved to its final
	 * position again later... */
#define ZIMAGE_LOAD 0x100000

	ulong checksum;

	if (KERNEL_MAGIC != *(u16*)(ptr + BOOT_FLAG_OFF)) {
		/* not a zImage or bzImage */
		return NULL;
	}

	if (-1 == size) {
		size = ZIMAGE_SIZE;
	}
#if 0
	checksum = crc32 (0, ptr, size);
#else
	checksum = 0;
#endif
	memset(hdr, 0, sizeof(image_header_t));

	/* Build new header */
	hdr->ih_magic = htonl(IH_MAGIC);
	hdr->ih_time  = 0;
	hdr->ih_size  = htonl(size);
	hdr->ih_load  = htonl(ZIMAGE_LOAD);
	hdr->ih_ep    = 0;
	hdr->ih_dcrc  = htonl(checksum);
	hdr->ih_os    = IH_OS_LINUX;
	hdr->ih_arch  = IH_CPU_I386;
	hdr->ih_type  = IH_TYPE_KERNEL;
	hdr->ih_comp  = IH_COMP_NONE;

	strncpy((char *)hdr->ih_name, "(none)", IH_NMLEN);

	checksum = crc32(0,(const char *)hdr,sizeof(image_header_t));

	hdr->ih_hcrc = htonl(checksum);

	return hdr;
}
