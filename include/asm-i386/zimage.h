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

#ifndef _ASM_ZIMAGE_H_
#define _ASM_ZIMAGE_H_

/* linux i386 zImage/bzImage header. Offsets relative to
 * the start of the image */

#define CMD_LINE_MAGIC_OFF  0x020 /* Magic 0xa33f if the offset below is valid */
#define CMD_LINE_OFFSET_OFF 0x022 /* Offset to comandline */
#define SETUP_SECTS_OFF     0x1F1 /* The size of the setup in sectors */
#define ROOT_FLAGS_OFF      0x1F2 /* If set, the root is mounted readonly */
#define VID_MODE_OFF        0x1FA /* Video mode control */
#define ROOT_DEV_OFF        0x1FC /* Default root device number */
#define BOOT_FLAG_OFF       0x1FE /* 0xAA55 magic number */
#define HEADER_OFF          0x202 /* Magic signature "HdrS" */
#define VERSION_OFF         0x206 /* Boot protocol version supported */
#define REALMODE_SWTCH_OFF  0x208 /* Boot loader hook (see below) */
#define START_SYS_OFF       0x20C /* Points to kernel version string */
#define TYPE_OF_LOADER_OFF  0x210 /* Boot loader identifier */
#define LOADFLAGS_OFF       0x211 /* Boot protocol option flags */
#define SETUP_MOVE_SIZE_OFF 0x212 /* Move to high memory size (used with hooks) */
#define CODE32_START_OFF    0x214 /* Boot loader hook (see below) */
#define RAMDISK_IMAGE_OFF   0x218 /* initrd load address (set by boot loader) */
#define RAMDISK_SIZE_OFF    0x21C /* initrd size (set by boot loader) */
#define HEAP_END_PTR_OFF    0x224 /* Free memory after setup end */
#define CMD_LINE_PTR_OFF    0x228 /* 32-bit pointer to the kernel command line */


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

void *load_zimage(char *image, unsigned long kernel_size,
		  unsigned long initrd_addr, unsigned long initrd_size,
		  int auto_boot);

void boot_zimage(void *setup_base);

#endif
