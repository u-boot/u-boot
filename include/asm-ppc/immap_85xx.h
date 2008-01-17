/*
 * MPC85xx Internal Memory Map
 *
 * Copyright 2007 Freescale Semiconductor.
 *
 * Copyright(c) 2002,2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
 *
 */

#ifndef __IMMAP_85xx__
#define __IMMAP_85xx__

#include <asm/types.h>
#include <asm/fsl_i2c.h>

/*
 * Local-Access Registers and ECM Registers(0x0000-0x2000)
 */
typedef struct ccsr_local_ecm {
	uint	ccsrbar;	/* 0x0 - Control Configuration Status Registers Base Address Register */
	char	res1[4];
	uint	altcbar;	/* 0x8 - Alternate Configuration Base Address Register */
	char	res2[4];
	uint	altcar;		/* 0x10 - Alternate Configuration Attribute Register */
	char	res3[12];
	uint	bptr;		/* 0x20 - Boot Page Translation Register */
	char	res4[3044];
	uint	lawbar0;	/* 0xc08 - Local Access Window 0 Base Address Register */
	char	res5[4];
	uint	lawar0;		/* 0xc10 - Local Access Window 0 Attributes Register */
	char	res6[20];
	uint	lawbar1;	/* 0xc28 - Local Access Window 1 Base Address Register */
	char	res7[4];
	uint	lawar1;		/* 0xc30 - Local Access Window 1 Attributes Register */
	char	res8[20];
	uint	lawbar2;	/* 0xc48 - Local Access Window 2 Base Address Register */
	char	res9[4];
	uint	lawar2;		/* 0xc50 - Local Access Window 2 Attributes Register */
	char	res10[20];
	uint	lawbar3;	/* 0xc68 - Local Access Window 3 Base Address Register */
	char	res11[4];
	uint	lawar3;		/* 0xc70 - Local Access Window 3 Attributes Register */
	char	res12[20];
	uint	lawbar4;	/* 0xc88 - Local Access Window 4 Base Address Register */
	char	res13[4];
	uint	lawar4;		/* 0xc90 - Local Access Window 4 Attributes Register */
	char	res14[20];
	uint	lawbar5;	/* 0xca8 - Local Access Window 5 Base Address Register */
	char	res15[4];
	uint	lawar5;		/* 0xcb0 - Local Access Window 5 Attributes Register */
	char	res16[20];
	uint	lawbar6;	/* 0xcc8 - Local Access Window 6 Base Address Register */
	char	res17[4];
	uint	lawar6;		/* 0xcd0 - Local Access Window 6 Attributes Register */
	char	res18[20];
	uint	lawbar7;	/* 0xce8 - Local Access Window 7 Base Address Register */
	char	res19[4];
	uint	lawar7;		/* 0xcf0 - Local Access Window 7 Attributes Register */
	char	res20[780];
	uint	eebacr;		/* 0x1000 - ECM CCB Address Configuration Register */
	char	res21[12];
	uint	eebpcr;		/* 0x1010 - ECM CCB Port Configuration Register */
	char	res22[3564];
	uint	eedr;		/* 0x1e00 - ECM Error Detect Register */
	char	res23[4];
	uint	eeer;		/* 0x1e08 - ECM Error Enable Register */
	uint	eeatr;		/* 0x1e0c - ECM Error Attributes Capture Register */
	uint	eeadr;		/* 0x1e10 - ECM Error Address Capture Register */
	char	res24[492];
} ccsr_local_ecm_t;

/*
 * DDR memory controller registers(0x2000-0x3000)
 */
typedef struct ccsr_ddr {
	uint	cs0_bnds;		/* 0x2000 - DDR Chip Select 0 Memory Bounds */
	char	res1[4];
	uint	cs1_bnds;		/* 0x2008 - DDR Chip Select 1 Memory Bounds */
	char	res2[4];
	uint	cs2_bnds;		/* 0x2010 - DDR Chip Select 2 Memory Bounds */
	char	res3[4];
	uint	cs3_bnds;		/* 0x2018 - DDR Chip Select 3 Memory Bounds */
	char	res4[100];
	uint	cs0_config;		/* 0x2080 - DDR Chip Select Configuration */
	uint	cs1_config;		/* 0x2084 - DDR Chip Select Configuration */
	uint	cs2_config;		/* 0x2088 - DDR Chip Select Configuration */
	uint	cs3_config;		/* 0x208c - DDR Chip Select Configuration */
	char	res5[112];
	uint	ext_refrec;		/* 0x2100 - DDR SDRAM Extended Refresh Recovery */
	uint	timing_cfg_0;		/* 0x2104 - DDR SDRAM Timing Configuration Register 0 */
	uint	timing_cfg_1;		/* 0x2108 - DDR SDRAM Timing Configuration Register 1 */
	uint	timing_cfg_2;		/* 0x210c - DDR SDRAM Timing Configuration Register 2 */
	uint	sdram_cfg;		/* 0x2110 - DDR SDRAM Control Configuration */
	uint	sdram_cfg_2;		/* 0x2114 - DDR SDRAM Control Configuration 2 */
	uint	sdram_mode;		/* 0x2118 - DDR SDRAM Mode Configuration */
	uint	sdram_mode_2;		/* 0x211c - DDR SDRAM Mode Configuration 2*/
	uint	sdram_md_cntl;		/* 0x2120 - DDR SDRAM Mode Control */
	uint	sdram_interval;		/* 0x2124 - DDR SDRAM Interval Configuration */
	uint	sdram_data_init;	/* 0x2128 - DDR SDRAM Data initialization */
	char	res6[4];
	uint	sdram_clk_cntl;		/* 0x2130 - DDR SDRAM Clock Control */
	char	res7[20];
	uint	init_address;		/* 0x2148 - DDR training initialization address */
	uint	init_ext_address;	/* 0x214C - DDR training initialization extended address */
	char	res8_1[2728];
	uint	ip_rev1;		/* 0x2BF8 - DDR IP Block Revision 1 */
	uint	ip_rev2;		/* 0x2BFC - DDR IP Block Revision 2 */
	char	res8_2[512];
	uint	data_err_inject_hi;	/* 0x2e00 - DDR Memory Data Path Error Injection Mask High */
	uint	data_err_inject_lo;	/* 0x2e04 - DDR Memory Data Path Error Injection Mask Low */
	uint	ecc_err_inject;		/* 0x2e08 - DDR Memory Data Path Error Injection Mask ECC */
	char	res9[20];
	uint	capture_data_hi;	/* 0x2e20 - DDR Memory Data Path Read Capture High */
	uint	capture_data_lo;	/* 0x2e24 - DDR Memory Data Path Read Capture Low */
	uint	capture_ecc;		/* 0x2e28 - DDR Memory Data Path Read Capture ECC */
	char	res10[20];
	uint	err_detect;		/* 0x2e40 - DDR Memory Error Detect */
	uint	err_disable;		/* 0x2e44 - DDR Memory Error Disable */
	uint	err_int_en;		/* 0x2e48 - DDR  */
	uint	capture_attributes;	/* 0x2e4c - DDR Memory Error Attributes Capture */
	uint	capture_address;	/* 0x2e50 - DDR Memory Error Address Capture */
	uint	capture_ext_address;	/* 0x2e54 - DDR Memory Error Extended Address Capture */
	uint	err_sbe;		/* 0x2e58 - DDR Memory Single-Bit ECC Error Management */
	char	res11[164];
	uint	debug_1;		/* 0x2f00 */
	uint	debug_2;
	uint	debug_3;
	uint	debug_4;
	char	res12[240];
} ccsr_ddr_t;

/*
 * I2C Registers(0x3000-0x4000)
 */
typedef struct ccsr_i2c {
	struct fsl_i2c	i2c[1];
	u8	res[4096 - 1 * sizeof(struct fsl_i2c)];
} ccsr_i2c_t;

#if defined(CONFIG_MPC8540) \
	|| defined(CONFIG_MPC8541) \
	|| defined(CONFIG_MPC8548) \
	|| defined(CONFIG_MPC8555)
/* DUART Registers(0x4000-0x5000) */
typedef struct ccsr_duart {
	char	res1[1280];
	u_char	urbr1_uthr1_udlb1;/* 0x4500 - URBR1, UTHR1, UDLB1 with the same address offset of 0x04500 */
	u_char	uier1_udmb1;	/* 0x4501 - UIER1, UDMB1 with the same address offset of 0x04501 */
	u_char	uiir1_ufcr1_uafr1;/* 0x4502 - UIIR1, UFCR1, UAFR1 with the same address offset of 0x04502 */
	u_char	ulcr1;		/* 0x4503 - UART1 Line Control Register */
	u_char	umcr1;		/* 0x4504 - UART1 Modem Control Register */
	u_char	ulsr1;		/* 0x4505 - UART1 Line Status Register */
	u_char	umsr1;		/* 0x4506 - UART1 Modem Status Register */
	u_char	uscr1;		/* 0x4507 - UART1 Scratch Register */
	char	res2[8];
	u_char	udsr1;		/* 0x4510 - UART1 DMA Status Register */
	char	res3[239];
	u_char	urbr2_uthr2_udlb2;/* 0x4600 - URBR2, UTHR2, UDLB2 with the same address offset of 0x04600 */
	u_char	uier2_udmb2;	/* 0x4601 - UIER2, UDMB2 with the same address offset of 0x04601 */
	u_char	uiir2_ufcr2_uafr2;/* 0x4602 - UIIR2, UFCR2, UAFR2 with the same address offset of 0x04602 */
	u_char	ulcr2;		/* 0x4603 - UART2 Line Control Register */
	u_char	umcr2;		/* 0x4604 - UART2 Modem Control Register */
	u_char	ulsr2;		/* 0x4605 - UART2 Line Status Register */
	u_char	umsr2;		/* 0x4606 - UART2 Modem Status Register */
	u_char	uscr2;		/* 0x4607 - UART2 Scratch Register */
	char	res4[8];
	u_char	udsr2;		/* 0x4610 - UART2 DMA Status Register */
	char	res5[2543];
} ccsr_duart_t;
#else /* MPC8560 uses UART on its CPM */
typedef struct ccsr_duart {
	char res[4096];
} ccsr_duart_t;
#endif

/* Local Bus Controller Registers(0x5000-0x6000) */
/* Omitting OCeaN(0x6000) and Reserved(0x7000) block */

typedef struct ccsr_lbc {
	uint	br0;		/* 0x5000 - LBC Base Register 0 */
	uint	or0;		/* 0x5004 - LBC Options Register 0 */
	uint	br1;		/* 0x5008 - LBC Base Register 1 */
	uint	or1;		/* 0x500c - LBC Options Register 1 */
	uint	br2;		/* 0x5010 - LBC Base Register 2 */
	uint	or2;		/* 0x5014 - LBC Options Register 2 */
	uint	br3;		/* 0x5018 - LBC Base Register 3 */
	uint	or3;		/* 0x501c - LBC Options Register 3 */
	uint	br4;		/* 0x5020 - LBC Base Register 4 */
	uint	or4;		/* 0x5024 - LBC Options Register 4 */
	uint	br5;		/* 0x5028 - LBC Base Register 5 */
	uint	or5;		/* 0x502c - LBC Options Register 5 */
	uint	br6;		/* 0x5030 - LBC Base Register 6 */
	uint	or6;		/* 0x5034 - LBC Options Register 6 */
	uint	br7;		/* 0x5038 - LBC Base Register 7 */
	uint	or7;		/* 0x503c - LBC Options Register 7 */
	char	res1[40];
	uint	mar;		/* 0x5068 - LBC UPM Address Register */
	char	res2[4];
	uint	mamr;		/* 0x5070 - LBC UPMA Mode Register */
	uint	mbmr;		/* 0x5074 - LBC UPMB Mode Register */
	uint	mcmr;		/* 0x5078 - LBC UPMC Mode Register */
	char	res3[8];
	uint	mrtpr;		/* 0x5084 - LBC Memory Refresh Timer Prescaler Register */
	uint	mdr;		/* 0x5088 - LBC UPM Data Register */
	char	res4[8];
	uint	lsdmr;		/* 0x5094 - LBC SDRAM Mode Register */
	char	res5[8];
	uint	lurt;		/* 0x50a0 - LBC UPM Refresh Timer */
	uint	lsrt;		/* 0x50a4 - LBC SDRAM Refresh Timer */
	char	res6[8];
	uint	ltesr;		/* 0x50b0 - LBC Transfer Error Status Register */
	uint	ltedr;		/* 0x50b4 - LBC Transfer Error Disable Register */
	uint	lteir;		/* 0x50b8 - LBC Transfer Error Interrupt Register */
	uint	lteatr;		/* 0x50bc - LBC Transfer Error Attributes Register */
	uint	ltear;		/* 0x50c0 - LBC Transfer Error Address Register */
	char	res7[12];
	uint	lbcr;		/* 0x50d0 - LBC Configuration Register */
	uint	lcrr;		/* 0x50d4 - LBC Clock Ratio Register */
	char	res8[12072];
} ccsr_lbc_t;

