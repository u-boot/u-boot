/*
 * LayerScape Internal Memory Map
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_FSL_LSCH3_IMMAP_H_
#define __ARCH_FSL_LSCH3_IMMAP_H_

#define CONFIG_SYS_IMMR				0x01000000
#define CONFIG_SYS_FSL_DDR_ADDR			(CONFIG_SYS_IMMR + 0x00080000)
#define CONFIG_SYS_FSL_DDR2_ADDR		(CONFIG_SYS_IMMR + 0x00090000)
#define CONFIG_SYS_FSL_DDR3_ADDR		0x08210000
#define CONFIG_SYS_FSL_GUTS_ADDR		(CONFIG_SYS_IMMR + 0x00E00000)
#define CONFIG_SYS_FSL_PMU_ADDR			(CONFIG_SYS_IMMR + 0x00E30000)
#define CONFIG_SYS_FSL_RST_ADDR			(CONFIG_SYS_IMMR + 0x00E60000)
#define CONFIG_SYS_FSL_CH3_CLK_GRPA_ADDR	(CONFIG_SYS_IMMR + 0x00300000)
#define CONFIG_SYS_FSL_CH3_CLK_GRPB_ADDR	(CONFIG_SYS_IMMR + 0x00310000)
#define CONFIG_SYS_FSL_CH3_CLK_CTRL_ADDR	(CONFIG_SYS_IMMR + 0x00370000)
#define CONFIG_SYS_FSL_ESDHC_ADDR		(CONFIG_SYS_IMMR + 0x01140000)
#define CONFIG_SYS_IFC_ADDR			(CONFIG_SYS_IMMR + 0x01240000)
#define CONFIG_SYS_NS16550_COM1			(CONFIG_SYS_IMMR + 0x011C0500)
#define CONFIG_SYS_NS16550_COM2			(CONFIG_SYS_IMMR + 0x011C0600)
#define SYS_FSL_LS2080A_LS2085A_TIMER_ADDR	0x023d0000
#define CONFIG_SYS_FSL_TIMER_ADDR		0x023e0000
#define CONFIG_SYS_FSL_PMU_CLTBENR		(CONFIG_SYS_FSL_PMU_ADDR + \
						 0x18A0)
#define FSL_PMU_PCTBENR_OFFSET (CONFIG_SYS_FSL_PMU_ADDR + 0x8A0)
#define FSL_LSCH3_SVR		(CONFIG_SYS_FSL_GUTS_ADDR + 0xA4)

#define CONFIG_SYS_FSL_WRIOP1_ADDR		(CONFIG_SYS_IMMR + 0x7B80000)
#define CONFIG_SYS_FSL_WRIOP1_MDIO1	(CONFIG_SYS_FSL_WRIOP1_ADDR + 0x16000)
#define CONFIG_SYS_FSL_WRIOP1_MDIO2	(CONFIG_SYS_FSL_WRIOP1_ADDR + 0x17000)
#define CONFIG_SYS_FSL_LSCH3_SERDES_ADDR	(CONFIG_SYS_IMMR + 0xEA0000)

#define CONFIG_SYS_FSL_DCSR_DDR_ADDR		0x70012c000ULL
#define CONFIG_SYS_FSL_DCSR_DDR2_ADDR		0x70012d000ULL
#define CONFIG_SYS_FSL_DCSR_DDR3_ADDR		0x700132000ULL
#define CONFIG_SYS_FSL_DCSR_DDR4_ADDR		0x700133000ULL

#define I2C1_BASE_ADDR				(CONFIG_SYS_IMMR + 0x01000000)
#define I2C2_BASE_ADDR				(CONFIG_SYS_IMMR + 0x01010000)
#define I2C3_BASE_ADDR				(CONFIG_SYS_IMMR + 0x01020000)
#define I2C4_BASE_ADDR				(CONFIG_SYS_IMMR + 0x01030000)

#define CONFIG_SYS_XHCI_USB1_ADDR		(CONFIG_SYS_IMMR + 0x02100000)
#define CONFIG_SYS_XHCI_USB2_ADDR		(CONFIG_SYS_IMMR + 0x02110000)

/* TZ Address Space Controller Definitions */
#define TZASC1_BASE			0x01100000	/* as per CCSR map. */
#define TZASC2_BASE			0x01110000	/* as per CCSR map. */
#define TZASC3_BASE			0x01120000	/* as per CCSR map. */
#define TZASC4_BASE			0x01130000	/* as per CCSR map. */
#define TZASC_BUILD_CONFIG_REG(x)	((TZASC1_BASE + (x * 0x10000)))
#define TZASC_ACTION_REG(x)		((TZASC1_BASE + (x * 0x10000)) + 0x004)
#define TZASC_GATE_KEEPER(x)		((TZASC1_BASE + (x * 0x10000)) + 0x008)
#define TZASC_REGION_BASE_LOW_0(x)	((TZASC1_BASE + (x * 0x10000)) + 0x100)
#define TZASC_REGION_BASE_HIGH_0(x)	((TZASC1_BASE + (x * 0x10000)) + 0x104)
#define TZASC_REGION_TOP_LOW_0(x)	((TZASC1_BASE + (x * 0x10000)) + 0x108)
#define TZASC_REGION_TOP_HIGH_0(x)	((TZASC1_BASE + (x * 0x10000)) + 0x10C)
#define TZASC_REGION_ATTRIBUTES_0(x)	((TZASC1_BASE + (x * 0x10000)) + 0x110)
#define TZASC_REGION_ID_ACCESS_0(x)	((TZASC1_BASE + (x * 0x10000)) + 0x114)

