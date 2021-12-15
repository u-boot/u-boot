/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: AM64 SoC definitions, structures etc.
 *
 * (C) Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 */
#ifndef __ASM_ARCH_AM64_HARDWARE_H
#define __ASM_ARCH_AM64_HARDWARE_H

#define CTRL_MMR0_BASE					0x43000000
#define CTRLMMR_MAIN_DEVSTAT				(CTRL_MMR0_BASE + 0x30)

#define PADCFG_MMR1_BASE				0xf0000

#define MCU_PADCFG_MMR1_BASE				0x04080000

#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK		0x00000078
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT		3

#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK		0x00000380
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT		7

#define MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK		0x00001c00
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT		10

#define MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK		0x00002000
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT		13

/* After the cfg mask and shifts have been applied */
#define MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT		2
#define MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK		0x04

#define MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT		1
#define MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK		0x02

#define MAIN_DEVSTAT_BACKUP_USB_MODE_MASK		0x01

/*
 * The CTRL_MMR and PADCFG_MMR memory space is divided into several
 * equally-spaced partitions, so defining the partition size allows us to
 * determine register addresses common to those partitions.
 */
#define CTRL_MMR0_PARTITION_SIZE			0x4000

/*
 * CTRL_MMR and PADCFG_MMR lock/kick-mechanism shared register definitions.
 */
#define CTRLMMR_LOCK_KICK0				0x01008
#define CTRLMMR_LOCK_KICK0_UNLOCK_VAL			0x68ef3490
#define CTRLMMR_LOCK_KICK0_UNLOCKED_MASK		BIT(0)
#define CTRLMMR_LOCK_KICK0_UNLOCKED_SHIFT		0
#define CTRLMMR_LOCK_KICK1				0x0100c
#define CTRLMMR_LOCK_KICK1_UNLOCK_VAL			0xd172bc5a

#define ROM_ENTENDED_BOOT_DATA_INFO			0x701beb00

/* Use Last 2K as Scratch pad */
#define TI_SRAM_SCRATCH_BOARD_EEPROM_START		0x7019f800

#endif /* __ASM_ARCH_DRA8_HARDWARE_H */
