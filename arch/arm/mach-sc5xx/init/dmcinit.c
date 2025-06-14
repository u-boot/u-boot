// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#include <asm/io.h>
#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <linux/types.h>
#include "clkinit.h"
#include "dmcinit.h"

#define REG_DMC0_BASE		0x31070000
#define REG_DMC1_BASE		0x31073000

#define REG_DMC_CTL		0x0004 // Control Register
#define REG_DMC_STAT		0x0008 // Status Register
#define REG_DMC_CFG		0x0040 // Configuration Register
#define REG_DMC_TR0		0x0044 // Timing 0 Register
#define REG_DMC_TR1		0x0048 // Timing 1 Register
#define REG_DMC_TR2		0x004C // Timing 2 Register
#define REG_DMC_MR		0x0060 // Shadow MR Register (DDR3)
#define REG_DMC_EMR1		0x0064 // Shadow EMR1 Register
#define REG_DMC_EMR2		0x0068 // Shadow EMR2 Register
#define REG_DMC_EMR3		0x006C
#define REG_DMC_DLLCTL		0x0080 // DLL Control Register
#define REG_DMC_DT_CALIB_ADDR	0x0090 // Data Calibration Address Register
#define REG_DMC_CPHY_CTL	0x01C0 // Controller to PHY Interface Register

/* SC57x && SC58x DMC REGs */
#define REG_DMC_PHY_CTL0	0x1000 // PHY Control 0 Register
#define REG_DMC_PHY_CTL1	0x1004 // PHY Control 1 Register
#define REG_DMC_PHY_CTL2	0x1008 // PHY Control 2 Register
#define REG_DMC_PHY_CTL3	0x100c // PHY Control 3 Register
#define REG_DMC_PHY_CTL4	0x1010 // PHY Control 4 Register
#define REG_DMC_CAL_PADCTL0	0x1034 // CALIBRATION PAD CTL 0 Register
#define REG_DMC_CAL_PADCTL2	0x103C // CALIBRATION PAD CTL2 Register
/* END */

/* SC59x DMC REGs */
#define REG_DMC_DDR_LANE0_CTL0	0x1000 // Data Lane 0 Control Register 0
#define REG_DMC_DDR_LANE0_CTL1	0x1004 // Data Lane 0 Control Register 1
#define REG_DMC_DDR_LANE1_CTL0	0x100C // Data Lane 1 Control Register 0
#define REG_DMC_DDR_LANE1_CTL1	0x1010 // Data Lane 1 Control Register 1
#define REG_DMC_DDR_ROOT_CTL	0x1018 // DDR ROOT Module Control Register
#define REG_DMC_DDR_ZQ_CTL0	0x1034 // DDR Calibration Control Register 0
#define REG_DMC_DDR_ZQ_CTL1	0x1038 // DDR Calibration Control Register 1
#define REG_DMC_DDR_ZQ_CTL2	0x103C // DDR Calibration Control Register 2
#define REG_DMC_DDR_CA_CTL	0x1068 // DDR CA Lane Control Register
/* END */

#define REG_DMC_DDR_SCRATCH_2	0x1074
#define REG_DMC_DDR_SCRATCH_3	0x1078
#define REG_DMC_DDR_SCRATCH_6	0x1084
#define REG_DMC_DDR_SCRATCH_7	0x1088

#define REG_DMC_DDR_SCRATCH_STAT0	0x107C
#define REG_DMC_DDR_SCRATCH_STAT1	0x1080

#define DMC0_DATA_CALIB_ADD	0x80000000
#define DMC1_DATA_CALIB_ADD	0xC0000000

#define BITM_DMC_CFG_EXTBANK	0x0000F000  /* External Banks */
#define ENUM_DMC_CFG_EXTBANK1	0x00000000  /* EXTBANK: 1 External Bank */
#define BITM_DMC_CFG_SDRSIZE	0x00000F00  /* SDRAM Size */
#define ENUM_DMC_CFG_SDRSIZE64	0x00000000  /* SDRSIZE: 64M Bit SDRAM (LPDDR Only) */
#define ENUM_DMC_CFG_SDRSIZE128	0x00000100  /* SDRSIZE: 128M Bit SDRAM (LPDDR Only) */
#define ENUM_DMC_CFG_SDRSIZE256	0x00000200  /* SDRSIZE: 256M Bit SDRAM */
#define ENUM_DMC_CFG_SDRSIZE512	0x00000300  /* SDRSIZE: 512M Bit SDRAM */
#define ENUM_DMC_CFG_SDRSIZE1G	0x00000400  /* SDRSIZE: 1G Bit SDRAM */
#define ENUM_DMC_CFG_SDRSIZE2G	0x00000500  /* SDRSIZE: 2G Bit SDRAM */
#define ENUM_DMC_CFG_SDRSIZE4G	0x00000600  /* SDRSIZE: 4G Bit SDRAM */
#define ENUM_DMC_CFG_SDRSIZE8G	0x00000700  /* SDRSIZE: 8G Bit SDRAM */
#define BITM_DMC_CFG_SDRWID	0x000000F0  /* SDRAM Width */
#define ENUM_DMC_CFG_SDRWID16	0x00000020  /* SDRWID: 16-Bit Wide SDRAM */
#define BITM_DMC_CFG_IFWID	0x0000000F  /* Interface Width */
#define ENUM_DMC_CFG_IFWID16	0x00000002  /* IFWID: 16-Bit Wide Interface */

#define BITM_DMC_CTL_DDR3EN	0x00000001
#define BITM_DMC_CTL_INIT	0x00000004
#define BITP_DMC_STAT_INITDONE	2            /* Initialization Done */
#define BITM_DMC_STAT_INITDONE	0x00000004

