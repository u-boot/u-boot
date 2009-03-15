/*
 * (C) Copyright 2006-2008
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef _CPU_H
#define _CPU_H

/* Register offsets of common modules */
/* Control */
#ifndef __ASSEMBLY__
typedef struct ctrl {
	unsigned char res1[0xC0];
	unsigned short gpmc_nadv_ale;	/* 0xC0 */
	unsigned short gpmc_noe;	/* 0xC2 */
	unsigned short gpmc_nwe;	/* 0xC4 */
	unsigned char res2[0x22A];
	unsigned int status;		/* 0x2F0 */
	unsigned int gpstatus;		/* 0x2F4 */
	unsigned char res3[0x08];
	unsigned int rpubkey_0;		/* 0x300 */
	unsigned int rpubkey_1;		/* 0x304 */
	unsigned int rpubkey_2;		/* 0x308 */
	unsigned int rpubkey_3;		/* 0x30C */
	unsigned int rpubkey_4;		/* 0x310 */
	unsigned char res4[0x04];
	unsigned int randkey_0;		/* 0x318 */
	unsigned int randkey_1;		/* 0x31C */
	unsigned int randkey_2;		/* 0x320 */
	unsigned int randkey_3;		/* 0x324 */
	unsigned char res5[0x124];
	unsigned int ctrl_omap_stat;	/* 0x44C */
} ctrl_t;
#else /* __ASSEMBLY__ */
#define CONTROL_STATUS		0x2F0
#endif /* __ASSEMBLY__ */

/* cpu type */
#define OMAP3503		0x5c00
#define OMAP3515		0x1c00
#define OMAP3525		0x4c00
#define OMAP3530		0x0c00

#ifndef __ASSEMBLY__
typedef struct ctrl_id {
	unsigned char res1[0x4];
	unsigned int idcode;		/* 0x04 */
	unsigned int prod_id;		/* 0x08 */
	unsigned char res2[0x0C];
	unsigned int die_id_0;		/* 0x18 */
	unsigned int die_id_1;		/* 0x1C */
	unsigned int die_id_2;		/* 0x20 */
	unsigned int die_id_3;		/* 0x24 */
} ctrl_id_t;
#endif /* __ASSEMBLY__ */

/* device type */
#define DEVICE_MASK		(0x7 << 8)
#define SYSBOOT_MASK		0x1F
#define TST_DEVICE		0x0
#define EMU_DEVICE		0x1
#define HS_DEVICE		0x2
#define GP_DEVICE		0x3

/* GPMC CS3/cs4/cs6 not avaliable */
#define GPMC_BASE		(OMAP34XX_GPMC_BASE)
#define GPMC_CONFIG_CS0		0x60
#define GPMC_CONFIG_CS6		0x150
#define GPMC_CONFIG_CS0_BASE	(GPMC_BASE + GPMC_CONFIG_CS0)
#define GPMC_CONFIG_CS6_BASE	(GPMC_BASE + GPMC_CONFIG_CS6)
#define GPMC_CONFIG_WP		0x10

#define GPMC_CONFIG_WIDTH	0x30

#ifndef __ASSEMBLY__
typedef struct gpmc {
	unsigned char res1[0x10];
	unsigned int sysconfig;		/* 0x10 */
	unsigned char res2[0x4];
	unsigned int irqstatus;		/* 0x18 */
	unsigned int irqenable; 	/* 0x1C */
	unsigned char res3[0x20];
	unsigned int timeout_control; 	/* 0x40 */
	unsigned char res4[0xC];
	unsigned int config;		/* 0x50 */
	unsigned int status;		/* 0x54 */
	unsigned char res5[0x19C];
	unsigned int ecc_config;	/* 0x1F4 */
	unsigned int ecc_control;	/* 0x1F8 */
	unsigned int ecc_size_config;	/* 0x1FC */
	unsigned int ecc1_result;	/* 0x200 */
	unsigned int ecc2_result;	/* 0x204 */
	unsigned int ecc3_result;	/* 0x208 */
	unsigned int ecc4_result;	/* 0x20C */
	unsigned int ecc5_result;	/* 0x210 */
	unsigned int ecc6_result;	/* 0x214 */
	unsigned int ecc7_result;	/* 0x218 */
	unsigned int ecc8_result;	/* 0x21C */
	unsigned int ecc9_result;	/* 0x220 */
} gpmc_t;