/*
 * PCI Registers(0x8000-0x9000)
 */
typedef struct ccsr_pcix {
	uint	cfg_addr;	/* 0x8000 - PCIX Configuration Address Register */
	uint	cfg_data;	/* 0x8004 - PCIX Configuration Data Register */
	uint	int_ack;	/* 0x8008 - PCIX Interrupt Acknowledge Register */
	char	res1[3060];
	uint	potar0;		/* 0x8c00 - PCIX Outbound Transaction Address Register 0 */
	uint	potear0;	/* 0x8c04 - PCIX Outbound Translation Extended Address Register 0 */
	uint	powbar0;	/* 0x8c08 - PCIX Outbound Window Base Address Register 0 */
	uint	powbear0;	/* 0x8c0c - PCIX Outbound Window Base Extended Address Register 0 */
	uint	powar0;		/* 0x8c10 - PCIX Outbound Window Attributes Register 0 */
	char	res2[12];
	uint	potar1;		/* 0x8c20 - PCIX Outbound Transaction Address Register 1 */
	uint	potear1;	/* 0x8c24 - PCIX Outbound Translation Extended Address Register 1 */
	uint	powbar1;	/* 0x8c28 - PCIX Outbound Window Base Address Register 1 */
	uint	powbear1;	/* 0x8c2c - PCIX Outbound Window Base Extended Address Register 1 */
	uint	powar1;		/* 0x8c30 - PCIX Outbound Window Attributes Register 1 */
	char	res3[12];
	uint	potar2;		/* 0x8c40 - PCIX Outbound Transaction Address Register 2 */
	uint	potear2;	/* 0x8c44 - PCIX Outbound Translation Extended Address Register 2 */
	uint	powbar2;	/* 0x8c48 - PCIX Outbound Window Base Address Register 2 */
	uint	powbear2;	/* 0x8c4c - PCIX Outbound Window Base Extended Address Register 2 */
	uint	powar2;		/* 0x8c50 - PCIX Outbound Window Attributes Register 2 */
	char	res4[12];
	uint	potar3;		/* 0x8c60 - PCIX Outbound Transaction Address Register 3 */
	uint	potear3;	/* 0x8c64 - PCIX Outbound Translation Extended Address Register 3 */
	uint	powbar3;	/* 0x8c68 - PCIX Outbound Window Base Address Register 3 */
	uint	powbear3;	/* 0x8c6c - PCIX Outbound Window Base Extended Address Register 3 */
	uint	powar3;		/* 0x8c70 - PCIX Outbound Window Attributes Register 3 */
	char	res5[12];
	uint	potar4;		/* 0x8c80 - PCIX Outbound Transaction Address Register 4 */
	uint	potear4;	/* 0x8c84 - PCIX Outbound Translation Extended Address Register 4 */
	uint	powbar4;	/* 0x8c88 - PCIX Outbound Window Base Address Register 4 */
	uint	powbear4;	/* 0x8c8c - PCIX Outbound Window Base Extended Address Register 4 */
	uint	powar4;		/* 0x8c90 - PCIX Outbound Window Attributes Register 4 */
	char	res6[268];
	uint	pitar3;		/* 0x8da0 - PCIX Inbound Translation Address Register 3  */
	uint	pitear3;	/* 0x8da4 - PCIX Inbound Translation Extended Address Register 3 */
	uint	piwbar3;	/* 0x8da8 - PCIX Inbound Window Base Address Register 3 */
	uint	piwbear3;	/* 0x8dac - PCIX Inbound Window Base Extended Address Register 3 */
	uint	piwar3;		/* 0x8db0 - PCIX Inbound Window Attributes Register 3 */
	char	res7[12];
	uint	pitar2;		/* 0x8dc0 - PCIX Inbound Translation Address Register 2  */
	uint	pitear2;	/* 0x8dc4 - PCIX Inbound Translation Extended Address Register 2 */
	uint	piwbar2;	/* 0x8dc8 - PCIX Inbound Window Base Address Register 2 */
	uint	piwbear2;	/* 0x8dcc - PCIX Inbound Window Base Extended Address Register 2 */
	uint	piwar2;		/* 0x8dd0 - PCIX Inbound Window Attributes Register 2 */
	char	res8[12];
	uint	pitar1;		/* 0x8de0 - PCIX Inbound Translation Address Register 1  */
	uint	pitear1;	/* 0x8de4 - PCIX Inbound Translation Extended Address Register 1 */
	uint	piwbar1;	/* 0x8de8 - PCIX Inbound Window Base Address Register 1 */
	char	res9[4];
	uint	piwar1;		/* 0x8df0 - PCIX Inbound Window Attributes Register 1 */
	char	res10[12];
	uint	pedr;		/* 0x8e00 - PCIX Error Detect Register */
	uint	pecdr;		/* 0x8e04 - PCIX Error Capture Disable Register */
	uint	peer;		/* 0x8e08 - PCIX Error Enable Register */
	uint	peattrcr;	/* 0x8e0c - PCIX Error Attributes Capture Register */
	uint	peaddrcr;	/* 0x8e10 - PCIX Error Address Capture Register */
	uint	peextaddrcr;	/* 0x8e14 - PCIX  Error Extended Address Capture Register */
	uint	pedlcr;		/* 0x8e18 - PCIX Error Data Low Capture Register */
	uint	pedhcr;		/* 0x8e1c - PCIX Error Error Data High Capture Register */
	uint	gas_timr;	/* 0x8e20 - PCIX Gasket Timer Register */
	char	res11[476];
} ccsr_pcix_t;

#define PCIX_COMMAND	0x62
#define POWAR_EN	0x80000000
#define POWAR_IO_READ	0x00080000
#define POWAR_MEM_READ	0x00040000
#define POWAR_IO_WRITE	0x00008000
#define POWAR_MEM_WRITE	0x00004000
#define POWAR_MEM_512M	0x0000001c
#define POWAR_IO_1M	0x00000013

#define PIWAR_EN	0x80000000
#define PIWAR_PF	0x20000000
#define PIWAR_LOCAL	0x00f00000
#define PIWAR_READ_SNOOP	0x00050000
#define PIWAR_WRITE_SNOOP	0x00005000
#define PIWAR_MEM_2G		0x0000001e


/*
 * L2 Cache Registers(0x2_0000-0x2_1000)
 */
typedef struct ccsr_l2cache {
	uint	l2ctl;		/* 0x20000 - L2 configuration register 0 */
	char	res1[12];
	uint	l2cewar0;	/* 0x20010 - L2 cache external write address register 0 */
	char	res2[4];
	uint	l2cewcr0;	/* 0x20018 - L2 cache external write control register 0 */
	char	res3[4];
	uint	l2cewar1;	/* 0x20020 - L2 cache external write address register 1 */
	char	res4[4];
	uint	l2cewcr1;	/* 0x20028 - L2 cache external write control register 1 */
	char	res5[4];
	uint	l2cewar2;	/* 0x20030 - L2 cache external write address register 2 */
	char	res6[4];
	uint	l2cewcr2;	/* 0x20038 - L2 cache external write control register 2 */
	char	res7[4];
	uint	l2cewar3;	/* 0x20040 - L2 cache external write address register 3 */
	char	res8[4];
	uint	l2cewcr3;	/* 0x20048 - L2 cache external write control register 3 */
	char	res9[180];
	uint	l2srbar0;	/* 0x20100 - L2 memory-mapped SRAM base address register 0 */
	char	res10[4];
	uint	l2srbar1;	/* 0x20108 - L2 memory-mapped SRAM base address register 1 */
	char	res11[3316];
	uint	l2errinjhi;	/* 0x20e00 - L2 error injection mask high register */
	uint	l2errinjlo;	/* 0x20e04 - L2 error injection mask low register */
	uint	l2errinjctl;	/* 0x20e08 - L2 error injection tag/ECC control register */
	char	res12[20];
	uint	l2captdatahi;	/* 0x20e20 - L2 error data high capture register */
	uint	l2captdatalo;	/* 0x20e24 - L2 error data low capture register */
	uint	l2captecc;	/* 0x20e28 - L2 error ECC capture register */
	char	res13[20];
	uint	l2errdet;	/* 0x20e40 - L2 error detect register */
	uint	l2errdis;	/* 0x20e44 - L2 error disable register */
	uint	l2errinten;	/* 0x20e48 - L2 error interrupt enable register */
	uint	l2errattr;	/* 0x20e4c - L2 error attributes capture register */
	uint	l2erraddr;	/* 0x20e50 - L2 error address capture register */
	char	res14[4];
	uint	l2errctl;	/* 0x20e58 - L2 error control register */
	char	res15[420];
} ccsr_l2cache_t;

/*
 * DMA Registers(0x2_1000-0x2_2000)
 */
