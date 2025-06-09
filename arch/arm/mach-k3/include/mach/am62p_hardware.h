/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: AM62Px SoC definitions, structures etc.
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __ASM_ARCH_AM62P_HARDWARE_H
#define __ASM_ARCH_AM62P_HARDWARE_H

#include <config.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define PADCFG_MMR0_BASE			0x04080000
#define PADCFG_MMR1_BASE			0x000f0000
#define CTRL_MMR0_BASE				0x00100000
#define MCU_CTRL_MMR0_BASE			0x04500000
#define WKUP_CTRL_MMR0_BASE			0x43000000

#define CTRLMMR_WKUP_JTAG_DEVICE_ID		(WKUP_CTRL_MMR0_BASE + 0x18)
#define JTAG_DEV_CORE_NR_MASK			GENMASK(19, 18)
#define JTAG_DEV_CORE_NR_SHIFT			18
#define JTAG_DEV_CANFD_MASK			BIT(15)
#define JTAG_DEV_CANFD_SHIFT			15
#define JTAG_DEV_VIDEO_CODEC_MASK			BIT(14)
#define JTAG_DEV_VIDEO_CODEC_SHIFT			14
#define JTAG_DEV_SPEED_MASK			GENMASK(10, 6)
#define JTAG_DEV_SPEED_SHIFT			6
#define JTAG_DEV_TEMP_MASK			GENMASK(5, 3)
#define JTAG_DEV_TEMP_SHIFT			3

#define JTAG_DEV_TEMP_AUTOMOTIVE        0x5
#define JTAG_DEV_TEMP_EXTENDED_VALUE    105
#define JTAG_DEV_TEMP_AUTOMOTIVE_VALUE  125

#define CTRLMMR_MAIN_DEVSTAT			(WKUP_CTRL_MMR0_BASE + 0x30)
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK	GENMASK(6, 3)
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT	3
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK	GENMASK(9, 7)
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT	7
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK	GENMASK(12, 10)
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT	10
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK	BIT(13)
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT	13

/* Primary Bootmode MMC Config macros */
#define MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK	0x4
#define MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT	2
#define MAIN_DEVSTAT_PRIMARY_MMC_FS_RAW_MASK	0x1
#define MAIN_DEVSTAT_PRIMARY_MMC_FS_RAW_SHIFT	0

/* Primary Bootmode USB Config macros */
#define MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT	1
#define MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK	0x02

/* Backup Bootmode USB Config macros */
#define MAIN_DEVSTAT_BACKUP_USB_MODE_MASK	0x01

/*
 * The CTRL_MMR0 memory space is divided into several equally-spaced
 * partitions, so defining the partition size allows us to determine
 * register addresses common to those partitions.
 */
#define CTRL_MMR0_PARTITION_SIZE		0x4000

/*
 * CTRL_MMR0, WKUP_CTRL_MMR0, and MCU_CTRL_MMR0 lock/kick-mechanism
 * shared register definitions. The same registers are also used for
 * PADCFG_MMR lock/kick-mechanism.
 */
#define CTRLMMR_LOCK_KICK0			0x1008
#define CTRLMMR_LOCK_KICK0_UNLOCK_VAL		0x68ef3490
#define CTRLMMR_LOCK_KICK1			0x100c
#define CTRLMMR_LOCK_KICK1_UNLOCK_VAL		0xd172bc5a

#define MCU_CTRL_LFXOSC_CTRL			(MCU_CTRL_MMR0_BASE + 0x8038)
#define MCU_CTRL_LFXOSC_TRIM			(MCU_CTRL_MMR0_BASE + 0x803c)
#define MCU_CTRL_LFXOSC_32K_DISABLE_VAL		BIT(7)

#define MCU_CTRL_DEVICE_CLKOUT_32K_CTRL		(MCU_CTRL_MMR0_BASE + 0x8058)
#define MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL	(0x3)

#define ROM_EXTENDED_BOOT_DATA_INFO		0x43c4f1e0

#define K3_BOOT_PARAM_TABLE_INDEX_OCRAM         0x7000F290

#define TI_SRAM_SCRATCH_BOARD_EEPROM_START	0x43c30000

static inline int k3_get_core_nr(void)
{
	u32 dev_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	return ((dev_id & JTAG_DEV_CORE_NR_MASK) >> JTAG_DEV_CORE_NR_SHIFT) + 1;
}

static inline int k3_has_video_codec(void)
{
	u32 dev_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	return !((dev_id & JTAG_DEV_VIDEO_CODEC_MASK) >> JTAG_DEV_VIDEO_CODEC_SHIFT);
}

static inline int k3_has_canfd(void)
{
	u32 dev_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	return (dev_id & JTAG_DEV_CANFD_MASK) >> JTAG_DEV_CANFD_SHIFT;
}

static inline int k3_get_max_temp(void)
{
	u32 dev_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);
	u32 dev_temp = (dev_id & JTAG_DEV_TEMP_MASK) >> JTAG_DEV_TEMP_SHIFT;

	if (dev_temp == JTAG_DEV_TEMP_AUTOMOTIVE)
		return JTAG_DEV_TEMP_AUTOMOTIVE_VALUE;
	else
		return JTAG_DEV_TEMP_EXTENDED_VALUE;
}

static inline char k3_get_speed_grade(void)
{
	u32 dev_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);
	u32 speed_grade = (dev_id & JTAG_DEV_SPEED_MASK) >>
			   JTAG_DEV_SPEED_SHIFT;

	return 'A' - 1 + speed_grade;
}

static inline int k3_get_a53_max_frequency(void)
{
	if (k3_get_speed_grade() == 'O')
		return 1000000000;
	else
		return 1250000000;
}

#if defined(CONFIG_SYS_K3_SPL_ATF) && !defined(__ASSEMBLY__)

static const u32 put_device_ids[] = {};

#endif

static const u32 put_core_ids[] = {};

#endif /* __ASM_ARCH_AM62P_HARDWARE_H */
