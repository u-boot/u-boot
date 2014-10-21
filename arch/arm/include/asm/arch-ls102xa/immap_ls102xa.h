/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_LS102XA_IMMAP_H_
#define __ASM_ARCH_LS102XA_IMMAP_H_

#define SVR_MAJ(svr)		(((svr) >>  4) & 0xf)
#define SVR_MIN(svr)		(((svr) >>  0) & 0xf)
#define SVR_SOC_VER(svr)	(((svr) >> 8) & 0x7ff)
#define IS_E_PROCESSOR(svr)	(svr & 0x80000)

#define SOC_VER_SLS1020		0x00
#define SOC_VER_LS1020		0x10
#define SOC_VER_LS1021		0x11
#define SOC_VER_LS1022		0x12

#define RCWSR0_SYS_PLL_RAT_SHIFT	25
#define RCWSR0_SYS_PLL_RAT_MASK		0x1f
#define RCWSR0_MEM_PLL_RAT_SHIFT	16
#define RCWSR0_MEM_PLL_RAT_MASK		0x3f

#define RCWSR4_SRDS1_PRTCL_SHIFT	24
#define RCWSR4_SRDS1_PRTCL_MASK		0xff000000

#define TIMER_COMP_VAL			0xffffffff
#define ARCH_TIMER_CTRL_ENABLE		(1 << 0)
#define SYS_COUNTER_CTRL_ENABLE		(1 << 24)

struct sys_info {
	unsigned long freq_processor[CONFIG_MAX_CPUS];
	unsigned long freq_systembus;
	unsigned long freq_ddrbus;
	unsigned long freq_localbus;
};

/* Device Configuration and Pin Control */
struct ccsr_gur {
	u32     porsr1;         /* POR status 1 */
	u32     porsr2;         /* POR status 2 */
	u8      res_008[0x20-0x8];
	u32     gpporcr1;       /* General-purpose POR configuration */
	u32	gpporcr2;
	u32     dcfg_fusesr;    /* Fuse status register */
	u8      res_02c[0x70-0x2c];
	u32     devdisr;        /* Device disable control */
	u32     devdisr2;       /* Device disable control 2 */
	u32     devdisr3;       /* Device disable control 3 */
	u32     devdisr4;       /* Device disable control 4 */
	u32     devdisr5;       /* Device disable control 5 */
	u8      res_084[0x94-0x84];
	u32     coredisru;      /* uppper portion for support of 64 cores */
	u32     coredisrl;      /* lower portion for support of 64 cores */
	u8      res_09c[0xa4-0x9c];
	u32     svr;            /* System version */
	u8	res_0a8[0xb0-0xa8];
	u32	rstcr;		/* Reset control */
	u32	rstrqpblsr;	/* Reset request preboot loader status */
	u8	res_0b8[0xc0-0xb8];
	u32	rstrqmr1;	/* Reset request mask */
	u8	res_0c4[0xc8-0xc4];
	u32	rstrqsr1;	/* Reset request status */
	u8	res_0cc[0xd4-0xcc];
	u32	rstrqwdtmrl;	/* Reset request WDT mask */
	u8	res_0d8[0xdc-0xd8];
	u32	rstrqwdtsrl;	/* Reset request WDT status */
	u8	res_0e0[0xe4-0xe0];
	u32	brrl;		/* Boot release */
	u8      res_0e8[0x100-0xe8];
	u32     rcwsr[16];      /* Reset control word status */
	u8      res_140[0x200-0x140];
	u32     scratchrw[4];  /* Scratch Read/Write */
	u8      res_210[0x300-0x210];
	u32     scratchw1r[4];  /* Scratch Read (Write once) */
	u8      res_310[0x400-0x310];
	u32	crstsr;
	u8      res_404[0x550-0x404];
	u32	sataliodnr;
	u8	res_554[0x604-0x554];
	u32	pamubypenr;
	u32	dmacr1;
	u8      res_60c[0x740-0x60c];   /* add more registers when needed */
	u32     tp_ityp[64];    /* Topology Initiator Type Register */
	struct {
		u32     upper;
		u32     lower;
	} tp_cluster[1];        /* Core Cluster n Topology Register */
	u8	res_848[0xe60-0x848];
	u32	ddrclkdr;
	u8	res_e60[0xe68-0xe64];
	u32	ifcclkdr;
	u8	res_e68[0xe80-0xe6c];
	u32	sdhcpcr;
};