typedef struct gpmc_csx {
	unsigned int config1;		/* 0x00 */
	unsigned int config2;		/* 0x04 */
	unsigned int config3;		/* 0x08 */
	unsigned int config4;		/* 0x0C */
	unsigned int config5;		/* 0x10 */
	unsigned int config6;		/* 0x14 */
	unsigned int config7;		/* 0x18 */
	unsigned int nand_cmd;		/* 0x1C */
	unsigned int nand_adr;		/* 0x20 */
	unsigned int nand_dat;		/* 0x24 */
} gpmc_csx_t;
#else /* __ASSEMBLY__ */
#define GPMC_CONFIG1		0x00
#define GPMC_CONFIG2		0x04
#define GPMC_CONFIG3		0x08
#define GPMC_CONFIG4		0x0C
#define GPMC_CONFIG5		0x10
#define GPMC_CONFIG6		0x14
#define GPMC_CONFIG7		0x18
#endif /* __ASSEMBLY__ */

/* GPMC Mapping */
#define FLASH_BASE		0x10000000	/* NOR flash, */
						/* aligned to 256 Meg */
#define FLASH_BASE_SDPV1	0x04000000	/* NOR flash, */
						/* aligned to 64 Meg */
#define FLASH_BASE_SDPV2	0x10000000	/* NOR flash, */
						/* aligned to 256 Meg */
#define DEBUG_BASE		0x08000000	/* debug board */
#define NAND_BASE		0x30000000	/* NAND addr */
						/* (actual size small port) */
#define PISMO2_BASE		0x18000000	/* PISMO2 CS1/2 */
#define ONENAND_MAP		0x20000000	/* OneNand addr */
						/* (actual size small port) */
/* SMS */
#ifndef __ASSEMBLY__
typedef struct sms {
	unsigned char res1[0x10];
	unsigned int sysconfig;		/* 0x10 */
	unsigned char res2[0x34];
	unsigned int rg_att0;		/* 0x48 */
	unsigned char res3[0x84];
	unsigned int class_arb0;	/* 0xD0 */
} sms_t;
#endif /* __ASSEMBLY__ */

#define BURSTCOMPLETE_GROUP7	(0x1 << 31)

/* SDRC */
#ifndef __ASSEMBLY__
typedef struct sdrc_cs {
	unsigned int mcfg;		/* 0x80 || 0xB0 */
	unsigned int mr;		/* 0x84 || 0xB4 */
	unsigned char res1[0x4];
	unsigned int emr2;		/* 0x8C || 0xBC */
	unsigned char res2[0x14];
	unsigned int rfr_ctrl;		/* 0x84 || 0xD4 */
	unsigned int manual;		/* 0xA8 || 0xD8 */
	unsigned char res3[0x4];
} sdrc_cs_t;

typedef struct sdrc_actim {
	unsigned int ctrla;		/* 0x9C || 0xC4 */
	unsigned int ctrlb;		/* 0xA0 || 0xC8 */
} sdrc_actim_t;

typedef struct sdrc {
	unsigned char res1[0x10];
	unsigned int sysconfig;		/* 0x10 */
	unsigned int status;		/* 0x14 */
	unsigned char res2[0x28];
	unsigned int cs_cfg;		/* 0x40 */
	unsigned int sharing;		/* 0x44 */
	unsigned char res3[0x18];
	unsigned int dlla_ctrl;		/* 0x60 */
	unsigned int dlla_status;	/* 0x64 */
	unsigned int dllb_ctrl;		/* 0x68 */
	unsigned int dllb_status;	/* 0x6C */
	unsigned int power;		/* 0x70 */
	unsigned char res4[0xC];
	sdrc_cs_t cs[2];		/* 0x80 || 0xB0 */
} sdrc_t;
#endif /* __ASSEMBLY__ */

#define DLLPHASE_90		(0x1 << 1)
#define LOADDLL			(0x1 << 2)
#define ENADLL			(0x1 << 3)
#define DLL_DELAY_MASK		0xFF00
#define DLL_NO_FILTER_MASK	((0x1 << 9) | (0x1 << 8))

#define PAGEPOLICY_HIGH		(0x1 << 0)
#define SRFRONRESET		(0x1 << 7)
#define WAKEUPPROC		(0x1 << 26)

