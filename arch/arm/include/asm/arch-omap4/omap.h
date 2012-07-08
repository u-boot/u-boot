/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Authors:
 *	Aneesh V <aneesh@ti.com>
 *
 * Derived from OMAP3 work by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <x0khasim@ti.com>
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

#ifndef _OMAP4_H_
#define _OMAP4_H_

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP44XX_L4_CORE_BASE	0x4A000000
#define OMAP44XX_L4_WKUP_BASE	0x4A300000
#define OMAP44XX_L4_PER_BASE	0x48000000

#define OMAP44XX_DRAM_ADDR_SPACE_START	0x80000000
#define OMAP44XX_DRAM_ADDR_SPACE_END	0xD0000000
#define DRAM_ADDR_SPACE_START	OMAP44XX_DRAM_ADDR_SPACE_START
#define DRAM_ADDR_SPACE_END	OMAP44XX_DRAM_ADDR_SPACE_END

/* CONTROL */
#define CTRL_BASE		(OMAP44XX_L4_CORE_BASE + 0x2000)
#define CONTROL_PADCONF_CORE	(OMAP44XX_L4_CORE_BASE + 0x100000)
#define CONTROL_PADCONF_WKUP	(OMAP44XX_L4_CORE_BASE + 0x31E000)

/* LPDDR2 IO regs */
#define LPDDR2_IO_REGS_BASE	0x4A100638

/* CONTROL_ID_CODE */
#define CONTROL_ID_CODE		0x4A002204

#define OMAP4_CONTROL_ID_CODE_ES1_0	0x0B85202F
#define OMAP4_CONTROL_ID_CODE_ES2_0	0x1B85202F
#define OMAP4_CONTROL_ID_CODE_ES2_1	0x3B95C02F
#define OMAP4_CONTROL_ID_CODE_ES2_2	0x4B95C02F
#define OMAP4_CONTROL_ID_CODE_ES2_3	0x6B95C02F
#define OMAP4460_CONTROL_ID_CODE_ES1_0	0x0B94E02F
#define OMAP4460_CONTROL_ID_CODE_ES1_1	0x2B94E02F

/* UART */
#define UART1_BASE		(OMAP44XX_L4_PER_BASE + 0x6a000)
#define UART2_BASE		(OMAP44XX_L4_PER_BASE + 0x6c000)
#define UART3_BASE		(OMAP44XX_L4_PER_BASE + 0x20000)

/* General Purpose Timers */
#define GPT1_BASE		(OMAP44XX_L4_WKUP_BASE + 0x18000)
#define GPT2_BASE		(OMAP44XX_L4_PER_BASE  + 0x32000)
#define GPT3_BASE		(OMAP44XX_L4_PER_BASE  + 0x34000)

/* Watchdog Timer2 - MPU watchdog */
#define WDT2_BASE		(OMAP44XX_L4_WKUP_BASE + 0x14000)

/* 32KTIMER */
#define SYNC_32KTIMER_BASE	(OMAP44XX_L4_WKUP_BASE + 0x4000)

/* GPMC */
#define OMAP44XX_GPMC_BASE	0x50000000

/* SYSTEM CONTROL MODULE */
#define SYSCTRL_GENERAL_CORE_BASE	0x4A002000

/*
 * Hardware Register Details
 */

/* Watchdog Timer */
#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

/* GP Timer */
#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

/* Control Module */
#define LDOSRAM_ACTMODE_VSET_IN_MASK	(0x1F << 5)
#define LDOSRAM_VOLT_CTRL_OVERRIDE	0x0401040f
#define CONTROL_EFUSE_1_OVERRIDE	0x1C4D0110
#define CONTROL_EFUSE_2_OVERRIDE	0x99084000

/* LPDDR2 IO regs */
#define CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN	0x1C1C1C1C
#define CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER	0x9E9E9E9E
#define CONTROL_LPDDR2IO_SLEW_315PS_DRV12_PULL_DOWN	0x7C7C7C7C
#define LPDDR2IO_GR10_WD_MASK				(3 << 17)
#define CONTROL_LPDDR2IO_3_VAL		0xA0888C0F

/* CONTROL_EFUSE_2 */
#define CONTROL_EFUSE_2_NMOS_PMOS_PTV_CODE_1		0x00ffc000