#define SCFG_SCFGREVCR_REV		0xffffffff
#define SCFG_SCFGREVCR_NOREV		0
#define SCFG_ETSECDMAMCR_LE_BD_FR	0xf8001a0f
#define SCFG_ETSECCMCR_GE2_CLK125	0x04000000
#define SCFG_PIXCLKCR_PXCKEN		0x80000000

/* Supplemental Configuration Unit */
struct ccsr_scfg {
	u32 dpslpcr;
	u32 resv0[2];
	u32 etsecclkdpslpcr;
	u32 resv1[5];
	u32 fuseovrdcr;
	u32 pixclkcr;
	u32 resv2[5];
	u32 spimsicr;
	u32 resv3[6];
	u32 pex1pmwrcr;
	u32 pex1pmrdsr;
	u32 resv4[3];
	u32 usb3prm1cr;
	u32 usb4prm2cr;
	u32 pex1rdmsgpldlsbsr;
	u32 pex1rdmsgpldmsbsr;
	u32 pex2rdmsgpldlsbsr;
	u32 pex2rdmsgpldmsbsr;
	u32 pex1rdmmsgrqsr;
	u32 pex2rdmmsgrqsr;
	u32 spimsiclrcr;
	u32 pex1mscportsr;
	u32 pex2mscportsr;
	u32 pex2pmwrcr;
	u32 resv5[24];
	u32 mac1_streamid;
	u32 mac2_streamid;
	u32 mac3_streamid;
	u32 pex1_streamid;
	u32 pex2_streamid;
	u32 dma_streamid;
	u32 sata_streamid;
	u32 usb3_streamid;
	u32 qe_streamid;
	u32 sdhc_streamid;
	u32 adma_streamid;
	u32 letechsftrstcr;
	u32 core0_sft_rst;
	u32 core1_sft_rst;
	u32 resv6[1];
	u32 usb_hi_addr;
	u32 etsecclkadjcr;
	u32 sai_clk;
	u32 resv7[1];
	u32 dcu_streamid;
	u32 usb2_streamid;
	u32 ftm_reset;
	u32 altcbar;
	u32 qspi_cfg;
	u32 pmcintecr;
	u32 pmcintlecr;
	u32 pmcintsr;
	u32 qos1;
	u32 qos2;
	u32 qos3;
	u32 cci_cfg;
	u32 resv8[1];
	u32 etsecdmamcr;
	u32 usb3prm3cr;
	u32 resv9[1];
	u32 debug_streamid;
	u32 resv10[5];
	u32 snpcnfgcr;
	u32 resv11[1];
	u32 intpcr;
	u32 resv12[20];
	u32 scfgrevcr;
	u32 coresrencr;
	u32 pex2pmrdsr;
	u32 ddrc1cr;
	u32 ddrc2cr;
	u32 ddrc3cr;
	u32 ddrc4cr;
	u32 ddrgcr;
	u32 resv13[120];
	u32 qeioclkcr;
	u32 etsecmcr;
	u32 sdhciovserlcr;
	u32 resv14[61];
	u32 sparecr;
};

/* Clocking */
struct ccsr_clk {
	struct {
		u32 clkcncsr;	/* core cluster n clock control status */
		u8  res_004[0x1c];
	} clkcsr[2];
	u8	res_040[0x7c0]; /* 0x100 */
	struct {
		u32 pllcngsr;
		u8 res_804[0x1c];
	} pllcgsr[2];
	u8	res_840[0x1c0];
	u32	clkpcsr;	/* 0xa00 Platform clock domain control/status */
	u8	res_a04[0x1fc];
	u32	pllpgsr;	/* 0xc00 Platform PLL General Status */
	u8	res_c04[0x1c];
	u32	plldgsr;	/* 0xc20 DDR PLL General Status */
	u8	res_c24[0x3dc];
};