#define BITP_DMC_CTL_AL_EN	27
#define BITP_DMC_CTL_ZQCL	25           /* ZQ Calibration Long */
#define BITP_DMC_CTL_ZQCS	24           /* ZQ Calibration Short */
#define BITP_DMC_CTL_DLLCAL	13           /* DLL Calibration Start */
#define BITP_DMC_CTL_PPREF	12           /* Postpone Refresh */
#define BITP_DMC_CTL_RDTOWR	 9           /* Read-to-Write Cycle */
#define BITP_DMC_CTL_ADDRMODE	 8           /* Addressing (Page/Bank) Mode */
#define BITP_DMC_CTL_RESET	 7           /* Reset SDRAM */
#define BITP_DMC_CTL_PREC	 6           /* Precharge */
#define BITP_DMC_CTL_DPDREQ	 5           /* Deep Power Down Request */
#define BITP_DMC_CTL_PDREQ	 4           /* Power Down Request */
#define BITP_DMC_CTL_SRREQ	 3           /* Self Refresh Request */
#define BITP_DMC_CTL_INIT	 2           /* Initialize DRAM Start */
#define BITP_DMC_CTL_LPDDR	 1           /* Low Power DDR Mode */
#define BITP_DMC_CTL_DDR3EN	 0           /* DDR3 Mode */

#ifdef CONFIG_TARGET_SC584_EZKIT
	#define DMC_PADCTL2_VALUE	0x0078283C
#elif CONFIG_TARGET_SC573_EZKIT
	#define DMC_PADCTL2_VALUE	0x00782828
#elif CONFIG_TARGET_SC589_MINI || CONFIG_TARGET_SC589_EZKIT
	#define DMC_PADCTL2_VALUE	0x00783C3C
#elif defined(CONFIG_SC57X) || defined(CONFIG_SC58X)
	#error "PADCTL2 not specified for custom board!"
#else
	//Newer DMC. Legacy calibration obsolete
	#define DMC_PADCTL2_VALUE	0x0
#endif

#define DMC_CPHYCTL_VALUE	0x0000001A

#define BITP_DMC_MR1_QOFF	12 /*  Output Buffer Enable */
#define BITP_DMC_MR1_TDQS	11 /*  Termination Data Strobe */
#define BITP_DMC_MR1_RTT2	 9 /*  Rtt_nom */
#define BITP_DMC_MR1_WL		 7 /*  Write Leveling Enable. */
#define BITP_DMC_MR1_RTT1	 6 /*  Rtt_nom */
#define BITP_DMC_MR1_DIC1	 5 /*  Output Driver Impedance Control */
#define BITP_DMC_MR1_AL		 3 /*  Additive Latency */
#define BITP_DMC_MR1_RTT0	 2 /*  Rtt_nom */
#define BITP_DMC_MR1_DIC0	 1 /*  Output Driver Impedance control */
#define BITP_DMC_MR1_DLLEN	 0 /*  DLL Enable */

#define BITP_DMC_MR2_CWL	 3 /* CAS write Latency */

#define BITP_DMC_TR0_TMRD	28 /* Timing Mode Register Delay */
#define BITP_DMC_TR0_TRC	20 /* Timing Row Cycle */
#define BITP_DMC_TR0_TRAS	12 /* Timing Row Active Time */
#define BITP_DMC_TR0_TRP	 8 /* Timing RAS Precharge. */
#define BITP_DMC_TR0_TWTR	 4 /* Timing Write to Read */
#define BITP_DMC_TR0_TRCD	 0 /* Timing RAS to CAS Delay */

#define BITP_DMC_TR1_TRRD	28 /* Timing Read-Read Delay */
#define BITP_DMC_TR1_TRFC	16 /* Timing Refresh-to-Command */
#define BITP_DMC_TR1_TREF	 0 /* Timing Refresh Interval */

#define BITP_DMC_TR2_TCKE	20 /* Timing Clock Enable */
#define BITP_DMC_TR2_TXP	16 /* Timing Exit Powerdown */
#define BITP_DMC_TR2_TWR	12 /* Timing Write Recovery */
#define BITP_DMC_TR2_TRTP	 8 /* Timing Read-to-Precharge */
#define BITP_DMC_TR2_TFAW	 0 /* Timing Four-Activated-Window */

#define BITP_DMC_MR_PD		12 /* Active Powerdown Mode */
#define BITP_DMC_MR_WRRECOV	 9 /* Write Recovery */
#define BITP_DMC_MR_DLLRST	 8 /* DLL Reset */
#define BITP_DMC_MR_CL		 4 /* CAS Latency */
#define BITP_DMC_MR_CL0		 2 /* CAS Latency */
#define BITP_DMC_MR_BLEN	 0 /* Burst Length */

#define BITP_DMC_DLLCTL_DATACYC		8 /* Data Cycles */
#define BITP_DMC_DLLCTL_DLLCALRDCNT	0 /* DLL Calibration RD Count */

#define BITM_DMC_DLLCTL_DATACYC		0x00000F00 /* Data Cycles */
#define BITM_DMC_DLLCTL_DLLCALRDCNT	0x000000FF /* DLL Calib RD Count */

#define BITP_DMC_STAT_PHYRDPHASE	20 /* PHY Read Phase */

#define BITM_DMC_DDR_LANE0_CTL0_CB_RSTDAT	0x08000000 /* Rst Data Pads */
#define BITM_DMC_DDR_LANE1_CTL0_CB_RSTDAT	0x08000000 /* Rst Data Pads */
#define BITM_DMC_DDR_LANE0_CTL1_COMP_DCYCLE	0x00000002 /* Compute Dcycle */
#define BITM_DMC_DDR_LANE1_CTL1_COMP_DCYCLE	0x00000002 /* Compute Dcycle */
#define BITM_DMC_DDR_LANE1_CTL0_CB_RSTDLL	0x00000100 /* Rst Lane DLL */
#define BITM_DMC_DDR_LANE0_CTL0_CB_RSTDLL	0x00000100 /* Rst Lane DLL */
#define BITP_DMC_DDR_ROOT_CTL_PIPE_OFSTDCYCLE	10         /* Pipeline offset for PHYC_DATACYCLE */
#define BITM_DMC_DDR_ROOT_CTL_SW_REFRESH	0x00002000 /* Refresh Lane DLL Code */
#define BITM_DMC_DDR_CA_CTL_SW_REFRESH		0x00004000 /* Refresh Lane DLL Code */

