/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 */

#ifndef _ASM_ZIMAGE_H_
#define _ASM_ZIMAGE_H_

#include <asm/bootparam.h>
#include <asm/e820.h>

/* linux i386 zImage/bzImage header. Offsets relative to
 * the start of the image */

#define HEAP_FLAG           0x80
#define BIG_KERNEL_FLAG     0x01

/* magic numbers */
#define KERNEL_MAGIC        0xaa55
#define KERNEL_V2_MAGIC     0x53726448
#define COMMAND_LINE_MAGIC  0xA33F

/* limits */
#define BZIMAGE_MAX_SIZE   15*1024*1024     /* 15MB */
#define ZIMAGE_MAX_SIZE    512*1024         /* 512k */
#define SETUP_MAX_SIZE     32768

#define SETUP_START_OFFSET 0x200
#define BZIMAGE_LOAD_ADDR  0x100000
#define ZIMAGE_LOAD_ADDR   0x10000

enum {
	ZBOOT_STATE_START	= BIT(0),
	ZBOOT_STATE_LOAD	= BIT(1),
	ZBOOT_STATE_SETUP	= BIT(2),
	ZBOOT_STATE_INFO	= BIT(3),
	ZBOOT_STATE_GO		= BIT(4),

	/* This one doesn't execute automatically, so stop the count before 5 */
	ZBOOT_STATE_DUMP	= BIT(5),
	ZBOOT_STATE_COUNT	= 5,
};

/**
 * struct zboot_state - Current state of the boot
 *
 * @bzimage_addr: Address of the bzImage to boot, or 0 if the image has already
 *	been loaded and does not exist (as a cohesive whole) in memory
 * @bzimage_size: Size of the bzImage, or 0 to detect this
 * @initrd_addr: Address of the initial ramdisk, or 0 if none
 * @initrd_size: Size of the initial ramdisk, or 0 if none
 * @load_address: Address where the bzImage is moved before booting, either
 *	BZIMAGE_LOAD_ADDR or ZIMAGE_LOAD_ADDR
 *	This is set up when loading the zimage
 * @base_ptr: Pointer to the boot parameters, typically at address
 *	DEFAULT_SETUP_BASE
 *	This is set up when loading the zimage
 * @cmdline: Environment variable containing the 'override' command line, or
 *	NULL to use the one in the setup block
 */
struct zboot_state {
	ulong bzimage_addr;
	ulong bzimage_size;
	ulong initrd_addr;
	ulong initrd_size;
	ulong load_address;
	struct boot_params *base_ptr;
	const char *cmdline;
};

extern struct zboot_state state;

/**
 * zimage_dump() - Dump information about a zimage
 *
 * @base_ptr: Pointer to the boot parameters
 * @show_cmdline: true to show the kernel command line
 */
void zimage_dump(struct boot_params *base_ptr, bool show_cmdline);

/**
 * zboot_load() - Load a zimage
 *
 * Load the zimage into the correct place
 *
 * Return: 0 if OK, -ve on error
 */
int zboot_load(void);

/**
 * zboot_setup() - Set up the zboot image reeady for booting
 *
 * Return: 0 if OK, -ve on error
 */
int zboot_setup(void);

/**
 * zboot_go() - Start the image
 *
 * Return: 0 if OK, -ve on error
 */
int zboot_go(void);

/**
 * load_zimage() - Load a zImage or bzImage
 *
 * This copies an image into the standard location ready for setup
 *
 * @image: Address of image to load
 * @kernel_size: Size of kernel including setup block (or 0 if the kernel is
 *	new enough to have a 'syssize' value)
 * @load_addressp: Returns the address where the kernel has been loaded
 * Return: address of setup block, or NULL if something went wrong
 */
struct boot_params *load_zimage(char *image, unsigned long kernel_size,
				ulong *load_addressp);

/**
 * setup_zimage() - Set up a loaded zImage or bzImage ready for booting
 *
 * @setup_base: Pointer to the boot parameters, typically at address
 *	DEFAULT_SETUP_BASE
 * @cmd_line: Place to put the command line, or NULL to use the one in the setup
 *	block
 * @initrd_addr: Address of the initial ramdisk, or 0 if none
 * @initrd_size: Size of the initial ramdisk, or 0 if none
 * @load_address: Address where the bzImage is moved before booting, either
 *	BZIMAGE_LOAD_ADDR or ZIMAGE_LOAD_ADDR
 * @cmdline_force: Address of 'override' command line, or 0 to use the one in
 *	the *	setup block
 * Return: 0 (always)
 */
int setup_zimage(struct boot_params *setup_base, char *cmd_line, int auto_boot,
		 ulong initrd_addr, ulong initrd_size, ulong cmdline_force);

/**
 * zboot_start() - Prepare to boot a zimage
 *
 * Record information about a zimage so it can be booted
 *
 * @bzimage_addr: Address of the bzImage to boot
 * @bzimage_size: Size of the bzImage, or 0 to detect this
 * @initrd_addr: Address of the initial ramdisk, or 0 if none
 * @initrd_size: Size of the initial ramdisk, or 0 if none
 * @base_addr: If non-zero, this indicates that the boot parameters have already
 *	been loaded by the caller to this address, so the load_zimage() call
 *	in zboot_load() will be skipped when booting
 * @cmdline: Environment variable containing the 'override' command line, or
 *	NULL to use the one in the setup block
 */
void zboot_start(ulong bzimage_addr, ulong bzimage_size, ulong initrd_addr,
		 ulong initrd_size, ulong base_addr, const char *cmdline);

/**
 * zboot_info() - Show simple info about a zimage
 *
 * Shows wherer the kernel was loaded and also the setup base
 */
void zboot_info(void);

#endif