typedef struct ccsr_dma {
	char	res1[256];
	uint	mr0;		/* 0x21100 - DMA 0 Mode Register */
	uint	sr0;		/* 0x21104 - DMA 0 Status Register */
	char	res2[4];
	uint	clndar0;	/* 0x2110c - DMA 0 Current Link Descriptor Address Register */
	uint	satr0;		/* 0x21110 - DMA 0 Source Attributes Register */
	uint	sar0;		/* 0x21114 - DMA 0 Source Address Register */
	uint	datr0;		/* 0x21118 - DMA 0 Destination Attributes Register */
	uint	dar0;		/* 0x2111c - DMA 0 Destination Address Register */
	uint	bcr0;		/* 0x21120 - DMA 0 Byte Count Register */
	char	res3[4];
	uint	nlndar0;	/* 0x21128 - DMA 0 Next Link Descriptor Address Register */
	char	res4[8];
	uint	clabdar0;	/* 0x21134 - DMA 0 Current List - Alternate Base Descriptor Address Register */
	char	res5[4];
	uint	nlsdar0;	/* 0x2113c - DMA 0 Next List Descriptor Address Register */
	uint	ssr0;		/* 0x21140 - DMA 0 Source Stride Register */
	uint	dsr0;		/* 0x21144 - DMA 0 Destination Stride Register */
	char	res6[56];
	uint	mr1;		/* 0x21180 - DMA 1 Mode Register */
	uint	sr1;		/* 0x21184 - DMA 1 Status Register */
	char	res7[4];
	uint	clndar1;	/* 0x2118c - DMA 1 Current Link Descriptor Address Register */
	uint	satr1;		/* 0x21190 - DMA 1 Source Attributes Register */
	uint	sar1;		/* 0x21194 - DMA 1 Source Address Register */
	uint	datr1;		/* 0x21198 - DMA 1 Destination Attributes Register */
	uint	dar1;		/* 0x2119c - DMA 1 Destination Address Register */
	uint	bcr1;		/* 0x211a0 - DMA 1 Byte Count Register */
	char	res8[4];
	uint	nlndar1;	/* 0x211a8 - DMA 1 Next Link Descriptor Address Register */
	char	res9[8];
	uint	clabdar1;	/* 0x211b4 - DMA 1 Current List - Alternate Base Descriptor Address Register */
	char	res10[4];
	uint	nlsdar1;	/* 0x211bc - DMA 1 Next List Descriptor Address Register */
	uint	ssr1;		/* 0x211c0 - DMA 1 Source Stride Register */
	uint	dsr1;		/* 0x211c4 - DMA 1 Destination Stride Register */
	char	res11[56];
	uint	mr2;		/* 0x21200 - DMA 2 Mode Register */
	uint	sr2;		/* 0x21204 - DMA 2 Status Register */
	char	res12[4];
	uint	clndar2;	/* 0x2120c - DMA 2 Current Link Descriptor Address Register */
	uint	satr2;		/* 0x21210 - DMA 2 Source Attributes Register */
	uint	sar2;		/* 0x21214 - DMA 2 Source Address Register */
	uint	datr2;		/* 0x21218 - DMA 2 Destination Attributes Register */
	uint	dar2;		/* 0x2121c - DMA 2 Destination Address Register */
	uint	bcr2;		/* 0x21220 - DMA 2 Byte Count Register */
	char	res13[4];
	uint	nlndar2;	/* 0x21228 - DMA 2 Next Link Descriptor Address Register */
	char	res14[8];
	uint	clabdar2;	/* 0x21234 - DMA 2 Current List - Alternate Base Descriptor Address Register */
	char	res15[4];
	uint	nlsdar2;	/* 0x2123c - DMA 2 Next List Descriptor Address Register */
	uint	ssr2;		/* 0x21240 - DMA 2 Source Stride Register */
	uint	dsr2;		/* 0x21244 - DMA 2 Destination Stride Register */
	char	res16[56];
	uint	mr3;		/* 0x21280 - DMA 3 Mode Register */
	uint	sr3;		/* 0x21284 - DMA 3 Status Register */
	char	res17[4];
	uint	clndar3;	/* 0x2128c - DMA 3 Current Link Descriptor Address Register */
	uint	satr3;		/* 0x21290 - DMA 3 Source Attributes Register */
	uint	sar3;		/* 0x21294 - DMA 3 Source Address Register */
	uint	datr3;		/* 0x21298 - DMA 3 Destination Attributes Register */
	uint	dar3;		/* 0x2129c - DMA 3 Destination Address Register */
	uint	bcr3;		/* 0x212a0 - DMA 3 Byte Count Register */
	char	res18[4];
	uint	nlndar3;	/* 0x212a8 - DMA 3 Next Link Descriptor Address Register */
	char	res19[8];
	uint	clabdar3;	/* 0x212b4 - DMA 3 Current List - Alternate Base Descriptor Address Register */
	char	res20[4];
	uint	nlsdar3;	/* 0x212bc - DMA 3 Next List Descriptor Address Register */
	uint	ssr3;		/* 0x212c0 - DMA 3 Source Stride Register */
	uint	dsr3;		/* 0x212c4 - DMA 3 Destination Stride Register */
	char	res21[56];
	uint	dgsr;		/* 0x21300 - DMA General Status Register */
	char	res22[11516];
} ccsr_dma_t;

/*
 * tsec1 tsec2: 24000-26000
 */