#define BITP_DMC_CTL_RL_DQS		26         /* RL_DQS */
#define BITM_DMC_CTL_RL_DQS		0x04000000 /* RL_DQS */
#define BITP_DMC_EMR3_MPR		 2         /* Multi Purpose Read Enable (Read Leveling)*/
#define BITM_DMC_EMR3_MPR		0x00000004 /* Multi Purpose Read Enable (Read Leveling)*/
#define BITM_DMC_MR1_WL			0x00000080 /* Write Leveling Enable.*/
#define BITM_DMC_STAT_PHYRDPHASE	0x00F00000 /* PHY Read Phase */

#define BITP_DMC_DDR_LANE0_CTL1_BYPCODE		10
#define BITM_DMC_DDR_LANE0_CTL1_BYPCODE		0x00007C00
#define BITP_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN	15
#define BITM_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN	0x00008000

#define DMC_ZQCTL0_VALUE			0x00785A64
#define DMC_ZQCTL1_VALUE			0
#define DMC_ZQCTL2_VALUE			0x70000000

#define DMC_TRIG_CALIB				0
#define DMC_OFSTDCYCLE				2

#define BITP_DMC_CAL_PADCTL0_RTTCALEN	31         /* RTT Calibration Enable */
#define BITP_DMC_CAL_PADCTL0_PDCALEN	30         /* PULLDOWN Calib Enable */
#define BITP_DMC_CAL_PADCTL0_PUCALEN	29         /* PULLUP Calib Enable */
#define BITP_DMC_CAL_PADCTL0_CALSTRT	28         /* Start New Calib ( Hardware Cleared) */
#define BITM_DMC_CAL_PADCTL0_RTTCALEN	0x80000000 /* RTT Calibration Enable */
#define BITM_DMC_CAL_PADCTL0_PDCALEN	0x40000000 /* PULLDOWN Calib Enable */
#define BITM_DMC_CAL_PADCTL0_PUCALEN	0x20000000 /* PULLUP Calib Enable */
#define BITM_DMC_CAL_PADCTL0_CALSTRT	0x10000000 /* Start New Calib ( Hardware Cleared) */
#define ENUM_DMC_PHY_CTL4_DDR3		0x00000000 /* DDRMODE: DDR3 Mode */
#define ENUM_DMC_PHY_CTL4_DDR2		0x00000001 /* DDRMODE: DDR2 Mode */
#define ENUM_DMC_PHY_CTL4_LPDDR		0x00000003 /* DDRMODE: LPDDR Mode */

#define BITP_DMC_DDR_ZQ_CTL0_IMPRTT	16         /*  Data/DQS ODT */
#define BITP_DMC_DDR_ZQ_CTL0_IMPWRDQ	 8         /*  Data/DQS/DM/CLK Drive Strength */
#define BITP_DMC_DDR_ZQ_CTL0_IMPWRADD	 0         /*  Address/Command Drive Strength */
#define BITM_DMC_DDR_ZQ_CTL0_IMPRTT	0x00FF0000 /* Data/DQS ODT */
#define BITM_DMC_DDR_ZQ_CTL0_IMPWRDQ	0x0000FF00 /* Data/DQS/DM/CLK Drive Strength */
#define BITM_DMC_DDR_ZQ_CTL0_IMPWRADD	0x000000FF /* Address/Command Drive Strength */

#define BITM_DMC_DDR_ROOT_CTL_TRIG_RD_XFER_ALL	0x00200000 /* All Lane Read Status */

#if defined(CONFIG_ADI_USE_DDR2)
	#define DMC_MR0_VALUE \
		((DMC_BL / 4 + 1) << BITP_DMC_MR_BLEN) | \
		(DMC_CL << BITP_DMC_MR_CL) | \
		(DMC_WRRECOV << BITP_DMC_MR_WRRECOV)

	#define DMC_MR1_VALUE \
		(DMC_MR1_AL << BITP_DMC_MR1_AL | 0x04) \

	#define DMC_MR2_VALUE 0
	#define DMC_MR3_VALUE 0

	#define DMC_CTL_VALUE \
		(DMC_RDTOWR << BITP_DMC_CTL_RDTOWR) | \
		(1 << BITP_DMC_CTL_DLLCAL) | \
		(BITM_DMC_CTL_INIT)
#else
	#define DMC_MR0_VALUE \
		(0 << BITP_DMC_MR_BLEN) | \
		(DMC_CL0 << BITP_DMC_MR_CL0) | \
		(DMC_CL123 << BITP_DMC_MR_CL) | \
		(DMC_WRRECOV << BITP_DMC_MR_WRRECOV) | \
		(1 << BITP_DMC_MR_DLLRST)

	#define DMC_MR1_VALUE \
		(DMC_MR1_DLLEN << BITP_DMC_MR1_DLLEN) | \
		(DMC_MR1_DIC0 << BITP_DMC_MR1_DIC0) | \
		(DMC_MR1_RTT0 << BITP_DMC_MR1_RTT0) | \
		(DMC_MR1_AL << BITP_DMC_MR1_AL) | \
		(DMC_MR1_DIC1 << BITP_DMC_MR1_DIC1) | \
		(DMC_MR1_RTT1 << BITP_DMC_MR1_RTT1) | \
		(DMC_MR1_RTT2 << BITP_DMC_MR1_RTT2) | \
		(DMC_MR1_WL << BITP_DMC_MR1_WL) | \
		(DMC_MR1_TDQS << BITP_DMC_MR1_TDQS) | \
		(DMC_MR1_QOFF << BITP_DMC_MR1_QOFF)

	#define DMC_MR2_VALUE \
		((DMC_WL) << BITP_DMC_MR2_CWL)

	#define DMC_MR3_VALUE \
		((DMC_WL) << BITP_DMC_MR2_CWL)

	#define DMC_CTL_VALUE \
		(DMC_RDTOWR << BITP_DMC_CTL_RDTOWR) | \
		(BITM_DMC_CTL_INIT) | \
		(BITM_DMC_CTL_DDR3EN) | \
		(DMC_CTL_AL_EN << BITP_DMC_CTL_AL_EN)
