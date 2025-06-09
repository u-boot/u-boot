/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: J722S SoC definitions, structures etc.
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __ASM_ARCH_J722S_HARDWARE_H
#define __ASM_ARCH_J722S_HARDWARE_H

#include <config.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define PADCFG_MMR0_BASE			0x04080000
#define PADCFG_MMR1_BASE			0x000f0000
#define CTRL_MMR0_BASE				0x00100000
#define MCU_CTRL_MMR0_BASE			0x04500000
#define WKUP_CTRL_MMR0_BASE			0x43000000

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

#define ROM_EXTENDED_BOOT_DATA_INFO		0x43c7f1e0

#define K3_BOOT_PARAM_TABLE_INDEX_OCRAM		0x7000F290

#define TI_SRAM_SCRATCH_BOARD_EEPROM_START	0x43c30000

#if defined(CONFIG_SYS_K3_SPL_ATF) && !defined(__ASSEMBLY__)

static const u32 put_device_ids[] = {};

#endif

static const u32 put_core_ids[] = {};

#endif /* __ASM_ARCH_J722S_HARDWARE_H */