/* SATA */
#define AHCI_BASE_ADDR1				(CONFIG_SYS_IMMR + 0x02200000)
#define AHCI_BASE_ADDR2				(CONFIG_SYS_IMMR + 0x02210000)

/* SFP */
#define CONFIG_SYS_SFP_ADDR		(CONFIG_SYS_IMMR + 0x00e80200)

/* SEC */
#define CONFIG_SYS_FSL_SEC_OFFSET		0x07000000ull
#define CONFIG_SYS_FSL_JR0_OFFSET		0x07010000ull
#define CONFIG_SYS_FSL_SEC_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_SEC_OFFSET)
#define CONFIG_SYS_FSL_JR0_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_JR0_OFFSET)

/* Security Monitor */
#define CONFIG_SYS_SEC_MON_ADDR		(CONFIG_SYS_IMMR + 0x00e90000)

/* MMU 500 */
#define SMMU_SCR0			(SMMU_BASE + 0x0)
#define SMMU_SCR1			(SMMU_BASE + 0x4)
#define SMMU_SCR2			(SMMU_BASE + 0x8)
#define SMMU_SACR			(SMMU_BASE + 0x10)
#define SMMU_IDR0			(SMMU_BASE + 0x20)
#define SMMU_IDR1			(SMMU_BASE + 0x24)

#define SMMU_NSCR0			(SMMU_BASE + 0x400)
#define SMMU_NSCR2			(SMMU_BASE + 0x408)
#define SMMU_NSACR			(SMMU_BASE + 0x410)

#define SCR0_CLIENTPD_MASK		0x00000001
#define SCR0_USFCFG_MASK		0x00000400