#endif

#define DMC_DLLCTL_VALUE \
	(DMC_DATACYC << BITP_DMC_DLLCTL_DATACYC) | \
	(DMC_DLLCALRDCNT << BITP_DMC_DLLCTL_DLLCALRDCNT)

#define DMC_CFG_VALUE \
	ENUM_DMC_CFG_IFWID16 | \
	ENUM_DMC_CFG_SDRWID16 | \
	SDR_CHIP_SIZE | \
	ENUM_DMC_CFG_EXTBANK1

#define DMC_TR0_VALUE \
	(DMC_TRCD << BITP_DMC_TR0_TRCD) | \
	(DMC_TWTR << BITP_DMC_TR0_TWTR) | \
	(DMC_TRP << BITP_DMC_TR0_TRP) | \
	(DMC_TRAS << BITP_DMC_TR0_TRAS) | \
	(DMC_TRC << BITP_DMC_TR0_TRC) | \
	(DMC_TMRD << BITP_DMC_TR0_TMRD)

#define DMC_TR1_VALUE \
	(DMC_TREF << BITP_DMC_TR1_TREF) | \
	(DMC_TRFC << BITP_DMC_TR1_TRFC) | \
	(DMC_TRRD << BITP_DMC_TR1_TRRD)

#define DMC_TR2_VALUE \
	(DMC_TFAW << BITP_DMC_TR2_TFAW) | \
	(DMC_TRTP << BITP_DMC_TR2_TRTP) | \
	(DMC_TWR << BITP_DMC_TR2_TWR) | \
	(DMC_TXP << BITP_DMC_TR2_TXP) | \
	(DMC_TCKE << BITP_DMC_TR2_TCKE)

enum DDR_MODE {
	DDR3_MODE,
	DDR2_MODE,
	LPDDR_MODE,
};

enum CALIBRATION_MODE {
	CALIBRATION_LEGACY,
	CALIBRATION_METHOD1,
	CALIBRATION_METHOD2,
};

static struct dmc_param {
	phys_addr_t reg;
	u32 ddr_mode;
	u32 padctl2_value;
	u32 dmc_cphyctl_value;
	u32 dmc_cfg_value;
	u32 dmc_dllctl_value;
	u32 dmc_ctl_value;
	u32 dmc_tr0_value;
	u32 dmc_tr1_value;
	u32 dmc_tr2_value;
	u32 dmc_mr0_value;
	u32 dmc_mr1_value;
	u32 dmc_mr2_value;
	u32 dmc_mr3_value;
	u32 dmc_zqctl0_value;
	u32 dmc_zqctl1_value;
	u32 dmc_zqctl2_value;
	u32 dmc_data_calib_add_value;
	bool phy_init_required;
	bool anomaly_20000037_applicable;
	enum CALIBRATION_MODE calib_mode;
} dmc;

#ifdef CONFIG_SC59X_64
#define DQS_DEFAULT_DELAY	3ul

#define DELAYTRIM	1
#define LANE0_DQS_DELAY	1
#define LANE1_DQS_DELAY	1

#define CLKDIR		0ul

#define DQSTRIM		0
#define DQSCODE		0ul

#define CLKTRIM		0
#define CLKCODE		0ul
#endif

static inline void calibration_legacy(void)
{
	u32 temp;

	/* 1. Set DDR mode to DDR3/DDR2/LPDDR in DMCx_PHY_CTL4 register */
	if (dmc.ddr_mode == DDR3_MODE)
		writel(ENUM_DMC_PHY_CTL4_DDR3, dmc.reg + REG_DMC_PHY_CTL4);
	else if (dmc.ddr_mode == DDR2_MODE)
		writel(ENUM_DMC_PHY_CTL4_DDR2, dmc.reg + REG_DMC_PHY_CTL4);
	else if (dmc.ddr_mode == LPDDR_MODE)
		writel(ENUM_DMC_PHY_CTL4_LPDDR, dmc.reg + REG_DMC_PHY_CTL4);

	/*
	 * 2. Make sure that the bits 6, 7, 25, and 27 of the DMC_PHY_
	 * CTL3 register are set
	 */
	writel(0x0A0000C0, dmc.reg + REG_DMC_PHY_CTL3);

	/*
	 * 3. For DDR2/DDR3 mode, make sure that the bits 0, 1, 2, 3 of
	 * the DMC_PHY_CTL0 register and the bits 26, 27, 28, 29, 30, 31
	 * of the DMC_PHY_CTL2 are set.
	 */
	if (dmc.ddr_mode == DDR3_MODE ||
	    dmc.ddr_mode == DDR2_MODE) {
		writel(0xFC000000, dmc.reg + REG_DMC_PHY_CTL2);
		writel(0x0000000f, dmc.reg + REG_DMC_PHY_CTL0);
	}

	writel(0x00000000, dmc.reg + REG_DMC_PHY_CTL1);

	/* 4. For DDR3 mode, set bit 1 and configure bits [5:2] of the
	 * DMC_CPHY_CTL register with WL=CWL+AL in DCLK cycles.
	 */
	if (dmc.ddr_mode == DDR3_MODE)
		writel(dmc.dmc_cphyctl_value, dmc.reg + REG_DMC_CPHY_CTL);
	/* 5. Perform On Die Termination(ODT) & Driver Impedance Calibration */
	if (dmc.ddr_mode == LPDDR_MODE) {
		/* Bypass processor ODT */
		writel(0x80000, dmc.reg + REG_DMC_PHY_CTL1);
	} else {
		/* Set bits RTTCALEN, PDCALEN, PUCALEN of register */
		temp = BITM_DMC_CAL_PADCTL0_RTTCALEN |
		       BITM_DMC_CAL_PADCTL0_PDCALEN |
		       BITM_DMC_CAL_PADCTL0_PUCALEN;
		writel(temp, dmc.reg + REG_DMC_CAL_PADCTL0);
		/* Configure ODT and drive impedance values in the
		 * DMCx_CAL_PADCTL2 register
		 */
		writel(dmc.padctl2_value, dmc.reg + REG_DMC_CAL_PADCTL2);
		/* start calibration */
		temp |= BITM_DMC_CAL_PADCTL0_CALSTRT;
		writel(temp, dmc.reg + REG_DMC_CAL_PADCTL0);
		/* Wait for PAD calibration to complete - 300 DCLK cycle.
		 * Worst case: CCLK=450 MHz, DCLK=125 MHz
		 */
		dmcdelay(300);
	}
}