#define DDR_SDRAM		(0x1 << 0)
#define DEEPPD			(0x1 << 3)
#define B32NOT16		(0x1 << 4)
#define BANKALLOCATION		(0x2 << 6)
#define RAMSIZE_128		(0x40 << 8) /* RAM size in 2MB chunks */
#define ADDRMUXLEGACY		(0x1 << 19)
#define CASWIDTH_10BITS		(0x5 << 20)
#define RASWIDTH_13BITS		(0x2 << 24)
#define BURSTLENGTH4		(0x2 << 0)
#define CASL3			(0x3 << 4)
#define SDRC_ACTIM_CTRL0_BASE	(OMAP34XX_SDRC_BASE + 0x9C)
#define SDRC_ACTIM_CTRL1_BASE	(OMAP34XX_SDRC_BASE + 0xC4)
#define ARE_ARCV_1		(0x1 << 0)
#define ARCV			(0x4e2 << 8) /* Autorefresh count */
#define OMAP34XX_SDRC_CS0	0x80000000
#define OMAP34XX_SDRC_CS1	0xA0000000
#define CMD_NOP			0x0
#define CMD_PRECHARGE		0x1
#define CMD_AUTOREFRESH		0x2
#define CMD_ENTR_PWRDOWN	0x3
#define CMD_EXIT_PWRDOWN	0x4
#define CMD_ENTR_SRFRSH		0x5
#define CMD_CKE_HIGH		0x6
#define CMD_CKE_LOW		0x7
#define SOFTRESET		(0x1 << 1)
#define SMART_IDLE		(0x2 << 3)
#define REF_ON_IDLE		(0x1 << 6)

/* timer regs offsets (32 bit regs) */

#ifndef __ASSEMBLY__
typedef struct gptimer {
	unsigned int tidr;	/* 0x00 r */
	unsigned char res[0xc];
	unsigned int tiocp_cfg;	/* 0x10 rw */
	unsigned int tistat;	/* 0x14 r */
	unsigned int tisr;	/* 0x18 rw */
	unsigned int tier;	/* 0x1c rw */
	unsigned int twer;	/* 0x20 rw */
	unsigned int tclr;	/* 0x24 rw */
	unsigned int tcrr;	/* 0x28 rw */
	unsigned int tldr;	/* 0x2c rw */
	unsigned int ttgr;	/* 0x30 rw */
	unsigned int twpc;	/* 0x34 r*/
	unsigned int tmar;	/* 0x38 rw*/
	unsigned int tcar1;	/* 0x3c r */
	unsigned int tcicr;	/* 0x40 rw */
	unsigned int tcar2;	/* 0x44 r */
} gptimer_t;
#endif /* __ASSEMBLY__ */

/* enable sys_clk NO-prescale /1 */
#define GPT_EN			((0x0 << 2) | (0x1 << 1) | (0x1 << 0))

/* Watchdog */
#ifndef __ASSEMBLY__
typedef struct watchdog {
	unsigned char res1[0x34];
	unsigned int wwps;	/* 0x34 r */
	unsigned char res2[0x10];
	unsigned int wspr;	/* 0x48 rw */
} watchdog_t;
#endif /* __ASSEMBLY__ */

#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

/* PRCM */
#define PRCM_BASE		0x48004000

#ifndef __ASSEMBLY__
typedef struct prcm {
	unsigned int fclken_iva2;	/* 0x00 */
	unsigned int clken_pll_iva2;	/* 0x04 */
	unsigned char res1[0x1c];
	unsigned int idlest_pll_iva2;	/* 0x24 */
	unsigned char res2[0x18];
	unsigned int clksel1_pll_iva2 ;	/* 0x40 */
	unsigned int clksel2_pll_iva2;	/* 0x44 */
	unsigned char res3[0x8bc];
	unsigned int clken_pll_mpu;	/* 0x904 */
	unsigned char res4[0x1c];
	unsigned int idlest_pll_mpu;	/* 0x924 */
	unsigned char res5[0x18];
	unsigned int clksel1_pll_mpu;	/* 0x940 */
	unsigned int clksel2_pll_mpu;	/* 0x944 */
	unsigned char res6[0xb8];
	unsigned int fclken1_core;	/* 0xa00 */
	unsigned char res7[0xc];
	unsigned int iclken1_core;	/* 0xa10 */
	unsigned int iclken2_core;	/* 0xa14 */
	unsigned char res8[0x28];
	unsigned int clksel_core;	/* 0xa40 */
	unsigned char res9[0xbc];
	unsigned int fclken_gfx;	/* 0xb00 */
	unsigned char res10[0xc];
	unsigned int iclken_gfx;	/* 0xb10 */
	unsigned char res11[0x2c];
	unsigned int clksel_gfx;	/* 0xb40 */
	unsigned char res12[0xbc];
	unsigned int fclken_wkup;	/* 0xc00 */
	unsigned char res13[0xc];
	unsigned int iclken_wkup;	/* 0xc10 */
	unsigned char res14[0xc];
	unsigned int idlest_wkup;	/* 0xc20 */
	unsigned char res15[0x1c];
	unsigned int clksel_wkup;	/* 0xc40 */
	unsigned char res16[0xbc];
	unsigned int clken_pll;		/* 0xd00 */
	unsigned char res17[0x1c];
	unsigned int idlest_ckgen;	/* 0xd20 */
	unsigned char res18[0x1c];
	unsigned int clksel1_pll;	/* 0xd40 */
	unsigned int clksel2_pll;	/* 0xd44 */
	unsigned int clksel3_pll;	/* 0xd48 */
	unsigned char res19[0xb4];
	unsigned int fclken_dss;	/* 0xe00 */
	unsigned char res20[0xc];
	unsigned int iclken_dss;	/* 0xe10 */
	unsigned char res21[0x2c];
	unsigned int clksel_dss;	/* 0xe40 */
	unsigned char res22[0xbc];
	unsigned int fclken_cam;	/* 0xf00 */
	unsigned char res23[0xc];
	unsigned int iclken_cam;	/* 0xf10 */
	unsigned char res24[0x2c];
	unsigned int clksel_cam;	/* 0xf40 */
	unsigned char res25[0xbc];
	unsigned int fclken_per;	/* 0x1000 */
	unsigned char res26[0xc];
	unsigned int iclken_per;	/* 0x1010 */
	unsigned char res27[0x2c];
	unsigned int clksel_per;	/* 0x1040 */
	unsigned char res28[0xfc];
	unsigned int clksel1_emu;	/* 0x1140 */
} prcm_t;
#else /* __ASSEMBLY__ */
#define CM_CLKSEL_CORE		0x48004a40
#define CM_CLKSEL_GFX		0x48004b40
#define CM_CLKSEL_WKUP		0x48004c40
#define CM_CLKEN_PLL		0x48004d00
#define CM_CLKSEL1_PLL		0x48004d40
#define CM_CLKSEL1_EMU		0x48005140
#endif /* __ASSEMBLY__ */

