/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Authors:
 *	Aneesh V <aneesh@ti.com>
 *	Sricharan R <r.sricharan@ti.com>
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

#ifndef _OMAP5_H_
#define _OMAP5_H_

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP54XX_L4_CORE_BASE	0x4A000000
#define OMAP54XX_L4_WKUP_BASE	0x4Ae00000
#define OMAP54XX_L4_PER_BASE	0x48000000

#define OMAP54XX_DRAM_ADDR_SPACE_START	0x80000000
#define OMAP54XX_DRAM_ADDR_SPACE_END	0xFFFFFFFF
#define DRAM_ADDR_SPACE_START	OMAP54XX_DRAM_ADDR_SPACE_START
#define DRAM_ADDR_SPACE_END	OMAP54XX_DRAM_ADDR_SPACE_END

/* CONTROL */
#define CTRL_BASE		(OMAP54XX_L4_CORE_BASE + 0x2000)
#define CONTROL_PADCONF_CORE	(CTRL_BASE + 0x0800)
#define CONTROL_PADCONF_WKUP	(OMAP54XX_L4_WKUP_BASE + 0xc800)

/* LPDDR2 IO regs. To be verified */
#define LPDDR2_IO_REGS_BASE	0x4A100638

/* CONTROL_ID_CODE */
#define CONTROL_ID_CODE		(CTRL_BASE + 0x204)

/* To be verified */
#define OMAP5430_CONTROL_ID_CODE_ES1_0		0x0B94202F
#define OMAP5432_CONTROL_ID_CODE_ES1_0		0x0B99802F

/* STD_FUSE_PROD_ID_1 */
#define STD_FUSE_PROD_ID_1		(CTRL_BASE + 0x218)
#define PROD_ID_1_SILICON_TYPE_SHIFT	16
#define PROD_ID_1_SILICON_TYPE_MASK	(3 << 16)

/* UART */
#define UART1_BASE		(OMAP54XX_L4_PER_BASE + 0x6a000)
#define UART2_BASE		(OMAP54XX_L4_PER_BASE + 0x6c000)
#define UART3_BASE		(OMAP54XX_L4_PER_BASE + 0x20000)

/* General Purpose Timers */
#define GPT1_BASE		(OMAP54XX_L4_WKUP_BASE + 0x18000)
#define GPT2_BASE		(OMAP54XX_L4_PER_BASE  + 0x32000)
#define GPT3_BASE		(OMAP54XX_L4_PER_BASE  + 0x34000)

/* Watchdog Timer2 - MPU watchdog */
#define WDT2_BASE		(OMAP54XX_L4_WKUP_BASE + 0x14000)

/* 32KTIMER */
#define SYNC_32KTIMER_BASE	(OMAP54XX_L4_WKUP_BASE + 0x4000)

/* GPMC */
#define OMAP54XX_GPMC_BASE	0x50000000

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
#define CONTROL_EFUSE_2_OVERRIDE	0x00084000

/* LPDDR2 IO regs */
#define CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN	0x1C1C1C1C
#define CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER	0x9E9E9E9E
#define CONTROL_LPDDR2IO_SLEW_315PS_DRV12_PULL_DOWN	0x7C7C7C7C
#define LPDDR2IO_GR10_WD_MASK				(3 << 17)
#define CONTROL_LPDDR2IO_3_VAL		0xA0888C00

/* CONTROL_EFUSE_2 */
#define CONTROL_EFUSE_2_NMOS_PMOS_PTV_CODE_1		0x00ffc000

#define SDCARD_PWRDNZ					(1 << 26)
#define SDCARD_BIAS_HIZ_MODE				(1 << 25)
#define SDCARD_BIAS_PWRDNZ				(1 << 22)
#define SDCARD_PBIASLITE_VMODE				(1 << 21)

#ifndef __ASSEMBLY__

struct s32ktimer {
	unsigned char res[0x10];
	unsigned int s32k_cr;	/* 0x10 */
};

#define DEVICE_TYPE_SHIFT 0x6
#define DEVICE_TYPE_MASK (0x7 << DEVICE_TYPE_SHIFT)
#define DEVICE_GP 0x3