static inline void calibration_method1(void)
{
#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	writel(dmc.dmc_zqctl0_value, dmc.reg + REG_DMC_DDR_ZQ_CTL0);
	writel(dmc.dmc_zqctl1_value, dmc.reg + REG_DMC_DDR_ZQ_CTL1);
	writel(dmc.dmc_zqctl2_value, dmc.reg + REG_DMC_DDR_ZQ_CTL2);

	/* Generate the trigger */
	writel(0x0ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	writel(0x00010000ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(8000u);

	/* The [31:26] bits may change if pad ring changes */
	writel(0x0C000001ul | DMC_TRIG_CALIB,  dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(8000u);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
#endif
}

static inline void calibration_method2(void)
{
#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	u32 stat_value = 0x0u;
	u32 drv_pu, drv_pd, odt_pu, odt_pd;
	u32 ro_dt, clk_dqs_drv_impedance;
	u32 temp;

	/* Reset trigger */
	writel(0x0ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_3);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_2);

	/* Writing internal registers in calib pad to zero. Calib mode set
	 * to 1 [26], trig M1 S1 write [16], this enables usage of scratch
	 * registers instead of ZQCTL registers
	 */
	writel(0x04010000ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);

	/* TRIGGER FOR M2-S2 WRITE     -> slave id 31:26  trig m2,s2 write
	 * bit 1->1 slave1 address is 4
	 */
	writel(0x10000002ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);

	/* reset Trigger */
	writel(0x0u, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0u, dmc.reg + REG_DMC_DDR_ROOT_CTL);

	/* write to slave 1, make the power down bit high */
	writel(0x1ul << 12, dmc.reg + REG_DMC_DDR_SCRATCH_3);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_2);
	dmcdelay(2500u);

	/* Calib mode set to 1 [26], trig M1 S1 write [16] */
	writel(0x04010000ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);

	writel(0x10000002ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);

	writel(0x0ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	writel(0x0, dmc.reg + REG_DMC_DDR_SCRATCH_3);

	/* for slave 0 */
	writel(dmc.dmc_zqctl0_value, dmc.reg + REG_DMC_DDR_SCRATCH_2);

	/* Calib mode set to 1 [26], trig M1 S1 write [16] */
	writel(0x04010000ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);

	writel(0x0C000002ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);

	writel(0x0ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);

	/* writing to slave 1
	 * calstrt is 0, but other programming is done
	 *
	 * make power down LOW again, to kickstart BIAS circuit
	 */
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_3);
	writel(0x30000000ul, dmc.reg + REG_DMC_DDR_SCRATCH_2);

	/* write to ca_ctl lane, calib mode set to 1 [26],
	 * trig M1 S1 write [16]
	 */
	writel(0x04010000ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);

	/*  copies data to lane controller slave
	 *  TRIGGER FOR M2-S2 WRITE     -> slave id 31:26
	 *  trig m2,s2 write bit 1->1
	 *  slave1 address is 4
	 */
	writel(0x10000002ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);

	/* reset Trigger */
	writel(0x0ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_3);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_2);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_3);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_2);
	writel(0x04010000ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);
	writel(0x10000002ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_3);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_2);
	writel(0x0ul, dmc.reg + REG_DMC_DDR_SCRATCH_3);
	writel(0x50000000ul, dmc.reg + REG_DMC_DDR_SCRATCH_2);
	writel(0x04010000ul, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);
	writel(0x10000002ul, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);
	writel(0u, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0u, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	writel(0x0C000004u, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);
	writel(BITM_DMC_DDR_ROOT_CTL_TRIG_RD_XFER_ALL,
	       dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);
	writel(0u, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0u, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	// calculate ODT PU and PD values
	stat_value = ((readl(dmc.reg + REG_DMC_DDR_SCRATCH_7) & 0x0000FFFFu) <<
		16);
	stat_value |= ((readl(dmc.reg + REG_DMC_DDR_SCRATCH_6) & 0xFFFF0000u) >>
		16);
	clk_dqs_drv_impedance = ((dmc.dmc_zqctl0_value) &
		BITM_DMC_DDR_ZQ_CTL0_IMPWRDQ) >> BITP_DMC_DDR_ZQ_CTL0_IMPWRDQ;
	ro_dt = ((dmc.dmc_zqctl0_value) & BITM_DMC_DDR_ZQ_CTL0_IMPRTT) >>
		BITP_DMC_DDR_ZQ_CTL0_IMPRTT;
	drv_pu = stat_value & 0x0000003Fu;
	drv_pd = (stat_value >> 12) & 0x0000003Fu;
	odt_pu = (drv_pu * clk_dqs_drv_impedance) / ro_dt;
	odt_pd = (drv_pd * clk_dqs_drv_impedance) / ro_dt;
	temp = ((1uL << 24)                   |
	       ((drv_pd & 0x0000003Fu))       |
	       ((odt_pd & 0x0000003Fu) << 6)  |
	       ((drv_pu & 0x0000003Fu) << 12) |
	       ((odt_pu & 0x0000003Fu) << 18));
	temp |= readl(dmc.reg + REG_DMC_DDR_SCRATCH_2);
	writel(temp, dmc.reg + REG_DMC_DDR_SCRATCH_2);
	writel(0x0C010000u, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);
	writel(0x08000002u, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);
	writel(0u, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0u, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	writel(0x04010000u, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);
	writel(0x80000002u, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(2500u);
	writel(0u, dmc.reg + REG_DMC_DDR_CA_CTL);
	writel(0u, dmc.reg + REG_DMC_DDR_ROOT_CTL);
#endif
}

static inline void adi_dmc_lane_reset(bool reset, uint32_t dmc_no)
{
#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	u32 temp;
	phys_addr_t base = (dmc_no == 0) ? REG_DMC0_BASE : REG_DMC1_BASE;
	phys_addr_t ln0 = base + REG_DMC_DDR_LANE0_CTL0;
	phys_addr_t ln1 = base + REG_DMC_DDR_LANE1_CTL0;

	if (reset) {
		temp = readl(ln0);
		temp |= BITM_DMC_DDR_LANE0_CTL0_CB_RSTDLL;
		writel(temp, ln0);

		temp = readl(ln1);
		temp |= BITM_DMC_DDR_LANE1_CTL0_CB_RSTDLL;
		writel(temp, ln1);
	} else {
		temp = readl(ln0);
		temp &= ~BITM_DMC_DDR_LANE0_CTL0_CB_RSTDLL;
		writel(temp, ln0);

		temp = readl(ln1);
		temp &= ~BITM_DMC_DDR_LANE1_CTL0_CB_RSTDLL;
		writel(temp, ln1);
	}
	dmcdelay(9000u);
#endif
}

void adi_dmc_reset_lanes(bool reset)
{
	if (!IS_ENABLED(CONFIG_ADI_USE_DDR2)) {
		if (IS_ENABLED(CONFIG_SC59X) || IS_ENABLED(CONFIG_SC59X_64)) {
			if (IS_ENABLED(CONFIG_ADI_USE_DMC0))
				adi_dmc_lane_reset(reset, 0);
			if (IS_ENABLED(CONFIG_ADI_USE_DMC1))
				adi_dmc_lane_reset(reset, 1);
		}
		else {
			u32 temp = reset ? 0x800 : 0x0;

			if (IS_ENABLED(CONFIG_ADI_USE_DMC0))
				writel(temp, REG_DMC0_BASE + REG_DMC_PHY_CTL0);
			if (IS_ENABLED(CONFIG_ADI_USE_DMC1))
				writel(temp, REG_DMC1_BASE + REG_DMC_PHY_CTL0);
		}
	}
}

static inline void dmc_controller_init(void)
{
#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	u32 phyphase, rd_cnt, t_EMR1, t_EMR3, t_CTL, data_cyc, temp;
#endif

	/* 1. Program the DMC controller registers: DMCx_CFG, DMCx_TR0,
	 * DMCx_TR1, DMCx_TR2, DMCx_MR(DDR2/LPDDR)/DMCx_MR0(DDR3),
	 * DMCx_EMR1(DDR2)/DMCx_MR1(DDR3),
	 * DMCx_EMR2(DDR2)/DMCx_EMR(LPDDR)/DMCx_MR2(DDR3)
	 */
	writel(dmc.dmc_cfg_value, dmc.reg + REG_DMC_CFG);
	writel(dmc.dmc_tr0_value, dmc.reg + REG_DMC_TR0);
	writel(dmc.dmc_tr1_value, dmc.reg + REG_DMC_TR1);
	writel(dmc.dmc_tr2_value, dmc.reg + REG_DMC_TR2);
	writel(dmc.dmc_mr0_value, dmc.reg + REG_DMC_MR);
	writel(dmc.dmc_mr1_value, dmc.reg + REG_DMC_EMR1);
	writel(dmc.dmc_mr2_value, dmc.reg + REG_DMC_EMR2);

#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	writel(dmc.dmc_mr3_value, dmc.reg + REG_DMC_EMR3);
	writel(dmc.dmc_dllctl_value, dmc.reg + REG_DMC_DLLCTL);
	dmcdelay(2000u);

	temp = readl(dmc.reg + REG_DMC_DDR_CA_CTL);
	temp |= BITM_DMC_DDR_CA_CTL_SW_REFRESH;
	writel(temp, dmc.reg + REG_DMC_DDR_CA_CTL);
	dmcdelay(5u);

	temp = readl(dmc.reg + REG_DMC_DDR_ROOT_CTL);
	temp |= BITM_DMC_DDR_ROOT_CTL_SW_REFRESH |
		(DMC_OFSTDCYCLE << BITP_DMC_DDR_ROOT_CTL_PIPE_OFSTDCYCLE);
	writel(temp, dmc.reg + REG_DMC_DDR_ROOT_CTL);
#endif

	/* 2. Make sure that the REG_DMC_DT_CALIB_ADDR register is programmed
	 * to an unused DMC location corresponding to a burst of 16 bytes
	 * (by default it is the starting address of the DMC address range).
	 */
#ifndef CONFIG_SC59X
	writel(dmc.dmc_data_calib_add_value, dmc.reg + REG_DMC_DT_CALIB_ADDR);
#endif
	/* 3. Program the DMCx_CTL register with INIT bit set to start
	 * the DMC initialization sequence
	 */
	writel(dmc.dmc_ctl_value, dmc.reg + REG_DMC_CTL);
	/* 4. Wait for the DMC initialization to complete by polling
	 * DMCx_STAT.INITDONE bit.
	 */

#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	dmcdelay(722000u);

	/* Add necessary delay depending on the configuration */
	t_EMR1 = (dmc.dmc_mr1_value & BITM_DMC_MR1_WL) >> BITP_DMC_MR1_WL;

	dmcdelay(600u);
	if (t_EMR1 != 0u)
		while ((readl(dmc.reg + REG_DMC_EMR1) & BITM_DMC_MR1_WL) != 0)
			;

	t_EMR3 = (dmc.dmc_mr3_value & BITM_DMC_EMR3_MPR) >>
		 BITP_DMC_EMR3_MPR;
	dmcdelay(2000u);
	if (t_EMR3 != 0u)
		while ((readl(dmc.reg + REG_DMC_EMR3) & BITM_DMC_EMR3_MPR) != 0)
			;

	t_CTL = (dmc.dmc_ctl_value & BITM_DMC_CTL_RL_DQS) >> BITP_DMC_CTL_RL_DQS;
	dmcdelay(600u);
	if (t_CTL != 0u)
		while ((readl(dmc.reg + REG_DMC_CTL) & BITM_DMC_CTL_RL_DQS) != 0)
			;
#endif

	/* check if DMC initialization finished*/
	while ((readl(dmc.reg + REG_DMC_STAT) & BITM_DMC_STAT_INITDONE) == 0)
		;

#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	/* toggle DCYCLE */
	temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL1);
	temp |= BITM_DMC_DDR_LANE0_CTL1_COMP_DCYCLE;
	writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL1);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL1);
	temp |= BITM_DMC_DDR_LANE1_CTL1_COMP_DCYCLE;
	writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL1);

	dmcdelay(10u);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL1);
	temp &= (~BITM_DMC_DDR_LANE0_CTL1_COMP_DCYCLE);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL1);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL1);
	temp &= (~BITM_DMC_DDR_LANE1_CTL1_COMP_DCYCLE);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL1);

	/* toggle RSTDAT */
	temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL0);
	temp |= BITM_DMC_DDR_LANE0_CTL0_CB_RSTDAT;
	writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL0);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL0);
	temp &= (~BITM_DMC_DDR_LANE0_CTL0_CB_RSTDAT);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL0);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL0);
	temp |= BITM_DMC_DDR_LANE1_CTL0_CB_RSTDAT;
	writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL0);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL0);
	temp &= (~BITM_DMC_DDR_LANE1_CTL0_CB_RSTDAT);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL0);

	dmcdelay(2500u);

	/* Program phyphase*/
	phyphase = (readl(dmc.reg + REG_DMC_STAT) &
		   BITM_DMC_STAT_PHYRDPHASE) >> BITP_DMC_STAT_PHYRDPHASE;
	data_cyc = (phyphase << BITP_DMC_DLLCTL_DATACYC) &
		   BITM_DMC_DLLCTL_DATACYC;
	rd_cnt = dmc.dmc_dllctl_value;
	rd_cnt <<= BITP_DMC_DLLCTL_DLLCALRDCNT;
	rd_cnt &= BITM_DMC_DLLCTL_DLLCALRDCNT;
	writel(rd_cnt | data_cyc, dmc.reg + REG_DMC_DLLCTL);
	writel((dmc.dmc_ctl_value & (~BITM_DMC_CTL_INIT) &
	       (~BITM_DMC_CTL_RL_DQS)), dmc.reg + REG_DMC_CTL);