/* System Counter */
struct sctr_regs {
	u32 cntcr;
	u32 cntsr;
	u32 cntcv1;
	u32 cntcv2;
	u32 resv1[4];
	u32 cntfid0;
	u32 cntfid1;
	u32 resv2[1002];
	u32 counterid[12];
};

#define MAX_SERDES			1
#define SRDS_MAX_LANES			4
#define SRDS_MAX_BANK			2

#define SRDS_RSTCTL_RST			0x80000000
#define SRDS_RSTCTL_RSTDONE		0x40000000
#define SRDS_RSTCTL_RSTERR		0x20000000
#define SRDS_RSTCTL_SWRST		0x10000000
#define SRDS_RSTCTL_SDEN		0x00000020
#define SRDS_RSTCTL_SDRST_B		0x00000040
#define SRDS_RSTCTL_PLLRST_B		0x00000080
#define SRDS_PLLCR0_POFF		0x80000000
#define SRDS_PLLCR0_RFCK_SEL_MASK	0x70000000
#define SRDS_PLLCR0_RFCK_SEL_100	0x00000000
#define SRDS_PLLCR0_RFCK_SEL_125	0x10000000
#define SRDS_PLLCR0_RFCK_SEL_156_25	0x20000000
#define SRDS_PLLCR0_RFCK_SEL_150	0x30000000
#define SRDS_PLLCR0_RFCK_SEL_161_13	0x40000000
#define SRDS_PLLCR0_RFCK_SEL_122_88	0x50000000
#define SRDS_PLLCR0_PLL_LCK		0x00800000
#define SRDS_PLLCR0_FRATE_SEL_MASK	0x000f0000
#define SRDS_PLLCR0_FRATE_SEL_5		0x00000000
#define SRDS_PLLCR0_FRATE_SEL_3_75	0x00050000
#define SRDS_PLLCR0_FRATE_SEL_5_15	0x00060000
#define SRDS_PLLCR0_FRATE_SEL_4		0x00070000
#define SRDS_PLLCR0_FRATE_SEL_3_12	0x00090000
#define SRDS_PLLCR0_FRATE_SEL_3		0x000a0000
#define SRDS_PLLCR1_PLL_BWSEL		0x08000000

struct ccsr_serdes {
	struct {
		u32	rstctl;	/* Reset Control Register */

		u32	pllcr0; /* PLL Control Register 0 */

		u32	pllcr1; /* PLL Control Register 1 */
		u32	res_0c;	/* 0x00c */
		u32	pllcr3;
		u32	pllcr4;
		u8	res_18[0x20-0x18];
	} bank[2];
	u8	res_40[0x90-0x40];
	u32	srdstcalcr;	/* 0x90 TX Calibration Control */
	u8	res_94[0xa0-0x94];
	u32	srdsrcalcr;	/* 0xa0 RX Calibration Control */
	u8	res_a4[0xb0-0xa4];
	u32	srdsgr0;	/* 0xb0 General Register 0 */
	u8	res_b4[0xe0-0xb4];
	u32	srdspccr0;	/* 0xe0 Protocol Converter Config 0 */
	u32	srdspccr1;	/* 0xe4 Protocol Converter Config 1 */
	u32	srdspccr2;	/* 0xe8 Protocol Converter Config 2 */
	u32	srdspccr3;	/* 0xec Protocol Converter Config 3 */
	u32	srdspccr4;	/* 0xf0 Protocol Converter Config 4 */
	u8	res_f4[0x100-0xf4];
	struct {
		u32	lnpssr;	/* 0x100, 0x120, ..., 0x1e0 */
		u8	res_104[0x120-0x104];
	} srdslnpssr[4];
	u8	res_180[0x300-0x180];
	u32	srdspexeqcr;
	u32	srdspexeqpcr[11];
	u8	res_330[0x400-0x330];
	u32	srdspexapcr;
	u8	res_404[0x440-0x404];
	u32	srdspexbpcr;
	u8	res_444[0x800-0x444];
	struct {
		u32	gcr0;	/* 0x800 General Control Register 0 */
		u32	gcr1;	/* 0x804 General Control Register 1 */
		u32	gcr2;	/* 0x808 General Control Register 2 */
		u32	sscr0;
		u32	recr0;	/* 0x810 Receive Equalization Control */
		u32	recr1;
		u32	tecr0;	/* 0x818 Transmit Equalization Control */
		u32	sscr1;
		u32	ttlcr0;	/* 0x820 Transition Tracking Loop Ctrl 0 */
		u8	res_824[0x83c-0x824];
		u32	tcsr3;
	} lane[4];	/* Lane A, B, C, D, E, F, G, H */
	u8	res_a00[0x1000-0xa00];	/* from 0xa00 to 0xfff */
};