/* PCIe */
#define CONFIG_SYS_PCIE1_ADDR			(CONFIG_SYS_IMMR + 0x2400000)
#define CONFIG_SYS_PCIE2_ADDR			(CONFIG_SYS_IMMR + 0x2500000)
#define CONFIG_SYS_PCIE3_ADDR			(CONFIG_SYS_IMMR + 0x2600000)
#define CONFIG_SYS_PCIE4_ADDR			(CONFIG_SYS_IMMR + 0x2700000)
#define CONFIG_SYS_PCIE1_PHYS_ADDR		0x1000000000ULL
#define CONFIG_SYS_PCIE2_PHYS_ADDR		0x1200000000ULL
#define CONFIG_SYS_PCIE3_PHYS_ADDR		0x1400000000ULL
#define CONFIG_SYS_PCIE4_PHYS_ADDR		0x1600000000ULL
/* LUT registers */
#define PCIE_LUT_BASE				0x80000
#define PCIE_LUT_LCTRL0				0x7F8
#define PCIE_LUT_DBG				0x7FC
#define PCIE_LUT_UDR(n)         (0x800 + (n) * 8)
#define PCIE_LUT_LDR(n)         (0x804 + (n) * 8)
#define PCIE_LUT_ENABLE         (1 << 31)
#define PCIE_LUT_ENTRY_COUNT    32

/* Device Configuration */
#define DCFG_BASE		0x01e00000
#define DCFG_PORSR1			0x000
#define DCFG_PORSR1_RCW_SRC		0xff800000
#define DCFG_PORSR1_RCW_SRC_NOR		0x12f00000
#define DCFG_RCWSR13			0x130
#define DCFG_RCWSR13_DSPI		(0 << 8)
#define DCFG_RCWSR15			0x138
#define DCFG_RCWSR15_IFCGRPABASE_QSPI	0x3

#define DCFG_DCSR_BASE		0X700100000ULL
#define DCFG_DCSR_PORCR1		0x000

/* Interrupt Sampling Control */
#define ISC_BASE		0x01F70000
#define IRQCR_OFFSET		0x14

/* Supplemental Configuration */
#define SCFG_BASE		0x01fc0000
#define SCFG_USB3PRM1CR			0x000
#define SCFG_USB3PRM1CR_INIT		0x27672b2a
#define SCFG_QSPICLKCTLR	0x10

#define TP_ITYP_AV		0x00000001	/* Initiator available */
#define TP_ITYP_TYPE(x)	(((x) & 0x6) >> 1)	/* Initiator Type */
#define TP_ITYP_TYPE_ARM	0x0
#define TP_ITYP_TYPE_PPC	0x1		/* PowerPC */
#define TP_ITYP_TYPE_OTHER	0x2		/* StarCore DSP */
#define TP_ITYP_TYPE_HA		0x3		/* HW Accelerator */
#define TP_ITYP_THDS(x)	(((x) & 0x18) >> 3)	/* # threads */
#define TP_ITYP_VER(x)	(((x) & 0xe0) >> 5)	/* Initiator Version */
#define TY_ITYP_VER_A7		0x1
#define TY_ITYP_VER_A53		0x2
#define TY_ITYP_VER_A57		0x3
#define TY_ITYP_VER_A72		0x4

#define TP_CLUSTER_EOC		0x80000000	/* end of clusters */
#define TP_CLUSTER_INIT_MASK	0x0000003f	/* initiator mask */
#define TP_INIT_PER_CLUSTER     4
/* This is chassis generation 3 */
#ifndef __ASSEMBLY__
struct sys_info {
	unsigned long freq_processor[CONFIG_MAX_CPUS];
	unsigned long freq_systembus;
	unsigned long freq_ddrbus;
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	unsigned long freq_ddrbus2;
#endif
	unsigned long freq_localbus;
	unsigned long freq_qe;
#ifdef CONFIG_SYS_DPAA_FMAN
	unsigned long freq_fman[CONFIG_SYS_NUM_FMAN];
#endif
#ifdef CONFIG_SYS_DPAA_QBMAN
	unsigned long freq_qman;
#endif
#ifdef CONFIG_SYS_DPAA_PME
	unsigned long freq_pme;
#endif
};