#if DELAYTRIM
	/* DQS delay trim*/
	u32 stat_value, WL_code_LDQS, WL_code_UDQS;

	/* For LDQS */
	temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL1) | (0x000000D0);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL1);
	dmcdelay(2500u);
	writel(0x00400000, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);
	writel(0x0, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	stat_value = (readl(dmc.reg + REG_DMC_DDR_SCRATCH_STAT0) &
		     (0xFFFF0000)) >> 16;
	WL_code_LDQS = (stat_value) & (0x0000001F);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL1);
	temp &= ~(BITM_DMC_DDR_LANE0_CTL1_BYPCODE |
		  BITM_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL1);

	/* If write leveling is enabled */
	if ((dmc.dmc_mr1_value & BITM_DMC_MR1_WL) >> BITP_DMC_MR1_WL) {
		temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL1);
		temp |= (((WL_code_LDQS + LANE0_DQS_DELAY) <<
			   BITP_DMC_DDR_LANE0_CTL1_BYPCODE) &
			    BITM_DMC_DDR_LANE0_CTL1_BYPCODE) |
			     BITM_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN;
		writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL1);
	} else {
		temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL1);
		temp |= (((DQS_DEFAULT_DELAY + LANE0_DQS_DELAY) <<
			   BITP_DMC_DDR_LANE0_CTL1_BYPCODE) &
			    BITM_DMC_DDR_LANE0_CTL1_BYPCODE) |
			     BITM_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN;
		writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL1);
	}
	dmcdelay(2500u);

	/* For UDQS */
	temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL1) | (0x000000D0);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL1);
	dmcdelay(2500u);
	writel(0x00800000, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	dmcdelay(2500u);
	writel(0x0, dmc.reg + REG_DMC_DDR_ROOT_CTL);
	stat_value = (readl(dmc.reg + REG_DMC_DDR_SCRATCH_STAT1) &
		     (0xFFFF0000)) >> 16;
	WL_code_UDQS = (stat_value) & (0x0000001F);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL1);
	temp &= ~(BITM_DMC_DDR_LANE0_CTL1_BYPCODE |
		BITM_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL1);

	/* If write leveling is enabled */
	if ((dmc.dmc_mr1_value & BITM_DMC_MR1_WL) >> BITP_DMC_MR1_WL) {
		temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL1);
		temp |= (((WL_code_UDQS + LANE1_DQS_DELAY) <<
			   BITP_DMC_DDR_LANE0_CTL1_BYPCODE) &
			    BITM_DMC_DDR_LANE0_CTL1_BYPCODE) |
			     BITM_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN;
		writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL1);
	} else {
		temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL1);
		temp |= (((DQS_DEFAULT_DELAY + LANE1_DQS_DELAY) <<
			   BITP_DMC_DDR_LANE0_CTL1_BYPCODE) &
			    BITM_DMC_DDR_LANE0_CTL1_BYPCODE) |
			     BITM_DMC_DDR_LANE0_CTL1_BYPDELCHAINEN;
		writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL1);
	}
	dmcdelay(2500u);
