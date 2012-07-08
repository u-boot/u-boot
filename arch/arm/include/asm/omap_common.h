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
#ifdef CONFIG_OMAP54XX
#define BOOT_DEVICE_NONE        0
#define BOOT_DEVICE_XIP         1
#define BOOT_DEVICE_XIPWAIT     2
#define BOOT_DEVICE_NAND        3
#define BOOT_DEVICE_ONE_NAND    4
#define BOOT_DEVICE_MMC1        5
#define BOOT_DEVICE_MMC2        6
#define BOOT_DEVICE_MMC2_2	7
#elif defined(CONFIG_OMAP44XX) /* OMAP4 */
#define BOOT_DEVICE_NONE	0
#define BOOT_DEVICE_XIP		1
#define BOOT_DEVICE_XIPWAIT	2
#define BOOT_DEVICE_NAND	3
#define BOOT_DEVICE_ONE_NAND	4
#define BOOT_DEVICE_MMC1	5
#define BOOT_DEVICE_MMC2	6
#define BOOT_DEVICE_MMC2_2	0xFF
#elif defined(CONFIG_OMAP34XX)	/* OMAP3 */
#define BOOT_DEVICE_NONE	0
#define BOOT_DEVICE_XIP		1
#define BOOT_DEVICE_NAND	2
#define BOOT_DEVICE_ONE_NAND	3
#define BOOT_DEVICE_MMC2	5 /*emmc*/
#define BOOT_DEVICE_MMC1	6
#define BOOT_DEVICE_XIPWAIT	7
#define BOOT_DEVICE_MMC2_2      0xFF
#elif defined(CONFIG_AM33XX)	/* AM33XX */
#define BOOT_DEVICE_NAND	5
#define BOOT_DEVICE_MMC1	8
#define BOOT_DEVICE_MMC2	0
#define BOOT_DEVICE_UART	65
#define BOOT_DEVICE_MMC2_2      0xFF
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

extern u32* boot_params_ptr;
u32 omap_boot_device(void);
u32 omap_boot_mode(void);

/* SPL common function s*/
void spl_parse_image_header(const struct image_header *header);
void omap_rev_string(void);
void spl_board_prepare_for_linux(void);
int spl_start_uboot(void);

/* NAND SPL functions */
void spl_nand_load_image(void);

/* MMC SPL functions */
void spl_mmc_load_image(void);

/* YMODEM SPL functions */
void spl_ymodem_load_image(void);

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void);
#endif

static inline u32 omap_revision(void)
{
	extern u32 *const omap_si_rev;
	return *omap_si_rev;
}

/*
 * silicon revisions.
 * Moving this to common, so that most of code can be moved to common,
 * directories.
 */

/* omap4 */
#define OMAP4430_SILICON_ID_INVALID	0xFFFFFFFF
#define OMAP4430_ES1_0	0x44300100
#define OMAP4430_ES2_0	0x44300200
#define OMAP4430_ES2_1	0x44300210
#define OMAP4430_ES2_2	0x44300220
#define OMAP4430_ES2_3	0x44300230
#define OMAP4460_ES1_0	0x44600100
#define OMAP4460_ES1_1	0x44600110

/* omap5 */
#define OMAP5430_SILICON_ID_INVALID	0
#define OMAP5430_ES1_0	0x54300100
#define OMAP5432_ES1_0	0x54320100
#endif /* _OMAP_COMMON_H_ */