struct omap_sys_ctrl_regs {
	u32 pad0[77]; /* 0x4A002000 */
	u32 control_status; /* 0x4A002134 */
	u32 pad1[794]; /* 0x4A002138 */
	u32 control_paconf_global; /* 0x4A002DA0 */
	u32 control_paconf_mode;  /* 0x4A002DA4 */
	u32 control_smart1io_padconf_0; /* 0x4A002DA8 */
	u32 control_smart1io_padconf_1; /* 0x4A002DAC */
	u32 control_smart1io_padconf_2; /* 0x4A002DB0 */
	u32 control_smart2io_padconf_0; /* 0x4A002DB4 */
	u32 control_smart2io_padconf_1; /* 0x4A002DB8 */
	u32 control_smart2io_padconf_2; /* 0x4A002DBC */
	u32 control_smart3io_padconf_0; /* 0x4A002DC0 */
	u32 control_smart3io_padconf_1; /* 0x4A002DC4 */
	u32 pad2[14];
	u32 control_pbias; /* 0x4A002E00 */
	u32 control_i2c_0; /* 0x4A002E04 */
	u32 control_camera_rx; /* 0x4A002E08 */
	u32 control_hdmi_tx_phy; /* 0x4A002E0C */
	u32 control_uniportm; /* 0x4A002E10 */
	u32 control_dsiphy; /* 0x4A002E14 */
	u32 control_mcbsplp; /* 0x4A002E18 */
	u32 control_usb2phycore; /* 0x4A002E1C */
	u32 control_hdmi_1; /*0x4A002E20*/
	u32 control_hsi; /*0x4A002E24*/
	u32 pad3[2];
	u32 control_ddr3ch1_0; /*0x4A002E30*/
	u32 control_ddr3ch2_0; /*0x4A002E34*/
	u32 control_ddrch1_0;	/*0x4A002E38*/
	u32 control_ddrch1_1;	/*0x4A002E3C*/
	u32 control_ddrch2_0;	/*0x4A002E40*/
	u32 control_ddrch2_1;	/*0x4A002E44*/
	u32 control_lpddr2ch1_0; /*0x4A002E48*/
	u32 control_lpddr2ch1_1; /*0x4A002E4C*/
	u32 control_ddrio_0;  /*0x4A002E50*/
	u32 control_ddrio_1;  /*0x4A002E54*/
	u32 control_ddrio_2;  /*0x4A002E58*/
	u32 control_hyst_1; /*0x4A002E5C*/
	u32 control_usbb_hsic_control; /*0x4A002E60*/
	u32 control_c2c; /*0x4A002E64*/
	u32 control_core_control_spare_rw; /*0x4A002E68*/
	u32 control_core_control_spare_r; /*0x4A002E6C*/
	u32 control_core_control_spare_r_c0; /*0x4A002E70*/
	u32 control_srcomp_north_side; /*0x4A002E74*/
	u32 control_srcomp_south_side; /*0x4A002E78*/
	u32 control_srcomp_east_side; /*0x4A002E7C*/
	u32 control_srcomp_west_side; /*0x4A002E80*/
	u32 control_srcomp_code_latch; /*0x4A002E84*/
	u32 pad4[3679394];
	u32 control_port_emif1_sdram_config;		/*0x4AE0C110*/
	u32 control_port_emif1_lpddr2_nvm_config;	/*0x4AE0C114*/
	u32 control_port_emif2_sdram_config;		/*0x4AE0C118*/
	u32 pad5[10];
	u32 control_emif1_sdram_config_ext;		/* 0x4AE0C144 */
	u32 control_emif2_sdram_config_ext;		/* 0x4AE0C148 */
	u32 pad6[789];
	u32 control_smart1nopmio_padconf_0; /* 0x4AE0CDA0 */
	u32 control_smart1nopmio_padconf_1; /* 0x4AE0CDA4 */
	u32 control_padconf_mode; /* 0x4AE0CDA8 */
	u32 control_xtal_oscillator; /* 0x4AE0CDAC */
	u32 control_i2c_2; /* 0x4AE0CDB0 */
	u32 control_ckobuffer; /* 0x4AE0CDB4 */
	u32 control_wkup_control_spare_rw; /* 0x4AE0CDB8 */
	u32 control_wkup_control_spare_r; /* 0x4AE0CDBC */
	u32 control_wkup_control_spare_r_c0; /* 0x4AE0CDC0 */
	u32 control_srcomp_east_side_wkup; /* 0x4AE0CDC4 */
	u32 control_efuse_1; /* 0x4AE0CDC8 */
	u32 control_efuse_2; /* 0x4AE0CDCC */
	u32 control_efuse_3; /* 0x4AE0CDD0 */
	u32 control_efuse_4; /* 0x4AE0CDD4 */
	u32 control_efuse_5; /* 0x4AE0CDD8 */
	u32 control_efuse_6; /* 0x4AE0CDDC */
	u32 control_efuse_7; /* 0x4AE0CDE0 */
	u32 control_efuse_8; /* 0x4AE0CDE4 */
	u32 control_efuse_9; /* 0x4AE0CDE8 */
	u32 control_efuse_10; /* 0x4AE0CDEC */
	u32 control_efuse_11; /* 0x4AE0CDF0 */
	u32 control_efuse_12; /* 0x4AE0CDF4 */
	u32 control_efuse_13; /* 0x4AE0CDF8 */
};