#define DDR_SDRAM_CFG			0x470c0008
#define DDR_CS0_BNDS			0x008000bf
#define DDR_CS0_CONFIG			0x80014302
#define DDR_TIMING_CFG_0		0x50550004
#define DDR_TIMING_CFG_1		0xbcb38c56
#define DDR_TIMING_CFG_2		0x0040d120
#define DDR_TIMING_CFG_3		0x010e1000
#define DDR_TIMING_CFG_4		0x00000001
#define DDR_TIMING_CFG_5		0x03401400
#define DDR_SDRAM_CFG_2			0x00401010
#define DDR_SDRAM_MODE			0x00061c60
#define DDR_SDRAM_MODE_2		0x00180000
#define DDR_SDRAM_INTERVAL		0x18600618
#define DDR_DDR_WRLVL_CNTL		0x8655f605
#define DDR_DDR_WRLVL_CNTL_2		0x05060607
#define DDR_DDR_WRLVL_CNTL_3		0x05050505
#define DDR_DDR_CDR1			0x80040000
#define DDR_DDR_CDR2			0x00000001
#define DDR_SDRAM_CLK_CNTL		0x02000000
#define DDR_DDR_ZQ_CNTL			0x89080600
#define DDR_CS0_CONFIG_2		0
#define DDR_SDRAM_CFG_MEM_EN		0x80000000

