/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef	_SPL_H_
#define	_SPL_H_

/* Platform-specific defines */
#include <linux/compiler.h>
#include <asm/spl.h>

/* Boot type */
#define MMCSD_MODE_UNDEFINED	0
#define MMCSD_MODE_RAW		1
#define MMCSD_MODE_FAT		2

struct spl_image_info {
	const char *name;
	u8 os;
	u32 load_addr;
	u32 entry_point;
	u32 size;
	u32 flags;
};

#define SPL_COPY_PAYLOAD_ONLY	1

extern struct spl_image_info spl_image;
extern u32 *boot_params_ptr;

/* SPL common functions */
void preloader_console_init(void);
u32 spl_boot_device(void);
u32 spl_boot_mode(void);
void spl_parse_image_header(const struct image_header *header);
void spl_board_prepare_for_linux(void);
void __noreturn jump_to_image_linux(void *arg);
int spl_start_uboot(void);
void spl_display_print(void);

/* NAND SPL functions */
void spl_nand_load_image(void);

/* NOR SPL functions */
void spl_nor_load_image(void);

/* MMC SPL functions */
void spl_mmc_load_image(void);

/* YMODEM SPL functions */
void spl_ymodem_load_image(void);

/* SPI SPL functions */
void spl_spi_load_image(void);

/* Ethernet SPL functions */
void spl_net_load_image(const char *device);

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void);
#endif
#endif
