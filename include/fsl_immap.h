/*
 * Common internal memory map for some Freescale SoCs
 *
 * Copyright 2013-2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_IMMAP_H
#define __FSL_IMMAP_H
/*
 * DDR memory controller registers
 * This structure works for mpc83xx (DDR2 and DDR3), mpc85xx, mpc86xx.
 */
struct ccsr_ddr {
	u32	cs0_bnds;		/* Chip Select 0 Memory Bounds */
	u8	res_04[4];
	u32	cs1_bnds;		/* Chip Select 1 Memory Bounds */
	u8	res_0c[4];
	u32	cs2_bnds;		/* Chip Select 2 Memory Bounds */
	u8	res_14[4];
	u32	cs3_bnds;		/* Chip Select 3 Memory Bounds */
	u8	res_1c[100];
	u32	cs0_config;		/* Chip Select Configuration */
	u32	cs1_config;		/* Chip Select Configuration */
	u32	cs2_config;		/* Chip Select Configuration */
	u32	cs3_config;		/* Chip Select Configuration */
	u8	res_90[48];
	u32	cs0_config_2;		/* Chip Select Configuration 2 */
	u32	cs1_config_2;		/* Chip Select Configuration 2 */
	u32	cs2_config_2;		/* Chip Select Configuration 2 */
	u32	cs3_config_2;		/* Chip Select Configuration 2 */
	u8	res_d0[48];
	u32	timing_cfg_3;		/* SDRAM Timing Configuration 3 */
	u32	timing_cfg_0;		/* SDRAM Timing Configuration 0 */
	u32	timing_cfg_1;		/* SDRAM Timing Configuration 1 */
	u32	timing_cfg_2;		/* SDRAM Timing Configuration 2 */
	u32	sdram_cfg;		/* SDRAM Control Configuration */
	u32	sdram_cfg_2;		/* SDRAM Control Configuration 2 */
	u32	sdram_mode;		/* SDRAM Mode Configuration */
	u32	sdram_mode_2;		/* SDRAM Mode Configuration 2 */
	u32	sdram_md_cntl;		/* SDRAM Mode Control */
	u32	sdram_interval;		/* SDRAM Interval Configuration */
	u32	sdram_data_init;	/* SDRAM Data initialization */
	u8	res_12c[4];
	u32	sdram_clk_cntl;		/* SDRAM Clock Control */
	u8	res_134[20];
	u32	init_addr;		/* training init addr */
	u32	init_ext_addr;		/* training init extended addr */
	u8	res_150[16];
	u32	timing_cfg_4;		/* SDRAM Timing Configuration 4 */
	u32	timing_cfg_5;		/* SDRAM Timing Configuration 5 */
	u32	timing_cfg_6;		/* SDRAM Timing Configuration 6 */
	u32	timing_cfg_7;		/* SDRAM Timing Configuration 7 */
	u32	ddr_zq_cntl;		/* ZQ calibration control*/
	u32	ddr_wrlvl_cntl;		/* write leveling control*/
	u8	reg_178[4];
	u32	ddr_sr_cntr;		/* self refresh counter */
	u32	ddr_sdram_rcw_1;	/* Control Words 1 */
	u32	ddr_sdram_rcw_2;	/* Control Words 2 */
	u8	reg_188[8];
	u32	ddr_wrlvl_cntl_2;	/* write leveling control 2 */
	u32	ddr_wrlvl_cntl_3;	/* write leveling control 3 */
	u8	res_198[0x1a0-0x198];
	u32	ddr_sdram_rcw_3;
	u32	ddr_sdram_rcw_4;
	u32	ddr_sdram_rcw_5;
	u32	ddr_sdram_rcw_6;
	u8	res_1b0[0x200-0x1b0];
	u32	sdram_mode_3;		/* SDRAM Mode Configuration 3 */
	u32	sdram_mode_4;		/* SDRAM Mode Configuration 4 */
	u32	sdram_mode_5;		/* SDRAM Mode Configuration 5 */
	u32	sdram_mode_6;		/* SDRAM Mode Configuration 6 */
	u32	sdram_mode_7;		/* SDRAM Mode Configuration 7 */
	u32	sdram_mode_8;		/* SDRAM Mode Configuration 8 */
	u8	res_218[0x220-0x218];
	u32	sdram_mode_9;		/* SDRAM Mode Configuration 9 */
	u32	sdram_mode_10;		/* SDRAM Mode Configuration 10 */
	u32	sdram_mode_11;		/* SDRAM Mode Configuration 11 */
	u32	sdram_mode_12;		/* SDRAM Mode Configuration 12 */
	u32	sdram_mode_13;		/* SDRAM Mode Configuration 13 */
	u32	sdram_mode_14;		/* SDRAM Mode Configuration 14 */
	u32	sdram_mode_15;		/* SDRAM Mode Configuration 15 */
	u32	sdram_mode_16;		/* SDRAM Mode Configuration 16 */
	u8	res_240[0x250-0x240];
	u32	timing_cfg_8;		/* SDRAM Timing Configuration 8 */
	u32	timing_cfg_9;		/* SDRAM Timing Configuration 9 */
	u8	res_258[0x260-0x258];
	u32	sdram_cfg_3;
	u8	res_264[0x400-0x264];
	u32	dq_map_0;
	u32	dq_map_1;
	u32	dq_map_2;
	u32	dq_map_3;
	u8	res_410[0xb20-0x410];
	u32	ddr_dsr1;		/* Debug Status 1 */
	u32	ddr_dsr2;		/* Debug Status 2 */
	u32	ddr_cdr1;		/* Control Driver 1 */
	u32	ddr_cdr2;		/* Control Driver 2 */
	u8	res_b30[200];
	u32	ip_rev1;		/* IP Block Revision 1 */
	u32	ip_rev2;		/* IP Block Revision 2 */
	u32	eor;			/* Enhanced Optimization Register */
	u8	res_c04[252];
	u32	mtcr;			/* Memory Test Control Register */
	u8	res_d04[28];
	u32	mtp1;			/* Memory Test Pattern 1 */
	u32	mtp2;			/* Memory Test Pattern 2 */
	u32	mtp3;			/* Memory Test Pattern 3 */
	u32	mtp4;			/* Memory Test Pattern 4 */
	u32	mtp5;			/* Memory Test Pattern 5 */
	u32	mtp6;			/* Memory Test Pattern 6 */
	u32	mtp7;			/* Memory Test Pattern 7 */
	u32	mtp8;			/* Memory Test Pattern 8 */
	u32	mtp9;			/* Memory Test Pattern 9 */
	u32	mtp10;			/* Memory Test Pattern 10 */
	u8	res_d48[184];
	u32	data_err_inject_hi;	/* Data Path Err Injection Mask High */
	u32	data_err_inject_lo;	/* Data Path Err Injection Mask Low */
	u32	ecc_err_inject;		/* Data Path Err Injection Mask ECC */
	u8	res_e0c[20];
	u32	capture_data_hi;	/* Data Path Read Capture High */
	u32	capture_data_lo;	/* Data Path Read Capture Low */
	u32	capture_ecc;		/* Data Path Read Capture ECC */
	u8	res_e2c[20];
	u32	err_detect;		/* Error Detect */
	u32	err_disable;		/* Error Disable */
	u32	err_int_en;
	u32	capture_attributes;	/* Error Attrs Capture */
	u32	capture_address;	/* Error Addr Capture */
	u32	capture_ext_address;	/* Error Extended Addr Capture */
	u32	err_sbe;		/* Single-Bit ECC Error Management */
	u8	res_e5c[164];
	u32	debug[32];		/* debug_1 to debug_32 */
	u8	res_f80[128];
};
#endif /* __FSL_IMMAP_H */