/* DDR memory controller registers */
struct ccsr_ddr {
	u32 cs0_bnds;			/* Chip Select 0 Memory Bounds */
	u32 resv1[1];
	u32 cs1_bnds;			/* Chip Select 1 Memory Bounds */
	u32 resv2[1];
	u32 cs2_bnds;			/* Chip Select 2 Memory Bounds */
	u32 resv3[1];
	u32 cs3_bnds;			/* Chip Select 3 Memory Bounds */
	u32 resv4[25];
	u32 cs0_config;			/* Chip Select Configuration */
	u32 cs1_config;			/* Chip Select Configuration */
	u32 cs2_config;			/* Chip Select Configuration */
	u32 cs3_config;			/* Chip Select Configuration */
	u32 resv5[12];
	u32 cs0_config_2;		/* Chip Select Configuration 2 */
	u32 cs1_config_2;		/* Chip Select Configuration 2 */
	u32 cs2_config_2;		/* Chip Select Configuration 2 */
	u32 cs3_config_2;		/* Chip Select Configuration 2 */
	u32 resv6[12];
	u32 timing_cfg_3;		/* SDRAM Timing Configuration 3 */
	u32 timing_cfg_0;		/* SDRAM Timing Configuration 0 */
	u32 timing_cfg_1;		/* SDRAM Timing Configuration 1 */
	u32 timing_cfg_2;		/* SDRAM Timing Configuration 2 */
	u32 sdram_cfg;			/* SDRAM Control Configuration */
	u32 sdram_cfg_2;		/* SDRAM Control Configuration 2 */
	u32 sdram_mode;			/* SDRAM Mode Configuration */
	u32 sdram_mode_2;		/* SDRAM Mode Configuration 2 */
	u32 sdram_md_cntl;		/* SDRAM Mode Control */
	u32 sdram_interval;		/* SDRAM Interval Configuration */
	u32 sdram_data_init;		/* SDRAM Data initialization */
	u32 resv7[1];
	u32 sdram_clk_cntl;		/* SDRAM Clock Control */
	u32 resv8[5];
	u32 init_addr;			/* training init addr */
	u32 init_ext_addr;		/* training init extended addr */
	u32 resv9[4];
	u32 timing_cfg_4;		/* SDRAM Timing Configuration 4 */
	u32 timing_cfg_5;		/* SDRAM Timing Configuration 5 */
	u32 timing_cfg_6;		/* SDRAM Timing Configuration 6 */
	u32 timing_cfg_7;		/* SDRAM Timing Configuration 7 */
	u32 ddr_zq_cntl;		/* ZQ calibration control*/
	u32 ddr_wrlvl_cntl;		/* write leveling control*/
	u32 resv10[1];
	u32 ddr_sr_cntr;		/* self refresvh counter */
	u32 ddr_sdram_rcw_1;		/* Control Words 1 */
	u32 ddr_sdram_rcw_2;		/* Control Words 2 */
	u32 resv11[2];
	u32 ddr_wrlvl_cntl_2;		/* write leveling control 2 */
	u32 ddr_wrlvl_cntl_3;		/* write leveling control 3 */
	u32 resv12[2];
	u32 ddr_sdram_rcw_3;		/* Control Words 3 */
	u32 ddr_sdram_rcw_4;		/* Control Words 4 */
	u32 ddr_sdram_rcw_5;		/* Control Words 5 */
	u32 ddr_sdram_rcw_6;		/* Control Words 6 */
	u32 resv13[20];
	u32 sdram_mode_3;		/* SDRAM Mode Configuration 3 */
	u32 sdram_mode_4;		/* SDRAM Mode Configuration 4 */
	u32 sdram_mode_5;		/* SDRAM Mode Configuration 5 */
	u32 sdram_mode_6;		/* SDRAM Mode Configuration 6 */
	u32 sdram_mode_7;		/* SDRAM Mode Configuration 7 */
	u32 sdram_mode_8;		/* SDRAM Mode Configuration 8 */
	u32 sdram_mode_9;		/* SDRAM Mode Configuration 9 */
	u32 sdram_mode_10;		/* SDRAM Mode Configuration 10 */
	u32 sdram_mode_11;		/* SDRAM Mode Configuration 11 */
	u32 sdram_mode_12;		/* SDRAM Mode Configuration 12 */
	u32 sdram_mode_13;		/* SDRAM Mode Configuration 13 */
	u32 sdram_mode_14;		/* SDRAM Mode Configuration 14 */
	u32 sdram_mode_15;		/* SDRAM Mode Configuration 15 */
	u32 sdram_mode_16;		/* SDRAM Mode Configuration 16 */
	u32 resv14[4];
	u32 timing_cfg_8;		/* SDRAM Timing Configuration 8 */
	u32 timing_cfg_9;		/* SDRAM Timing Configuration 9 */
	u32 resv15[2];
	u32 sdram_cfg_3;		/* SDRAM Control Configuration 3 */
	u32 resv16[15];
	u32 deskew_cntl;		/* SDRAM Deskew Control */
	u32 resv17[545];
	u32 ddr_dsr1;			/* Debug Status 1 */
	u32 ddr_dsr2;			/* Debug Status 2 */
	u32 ddr_cdr1;			/* Control Driver 1 */
	u32 ddr_cdr2;			/* Control Driver 2 */
	u32 resv18[50];
	u32 ip_rev1;			/* IP Block Revision 1 */
	u32 ip_rev2;			/* IP Block Revision 2 */
	u32 eor;			/* Enhanced Optimization Register */
	u32 resv19[63];
	u32 mtcr;			/* Memory Test Control Register */
	u32 resv20[7];
	u32 mtp1;			/* Memory Test Pattern 1 */
	u32 mtp2;			/* Memory Test Pattern 2 */
	u32 mtp3;			/* Memory Test Pattern 3 */
	u32 mtp4;			/* Memory Test Pattern 4 */
	u32 mtp5;			/* Memory Test Pattern 5 */
	u32 mtp6;			/* Memory Test Pattern 6 */
	u32 mtp7;			/* Memory Test Pattern 7 */
	u32 mtp8;			/* Memory Test Pattern 8 */
	u32 mtp9;			/* Memory Test Pattern 9 */
	u32 mtp10;			/* Memory Test Pattern 10 */
	u32 resv21[6];
	u32 ddr_mt_st_ext_addr;		/* Memory Test Start Extended Address */
	u32 ddr_mt_st_addr;		/* Memory Test Start Address */
	u32 ddr_mt_end_ext_addr;	/* Memory Test End Extended Address */
	u32 ddr_mt_end_addr;		/* Memory Test End Address */
	u32 resv22[36];
	u32 data_err_inject_hi;		/* Data Path Err Injection Mask High */
	u32 data_err_inject_lo;		/* Data Path Err Injection Mask Low */
	u32 ecc_err_inject;		/* Data Path Err Injection Mask ECC */
	u32 resv23[5];
	u32 capture_data_hi;		/* Data Path Read Capture High */
	u32 capture_data_lo;		/* Data Path Read Capture Low */
	u32 capture_ecc;		/* Data Path Read Capture ECC */
	u32 resv24[5];
	u32 err_detect;			/* Error Detect */
	u32 err_disable;		/* Error Disable */
	u32 err_int_en;
	u32 capture_attributes;		/* Error Attrs Capture */
	u32 capture_address;		/* Error Addr Capture */
	u32 capture_ext_address;	/* Error Extended Addr Capture */
	u32 err_sbe;			/* Single-Bit ECC Error Management */
	u32 resv25[105];
};

