/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
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
#ifndef	_OMAP_COMMON_H_
#define	_OMAP_COMMON_H_

/* Max value for DPLL multiplier M */
#define OMAP_DPLL_MAX_N	127

/* HW Init Context */
#define OMAP_INIT_CONTEXT_SPL			0
#define OMAP_INIT_CONTEXT_UBOOT_FROM_NOR	1
#define OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL	2
#define OMAP_INIT_CONTEXT_UBOOT_AFTER_CH	3

void preloader_console_init(void);

/* Boot device */
#ifdef CONFIG_OMAP44XX /* OMAP4 */
#define BOOT_DEVICE_NONE	0
#define BOOT_DEVICE_XIP		1
#define BOOT_DEVICE_XIPWAIT	2
#define BOOT_DEVICE_NAND	3
#define BOOT_DEVICE_ONE_NAND	4
#define BOOT_DEVICE_MMC1	5
#define BOOT_DEVICE_MMC2	6
#elif CONFIG_OMAP34XX /* OMAP3 */
#define BOOT_DEVICE_NONE	0
#define BOOT_DEVICE_XIP		1
#define BOOT_DEVICE_NAND	2
#define BOOT_DEVICE_ONE_NAND	3
#define BOOT_DEVICE_MMC2	5 /*emmc*/
#define BOOT_DEVICE_MMC1	6
#define BOOT_DEVICE_XIPWAIT	7
#endif

/* Boot type */
#define	MMCSD_MODE_UNDEFINED	0
#define MMCSD_MODE_RAW		1
#define MMCSD_MODE_FAT		2
#define NAND_MODE_HW_ECC	3

struct spl_image_info {
	const char *name;
	u8 os;
	u32 load_addr;
	u32 entry_point;
	u32 size;
};

extern struct spl_image_info spl_image;

u32 omap_boot_device(void);
u32 omap_boot_mode(void);


/* SPL common function s*/
void spl_parse_image_header(const struct image_header *header);

/* NAND SPL functions */
void spl_nand_load_image(void);

/* MMC SPL functions */
void spl_mmc_load_image(void);

#endif /* _OMAP_COMMON_H_ */