/* Global Utilities Block */
struct ccsr_gur {
	u32	porsr1;		/* POR status 1 */
	u32	porsr2;		/* POR status 2 */
	u8	res_008[0x20-0x8];
	u32	gpporcr1;	/* General-purpose POR configuration */
	u32	gpporcr2;	/* General-purpose POR configuration 2 */
#define FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT	25
#define FSL_CHASSIS3_DCFG_FUSESR_VID_MASK	0x1F
#define FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT	20
#define FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK	0x1F
	u32	dcfg_fusesr;	/* Fuse status register */
	u32	gpporcr3;
	u32	gpporcr4;
	u8	res_034[0x70-0x34];
	u32	devdisr;	/* Device disable control */
	u32	devdisr2;	/* Device disable control 2 */
	u32	devdisr3;	/* Device disable control 3 */
	u32	devdisr4;	/* Device disable control 4 */
	u32	devdisr5;	/* Device disable control 5 */
	u32	devdisr6;	/* Device disable control 6 */
	u32	devdisr7;	/* Device disable control 7 */
#define FSL_CHASSIS3_DEVDISR2_DPMAC1	0x00000001
#define FSL_CHASSIS3_DEVDISR2_DPMAC2	0x00000002
#define FSL_CHASSIS3_DEVDISR2_DPMAC3	0x00000004
#define FSL_CHASSIS3_DEVDISR2_DPMAC4	0x00000008
#define FSL_CHASSIS3_DEVDISR2_DPMAC5	0x00000010
#define FSL_CHASSIS3_DEVDISR2_DPMAC6	0x00000020
#define FSL_CHASSIS3_DEVDISR2_DPMAC7	0x00000040
#define FSL_CHASSIS3_DEVDISR2_DPMAC8	0x00000080
#define FSL_CHASSIS3_DEVDISR2_DPMAC9	0x00000100
#define FSL_CHASSIS3_DEVDISR2_DPMAC10	0x00000200
#define FSL_CHASSIS3_DEVDISR2_DPMAC11	0x00000400
#define FSL_CHASSIS3_DEVDISR2_DPMAC12	0x00000800
#define FSL_CHASSIS3_DEVDISR2_DPMAC13	0x00001000
#define FSL_CHASSIS3_DEVDISR2_DPMAC14	0x00002000
#define FSL_CHASSIS3_DEVDISR2_DPMAC15	0x00004000
#define FSL_CHASSIS3_DEVDISR2_DPMAC16	0x00008000
#define FSL_CHASSIS3_DEVDISR2_DPMAC17	0x00010000
#define FSL_CHASSIS3_DEVDISR2_DPMAC18	0x00020000
#define FSL_CHASSIS3_DEVDISR2_DPMAC19	0x00040000
#define FSL_CHASSIS3_DEVDISR2_DPMAC20	0x00080000
#define FSL_CHASSIS3_DEVDISR2_DPMAC21	0x00100000
#define FSL_CHASSIS3_DEVDISR2_DPMAC22	0x00200000
#define FSL_CHASSIS3_DEVDISR2_DPMAC23	0x00400000
#define FSL_CHASSIS3_DEVDISR2_DPMAC24	0x00800000
	u8	res_08c[0x90-0x8c];
	u32	coredisru;	/* uppper portion for support of 64 cores */
	u32	coredisrl;	/* lower portion for support of 64 cores */
	u8	res_098[0xa0-0x98];
	u32	pvr;		/* Processor version */
	u32	svr;		/* System version */
	u32	mvr;		/* Manufacturing version */
	u8	res_0ac[0x100-0xac];
	u32	rcwsr[32];	/* Reset control word status */

#define FSL_CHASSIS3_RCWSR0_SYS_PLL_RAT_SHIFT	2
#define FSL_CHASSIS3_RCWSR0_SYS_PLL_RAT_MASK	0x1f
#define FSL_CHASSIS3_RCWSR0_MEM_PLL_RAT_SHIFT	10
#define FSL_CHASSIS3_RCWSR0_MEM_PLL_RAT_MASK	0x3f
#define FSL_CHASSIS3_RCWSR0_MEM2_PLL_RAT_SHIFT	18
#define FSL_CHASSIS3_RCWSR0_MEM2_PLL_RAT_MASK	0x3f
#define	FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK	0x00FF0000
#define	FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT	16
#define	FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK	0xFF000000
#define	FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT	24
#define RCW_SB_EN_REG_INDEX	9
#define RCW_SB_EN_MASK		0x00000400