/* Output impedance control */
#define ds_120_ohm	0x0
#define ds_60_ohm	0x1
#define ds_45_ohm	0x2
#define ds_30_ohm	0x3
#define ds_mask		0x3

/* Slew rate control */
#define sc_slow		0x0
#define sc_medium	0x1
#define sc_fast		0x2
#define sc_na		0x3
#define sc_mask		0x3

/* Target capacitance control */
#define lb_5_12_pf	0x0
#define lb_12_25_pf	0x1
#define lb_25_50_pf	0x2
#define lb_50_80_pf	0x3
#define lb_mask		0x3

#define usb_i_mask	0x7

#define DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN   0x80828082
#define DDR_IO_I_34OHM_SR_FASTEST_WD_CK_CKE_NCS_CA_PULL_DOWN 0x82828200
#define DDR_IO_0_DDR2_DQ_INT_EN_ALL_DDR3_CA_DIS_ALL 0x8421
#define DDR_IO_1_DQ_OUT_EN_ALL_DQ_INT_EN_ALL 0x8421084
#define DDR_IO_2_CA_OUT_EN_ALL_CA_INT_EN_ALL 0x8421000

#define DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL	0x7C7C7C6C
#define DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL	0x64646464
#define DDR_IO_0_VREF_CELLS_DDR3_VALUE				0xBAE8C631
#define DDR_IO_1_VREF_CELLS_DDR3_VALUE				0xBC6318DC
#define DDR_IO_2_VREF_CELLS_DDR3_VALUE				0x0

#define EFUSE_1 0x45145100
#define EFUSE_2 0x45145100
#define EFUSE_3 0x45145100
#define EFUSE_4 0x45145100
#endif /* __ASSEMBLY__ */

/*
 * Non-secure SRAM Addresses
 * Non-secure RAM starts at 0x40300000 for GP devices. But we keep SRAM_BASE
 * at 0x40304000(EMU base) so that our code works for both EMU and GP
 */
#define NON_SECURE_SRAM_START	0x40300000
#define NON_SECURE_SRAM_END	0x40320000	/* Not inclusive */
/* base address for indirect vectors (internal boot mode) */
#define SRAM_ROM_VECT_BASE	0x4031F000
/* Temporary SRAM stack used while low level init is done */
#define LOW_LEVEL_SRAM_STACK	NON_SECURE_SRAM_END

#define SRAM_SCRATCH_SPACE_ADDR		NON_SECURE_SRAM_START
/*
 * SRAM scratch space entries
 */
#define OMAP5_SRAM_SCRATCH_OMAP5_REV	SRAM_SCRATCH_SPACE_ADDR
#define OMAP5_SRAM_SCRATCH_EMIF_SIZE	(SRAM_SCRATCH_SPACE_ADDR + 0x4)
#define OMAP5_SRAM_SCRATCH_EMIF_T_NUM	(SRAM_SCRATCH_SPACE_ADDR + 0xC)
#define OMAP5_SRAM_SCRATCH_EMIF_T_DEN	(SRAM_SCRATCH_SPACE_ADDR + 0x10)
#define OMAP5_SRAM_SCRATCH_SPACE_END	(SRAM_SCRATCH_SPACE_ADDR + 0x14)

/* Silicon revisions */
#define OMAP4430_SILICON_ID_INVALID	0xFFFFFFFF
#define OMAP4430_ES1_0	0x44300100
#define OMAP4430_ES2_0	0x44300200
#define OMAP4430_ES2_1	0x44300210
#define OMAP4430_ES2_2	0x44300220
#define OMAP4430_ES2_3	0x44300230
#define OMAP4460_ES1_0	0x44600100
#define OMAP4460_ES1_1	0x44600110

/* ROM code defines */
/* Boot device */
#define BOOT_DEVICE_MASK	0xFF
#define BOOT_DEVICE_OFFSET	0x8
#define DEV_DESC_PTR_OFFSET	0x4
#define DEV_DATA_PTR_OFFSET	0x18
#define BOOT_MODE_OFFSET	0x8
#define RESET_REASON_OFFSET     0x9
#define CH_FLAGS_OFFSET         0xA

#define CH_FLAGS_CHSETTINGS	(0x1 << 0)
#define	CH_FLAGS_CHRAM		(0x1 << 1)
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
#endif /* __ASSEMBLY__ */
#endif