typedef struct ccsr_tsec {
	char	res1[16];
	uint	ievent;		/* 0x24010 - Interrupt Event Register */
	uint	imask;		/* 0x24014 - Interrupt Mask Register */
	uint	edis;		/* 0x24018 - Error Disabled Register */
	char	res2[4];
	uint	ecntrl;		/* 0x24020 - Ethernet Control Register */
	uint	minflr;		/* 0x24024 - Minimum Frame Length Register */
	uint	ptv;		/* 0x24028 - Pause Time Value Register */
	uint	dmactrl;	/* 0x2402c - DMA Control Register */
	uint	tbipa;		/* 0x24030 - TBI PHY Address Register */
	char	res3[88];
	uint	fifo_tx_thr;		/* 0x2408c - FIFO transmit threshold register */
	char	res4[8];
	uint	fifo_tx_starve;		/* 0x24098 - FIFO transmit starve register */
	uint	fifo_tx_starve_shutoff;		/* 0x2409c - FIFO transmit starve shutoff register */
	char	res5[96];
	uint	tctrl;		/* 0x24100 - Transmit Control Register */
	uint	tstat;		/* 0x24104 - Transmit Status Register */
	char	res6[4];
	uint	tbdlen;		/* 0x2410c - Transmit Buffer Descriptor Data Length Register */
	char	res7[16];
	uint	ctbptrh;	/* 0x24120 - Current Transmit Buffer Descriptor Pointer High Register */
	uint	ctbptr;		/* 0x24124 - Current Transmit Buffer Descriptor Pointer Register */
	char	res8[88];
	uint	tbptrh;		/* 0x24180 - Transmit Buffer Descriptor Pointer High Register */
	uint	tbptr;		/* 0x24184 - Transmit Buffer Descriptor Pointer Low Register */
	char	res9[120];
	uint	tbaseh;		/* 0x24200 - Transmit Descriptor Base Address High Register */
	uint	tbase;		/* 0x24204 - Transmit Descriptor Base Address Register */
	char	res10[168];
	uint	ostbd;		/* 0x242b0 - Out-of-Sequence Transmit Buffer Descriptor Register */
	uint	ostbdp;		/* 0x242b4 - Out-of-Sequence Transmit Data Buffer Pointer Register */
	uint	os32tbdp;	/* 0x242b8 - Out-of-Sequence 32 Bytes Transmit Data Buffer Pointer Low Register */
	uint	os32iptrh;	/* 0x242bc - Out-of-Sequence 32 Bytes Transmit Insert Pointer High Register */
	uint	os32iptrl;	/* 0x242c0 - Out-of-Sequence 32 Bytes Transmit Insert Pointer Low Register */
	uint	os32tbdr;	/* 0x242c4 - Out-of-Sequence 32 Bytes Transmit Reserved Register */
	uint	os32iil;	/* 0x242c8 - Out-of-Sequence 32 Bytes Transmit Insert Index/Length Register */
	char	res11[52];
	uint	rctrl;		/* 0x24300 - Receive Control Register */
	uint	rstat;		/* 0x24304 - Receive Status Register */
	char	res12[4];
	uint	rbdlen;		/* 0x2430c - RxBD Data Length Register */
	char	res13[16];
	uint	crbptrh;	/* 0x24320 - Current Receive Buffer Descriptor Pointer High */
	uint	crbptr;		/* 0x24324 - Current Receive Buffer Descriptor Pointer */
	char	res14[24];
	uint	mrblr;		/* 0x24340 - Maximum Receive Buffer Length Register */
	uint	mrblr2r3;	/* 0x24344 - Maximum Receive Buffer Length R2R3 Register */
	char	res15[56];
	uint	rbptrh;		/* 0x24380 - Receive Buffer Descriptor Pointer High 0 */
	uint	rbptr;		/* 0x24384 - Receive Buffer Descriptor Pointer */
	uint	rbptrh1;	/* 0x24388 - Receive Buffer Descriptor Pointer High 1 */
	uint	rbptrl1;	/* 0x2438c - Receive Buffer Descriptor Pointer Low 1 */
	uint	rbptrh2;	/* 0x24390 - Receive Buffer Descriptor Pointer High 2 */
	uint	rbptrl2;	/* 0x24394 - Receive Buffer Descriptor Pointer Low 2 */
	uint	rbptrh3;	/* 0x24398 - Receive Buffer Descriptor Pointer High 3 */
	uint	rbptrl3;	/* 0x2439c - Receive Buffer Descriptor Pointer Low 3 */
	char	res16[96];
	uint	rbaseh;		/* 0x24400 - Receive Descriptor Base Address High 0 */
	uint	rbase;		/* 0x24404 - Receive Descriptor Base Address */
	uint	rbaseh1;	/* 0x24408 - Receive Descriptor Base Address High 1 */
	uint	rbasel1;	/* 0x2440c - Receive Descriptor Base Address Low 1 */
	uint	rbaseh2;	/* 0x24410 - Receive Descriptor Base Address High 2 */
	uint	rbasel2;	/* 0x24414 - Receive Descriptor Base Address Low 2 */
	uint	rbaseh3;	/* 0x24418 - Receive Descriptor Base Address High 3 */
	uint	rbasel3;	/* 0x2441c - Receive Descriptor Base Address Low 3 */
	char	res17[224];
	uint	maccfg1;	/* 0x24500 - MAC Configuration 1 Register */
	uint	maccfg2;	/* 0x24504 - MAC Configuration 2 Register */
	uint	ipgifg;		/* 0x24508 - Inter Packet Gap/Inter Frame Gap Register */
	uint	hafdup;		/* 0x2450c - Half Duplex Register */
	uint	maxfrm;		/* 0x24510 - Maximum Frame Length Register */
	char	res18[12];
	uint	miimcfg;	/* 0x24520 - MII Management Configuration Register */
	uint	miimcom;	/* 0x24524 - MII Management Command Register */
	uint	miimadd;	/* 0x24528 - MII Management Address Register */
	uint	miimcon;	/* 0x2452c - MII Management Control Register */
	uint	miimstat;	/* 0x24530 - MII Management Status Register */
	uint	miimind;	/* 0x24534 - MII Management Indicator Register */
	char	res19[4];
	uint	ifstat;		/* 0x2453c - Interface Status Register */
	uint	macstnaddr1;	/* 0x24540 - Station Address Part 1 Register */
	uint	macstnaddr2;	/* 0x24544 - Station Address Part 2 Register */
	char	res20[312];
	uint	tr64;		/* 0x24680 - Transmit and Receive 64-byte Frame Counter */
	uint	tr127;		/* 0x24684 - Transmit and Receive 65-127 byte Frame Counter */
	uint	tr255;		/* 0x24688 - Transmit and Receive 128-255 byte Frame Counter */
	uint	tr511;		/* 0x2468c - Transmit and Receive 256-511 byte Frame Counter */
	uint	tr1k;		/* 0x24690 - Transmit and Receive 512-1023 byte Frame Counter */
	uint	trmax;		/* 0x24694 - Transmit and Receive 1024-1518 byte Frame Counter */
	uint	trmgv;		/* 0x24698 - Transmit and Receive 1519-1522 byte Good VLAN Frame */
	uint	rbyt;		/* 0x2469c - Receive Byte Counter */
	uint	rpkt;		/* 0x246a0 - Receive Packet Counter */
	uint	rfcs;		/* 0x246a4 - Receive FCS Error Counter */
	uint	rmca;		/* 0x246a8 - Receive Multicast Packet Counter */
	uint	rbca;		/* 0x246ac - Receive Broadcast Packet Counter */
	uint	rxcf;		/* 0x246b0 - Receive Control Frame Packet Counter */
	uint	rxpf;		/* 0x246b4 - Receive Pause Frame Packet Counter */
	uint	rxuo;		/* 0x246b8 - Receive Unknown OP Code Counter */
	uint	raln;		/* 0x246bc - Receive Alignment Error Counter */
	uint	rflr;		/* 0x246c0 - Receive Frame Length Error Counter */
	uint	rcde;		/* 0x246c4 - Receive Code Error Counter */
	uint	rcse;		/* 0x246c8 - Receive Carrier Sense Error Counter */
	uint	rund;		/* 0x246cc - Receive Undersize Packet Counter */
	uint	rovr;		/* 0x246d0 - Receive Oversize Packet Counter */
	uint	rfrg;		/* 0x246d4 - Receive Fragments Counter */
	uint	rjbr;		/* 0x246d8 - Receive Jabber Counter */
	uint	rdrp;		/* 0x246dc - Receive Drop Counter */
	uint	tbyt;		/* 0x246e0 - Transmit Byte Counter Counter */
	uint	tpkt;		/* 0x246e4 - Transmit Packet Counter */
	uint	tmca;		/* 0x246e8 - Transmit Multicast Packet Counter */
	uint	tbca;		/* 0x246ec - Transmit Broadcast Packet Counter */
	uint	txpf;		/* 0x246f0 - Transmit Pause Control Frame Counter */
	uint	tdfr;		/* 0x246f4 - Transmit Deferral Packet Counter */
	uint	tedf;		/* 0x246f8 - Transmit Excessive Deferral Packet Counter */
	uint	tscl;		/* 0x246fc - Transmit Single Collision Packet Counter */
	uint	tmcl;		/* 0x24700 - Transmit Multiple Collision Packet Counter */
	uint	tlcl;		/* 0x24704 - Transmit Late Collision Packet Counter */
	uint	txcl;		/* 0x24708 - Transmit Excessive Collision Packet Counter */
	uint	tncl;		/* 0x2470c - Transmit Total Collision Counter */
	char	res21[4];
	uint	tdrp;		/* 0x24714 - Transmit Drop Frame Counter */
	uint	tjbr;		/* 0x24718 - Transmit Jabber Frame Counter */
	uint	tfcs;		/* 0x2471c - Transmit FCS Error Counter */
	uint	txcf;		/* 0x24720 - Transmit Control Frame Counter */
	uint	tovr;		/* 0x24724 - Transmit Oversize Frame Counter */
	uint	tund;		/* 0x24728 - Transmit Undersize Frame Counter */
	uint	tfrg;		/* 0x2472c - Transmit Fragments Frame Counter */
	uint	car1;		/* 0x24730 - Carry Register One */
	uint	car2;		/* 0x24734 - Carry Register Two */
	uint	cam1;		/* 0x24738 - Carry Mask Register One */
	uint	cam2;		/* 0x2473c - Carry Mask Register Two */
	char	res22[192];
	uint	iaddr0;		/* 0x24800 - Indivdual address register 0 */
	uint	iaddr1;		/* 0x24804 - Indivdual address register 1 */
	uint	iaddr2;		/* 0x24808 - Indivdual address register 2 */
	uint	iaddr3;		/* 0x2480c - Indivdual address register 3 */
	uint	iaddr4;		/* 0x24810 - Indivdual address register 4 */
	uint	iaddr5;		/* 0x24814 - Indivdual address register 5 */
	uint	iaddr6;		/* 0x24818 - Indivdual address register 6 */
	uint	iaddr7;		/* 0x2481c - Indivdual address register 7 */
	char	res23[96];
	uint	gaddr0;		/* 0x24880 - Global address register 0 */
	uint	gaddr1;		/* 0x24884 - Global address register 1 */
	uint	gaddr2;		/* 0x24888 - Global address register 2 */
	uint	gaddr3;		/* 0x2488c - Global address register 3 */
	uint	gaddr4;		/* 0x24890 - Global address register 4 */
	uint	gaddr5;		/* 0x24894 - Global address register 5 */
	uint	gaddr6;		/* 0x24898 - Global address register 6 */
	uint	gaddr7;		/* 0x2489c - Global address register 7 */
	char	res24[96];
	uint	pmd0;		/* 0x24900 - Pattern Match Data Register */
	char	res25[4];
	uint	pmask0;		/* 0x24908 - Pattern Mask Register */
	char	res26[4];
	uint	pcntrl0;	/* 0x24910 - Pattern Match Control Register */
	char	res27[4];
	uint	pattrb0;	/* 0x24918 - Pattern Match Attributes Register */
	uint	pattrbeli0;	/* 0x2491c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd1;		/* 0x24920 - Pattern Match Data Register */
	char	res28[4];
	uint	pmask1;		/* 0x24928 - Pattern Mask Register */
	char	res29[4];
	uint	pcntrl1;	/* 0x24930 - Pattern Match Control Register */
	char	res30[4];
	uint	pattrb1;	/* 0x24938 - Pattern Match Attributes Register */
	uint	pattrbeli1;	/* 0x2493c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd2;		/* 0x24940 - Pattern Match Data Register */
	char	res31[4];
	uint	pmask2;		/* 0x24948 - Pattern Mask Register */
	char	res32[4];
	uint	pcntrl2;	/* 0x24950 - Pattern Match Control Register */
	char	res33[4];
	uint	pattrb2;	/* 0x24958 - Pattern Match Attributes Register */
	uint	pattrbeli2;	/* 0x2495c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd3;		/* 0x24960 - Pattern Match Data Register */
	char	res34[4];
	uint	pmask3;		/* 0x24968 - Pattern Mask Register */
	char	res35[4];
	uint	pcntrl3;	/* 0x24970 - Pattern Match Control Register */
	char	res36[4];
	uint	pattrb3;	/* 0x24978 - Pattern Match Attributes Register */
	uint	pattrbeli3;	/* 0x2497c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd4;		/* 0x24980 - Pattern Match Data Register */
	char	res37[4];
	uint	pmask4;		/* 0x24988 - Pattern Mask Register */
	char	res38[4];
	uint	pcntrl4;	/* 0x24990 - Pattern Match Control Register */
	char	res39[4];
	uint	pattrb4;	/* 0x24998 - Pattern Match Attributes Register */
	uint	pattrbeli4;	/* 0x2499c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd5;		/* 0x249a0 - Pattern Match Data Register */
	char	res40[4];
	uint	pmask5;		/* 0x249a8 - Pattern Mask Register */
	char	res41[4];
	uint	pcntrl5;	/* 0x249b0 - Pattern Match Control Register */
	char	res42[4];
	uint	pattrb5;	/* 0x249b8 - Pattern Match Attributes Register */
	uint	pattrbeli5;	/* 0x249bc - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd6;		/* 0x249c0 - Pattern Match Data Register */
	char	res43[4];
	uint	pmask6;		/* 0x249c8 - Pattern Mask Register */
	char	res44[4];
	uint	pcntrl6;	/* 0x249d0 - Pattern Match Control Register */
	char	res45[4];
	uint	pattrb6;	/* 0x249d8 - Pattern Match Attributes Register */
	uint	pattrbeli6;	/* 0x249dc - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd7;		/* 0x249e0 - Pattern Match Data Register */
	char	res46[4];
	uint	pmask7;		/* 0x249e8 - Pattern Mask Register */
	char	res47[4];
	uint	pcntrl7;	/* 0x249f0 - Pattern Match Control Register */
	char	res48[4];
	uint	pattrb7;	/* 0x249f8 - Pattern Match Attributes Register */
	uint	pattrbeli7;	/* 0x249fc - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd8;		/* 0x24a00 - Pattern Match Data Register */
	char	res49[4];
	uint	pmask8;		/* 0x24a08 - Pattern Mask Register */
	char	res50[4];
	uint	pcntrl8;	/* 0x24a10 - Pattern Match Control Register */
	char	res51[4];
	uint	pattrb8;	/* 0x24a18 - Pattern Match Attributes Register */
	uint	pattrbeli8;	/* 0x24a1c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd9;		/* 0x24a20 - Pattern Match Data Register */
	char	res52[4];
	uint	pmask9;		/* 0x24a28 - Pattern Mask Register */
	char	res53[4];
	uint	pcntrl9;	/* 0x24a30 - Pattern Match Control Register */
	char	res54[4];
	uint	pattrb9;	/* 0x24a38 - Pattern Match Attributes Register */
	uint	pattrbeli9;	/* 0x24a3c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd10;		/* 0x24a40 - Pattern Match Data Register */
	char	res55[4];
	uint	pmask10;	/* 0x24a48 - Pattern Mask Register */
	char	res56[4];
	uint	pcntrl10;	/* 0x24a50 - Pattern Match Control Register */
	char	res57[4];
	uint	pattrb10;	/* 0x24a58 - Pattern Match Attributes Register */
	uint	pattrbeli10;	/* 0x24a5c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd11;		/* 0x24a60 - Pattern Match Data Register */
	char	res58[4];
	uint	pmask11;	/* 0x24a68 - Pattern Mask Register */
	char	res59[4];
	uint	pcntrl11;	/* 0x24a70 - Pattern Match Control Register */
	char	res60[4];
	uint	pattrb11;	/* 0x24a78 - Pattern Match Attributes Register */
	uint	pattrbeli11;	/* 0x24a7c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd12;		/* 0x24a80 - Pattern Match Data Register */
	char	res61[4];
	uint	pmask12;	/* 0x24a88 - Pattern Mask Register */
	char	res62[4];
	uint	pcntrl12;	/* 0x24a90 - Pattern Match Control Register */
	char	res63[4];
	uint	pattrb12;	/* 0x24a98 - Pattern Match Attributes Register */
	uint	pattrbeli12;	/* 0x24a9c - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd13;		/* 0x24aa0 - Pattern Match Data Register */
	char	res64[4];
	uint	pmask13;	/* 0x24aa8 - Pattern Mask Register */
	char	res65[4];
	uint	pcntrl13;	/* 0x24ab0 - Pattern Match Control Register */
	char	res66[4];
	uint	pattrb13;	/* 0x24ab8 - Pattern Match Attributes Register */
	uint	pattrbeli13;	/* 0x24abc - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd14;		/* 0x24ac0 - Pattern Match Data Register */
	char	res67[4];
	uint	pmask14;	/* 0x24ac8 - Pattern Mask Register */
	char	res68[4];
	uint	pcntrl14;	/* 0x24ad0 - Pattern Match Control Register */
	char	res69[4];
	uint	pattrb14;	/* 0x24ad8 - Pattern Match Attributes Register */
	uint	pattrbeli14;	/* 0x24adc - Pattern Match Attributes Extract Length and Extract Index Register */
	uint	pmd15;		/* 0x24ae0 - Pattern Match Data Register */
	char	res70[4];
	uint	pmask15;	/* 0x24ae8 - Pattern Mask Register */
	char	res71[4];
	uint	pcntrl15;	/* 0x24af0 - Pattern Match Control Register */
	char	res72[4];
	uint	pattrb15;	/* 0x24af8 - Pattern Match Attributes Register */
	uint	pattrbeli15;	/* 0x24afc - Pattern Match Attributes Extract Length and Extract Index Register */
	char	res73[248];
	uint	attr;		/* 0x24bf8 - Attributes Register */
	uint	attreli;	/* 0x24bfc - Attributes Extract Length and Extract Index Register */
	char	res74[1024];
} ccsr_tsec_t;

/*
 * PIC Registers(0x4_0000-0x8_0000)
 */
