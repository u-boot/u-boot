/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

/* Implementation defined function to install an e820 map. */
unsigned install_e820_map(unsigned max_entries, struct e820entry *);

struct boot_params *load_zimage(char *image, unsigned long kernel_size,
				void **load_address);
int setup_zimage(struct boot_params *setup_base, char *cmd_line, int auto_boot,
		 unsigned long initrd_addr, unsigned long initrd_size);

void boot_zimage(void *setup_base, void *load_address);

#endif