#endif

#else
	/* 5. Program the DMCx_CTL.DLLCTL register with 0x948 value
	 * (DATACYC=9,    DLLCALRDCNT=72).
	 */
	writel(0x00000948, dmc.reg + REG_DMC_DLLCTL);
#endif

	/* 6. Workaround for anomaly#20000037 */
	if (dmc.anomaly_20000037_applicable) {
		/* Perform dummy read to any DMC location */
		readl(0x80000000);

		writel(readl(dmc.reg + REG_DMC_PHY_CTL0) | 0x1000,
		       dmc.reg + REG_DMC_PHY_CTL0);
		/* Clear DMCx_PHY_CTL0.RESETDAT bit */
		writel(readl(dmc.reg + REG_DMC_PHY_CTL0) & (~0x1000),
		       dmc.reg + REG_DMC_PHY_CTL0);
	}
}

static inline void dmc_init(void)
{
	/* PHY Calibration+Initialization */
	if (!dmc.phy_init_required)
		goto out;

	switch (dmc.calib_mode) {
	case CALIBRATION_LEGACY:
		calibration_legacy();
		break;
	case CALIBRATION_METHOD1:
		calibration_method1();
		break;
	case CALIBRATION_METHOD2:
		calibration_method2();
		break;
	}

#if DQSTRIM
	/* DQS duty trim */
	temp = readl(dmc.reg + REG_DMC_DDR_LANE0_CTL0);
	temp |= ((DQSCODE) << BITP_DMC_DDR_LANE0_CTL0_BYPENB) &
		 (BITM_DMC_DDR_LANE1_CTL0_BYPENB |
		  BITM_DMC_DDR_LANE0_CTL0_BYPSELP |
		  BITM_DMC_DDR_LANE0_CTL0_BYPCODE);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE0_CTL0);

	temp = readl(dmc.reg + REG_DMC_DDR_LANE1_CTL0);
	temp |= ((DQSCODE) << BITP_DMC_DDR_LANE1_CTL0_BYPENB) &
		 (BITM_DMC_DDR_LANE1_CTL1_BYPCODE |
		  BITM_DMC_DDR_LANE1_CTL0_BYPSELP |
		  BITM_DMC_DDR_LANE1_CTL0_BYPCODE);
	writel(temp, dmc.reg + REG_DMC_DDR_LANE1_CTL0);