typedef struct ccsr_pic {
	char	res1[64];	/* 0x40000 */
	uint	ipidr0;		/* 0x40040 - Interprocessor Interrupt Dispatch Register 0 */
	char	res2[12];
	uint	ipidr1;		/* 0x40050 - Interprocessor Interrupt Dispatch Register 1 */
	char	res3[12];
	uint	ipidr2;		/* 0x40060 - Interprocessor Interrupt Dispatch Register 2 */
	char	res4[12];
	uint	ipidr3;		/* 0x40070 - Interprocessor Interrupt Dispatch Register 3 */
	char	res5[12];
	uint	ctpr;		/* 0x40080 - Current Task Priority Register */
	char	res6[12];
	uint	whoami;		/* 0x40090 - Who Am I Register */
	char	res7[12];
	uint	iack;		/* 0x400a0 - Interrupt Acknowledge Register */
	char	res8[12];
	uint	eoi;		/* 0x400b0 - End Of Interrupt Register */
	char	res9[3916];
	uint	frr;		/* 0x41000 - Feature Reporting Register */
	char	res10[28];
	uint	gcr;		/* 0x41020 - Global Configuration Register */
#define MPC85xx_PICGCR_RST   0x80000000
#define MPC85xx_PICGCR_M     0x20000000
	char	res11[92];
	uint	vir;		/* 0x41080 - Vendor Identification Register */
	char	res12[12];
	uint	pir;		/* 0x41090 - Processor Initialization Register */
	char	res13[12];
	uint	ipivpr0;	/* 0x410a0 - IPI Vector/Priority Register 0 */
	char	res14[12];
	uint	ipivpr1;	/* 0x410b0 - IPI Vector/Priority Register 1 */
	char	res15[12];
	uint	ipivpr2;	/* 0x410c0 - IPI Vector/Priority Register 2 */
	char	res16[12];
	uint	ipivpr3;	/* 0x410d0 - IPI Vector/Priority Register 3 */
	char	res17[12];
	uint	svr;		/* 0x410e0 - Spurious Vector Register */
	char	res18[12];
	uint	tfrr;		/* 0x410f0 - Timer Frequency Reporting Register */
	char	res19[12];
	uint	gtccr0;		/* 0x41100 - Global Timer Current Count Register 0 */
	char	res20[12];
	uint	gtbcr0;		/* 0x41110 - Global Timer Base Count Register 0 */
	char	res21[12];
	uint	gtvpr0;		/* 0x41120 - Global Timer Vector/Priority Register 0 */
	char	res22[12];
	uint	gtdr0;		/* 0x41130 - Global Timer Destination Register 0 */
	char	res23[12];
	uint	gtccr1;		/* 0x41140 - Global Timer Current Count Register 1 */
	char	res24[12];
	uint	gtbcr1;		/* 0x41150 - Global Timer Base Count Register 1 */
	char	res25[12];
	uint	gtvpr1;		/* 0x41160 - Global Timer Vector/Priority Register 1 */
	char	res26[12];
	uint	gtdr1;		/* 0x41170 - Global Timer Destination Register 1 */
	char	res27[12];
	uint	gtccr2;		/* 0x41180 - Global Timer Current Count Register 2 */
	char	res28[12];
	uint	gtbcr2;		/* 0x41190 - Global Timer Base Count Register 2 */
	char	res29[12];
	uint	gtvpr2;		/* 0x411a0 - Global Timer Vector/Priority Register 2 */
	char	res30[12];
	uint	gtdr2;		/* 0x411b0 - Global Timer Destination Register 2 */
	char	res31[12];
	uint	gtccr3;		/* 0x411c0 - Global Timer Current Count Register 3 */
	char	res32[12];
	uint	gtbcr3;		/* 0x411d0 - Global Timer Base Count Register 3 */
	char	res33[12];
	uint	gtvpr3;		/* 0x411e0 - Global Timer Vector/Priority Register 3 */
	char	res34[12];
	uint	gtdr3;		/* 0x411f0 - Global Timer Destination Register 3 */
	char	res35[268];
	uint	tcr;		/* 0x41300 - Timer Control Register */
	char	res36[12];
	uint	irqsr0;		/* 0x41310 - IRQ_OUT Summary Register 0 */
	char	res37[12];
	uint	irqsr1;		/* 0x41320 - IRQ_OUT Summary Register 1 */
	char	res38[12];
	uint	cisr0;		/* 0x41330 - Critical Interrupt Summary Register 0 */
	char	res39[12];
	uint	cisr1;		/* 0x41340 - Critical Interrupt Summary Register 1 */
	char	res40[188];
	uint	msgr0;		/* 0x41400 - Message Register 0 */
	char	res41[12];
	uint	msgr1;		/* 0x41410 - Message Register 1 */
	char	res42[12];
	uint	msgr2;		/* 0x41420 - Message Register 2 */
	char	res43[12];
	uint	msgr3;		/* 0x41430 - Message Register 3 */
	char	res44[204];
	uint	mer;		/* 0x41500 - Message Enable Register */
	char	res45[12];
	uint	msr;		/* 0x41510 - Message Status Register */
	char	res46[60140];
	uint	eivpr0;		/* 0x50000 - External Interrupt Vector/Priority Register 0 */
	char	res47[12];
	uint	eidr0;		/* 0x50010 - External Interrupt Destination Register 0 */
	char	res48[12];
	uint	eivpr1;		/* 0x50020 - External Interrupt Vector/Priority Register 1 */
	char	res49[12];
	uint	eidr1;		/* 0x50030 - External Interrupt Destination Register 1 */
	char	res50[12];
	uint	eivpr2;		/* 0x50040 - External Interrupt Vector/Priority Register 2 */
	char	res51[12];
	uint	eidr2;		/* 0x50050 - External Interrupt Destination Register 2 */
	char	res52[12];
	uint	eivpr3;		/* 0x50060 - External Interrupt Vector/Priority Register 3 */
	char	res53[12];
	uint	eidr3;		/* 0x50070 - External Interrupt Destination Register 3 */
	char	res54[12];
	uint	eivpr4;		/* 0x50080 - External Interrupt Vector/Priority Register 4 */
	char	res55[12];
	uint	eidr4;		/* 0x50090 - External Interrupt Destination Register 4 */
	char	res56[12];
	uint	eivpr5;		/* 0x500a0 - External Interrupt Vector/Priority Register 5 */
	char	res57[12];
	uint	eidr5;		/* 0x500b0 - External Interrupt Destination Register 5 */
	char	res58[12];
	uint	eivpr6;		/* 0x500c0 - External Interrupt Vector/Priority Register 6 */
	char	res59[12];
	uint	eidr6;		/* 0x500d0 - External Interrupt Destination Register 6 */
	char	res60[12];
	uint	eivpr7;		/* 0x500e0 - External Interrupt Vector/Priority Register 7 */
	char	res61[12];
	uint	eidr7;		/* 0x500f0 - External Interrupt Destination Register 7 */
	char	res62[12];
	uint	eivpr8;		/* 0x50100 - External Interrupt Vector/Priority Register 8 */
	char	res63[12];
	uint	eidr8;		/* 0x50110 - External Interrupt Destination Register 8 */
	char	res64[12];
	uint	eivpr9;		/* 0x50120 - External Interrupt Vector/Priority Register 9 */
	char	res65[12];
	uint	eidr9;		/* 0x50130 - External Interrupt Destination Register 9 */
	char	res66[12];
	uint	eivpr10;	/* 0x50140 - External Interrupt Vector/Priority Register 10 */
	char	res67[12];
	uint	eidr10;		/* 0x50150 - External Interrupt Destination Register 10 */
	char	res68[12];
	uint	eivpr11;	/* 0x50160 - External Interrupt Vector/Priority Register 11 */
	char	res69[12];
	uint	eidr11;		/* 0x50170 - External Interrupt Destination Register 11 */
	char	res70[140];
	uint	iivpr0;		/* 0x50200 - Internal Interrupt Vector/Priority Register 0 */
	char	res71[12];
	uint	iidr0;		/* 0x50210 - Internal Interrupt Destination Register 0 */
	char	res72[12];
	uint	iivpr1;		/* 0x50220 - Internal Interrupt Vector/Priority Register 1 */
	char	res73[12];
	uint	iidr1;		/* 0x50230 - Internal Interrupt Destination Register 1 */
	char	res74[12];
	uint	iivpr2;		/* 0x50240 - Internal Interrupt Vector/Priority Register 2 */
	char	res75[12];
	uint	iidr2;		/* 0x50250 - Internal Interrupt Destination Register 2 */
	char	res76[12];
	uint	iivpr3;		/* 0x50260 - Internal Interrupt Vector/Priority Register 3 */
	char	res77[12];
	uint	iidr3;		/* 0x50270 - Internal Interrupt Destination Register 3 */
	char	res78[12];
	uint	iivpr4;		/* 0x50280 - Internal Interrupt Vector/Priority Register 4 */
	char	res79[12];
	uint	iidr4;		/* 0x50290 - Internal Interrupt Destination Register 4 */
	char	res80[12];
	uint	iivpr5;		/* 0x502a0 - Internal Interrupt Vector/Priority Register 5 */
	char	res81[12];
	uint	iidr5;		/* 0x502b0 - Internal Interrupt Destination Register 5 */
	char	res82[12];
	uint	iivpr6;		/* 0x502c0 - Internal Interrupt Vector/Priority Register 6 */
	char	res83[12];
	uint	iidr6;		/* 0x502d0 - Internal Interrupt Destination Register 6 */
	char	res84[12];
	uint	iivpr7;		/* 0x502e0 - Internal Interrupt Vector/Priority Register 7 */
	char	res85[12];
	uint	iidr7;		/* 0x502f0 - Internal Interrupt Destination Register 7 */
	char	res86[12];
	uint	iivpr8;		/* 0x50300 - Internal Interrupt Vector/Priority Register 8 */
	char	res87[12];
	uint	iidr8;		/* 0x50310 - Internal Interrupt Destination Register 8 */
	char	res88[12];
	uint	iivpr9;		/* 0x50320 - Internal Interrupt Vector/Priority Register 9 */
	char	res89[12];
	uint	iidr9;		/* 0x50330 - Internal Interrupt Destination Register 9 */
	char	res90[12];
	uint	iivpr10;	/* 0x50340 - Internal Interrupt Vector/Priority Register 10 */
	char	res91[12];
	uint	iidr10;		/* 0x50350 - Internal Interrupt Destination Register 10 */
	char	res92[12];
	uint	iivpr11;	/* 0x50360 - Internal Interrupt Vector/Priority Register 11 */
	char	res93[12];
	uint	iidr11;		/* 0x50370 - Internal Interrupt Destination Register 11 */
	char	res94[12];
	uint	iivpr12;	/* 0x50380 - Internal Interrupt Vector/Priority Register 12 */
	char	res95[12];
	uint	iidr12;		/* 0x50390 - Internal Interrupt Destination Register 12 */
	char	res96[12];
	uint	iivpr13;	/* 0x503a0 - Internal Interrupt Vector/Priority Register 13 */
	char	res97[12];
	uint	iidr13;		/* 0x503b0 - Internal Interrupt Destination Register 13 */
	char	res98[12];
	uint	iivpr14;	/* 0x503c0 - Internal Interrupt Vector/Priority Register 14 */
	char	res99[12];
	uint	iidr14;		/* 0x503d0 - Internal Interrupt Destination Register 14 */
	char	res100[12];
	uint	iivpr15;	/* 0x503e0 - Internal Interrupt Vector/Priority Register 15 */
	char	res101[12];
	uint	iidr15;		/* 0x503f0 - Internal Interrupt Destination Register 15 */
	char	res102[12];
	uint	iivpr16;	/* 0x50400 - Internal Interrupt Vector/Priority Register 16 */
	char	res103[12];
	uint	iidr16;		/* 0x50410 - Internal Interrupt Destination Register 16 */
	char	res104[12];
	uint	iivpr17;	/* 0x50420 - Internal Interrupt Vector/Priority Register 17 */
	char	res105[12];
	uint	iidr17;		/* 0x50430 - Internal Interrupt Destination Register 17 */
	char	res106[12];
	uint	iivpr18;	/* 0x50440 - Internal Interrupt Vector/Priority Register 18 */
	char	res107[12];
	uint	iidr18;		/* 0x50450 - Internal Interrupt Destination Register 18 */
	char	res108[12];
	uint	iivpr19;	/* 0x50460 - Internal Interrupt Vector/Priority Register 19 */
	char	res109[12];
	uint	iidr19;		/* 0x50470 - Internal Interrupt Destination Register 19 */
	char	res110[12];
	uint	iivpr20;	/* 0x50480 - Internal Interrupt Vector/Priority Register 20 */
	char	res111[12];
	uint	iidr20;		/* 0x50490 - Internal Interrupt Destination Register 20 */
	char	res112[12];
	uint	iivpr21;	/* 0x504a0 - Internal Interrupt Vector/Priority Register 21 */
	char	res113[12];
	uint	iidr21;		/* 0x504b0 - Internal Interrupt Destination Register 21 */
	char	res114[12];
	uint	iivpr22;	/* 0x504c0 - Internal Interrupt Vector/Priority Register 22 */
	char	res115[12];
	uint	iidr22;		/* 0x504d0 - Internal Interrupt Destination Register 22 */
	char	res116[12];
	uint	iivpr23;	/* 0x504e0 - Internal Interrupt Vector/Priority Register 23 */
	char	res117[12];
	uint	iidr23;		/* 0x504f0 - Internal Interrupt Destination Register 23 */
	char	res118[12];
	uint	iivpr24;	/* 0x50500 - Internal Interrupt Vector/Priority Register 24 */
	char	res119[12];
	uint	iidr24;		/* 0x50510 - Internal Interrupt Destination Register 24 */
	char	res120[12];
	uint	iivpr25;	/* 0x50520 - Internal Interrupt Vector/Priority Register 25 */
	char	res121[12];
	uint	iidr25;		/* 0x50530 - Internal Interrupt Destination Register 25 */
	char	res122[12];
	uint	iivpr26;	/* 0x50540 - Internal Interrupt Vector/Priority Register 26 */
	char	res123[12];
	uint	iidr26;		/* 0x50550 - Internal Interrupt Destination Register 26 */
	char	res124[12];
	uint	iivpr27;	/* 0x50560 - Internal Interrupt Vector/Priority Register 27 */
	char	res125[12];
	uint	iidr27;		/* 0x50570 - Internal Interrupt Destination Register 27 */
	char	res126[12];
	uint	iivpr28;	/* 0x50580 - Internal Interrupt Vector/Priority Register 28 */
	char	res127[12];
	uint	iidr28;		/* 0x50590 - Internal Interrupt Destination Register 28 */
	char	res128[12];
	uint	iivpr29;	/* 0x505a0 - Internal Interrupt Vector/Priority Register 29 */
	char	res129[12];
	uint	iidr29;		/* 0x505b0 - Internal Interrupt Destination Register 29 */
	char	res130[12];
	uint	iivpr30;	/* 0x505c0 - Internal Interrupt Vector/Priority Register 30 */
	char	res131[12];
	uint	iidr30;		/* 0x505d0 - Internal Interrupt Destination Register 30 */
	char	res132[12];
	uint	iivpr31;	/* 0x505e0 - Internal Interrupt Vector/Priority Register 31 */
	char	res133[12];
	uint	iidr31;		/* 0x505f0 - Internal Interrupt Destination Register 31 */
	char	res134[4108];
	uint	mivpr0;		/* 0x51600 - Messaging Interrupt Vector/Priority Register 0 */
	char	res135[12];
	uint	midr0;		/* 0x51610 - Messaging Interrupt Destination Register 0 */
	char	res136[12];
	uint	mivpr1;		/* 0x51620 - Messaging Interrupt Vector/Priority Register 1 */
	char	res137[12];
	uint	midr1;		/* 0x51630 - Messaging Interrupt Destination Register 1 */
	char	res138[12];
	uint	mivpr2;		/* 0x51640 - Messaging Interrupt Vector/Priority Register 2 */
	char	res139[12];
	uint	midr2;		/* 0x51650 - Messaging Interrupt Destination Register 2 */
	char	res140[12];
	uint	mivpr3;		/* 0x51660 - Messaging Interrupt Vector/Priority Register 3 */
	char	res141[12];
	uint	midr3;		/* 0x51670 - Messaging Interrupt Destination Register 3 */
	char	res142[59852];
	uint	ipi0dr0;	/* 0x60040 - Processor 0 Interprocessor Interrupt Dispatch Register 0 */
	char	res143[12];
	uint	ipi0dr1;	/* 0x60050 - Processor 0 Interprocessor Interrupt Dispatch Register 1 */
	char	res144[12];
	uint	ipi0dr2;	/* 0x60060 - Processor 0 Interprocessor Interrupt Dispatch Register 2 */
	char	res145[12];
	uint	ipi0dr3;	/* 0x60070 - Processor 0 Interprocessor Interrupt Dispatch Register 3 */
	char	res146[12];
	uint	ctpr0;		/* 0x60080 - Current Task Priority Register for Processor 0 */
	char	res147[12];
	uint	whoami0;	/* 0x60090 - Who Am I Register for Processor 0 */
	char	res148[12];
	uint	iack0;		/* 0x600a0 - Interrupt Acknowledge Register for Processor 0 */
	char	res149[12];
	uint	eoi0;		/* 0x600b0 - End Of Interrupt Register for Processor 0 */
	char	res150[130892];
} ccsr_pic_t;

