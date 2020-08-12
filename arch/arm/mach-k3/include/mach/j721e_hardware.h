/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: J721E SoC definitions, structures etc.
 *
 * (C) Copyright (C) 2018-2019 Texas Instruments Incorporated - http://www.ti.com/
 */
#ifndef __ASM_ARCH_J721E_HARDWARE_H
#define __ASM_ARCH_J721E_HARDWARE_H

#include <config.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define CTRL_MMR0_BASE					0x00100000
#define CTRLMMR_MAIN_DEVSTAT				(CTRL_MMR0_BASE + 0x30)

#define MAIN_DEVSTAT_BOOT_MODE_B_MASK		BIT(0)
#define MAIN_DEVSTAT_BOOT_MODE_B_SHIFT		0
#define MAIN_DEVSTAT_BKUP_BOOTMODE_MASK		GENMASK(3, 1)
#define MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT	1
#define MAIN_DEVSTAT_PRIM_BOOTMODE_MMC_PORT_MASK	BIT(6)
#define MAIN_DEVSTAT_PRIM_BOOTMODE_PORT_SHIFT		6
#define MAIN_DEVSTAT_BKUP_MMC_PORT_MASK			BIT(7)
#define MAIN_DEVSTAT_BKUP_MMC_PORT_SHIFT		7

#define WKUP_CTRL_MMR0_BASE				0x43000000
#define MCU_CTRL_MMR0_BASE				0x40f00000

#define CTRLMMR_WKUP_DEVSTAT			(WKUP_CTRL_MMR0_BASE + 0x30)
#define WKUP_DEVSTAT_PRIMARY_BOOTMODE_MASK	GENMASK(5, 3)
#define WKUP_DEVSTAT_PRIMARY_BOOTMODE_SHIFT	3
#define WKUP_DEVSTAT_MCU_OMLY_MASK		BIT(6)
#define WKUP_DEVSTAT_MCU_ONLY_SHIFT		6

/*
 * The CTRL_MMR0 memory space is divided into several equally-spaced
 * partitions, so defining the partition size allows us to determine
 * register addresses common to those partitions.
 */
#define CTRL_MMR0_PARTITION_SIZE			0x4000

/*
 * CTRL_MMR0, WKUP_CTRL_MMR0, and MCU_CTR_MMR0 lock/kick-mechanism
 * shared register definitions.
 */
#define CTRLMMR_LOCK_KICK0				0x01008
#define CTRLMMR_LOCK_KICK0_UNLOCK_VAL			0x68ef3490
#define CTRLMMR_LOCK_KICK0_UNLOCKED_MASK		BIT(0)
#define CTRLMMR_LOCK_KICK0_UNLOCKED_SHIFT		0
#define CTRLMMR_LOCK_KICK1				0x0100c
#define CTRLMMR_LOCK_KICK1_UNLOCK_VAL			0xd172bc5a

/* ROM HANDOFF Structure location */
#define ROM_ENTENDED_BOOT_DATA_INFO			0x41cffb00

/* MCU SCRATCHPAD usage */
#define TI_SRAM_SCRATCH_BOARD_EEPROM_START	CONFIG_SYS_K3_MCU_SCRATCHPAD_BASE

#endif /* __ASM_ARCH_J721E_HARDWARE_H */