#endif

#if CLKTRIM
	/* Clock duty trim */
	temp = readl(dmc.reg + REG_DMC_DDR_CA_CTL);
	temp |= (((CLKCODE << BITP_DMC_DDR_CA_CTL_BYPCODE1) &
		   BITM_DMC_DDR_CA_CTL_BYPCODE1) |
		 BITM_DMC_DDR_CA_CTL_BYPENB |
		 ((CLKDIR << BITP_DMC_DDR_CA_CTL_BYPSELP) &
		  BITM_DMC_DDR_CA_CTL_BYPSELP));
	writel(temp, dmc.reg + REG_DMC_DDR_CA_CTL);
#endif

out:
	/* Controller Initialization */
	dmc_controller_init();
}

static inline void __dmc_config(uint32_t dmc_no)
{
	if (dmc_no == 0) {
		dmc.reg = REG_DMC0_BASE;
		dmc.dmc_data_calib_add_value = DMC0_DATA_CALIB_ADD;
	} else if (dmc_no == 1) {
		dmc.reg = REG_DMC1_BASE;
		dmc.dmc_data_calib_add_value = DMC1_DATA_CALIB_ADD;
	} else {
		return;
	}

	if (IS_ENABLED(CONFIG_ADI_USE_DDR2))
		dmc.ddr_mode = DDR2_MODE;
	else
		dmc.ddr_mode = DDR3_MODE;

	dmc.phy_init_required = true;

#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	dmc.anomaly_20000037_applicable = false;
	dmc.dmc_dllctl_value = DMC_DLLCTL_VALUE;
	dmc.calib_mode = CALIBRATION_METHOD2;
#else
	dmc.anomaly_20000037_applicable = true;
	dmc.calib_mode = CALIBRATION_LEGACY;
#endif

	dmc.dmc_ctl_value = DMC_CTL_VALUE;
	dmc.dmc_cfg_value = DMC_CFG_VALUE;
	dmc.dmc_tr0_value = DMC_TR0_VALUE;
	dmc.dmc_tr1_value = DMC_TR1_VALUE;
	dmc.dmc_tr2_value = DMC_TR2_VALUE;
	dmc.dmc_mr0_value = DMC_MR0_VALUE;
	dmc.dmc_mr1_value = DMC_MR1_VALUE;
	dmc.dmc_mr2_value = DMC_MR2_VALUE;

#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	dmc.dmc_mr3_value = DMC_MR3_VALUE;
	dmc.dmc_zqctl0_value = DMC_ZQCTL0_VALUE;
	dmc.dmc_zqctl1_value = DMC_ZQCTL1_VALUE;
	dmc.dmc_zqctl2_value = DMC_ZQCTL2_VALUE;
#endif

	dmc.padctl2_value = DMC_PADCTL2_VALUE;
	dmc.dmc_cphyctl_value = DMC_CPHYCTL_VALUE;

	/* Initialize DMC now */
	dmc_init();
}

void DMC_Config(void)
{
	if (IS_ENABLED(CONFIG_ADI_USE_DMC0))
		__dmc_config(0);

	if (IS_ENABLED(CONFIG_ADI_USE_DMC1))
		__dmc_config(1);
}