/*
 * CPM Block(0x8_0000-0xc_0000)
 */
#ifndef CONFIG_CPM2
typedef struct ccsr_cpm {
	char res[262144];
} ccsr_cpm_t;
#else
/*
 * 0x8000-0x8ffff:DPARM
 * 0x9000-0x90bff: General SIU
 */
typedef struct ccsr_cpm_siu {
	char 	res1[80];
	uint	smaer;
	uint	smser;
	uint	smevr;
	char    res2[4];
	uint	lmaer;
	uint	lmser;
	uint	lmevr;
	char	res3[2964];
} ccsr_cpm_siu_t;

/* 0x90c00-0x90cff: Interrupt Controller */
typedef struct ccsr_cpm_intctl {
	ushort  sicr;
	char    res1[2];
	uint    sivec;
	uint    sipnrh;
	uint    sipnrl;
	uint    siprr;
	uint    scprrh;
	uint    scprrl;
	uint    simrh;
	uint    simrl;
	uint    siexr;
	char    res2[88];
	uint	sccr;
	char	res3[124];
} ccsr_cpm_intctl_t;

/* 0x90d00-0x90d7f: input/output port */
typedef struct ccsr_cpm_iop {
	uint    pdira;
	uint    ppara;
	uint    psora;
	uint    podra;
	uint    pdata;
	char    res1[12];
	uint    pdirb;
	uint    pparb;
	uint    psorb;
	uint    podrb;
	uint    pdatb;
	char    res2[12];
	uint    pdirc;
	uint    pparc;
	uint    psorc;
	uint    podrc;
	uint    pdatc;
	char    res3[12];
	uint    pdird;
	uint    ppard;
	uint    psord;
	uint    podrd;
	uint    pdatd;
	char    res4[12];
} ccsr_cpm_iop_t;

/* 0x90d80-0x91017: CPM timers */
typedef struct ccsr_cpm_timer {
	u_char  tgcr1;
	char    res1[3];
	u_char  tgcr2;
	char    res2[11];
	ushort  tmr1;
	ushort  tmr2;
	ushort  trr1;
	ushort  trr2;
	ushort  tcr1;
	ushort  tcr2;
	ushort  tcn1;
	ushort  tcn2;
	ushort  tmr3;
	ushort  tmr4;
	ushort  trr3;
	ushort  trr4;
	ushort  tcr3;
	ushort  tcr4;
	ushort  tcn3;
	ushort  tcn4;
	ushort  ter1;
	ushort  ter2;
	ushort  ter3;
	ushort  ter4;
	char    res3[608];
} ccsr_cpm_timer_t;

/* 0x91018-0x912ff: SDMA */
typedef struct ccsr_cpm_sdma {
	uchar	sdsr;
	char 	res1[3];
	uchar 	sdmr;
	char 	res2[739];
} ccsr_cpm_sdma_t;

/* 0x91300-0x9131f: FCC1 */
typedef struct ccsr_cpm_fcc1 {
	uint    gfmr;
	uint    fpsmr;
	ushort  ftodr;
	char    res1[2];
	ushort  fdsr;
	char    res2[2];
	ushort  fcce;
	char    res3[2];
	ushort  fccm;
	char    res4[2];
	u_char  fccs;
	char    res5[3];
	u_char  ftirr_phy[4];
} ccsr_cpm_fcc1_t;

/* 0x91320-0x9133f: FCC2 */
typedef struct ccsr_cpm_fcc2 {
	uint    gfmr;
	uint    fpsmr;
	ushort  ftodr;
	char    res1[2];
	ushort  fdsr;
	char    res2[2];
	ushort  fcce;
	char    res3[2];
	ushort  fccm;
	char    res4[2];
	u_char  fccs;
	char    res5[3];
	u_char  ftirr_phy[4];
} ccsr_cpm_fcc2_t;

/* 0x91340-0x9137f: FCC3 */
typedef struct ccsr_cpm_fcc3 {
	uint    gfmr;
	uint    fpsmr;
	ushort  ftodr;
	char    res1[2];
	ushort  fdsr;
	char    res2[2];
	ushort  fcce;
	char    res3[2];
	ushort  fccm;
	char    res4[2];
	u_char  fccs;
	char    res5[3];
	char	res[36];
} ccsr_cpm_fcc3_t;

/* 0x91380-0x9139f: FCC1 extended */
typedef struct ccsr_cpm_fcc1_ext {
	uint	firper;
	uint	firer;
	uint	firsr_h;
	uint	firsr_l;
	u_char	gfemr;
	char	res[15];

} ccsr_cpm_fcc1_ext_t;

/* 0x913a0-0x913cf: FCC2 extended */
typedef struct ccsr_cpm_fcc2_ext {
	uint	firper;
	uint	firer;
	uint	firsr_h;
	uint	firsr_l;
	u_char	gfemr;
	char	res[31];
} ccsr_cpm_fcc2_ext_t;

/* 0x913d0-0x913ff: FCC3 extended */
typedef struct ccsr_cpm_fcc3_ext {
	u_char	gfemr;
	char	res[47];
} ccsr_cpm_fcc3_ext_t;

/* 0x91400-0x915ef: TC layers */
typedef struct ccsr_cpm_tmp1 {
	char 	res[496];
} ccsr_cpm_tmp1_t;

/* 0x915f0-0x9185f: BRGs:5,6,7,8 */
typedef struct ccsr_cpm_brg2 {
	uint	brgc5;
	uint	brgc6;
	uint	brgc7;
	uint	brgc8;
	char	res[608];
} ccsr_cpm_brg2_t;

/* 0x91860-0x919bf: I2C */
typedef struct ccsr_cpm_i2c {
	u_char  i2mod;
	char    res1[3];
	u_char  i2add;
	char    res2[3];
	u_char  i2brg;
	char    res3[3];
	u_char  i2com;
	char    res4[3];
	u_char  i2cer;
	char    res5[3];
	u_char  i2cmr;
	char    res6[331];
} ccsr_cpm_i2c_t;

/* 0x919c0-0x919ef: CPM core */
typedef struct ccsr_cpm_cp {
	uint    cpcr;
	uint    rccr;
	char    res1[14];
	ushort  rter;
	char    res2[2];
	ushort  rtmr;
	ushort  rtscr;
	char    res3[2];
	uint    rtsr;
	char    res4[12];
} ccsr_cpm_cp_t;

/* 0x919f0-0x919ff: BRGs:1,2,3,4 */
typedef struct ccsr_cpm_brg1 {
	uint	brgc1;
	uint	brgc2;
	uint	brgc3;
	uint	brgc4;
} ccsr_cpm_brg1_t;