#define MMC1_PWRDNZ					(1 << 26)
#define MMC1_PBIASLITE_PWRDNZ				(1 << 22)
#define MMC1_PBIASLITE_VMODE				(1 << 21)

#ifndef __ASSEMBLY__

struct s32ktimer {
	unsigned char res[0x10];
	unsigned int s32k_cr;	/* 0x10 */
};

#define DEVICE_TYPE_SHIFT (0x8)
#define DEVICE_TYPE_MASK (0x7 << DEVICE_TYPE_SHIFT)
#define DEVICE_GP 0x3

struct omap_sys_ctrl_regs {
	unsigned int pad1[129];
	unsigned int control_id_code;			/* 0x4A002204 */
	unsigned int pad11[22];
	unsigned int control_std_fuse_opp_bgap;		/* 0x4a002260 */
	unsigned int pad2[24];				/* 0x4a002264 */
	unsigned int control_status;			/* 0x4a0022c4 */
	unsigned int pad3[22];				/* 0x4a0022c8 */
	unsigned int control_ldosram_iva_voltage_ctrl;	/* 0x4A002320 */
	unsigned int control_ldosram_mpu_voltage_ctrl;	/* 0x4A002324 */
	unsigned int control_ldosram_core_voltage_ctrl;	/* 0x4A002328 */
	unsigned int pad4[260277];
	unsigned int control_pbiaslite;                 /* 0x4A100600 */
	unsigned int pad5[63];
	unsigned int control_efuse_1;			/* 0x4A100700 */
	unsigned int control_efuse_2;			/* 0x4A100704 */
};

struct control_lpddr2io_regs {
	unsigned int control_lpddr2io1_0;
	unsigned int control_lpddr2io1_1;
	unsigned int control_lpddr2io1_2;
	unsigned int control_lpddr2io1_3;
	unsigned int control_lpddr2io2_0;
	unsigned int control_lpddr2io2_1;
	unsigned int control_lpddr2io2_2;
	unsigned int control_lpddr2io2_3;
};
#endif /* __ASSEMBLY__ */

/*
 * Non-secure SRAM Addresses
 * Non-secure RAM starts at 0x40300000 for GP devices. But we keep SRAM_BASE
 * at 0x40304000(EMU base) so that our code works for both EMU and GP
 */
#define NON_SECURE_SRAM_START	0x40304000
#define NON_SECURE_SRAM_END	0x4030E000	/* Not inclusive */
/* base address for indirect vectors (internal boot mode) */
#define SRAM_ROM_VECT_BASE	0x4030D000
/* Temporary SRAM stack used while low level init is done */
#define LOW_LEVEL_SRAM_STACK		NON_SECURE_SRAM_END
#define SRAM_SCRATCH_SPACE_ADDR		NON_SECURE_SRAM_START
/* SRAM scratch space entries */
#define OMAP4_SRAM_SCRATCH_OMAP4_REV	SRAM_SCRATCH_SPACE_ADDR
#define OMAP4_SRAM_SCRATCH_EMIF_SIZE	(SRAM_SCRATCH_SPACE_ADDR + 0x4)
#define OMAP4_SRAM_SCRATCH_EMIF_T_NUM	(SRAM_SCRATCH_SPACE_ADDR + 0xC)
#define OMAP4_SRAM_SCRATCH_EMIF_T_DEN	(SRAM_SCRATCH_SPACE_ADDR + 0x10)
#define OMAP4_SRAM_SCRATCH_SPACE_END	(SRAM_SCRATCH_SPACE_ADDR + 0x14)

/* ROM code defines */
/* Boot device */
#define BOOT_DEVICE_MASK	0xFF
#define BOOT_DEVICE_OFFSET	0x8
#define DEV_DESC_PTR_OFFSET	0x4
#define DEV_DATA_PTR_OFFSET	0x18
#define BOOT_MODE_OFFSET	0x8
#define RESET_REASON_OFFSET	0x9
#define CH_FLAGS_OFFSET		0xA

#define CH_FLAGS_CHSETTINGS	(0x1 << 0)
#define CH_FLAGS_CHRAM		(0x1 << 1)
#define CH_FLAGS_CHFLASH	(0x1 << 2)
#define CH_FLAGS_CHMMCSD	(0x1 << 3)

#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	char *boot_message;
	unsigned int mem_boot_descriptor;
	unsigned char omap_bootdevice;
	unsigned char reset_reason;
	unsigned char ch_flags;
};
#endif
#endif