#define PRM_BASE		0x48306000

#ifndef __ASSEMBLY__
typedef struct prm {
	unsigned char res1[0xd40];
	unsigned int clksel;		/* 0xd40 */
	unsigned char res2[0x50c];
	unsigned int rstctrl;		/* 0x1250 */
	unsigned char res3[0x1c];
	unsigned int clksrc_ctrl;	/* 0x1270 */
} prm_t;
#else /* __ASSEMBLY__ */
#define PRM_RSTCTRL		0x48307250
#endif /* __ASSEMBLY__ */

#define SYSCLKDIV_1		(0x1 << 6)
#define SYSCLKDIV_2		(0x1 << 7)

#define CLKSEL_GPT1		(0x1 << 0)

#define EN_GPT1			(0x1 << 0)
#define EN_32KSYNC		(0x1 << 2)

#define ST_WDT2			(0x1 << 5)

#define ST_MPU_CLK		(0x1 << 0)

#define ST_CORE_CLK		(0x1 << 0)

#define ST_PERIPH_CLK		(0x1 << 1)

#define ST_IVA2_CLK		(0x1 << 0)

#define RESETDONE		(0x1 << 0)

#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

/* SMX-APE */
#define PM_RT_APE_BASE_ADDR_ARM		(SMX_APE_BASE + 0x10000)
#define PM_GPMC_BASE_ADDR_ARM		(SMX_APE_BASE + 0x12400)
#define PM_OCM_RAM_BASE_ADDR_ARM	(SMX_APE_BASE + 0x12800)
#define PM_IVA2_BASE_ADDR_ARM		(SMX_APE_BASE + 0x14000)

#ifndef __ASSEMBLY__
typedef struct pm {
	unsigned char res1[0x48];
	unsigned int req_info_permission_0;	/* 0x48 */
	unsigned char res2[0x4];
	unsigned int read_permission_0;		/* 0x50 */
	unsigned char res3[0x4];
	unsigned int wirte_permission_0;	/* 0x58 */
	unsigned char res4[0x4];
	unsigned int addr_match_1;		/* 0x58 */
	unsigned char res5[0x4];
	unsigned int req_info_permission_1;	/* 0x68 */
	unsigned char res6[0x14];
	unsigned int addr_match_2;		/* 0x80 */
} pm_t;
#endif /*__ASSEMBLY__ */

/* Permission values for registers -Full fledged permissions to all */
#define UNLOCK_1			0xFFFFFFFF
#define UNLOCK_2			0x00000000
#define UNLOCK_3			0x0000FFFF

#define NOT_EARLY			0

/* I2C base */
#define I2C_BASE1		(OMAP34XX_CORE_L4_IO_BASE + 0x70000)
#define I2C_BASE2		(OMAP34XX_CORE_L4_IO_BASE + 0x72000)
#define I2C_BASE3		(OMAP34XX_CORE_L4_IO_BASE + 0x60000)

#endif /* _CPU_H */