/* 0x91a00-0x91a9f: SCC1-SCC4 */
typedef struct ccsr_cpm_scc {
	uint    gsmrl;
	uint    gsmrh;
	ushort  psmr;
	char    res1[2];
	ushort  todr;
	ushort  dsr;
	ushort  scce;
	char    res2[2];
	ushort  sccm;
	char    res3;
	u_char  sccs;
	char    res4[8];
} ccsr_cpm_scc_t;

/* 0x91a80-0x91a9f */
typedef struct ccsr_cpm_tmp2 {
	char 	res[32];
} ccsr_cpm_tmp2_t;

/* 0x91aa0-0x91aff: SPI */
typedef struct ccsr_cpm_spi {
	ushort  spmode;
	char    res1[4];
	u_char  spie;
	char    res2[3];
	u_char  spim;
	char    res3[2];
	u_char  spcom;
	char    res4[82];
} ccsr_cpm_spi_t;

/* 0x91b00-0x91b1f: CPM MUX */
typedef struct ccsr_cpm_mux {
	u_char  cmxsi1cr;
	char    res1;
	u_char  cmxsi2cr;
	char    res2;
	uint    cmxfcr;
	uint    cmxscr;
	char    res3[2];
	ushort  cmxuar;
	char    res4[16];
} ccsr_cpm_mux_t;

/* 0x91b20-0xbffff: SI,MCC,etc */
typedef struct ccsr_cpm_tmp3 {
	char res[58592];
} ccsr_cpm_tmp3_t;

typedef struct ccsr_cpm_iram {
	unsigned long iram[8192];
	char res[98304];
} ccsr_cpm_iram_t;

typedef struct ccsr_cpm {
	/* Some references are into the unique and known dpram spaces,
	 * others are from the generic base.
	 */
#define im_dprambase    	im_dpram1
	u_char          	im_dpram1[16*1024];
	char            	res1[16*1024];
	u_char          	im_dpram2[16*1024];
	char            	res2[16*1024];
	ccsr_cpm_siu_t  	im_cpm_siu;     /* SIU Configuration */
	ccsr_cpm_intctl_t    	im_cpm_intctl;  /* Interrupt Controller */
	ccsr_cpm_iop_t       	im_cpm_iop;     /* IO Port control/status */
	ccsr_cpm_timer_t  	im_cpm_timer;   /* CPM timers */
	ccsr_cpm_sdma_t      	im_cpm_sdma;    /* SDMA control/status */
	ccsr_cpm_fcc1_t		im_cpm_fcc1;
	ccsr_cpm_fcc2_t		im_cpm_fcc2;
	ccsr_cpm_fcc3_t		im_cpm_fcc3;
	ccsr_cpm_fcc1_ext_t	im_cpm_fcc1_ext;
	ccsr_cpm_fcc2_ext_t	im_cpm_fcc2_ext;
	ccsr_cpm_fcc3_ext_t	im_cpm_fcc3_ext;
	ccsr_cpm_tmp1_t		im_cpm_tmp1;
	ccsr_cpm_brg2_t		im_cpm_brg2;
	ccsr_cpm_i2c_t		im_cpm_i2c;
	ccsr_cpm_cp_t		im_cpm_cp;
	ccsr_cpm_brg1_t		im_cpm_brg1;
	ccsr_cpm_scc_t		im_cpm_scc[4];
	ccsr_cpm_tmp2_t		im_cpm_tmp2;
	ccsr_cpm_spi_t		im_cpm_spi;
	ccsr_cpm_mux_t		im_cpm_mux;
	ccsr_cpm_tmp3_t		im_cpm_tmp3;
	ccsr_cpm_iram_t		im_cpm_iram;
} ccsr_cpm_t;
#endif

/*
 * RapidIO Registers(0xc_0000-0xe_0000)
 */
typedef struct ccsr_rio {
	uint	didcar;		/* 0xc0000 - Device Identity Capability Register */
	uint	dicar;		/* 0xc0004 - Device Information Capability Register */
	uint	aidcar;		/* 0xc0008 - Assembly Identity Capability Register */
	uint	aicar;		/* 0xc000c - Assembly Information Capability Register */
	uint	pefcar;		/* 0xc0010 - Processing Element Features Capability Register */
	uint	spicar;		/* 0xc0014 - Switch Port Information Capability Register */
	uint	socar;		/* 0xc0018 - Source Operations Capability Register */
	uint	docar;		/* 0xc001c - Destination Operations Capability Register */
	char	res1[32];
	uint	msr;		/* 0xc0040 - Mailbox Command And Status Register */
	uint	pwdcsr;		/* 0xc0044 - Port-Write and Doorbell Command And Status Register */
	char	res2[4];
	uint	pellccsr;	/* 0xc004c - Processing Element Logic Layer Control Command and Status Register */
	char	res3[12];
	uint	lcsbacsr;	/* 0xc005c - Local Configuration Space Base Address Command and Status Register */
	uint	bdidcsr;	/* 0xc0060 - Base Device ID Command and Status Register */
	char	res4[4];
	uint	hbdidlcsr;	/* 0xc0068 - Host Base Device ID Lock Command and Status Register */
	uint	ctcsr;		/* 0xc006c - Component Tag Command and Status Register */
	char	res5[144];
	uint	pmbh0csr;	/* 0xc0100 - 8/16 LP-LVDS Port Maintenance Block Header 0 Command and Status Register */
	char	res6[28];
	uint	pltoccsr;	/* 0xc0120 - Port Link Time-out Control Command and Status Register */
	uint	prtoccsr;	/* 0xc0124 - Port Response Time-out Control Command and Status Register */
	char	res7[20];
	uint	pgccsr;		/* 0xc013c - Port General Command and Status Register */
	uint	plmreqcsr;	/* 0xc0140 - Port Link Maintenance Request Command and Status Register */
	uint	plmrespcsr;	/* 0xc0144 - Port Link Maintenance Response Command and Status Register */
	uint	plascsr;	/* 0xc0148 - Port Local Ackid Status Command and Status Register */
	char	res8[12];
	uint	pescsr;		/* 0xc0158 - Port Error and Status Command and Status Register */
	uint	pccsr;		/* 0xc015c - Port Control Command and Status Register */
	char	res9[65184];
	uint	cr;		/* 0xd0000 - Port Control Command and Status Register */
	char	res10[12];
	uint	pcr;		/* 0xd0010 - Port Configuration Register */
	uint	peir;		/* 0xd0014 - Port Error Injection Register */
	char	res11[3048];
	uint	rowtar0;	/* 0xd0c00 - RapidIO Outbound Window Translation Address Register 0 */
	char	res12[12];
	uint	rowar0;		/* 0xd0c10 - RapidIO Outbound Attributes Register 0 */
	char	res13[12];
	uint	rowtar1;	/* 0xd0c20 - RapidIO Outbound Window Translation Address Register 1 */
	char	res14[4];
	uint	rowbar1;	/* 0xd0c28 - RapidIO Outbound Window Base Address Register 1 */
	char	res15[4];
	uint	rowar1;		/* 0xd0c30 - RapidIO Outbound Attributes Register 1 */
	char	res16[12];
	uint	rowtar2;	/* 0xd0c40 - RapidIO Outbound Window Translation Address Register 2 */
	char	res17[4];
	uint	rowbar2;	/* 0xd0c48 - RapidIO Outbound Window Base Address Register 2 */
	char	res18[4];
	uint	rowar2;		/* 0xd0c50 - RapidIO Outbound Attributes Register 2 */
	char	res19[12];
	uint	rowtar3;	/* 0xd0c60 - RapidIO Outbound Window Translation Address Register 3 */
	char	res20[4];
	uint	rowbar3;	/* 0xd0c68 - RapidIO Outbound Window Base Address Register 3 */
	char	res21[4];
	uint	rowar3;		/* 0xd0c70 - RapidIO Outbound Attributes Register 3 */
	char	res22[12];
	uint	rowtar4;	/* 0xd0c80 - RapidIO Outbound Window Translation Address Register 4 */
	char	res23[4];
	uint	rowbar4;	/* 0xd0c88 - RapidIO Outbound Window Base Address Register 4 */
	char	res24[4];
	uint	rowar4;		/* 0xd0c90 - RapidIO Outbound Attributes Register 4 */
	char	res25[12];
	uint	rowtar5;	/* 0xd0ca0 - RapidIO Outbound Window Translation Address Register 5 */
	char	res26[4];
	uint	rowbar5;	/* 0xd0ca8 - RapidIO Outbound Window Base Address Register 5 */
	char	res27[4];
	uint	rowar5;		/* 0xd0cb0 - RapidIO Outbound Attributes Register 5 */
	char	res28[12];
	uint	rowtar6;	/* 0xd0cc0 - RapidIO Outbound Window Translation Address Register 6 */
	char	res29[4];
	uint	rowbar6;	/* 0xd0cc8 - RapidIO Outbound Window Base Address Register 6 */
	char	res30[4];
	uint	rowar6;		/* 0xd0cd0 - RapidIO Outbound Attributes Register 6 */
	char	res31[12];
	uint	rowtar7;	/* 0xd0ce0 - RapidIO Outbound Window Translation Address Register 7 */
	char	res32[4];
	uint	rowbar7;	/* 0xd0ce8 - RapidIO Outbound Window Base Address Register 7 */
	char	res33[4];
	uint	rowar7;		/* 0xd0cf0 - RapidIO Outbound Attributes Register 7 */
	char	res34[12];
	uint	rowtar8;	/* 0xd0d00 - RapidIO Outbound Window Translation Address Register 8 */
	char	res35[4];
	uint	rowbar8;	/* 0xd0d08 - RapidIO Outbound Window Base Address Register 8 */
	char	res36[4];
	uint	rowar8;		/* 0xd0d10 - RapidIO Outbound Attributes Register 8 */
	char	res37[76];
	uint	riwtar4;	/* 0xd0d60 - RapidIO Inbound Window Translation Address Register 4 */
	char	res38[4];
	uint	riwbar4;	/* 0xd0d68 - RapidIO Inbound Window Base Address Register 4 */
	char	res39[4];
	uint	riwar4;		/* 0xd0d70 - RapidIO Inbound Attributes Register 4 */
	char	res40[12];
	uint	riwtar3;	/* 0xd0d80 - RapidIO Inbound Window Translation Address Register 3 */
	char	res41[4];
	uint	riwbar3;	/* 0xd0d88 - RapidIO Inbound Window Base Address Register 3 */
	char	res42[4];
	uint	riwar3;		/* 0xd0d90 - RapidIO Inbound Attributes Register 3 */
	char	res43[12];
	uint	riwtar2;	/* 0xd0da0 - RapidIO Inbound Window Translation Address Register 2 */
	char	res44[4];
	uint	riwbar2;	/* 0xd0da8 - RapidIO Inbound Window Base Address Register 2 */
	char	res45[4];
	uint	riwar2;		/* 0xd0db0 - RapidIO Inbound Attributes Register 2 */
	char	res46[12];
	uint	riwtar1;	/* 0xd0dc0 - RapidIO Inbound Window Translation Address Register 1 */
	char	res47[4];
	uint	riwbar1;	/* 0xd0dc8 - RapidIO Inbound Window Base Address Register 1 */
	char	res48[4];
	uint	riwar1;		/* 0xd0dd0 - RapidIO Inbound Attributes Register 1 */
	char	res49[12];
	uint	riwtar0;	/* 0xd0de0 - RapidIO Inbound Window Translation Address Register 0 */
	char	res50[12];
	uint	riwar0;		/* 0xd0df0 - RapidIO Inbound Attributes Register 0 */
	char	res51[12];
	uint	pnfedr;		/* 0xd0e00 - Port Notification/Fatal Error Detect Register */
	uint	pnfedir;	/* 0xd0e04 - Port Notification/Fatal Error Detect Register */
	uint	pnfeier;	/* 0xd0e08 - Port Notification/Fatal Error Interrupt Enable Register */
	uint	pecr;		/* 0xd0e0c - Port Error Control Register */
	uint	pepcsr0;	/* 0xd0e10 - Port Error Packet/Control Symbol Register 0 */
	uint	pepr1;		/* 0xd0e14 - Port Error Packet Register 1 */
	uint	pepr2;		/* 0xd0e18 - Port Error Packet Register 2 */
	char	res52[4];
	uint	predr;		/* 0xd0e20 - Port Recoverable Error Detect Register */
	char	res53[4];
	uint	pertr;		/* 0xd0e28 - Port Error Recovery Threshold Register */
	uint	prtr;		/* 0xd0e2c - Port Retry Threshold Register */
	char	res54[464];
	uint	omr;		/* 0xd1000 - Outbound Mode Register */
	uint	osr;		/* 0xd1004 - Outbound Status Register */
	uint	eodqtpar;	/* 0xd1008 - Extended Outbound Descriptor Queue Tail Pointer Address Register */
	uint	odqtpar;	/* 0xd100c - Outbound Descriptor Queue Tail Pointer Address Register */
	uint	eosar;		/* 0xd1010 - Extended Outbound Unit Source Address Register */
	uint	osar;		/* 0xd1014 - Outbound Unit Source Address Register */
	uint	odpr;		/* 0xd1018 - Outbound Destination Port Register */
	uint	odatr;		/* 0xd101c - Outbound Destination Attributes Register */
	uint	odcr;		/* 0xd1020 - Outbound Doubleword Count Register */
	uint	eodqhpar;	/* 0xd1024 - Extended Outbound Descriptor Queue Head Pointer Address Register */
	uint	odqhpar;	/* 0xd1028 - Outbound Descriptor Queue Head Pointer Address Register */
	char	res55[52];
	uint	imr;		/* 0xd1060 - Outbound Mode Register */
	uint	isr;		/* 0xd1064 - Inbound Status Register */
	uint	eidqtpar;	/* 0xd1068 - Extended Inbound Descriptor Queue Tail Pointer Address Register */
	uint	idqtpar;	/* 0xd106c - Inbound Descriptor Queue Tail Pointer Address Register */
	uint	eifqhpar;	/* 0xd1070 - Extended Inbound Frame Queue Head Pointer Address Register */
	uint	ifqhpar;	/* 0xd1074 - Inbound Frame Queue Head Pointer Address Register */
	char	res56[1000];
	uint	dmr;		/* 0xd1460 - Doorbell Mode Register */
	uint	dsr;		/* 0xd1464 - Doorbell Status Register */
	uint	edqtpar;	/* 0xd1468 - Extended Doorbell Queue Tail Pointer Address Register */
	uint	dqtpar;		/* 0xd146c - Doorbell Queue Tail Pointer Address Register */
	uint	edqhpar;	/* 0xd1470 - Extended Doorbell Queue Head Pointer Address Register */
	uint	dqhpar;		/* 0xd1474 - Doorbell Queue Head Pointer Address Register */
	char	res57[104];
	uint	pwmr;		/* 0xd14e0 - Port-Write Mode Register */
	uint	pwsr;		/* 0xd14e4 - Port-Write Status Register */
	uint	epwqbar;	/* 0xd14e8 - Extended Port-Write Queue Base Address Register */
	uint	pwqbar;		/* 0xd14ec - Port-Write Queue Base Address Register */
	char	res58[60176];
} ccsr_rio_t;