#define CCI400_CTRLORD_TERM_BARRIER	0x00000008
#define CCI400_CTRLORD_EN_BARRIER	0

/* CCI-400 registers */
struct ccsr_cci400 {
	u32 ctrl_ord;			/* Control Override */
	u32 spec_ctrl;			/* Speculation Control */
	u32 secure_access;		/* Secure Access */
	u32 status;			/* Status */
	u32 impr_err;			/* Imprecise Error */
	u8 res_14[0x100 - 0x14];
	u32 pmcr;			/* Performance Monitor Control */
	u8 res_104[0xfd0 - 0x104];
	u32 pid[8];			/* Peripheral ID */
	u32 cid[4];			/* Component ID */
	struct {
		u32 snoop_ctrl;		/* Snoop Control */
		u32 sha_ord;		/* Shareable Override */
		u8 res_1008[0x1100 - 0x1008];
		u32 rc_qos_ord;		/* read channel QoS Value Override */
		u32 wc_qos_ord;		/* read channel QoS Value Override */
		u8 res_1108[0x110c - 0x1108];
		u32 qos_ctrl;		/* QoS Control */
		u32 max_ot;		/* Max OT */
		u8 res_1114[0x1130 - 0x1114];
		u32 target_lat;		/* Target Latency */
		u32 latency_regu;	/* Latency Regulation */
		u32 qos_range;		/* QoS Range */
		u8 res_113c[0x2000 - 0x113c];
	} slave[5];			/* Slave Interface */
	u8 res_6000[0x9004 - 0x6000];
	u32 cycle_counter;		/* Cycle counter */
	u32 count_ctrl;			/* Count Control */
	u32 overflow_status;		/* Overflow Flag Status */
	u8 res_9010[0xa000 - 0x9010];
	struct {
		u32 event_select;	/* Event Select */
		u32 event_count;	/* Event Count */
		u32 counter_ctrl;	/* Counter Control */
		u32 overflow_status;	/* Overflow Flag Status */
		u8 res_a010[0xb000 - 0xa010];
	} pcounter[4];			/* Performance Counter */
	u8 res_e004[0x10000 - 0xe004];
};
#endif	/* __ASM_ARCH_LS102XA_IMMAP_H_ */