	u8	res_180[0x200-0x180];
	u32	scratchrw[32];	/* Scratch Read/Write */
	u8	res_280[0x300-0x280];
	u32	scratchw1r[4];	/* Scratch Read (Write once) */
	u8	res_310[0x400-0x310];
	u32	bootlocptrl;	/* Boot location pointer low-order addr */
	u32	bootlocptrh;	/* Boot location pointer high-order addr */
	u8	res_408[0x500-0x408];
	u8	res_500[0x740-0x500];	/* add more registers when needed */
	u32	tp_ityp[64];	/* Topology Initiator Type Register */
	struct {
		u32	upper;
		u32	lower;
	} tp_cluster[3];	/* Core Cluster n Topology Register */
	u8	res_858[0x1000-0x858];
};


struct ccsr_clk_cluster_group {
	struct {
		u8	res_00[0x10];
		u32	csr;
		u8	res_14[0x20-0x14];
	} hwncsr[3];
	u8	res_60[0x80-0x60];
	struct {
		u32	gsr;
		u8	res_84[0xa0-0x84];
	} pllngsr[3];
	u8	res_e0[0x100-0xe0];
};

struct ccsr_clk_ctrl {
	struct {
		u32 csr;	/* core cluster n clock control status */
		u8  res_04[0x20-0x04];
	} clkcncsr[8];
};

struct ccsr_reset {
	u32 rstcr;			/* 0x000 */
	u32 rstcrsp;			/* 0x004 */
	u8 res_008[0x10-0x08];		/* 0x008 */
	u32 rstrqmr1;			/* 0x010 */
	u32 rstrqmr2;			/* 0x014 */
	u32 rstrqsr1;			/* 0x018 */
	u32 rstrqsr2;			/* 0x01c */
	u32 rstrqwdtmrl;		/* 0x020 */
	u32 rstrqwdtmru;		/* 0x024 */
	u8 res_028[0x30-0x28];		/* 0x028 */
	u32 rstrqwdtsrl;		/* 0x030 */
	u32 rstrqwdtsru;		/* 0x034 */
	u8 res_038[0x60-0x38];		/* 0x038 */
	u32 brrl;			/* 0x060 */
	u32 brru;			/* 0x064 */
	u8 res_068[0x80-0x68];		/* 0x068 */
	u32 pirset;			/* 0x080 */
	u32 pirclr;			/* 0x084 */
	u8 res_088[0x90-0x88];		/* 0x088 */
	u32 brcorenbr;			/* 0x090 */
	u8 res_094[0x100-0x94];		/* 0x094 */
	u32 rcw_reqr;			/* 0x100 */
	u32 rcw_completion;		/* 0x104 */
	u8 res_108[0x110-0x108];	/* 0x108 */
	u32 pbi_reqr;			/* 0x110 */
	u32 pbi_completion;		/* 0x114 */
	u8 res_118[0xa00-0x118];	/* 0x118 */
	u32 qmbm_warmrst;		/* 0xa00 */
	u32 soc_warmrst;		/* 0xa04 */
	u8 res_a08[0xbf8-0xa08];	/* 0xa08 */
	u32 ip_rev1;			/* 0xbf8 */
	u32 ip_rev2;			/* 0xbfc */
};

#endif /*__ASSEMBLY__*/
#endif /* __ARCH_FSL_LSCH3_IMMAP_H_ */