/* Quick Engine Block Pin Muxing Registers (0xe_0100 - 0xe_01bf) */
typedef struct par_io {
	uint	cpodr;		/* 0x100 */
	uint	cpdat;		/* 0x104 */
	uint	cpdir1;		/* 0x108 */
	uint	cpdir2;		/* 0x10c */
	uint	cppar1;		/* 0x110 */
	uint	cppar2;		/* 0x114 */
	char	res[8];
}par_io_t;

/*
 * Global Utilities Register Block(0xe_0000-0xf_ffff)
 */
typedef struct ccsr_gur {
	uint	porpllsr;	/* 0xe0000 - POR PLL ratio status register */
	uint	porbmsr;	/* 0xe0004 - POR boot mode status register */
#define MPC85xx_PORBMSR_HA 		0x00070000
	uint	porimpscr;	/* 0xe0008 - POR I/O impedance status and control register */
	uint	pordevsr;	/* 0xe000c - POR I/O device status regsiter */
#define MPC85xx_PORDEVSR_SGMII1_DIS	0x20000000
#define MPC85xx_PORDEVSR_SGMII2_DIS	0x10000000
#define MPC85xx_PORDEVSR_SGMII3_DIS	0x08000000
#define MPC85xx_PORDEVSR_SGMII4_DIS	0x04000000
#define MPC85xx_PORDEVSR_IO_SEL		0x00380000
#define MPC85xx_PORDEVSR_PCI2_ARB 	0x00040000
#define MPC85xx_PORDEVSR_PCI1_ARB 	0x00020000
#define MPC85xx_PORDEVSR_PCI1_PCI32 	0x00010000
#define MPC85xx_PORDEVSR_PCI1_SPD 	0x00008000
#define MPC85xx_PORDEVSR_PCI2_SPD 	0x00004000
#define MPC85xx_PORDEVSR_DRAM_RTYPE	0x00000060
#define MPC85xx_PORDEVSR_RIO_CTLS 	0x00000008
#define MPC85xx_PORDEVSR_RIO_DEV_ID	0x00000007
	uint	pordbgmsr;	/* 0xe0010 - POR debug mode status register */
	char	res1[12];
	uint	gpporcr;	/* 0xe0020 - General-purpose POR configuration register */
	char	res2[12];
	uint	gpiocr;		/* 0xe0030 - GPIO control register */
	char	res3[12];
	uint	gpoutdr;	/* 0xe0040 - General-purpose output data register */
	char	res4[12];
	uint	gpindr;		/* 0xe0050 - General-purpose input data register */
	char	res5[12];
	uint	pmuxcr;		/* 0xe0060 - Alternate function signal multiplex control */
	char	res6[12];
	uint	devdisr;	/* 0xe0070 - Device disable control */
#define MPC85xx_DEVDISR_PCI1		0x80000000
#define MPC85xx_DEVDISR_PCI2		0x40000000
#define MPC85xx_DEVDISR_PCIE		0x20000000
#define MPC85xx_DEVDISR_LBC		0x08000000
#define MPC85xx_DEVDISR_PCIE2		0x04000000
#define MPC85xx_DEVDISR_PCIE3		0x02000000
#define MPC85xx_DEVDISR_SEC		0x01000000
#define MPC85xx_DEVDISR_SRIO		0x00080000
#define MPC85xx_DEVDISR_RMSG		0x00040000
#define MPC85xx_DEVDISR_DDR 		0x00010000
#define MPC85xx_DEVDISR_CPU 		0x00008000
#define MPC85xx_DEVDISR_CPU0 		MPC85xx_DEVDISR_CPU
#define MPC85xx_DEVDISR_TB 		0x00004000
#define MPC85xx_DEVDISR_TB0 		MPC85xx_DEVDISR_TB
#define MPC85xx_DEVDISR_CPU1 		0x00002000
#define MPC85xx_DEVDISR_TB1 		0x00001000
#define MPC85xx_DEVDISR_DMA		0x00000400
#define MPC85xx_DEVDISR_TSEC1		0x00000080
#define MPC85xx_DEVDISR_TSEC2		0x00000040
#define MPC85xx_DEVDISR_TSEC3		0x00000020
#define MPC85xx_DEVDISR_TSEC4		0x00000010
#define MPC85xx_DEVDISR_I2C		0x00000004
#define MPC85xx_DEVDISR_DUART		0x00000002
	char	res7[12];
	uint	powmgtcsr;	/* 0xe0080 - Power management status and control register */
	char	res8[12];
	uint	mcpsumr;	/* 0xe0090 - Machine check summary register */
	char	res9[12];
	uint	pvr;		/* 0xe00a0 - Processor version register */
	uint	svr;		/* 0xe00a4 - System version register */
	char	res10a[8];
	uint	rstcr;		/* 0xe00b0 - Reset control register */
#ifdef CONFIG_MPC8568
	char	res10b[76];
	par_io_t qe_par_io[7];  /* 0xe0100 - 0xe01bf */
	char	res10c[3136];
#else
	char	res10b[3404];
#endif
	uint	clkocr;		/* 0xe0e00 - Clock out select register */
	char	res11[12];
	uint	ddrdllcr;	/* 0xe0e10 - DDR DLL control register */
	char	res12[12];
	uint	lbcdllcr;	/* 0xe0e20 - LBC DLL control register */
	char	res13[248];
	uint	lbiuiplldcr0;	/* 0xe0f1c -- LBIU PLL Debug Reg 0 */
	uint	lbiuiplldcr1;	/* 0xe0f20 -- LBIU PLL Debug Reg 1 */
	uint	ddrioovcr;	/* 0xe0f24 - DDR IO Override Control */
	uint	res14;		/* 0xe0f28 */
	uint	tsec34ioovcr;	/* 0xe0f2c - eTSEC 3/4 IO override control */
	char	res15[61648];	/* 0xe0f30 to 0xefffff */
} ccsr_gur_t;

#define PORDEVSR_PCI	(0x00800000)	/* PCI Mode */

#define CFG_MPC85xx_GUTS_OFFSET	(0xE0000)
#define CFG_MPC85xx_GUTS_ADDR	(CFG_IMMR + CFG_MPC85xx_GUTS_OFFSET)
#define CFG_MPC85xx_ECM_OFFSET	(0x0000)
#define CFG_MPC85xx_ECM_ADDR	(CFG_IMMR + CFG_MPC85xx_ECM_OFFSET)
#define CFG_MPC85xx_DDR_OFFSET	(0x2000)
#define CFG_MPC85xx_DDR_ADDR	(CFG_IMMR + CFG_MPC85xx_DDR_OFFSET)
#define CFG_MPC85xx_LBC_OFFSET	(0x5000)
#define CFG_MPC85xx_LBC_ADDR	(CFG_IMMR + CFG_MPC85xx_LBC_OFFSET)
#define CFG_MPC85xx_PCIX_OFFSET	(0x8000)
#define CFG_MPC85xx_PCIX_ADDR	(CFG_IMMR + CFG_MPC85xx_PCIX_OFFSET)
#define CFG_MPC85xx_PCIX2_OFFSET	(0x9000)
#define CFG_MPC85xx_PCIX2_ADDR	(CFG_IMMR + CFG_MPC85xx_PCIX2_OFFSET)
#define CFG_MPC85xx_L2_OFFSET	(0x20000)
#define CFG_MPC85xx_L2_ADDR	(CFG_IMMR + CFG_MPC85xx_L2_OFFSET)
#define CFG_MPC85xx_DMA_OFFSET	(0x21000)
#define CFG_MPC85xx_DMA_ADDR	(CFG_IMMR + CFG_MPC85xx_DMA_OFFSET)
#define CFG_MPC85xx_PIC_OFFSET	(0x40000)
#define CFG_MPC85xx_PIC_ADDR	(CFG_IMMR + CFG_MPC85xx_PIC_OFFSET)
#define CFG_MPC85xx_CPM_OFFSET	(0x80000)
#define CFG_MPC85xx_CPM_ADDR	(CFG_IMMR + CFG_MPC85xx_CPM_OFFSET)

#endif /*__IMMAP_85xx__*/
