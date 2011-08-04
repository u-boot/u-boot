/*
 * OMAP44xx EMIF header
 *
 * Copyright (C) 2009-2010 Texas Instruments, Inc.
 *
 * Aneesh V <aneesh@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _EMIF_H_
#define _EMIF_H_
#include <asm/types.h>
#include <common.h>

/* Base address */
#define OMAP44XX_EMIF1				0x4c000000
#define OMAP44XX_EMIF2				0x4d000000

/* Registers shifts and masks */

/* EMIF_MOD_ID_REV */
#define OMAP44XX_REG_SCHEME_SHIFT			30
#define OMAP44XX_REG_SCHEME_MASK			(0x3 << 30)
#define OMAP44XX_REG_MODULE_ID_SHIFT			16
#define OMAP44XX_REG_MODULE_ID_MASK			(0xfff << 16)
#define OMAP44XX_REG_RTL_VERSION_SHIFT			11
#define OMAP44XX_REG_RTL_VERSION_MASK			(0x1f << 11)
#define OMAP44XX_REG_MAJOR_REVISION_SHIFT		8
#define OMAP44XX_REG_MAJOR_REVISION_MASK		(0x7 << 8)
#define OMAP44XX_REG_MINOR_REVISION_SHIFT		0
#define OMAP44XX_REG_MINOR_REVISION_MASK		(0x3f << 0)

/* STATUS */
#define OMAP44XX_REG_BE_SHIFT				31
#define OMAP44XX_REG_BE_MASK				(1 << 31)
#define OMAP44XX_REG_DUAL_CLK_MODE_SHIFT		30
#define OMAP44XX_REG_DUAL_CLK_MODE_MASK			(1 << 30)
#define OMAP44XX_REG_FAST_INIT_SHIFT			29
#define OMAP44XX_REG_FAST_INIT_MASK			(1 << 29)
#define OMAP44XX_REG_PHY_DLL_READY_SHIFT		2
#define OMAP44XX_REG_PHY_DLL_READY_MASK			(1 << 2)

/* SDRAM_CONFIG */
#define OMAP44XX_REG_SDRAM_TYPE_SHIFT			29
#define OMAP44XX_REG_SDRAM_TYPE_MASK			(0x7 << 29)
#define OMAP44XX_REG_IBANK_POS_SHIFT			27
#define OMAP44XX_REG_IBANK_POS_MASK			(0x3 << 27)
#define OMAP44XX_REG_DDR_TERM_SHIFT			24
#define OMAP44XX_REG_DDR_TERM_MASK			(0x7 << 24)
#define OMAP44XX_REG_DDR2_DDQS_SHIFT			23
#define OMAP44XX_REG_DDR2_DDQS_MASK			(1 << 23)
#define OMAP44XX_REG_DYN_ODT_SHIFT			21
#define OMAP44XX_REG_DYN_ODT_MASK			(0x3 << 21)
#define OMAP44XX_REG_DDR_DISABLE_DLL_SHIFT		20
#define OMAP44XX_REG_DDR_DISABLE_DLL_MASK		(1 << 20)
#define OMAP44XX_REG_SDRAM_DRIVE_SHIFT			18
#define OMAP44XX_REG_SDRAM_DRIVE_MASK			(0x3 << 18)
#define OMAP44XX_REG_CWL_SHIFT				16
#define OMAP44XX_REG_CWL_MASK				(0x3 << 16)
#define OMAP44XX_REG_NARROW_MODE_SHIFT			14
#define OMAP44XX_REG_NARROW_MODE_MASK			(0x3 << 14)
#define OMAP44XX_REG_CL_SHIFT				10
#define OMAP44XX_REG_CL_MASK				(0xf << 10)
#define OMAP44XX_REG_ROWSIZE_SHIFT			7
#define OMAP44XX_REG_ROWSIZE_MASK			(0x7 << 7)
#define OMAP44XX_REG_IBANK_SHIFT			4
#define OMAP44XX_REG_IBANK_MASK				(0x7 << 4)
#define OMAP44XX_REG_EBANK_SHIFT			3
#define OMAP44XX_REG_EBANK_MASK				(1 << 3)
#define OMAP44XX_REG_PAGESIZE_SHIFT			0
#define OMAP44XX_REG_PAGESIZE_MASK			(0x7 << 0)

/* SDRAM_CONFIG_2 */
#define OMAP44XX_REG_CS1NVMEN_SHIFT			30
#define OMAP44XX_REG_CS1NVMEN_MASK			(1 << 30)
#define OMAP44XX_REG_EBANK_POS_SHIFT			27
#define OMAP44XX_REG_EBANK_POS_MASK			(1 << 27)
#define OMAP44XX_REG_RDBNUM_SHIFT			4
#define OMAP44XX_REG_RDBNUM_MASK			(0x3 << 4)
#define OMAP44XX_REG_RDBSIZE_SHIFT			0
#define OMAP44XX_REG_RDBSIZE_MASK			(0x7 << 0)

/* SDRAM_REF_CTRL */
#define OMAP44XX_REG_INITREF_DIS_SHIFT			31
#define OMAP44XX_REG_INITREF_DIS_MASK			(1 << 31)
#define OMAP44XX_REG_SRT_SHIFT				29
#define OMAP44XX_REG_SRT_MASK				(1 << 29)
#define OMAP44XX_REG_ASR_SHIFT				28
#define OMAP44XX_REG_ASR_MASK				(1 << 28)
#define OMAP44XX_REG_PASR_SHIFT				24
#define OMAP44XX_REG_PASR_MASK				(0x7 << 24)
#define OMAP44XX_REG_REFRESH_RATE_SHIFT			0
#define OMAP44XX_REG_REFRESH_RATE_MASK			(0xffff << 0)

/* SDRAM_REF_CTRL_SHDW */
#define OMAP44XX_REG_REFRESH_RATE_SHDW_SHIFT		0
#define OMAP44XX_REG_REFRESH_RATE_SHDW_MASK		(0xffff << 0)

/* SDRAM_TIM_1 */
#define OMAP44XX_REG_T_RP_SHIFT				25
#define OMAP44XX_REG_T_RP_MASK				(0xf << 25)
#define OMAP44XX_REG_T_RCD_SHIFT			21
#define OMAP44XX_REG_T_RCD_MASK				(0xf << 21)
#define OMAP44XX_REG_T_WR_SHIFT				17
#define OMAP44XX_REG_T_WR_MASK				(0xf << 17)
#define OMAP44XX_REG_T_RAS_SHIFT			12
#define OMAP44XX_REG_T_RAS_MASK				(0x1f << 12)
#define OMAP44XX_REG_T_RC_SHIFT				6
#define OMAP44XX_REG_T_RC_MASK				(0x3f << 6)
#define OMAP44XX_REG_T_RRD_SHIFT			3
#define OMAP44XX_REG_T_RRD_MASK				(0x7 << 3)
#define OMAP44XX_REG_T_WTR_SHIFT			0
#define OMAP44XX_REG_T_WTR_MASK				(0x7 << 0)

/* SDRAM_TIM_1_SHDW */
#define OMAP44XX_REG_T_RP_SHDW_SHIFT			25
#define OMAP44XX_REG_T_RP_SHDW_MASK			(0xf << 25)
#define OMAP44XX_REG_T_RCD_SHDW_SHIFT			21
#define OMAP44XX_REG_T_RCD_SHDW_MASK			(0xf << 21)
#define OMAP44XX_REG_T_WR_SHDW_SHIFT			17
#define OMAP44XX_REG_T_WR_SHDW_MASK			(0xf << 17)
#define OMAP44XX_REG_T_RAS_SHDW_SHIFT			12
#define OMAP44XX_REG_T_RAS_SHDW_MASK			(0x1f << 12)
#define OMAP44XX_REG_T_RC_SHDW_SHIFT			6
#define OMAP44XX_REG_T_RC_SHDW_MASK			(0x3f << 6)
#define OMAP44XX_REG_T_RRD_SHDW_SHIFT			3
#define OMAP44XX_REG_T_RRD_SHDW_MASK			(0x7 << 3)
#define OMAP44XX_REG_T_WTR_SHDW_SHIFT			0
#define OMAP44XX_REG_T_WTR_SHDW_MASK			(0x7 << 0)

/* SDRAM_TIM_2 */
#define OMAP44XX_REG_T_XP_SHIFT				28
#define OMAP44XX_REG_T_XP_MASK				(0x7 << 28)
#define OMAP44XX_REG_T_ODT_SHIFT			25
#define OMAP44XX_REG_T_ODT_MASK				(0x7 << 25)
#define OMAP44XX_REG_T_XSNR_SHIFT			16
#define OMAP44XX_REG_T_XSNR_MASK			(0x1ff << 16)
#define OMAP44XX_REG_T_XSRD_SHIFT			6
#define OMAP44XX_REG_T_XSRD_MASK			(0x3ff << 6)
#define OMAP44XX_REG_T_RTP_SHIFT			3
#define OMAP44XX_REG_T_RTP_MASK				(0x7 << 3)
#define OMAP44XX_REG_T_CKE_SHIFT			0
#define OMAP44XX_REG_T_CKE_MASK				(0x7 << 0)

/* SDRAM_TIM_2_SHDW */
#define OMAP44XX_REG_T_XP_SHDW_SHIFT			28
#define OMAP44XX_REG_T_XP_SHDW_MASK			(0x7 << 28)
#define OMAP44XX_REG_T_ODT_SHDW_SHIFT			25
#define OMAP44XX_REG_T_ODT_SHDW_MASK			(0x7 << 25)
#define OMAP44XX_REG_T_XSNR_SHDW_SHIFT			16
#define OMAP44XX_REG_T_XSNR_SHDW_MASK			(0x1ff << 16)
#define OMAP44XX_REG_T_XSRD_SHDW_SHIFT			6
#define OMAP44XX_REG_T_XSRD_SHDW_MASK			(0x3ff << 6)
#define OMAP44XX_REG_T_RTP_SHDW_SHIFT			3
#define OMAP44XX_REG_T_RTP_SHDW_MASK			(0x7 << 3)
#define OMAP44XX_REG_T_CKE_SHDW_SHIFT			0
#define OMAP44XX_REG_T_CKE_SHDW_MASK			(0x7 << 0)

/* SDRAM_TIM_3 */
#define OMAP44XX_REG_T_CKESR_SHIFT			21
#define OMAP44XX_REG_T_CKESR_MASK			(0x7 << 21)
#define OMAP44XX_REG_ZQ_ZQCS_SHIFT			15
#define OMAP44XX_REG_ZQ_ZQCS_MASK			(0x3f << 15)
#define OMAP44XX_REG_T_TDQSCKMAX_SHIFT			13
#define OMAP44XX_REG_T_TDQSCKMAX_MASK			(0x3 << 13)
#define OMAP44XX_REG_T_RFC_SHIFT			4
#define OMAP44XX_REG_T_RFC_MASK				(0x1ff << 4)
#define OMAP44XX_REG_T_RAS_MAX_SHIFT			0
#define OMAP44XX_REG_T_RAS_MAX_MASK			(0xf << 0)

/* SDRAM_TIM_3_SHDW */
#define OMAP44XX_REG_T_CKESR_SHDW_SHIFT			21
#define OMAP44XX_REG_T_CKESR_SHDW_MASK			(0x7 << 21)
#define OMAP44XX_REG_ZQ_ZQCS_SHDW_SHIFT			15
#define OMAP44XX_REG_ZQ_ZQCS_SHDW_MASK			(0x3f << 15)
#define OMAP44XX_REG_T_TDQSCKMAX_SHDW_SHIFT		13
#define OMAP44XX_REG_T_TDQSCKMAX_SHDW_MASK		(0x3 << 13)
#define OMAP44XX_REG_T_RFC_SHDW_SHIFT			4
#define OMAP44XX_REG_T_RFC_SHDW_MASK			(0x1ff << 4)
#define OMAP44XX_REG_T_RAS_MAX_SHDW_SHIFT		0
#define OMAP44XX_REG_T_RAS_MAX_SHDW_MASK		(0xf << 0)

/* LPDDR2_NVM_TIM */
#define OMAP44XX_REG_NVM_T_XP_SHIFT			28
#define OMAP44XX_REG_NVM_T_XP_MASK			(0x7 << 28)
#define OMAP44XX_REG_NVM_T_WTR_SHIFT			24
#define OMAP44XX_REG_NVM_T_WTR_MASK			(0x7 << 24)
#define OMAP44XX_REG_NVM_T_RP_SHIFT			20
#define OMAP44XX_REG_NVM_T_RP_MASK			(0xf << 20)
#define OMAP44XX_REG_NVM_T_WRA_SHIFT			16
#define OMAP44XX_REG_NVM_T_WRA_MASK			(0xf << 16)
#define OMAP44XX_REG_NVM_T_RRD_SHIFT			8
#define OMAP44XX_REG_NVM_T_RRD_MASK			(0xff << 8)
#define OMAP44XX_REG_NVM_T_RCDMIN_SHIFT			0
#define OMAP44XX_REG_NVM_T_RCDMIN_MASK			(0xff << 0)

/* LPDDR2_NVM_TIM_SHDW */
#define OMAP44XX_REG_NVM_T_XP_SHDW_SHIFT		28
#define OMAP44XX_REG_NVM_T_XP_SHDW_MASK			(0x7 << 28)
#define OMAP44XX_REG_NVM_T_WTR_SHDW_SHIFT		24
#define OMAP44XX_REG_NVM_T_WTR_SHDW_MASK		(0x7 << 24)
#define OMAP44XX_REG_NVM_T_RP_SHDW_SHIFT		20
#define OMAP44XX_REG_NVM_T_RP_SHDW_MASK			(0xf << 20)
#define OMAP44XX_REG_NVM_T_WRA_SHDW_SHIFT		16
#define OMAP44XX_REG_NVM_T_WRA_SHDW_MASK		(0xf << 16)
#define OMAP44XX_REG_NVM_T_RRD_SHDW_SHIFT		8
#define OMAP44XX_REG_NVM_T_RRD_SHDW_MASK		(0xff << 8)
#define OMAP44XX_REG_NVM_T_RCDMIN_SHDW_SHIFT		0
#define OMAP44XX_REG_NVM_T_RCDMIN_SHDW_MASK		(0xff << 0)

/* PWR_MGMT_CTRL */
#define OMAP44XX_REG_IDLEMODE_SHIFT			30
#define OMAP44XX_REG_IDLEMODE_MASK			(0x3 << 30)
#define OMAP44XX_REG_PD_TIM_SHIFT			12
#define OMAP44XX_REG_PD_TIM_MASK			(0xf << 12)
#define OMAP44XX_REG_DPD_EN_SHIFT			11
#define OMAP44XX_REG_DPD_EN_MASK			(1 << 11)
#define OMAP44XX_REG_LP_MODE_SHIFT			8
#define OMAP44XX_REG_LP_MODE_MASK			(0x7 << 8)
#define OMAP44XX_REG_SR_TIM_SHIFT			4
#define OMAP44XX_REG_SR_TIM_MASK			(0xf << 4)
#define OMAP44XX_REG_CS_TIM_SHIFT			0
#define OMAP44XX_REG_CS_TIM_MASK			(0xf << 0)

/* PWR_MGMT_CTRL_SHDW */
#define OMAP44XX_REG_PD_TIM_SHDW_SHIFT			8
#define OMAP44XX_REG_PD_TIM_SHDW_MASK			(0xf << 8)
#define OMAP44XX_REG_SR_TIM_SHDW_SHIFT			4
#define OMAP44XX_REG_SR_TIM_SHDW_MASK			(0xf << 4)
#define OMAP44XX_REG_CS_TIM_SHDW_SHIFT			0
#define OMAP44XX_REG_CS_TIM_SHDW_MASK			(0xf << 0)

/* LPDDR2_MODE_REG_DATA */
#define OMAP44XX_REG_VALUE_0_SHIFT			0
#define OMAP44XX_REG_VALUE_0_MASK			(0x7f << 0)

/* LPDDR2_MODE_REG_CFG */
#define OMAP44XX_REG_CS_SHIFT				31
#define OMAP44XX_REG_CS_MASK				(1 << 31)
#define OMAP44XX_REG_REFRESH_EN_SHIFT			30
#define OMAP44XX_REG_REFRESH_EN_MASK			(1 << 30)
#define OMAP44XX_REG_ADDRESS_SHIFT			0
#define OMAP44XX_REG_ADDRESS_MASK			(0xff << 0)

/* OCP_CONFIG */
#define OMAP44XX_REG_SYS_THRESH_MAX_SHIFT		24
#define OMAP44XX_REG_SYS_THRESH_MAX_MASK		(0xf << 24)
#define OMAP44XX_REG_MPU_THRESH_MAX_SHIFT		20
#define OMAP44XX_REG_MPU_THRESH_MAX_MASK		(0xf << 20)
#define OMAP44XX_REG_LL_THRESH_MAX_SHIFT		16
#define OMAP44XX_REG_LL_THRESH_MAX_MASK			(0xf << 16)
#define OMAP44XX_REG_PR_OLD_COUNT_SHIFT			0
#define OMAP44XX_REG_PR_OLD_COUNT_MASK			(0xff << 0)

/* OCP_CFG_VAL_1 */
#define OMAP44XX_REG_SYS_BUS_WIDTH_SHIFT		30
#define OMAP44XX_REG_SYS_BUS_WIDTH_MASK			(0x3 << 30)
#define OMAP44XX_REG_LL_BUS_WIDTH_SHIFT			28
#define OMAP44XX_REG_LL_BUS_WIDTH_MASK			(0x3 << 28)
#define OMAP44XX_REG_WR_FIFO_DEPTH_SHIFT		8
#define OMAP44XX_REG_WR_FIFO_DEPTH_MASK			(0xff << 8)
#define OMAP44XX_REG_CMD_FIFO_DEPTH_SHIFT		0
#define OMAP44XX_REG_CMD_FIFO_DEPTH_MASK		(0xff << 0)

/* OCP_CFG_VAL_2 */
#define OMAP44XX_REG_RREG_FIFO_DEPTH_SHIFT		16
#define OMAP44XX_REG_RREG_FIFO_DEPTH_MASK		(0xff << 16)
#define OMAP44XX_REG_RSD_FIFO_DEPTH_SHIFT		8
#define OMAP44XX_REG_RSD_FIFO_DEPTH_MASK		(0xff << 8)
#define OMAP44XX_REG_RCMD_FIFO_DEPTH_SHIFT		0
#define OMAP44XX_REG_RCMD_FIFO_DEPTH_MASK		(0xff << 0)

/* IODFT_TLGC */
#define OMAP44XX_REG_TLEC_SHIFT				16
#define OMAP44XX_REG_TLEC_MASK				(0xffff << 16)
#define OMAP44XX_REG_MT_SHIFT				14
#define OMAP44XX_REG_MT_MASK				(1 << 14)
#define OMAP44XX_REG_ACT_CAP_EN_SHIFT			13
#define OMAP44XX_REG_ACT_CAP_EN_MASK			(1 << 13)
#define OMAP44XX_REG_OPG_LD_SHIFT			12
#define OMAP44XX_REG_OPG_LD_MASK			(1 << 12)
#define OMAP44XX_REG_RESET_PHY_SHIFT			10
#define OMAP44XX_REG_RESET_PHY_MASK			(1 << 10)
#define OMAP44XX_REG_MMS_SHIFT				8
#define OMAP44XX_REG_MMS_MASK				(1 << 8)
#define OMAP44XX_REG_MC_SHIFT				4
#define OMAP44XX_REG_MC_MASK				(0x3 << 4)
#define OMAP44XX_REG_PC_SHIFT				1
#define OMAP44XX_REG_PC_MASK				(0x7 << 1)
#define OMAP44XX_REG_TM_SHIFT				0
#define OMAP44XX_REG_TM_MASK				(1 << 0)

/* IODFT_CTRL_MISR_RSLT */
#define OMAP44XX_REG_DQM_TLMR_SHIFT			16
#define OMAP44XX_REG_DQM_TLMR_MASK			(0x3ff << 16)
#define OMAP44XX_REG_CTL_TLMR_SHIFT			0
#define OMAP44XX_REG_CTL_TLMR_MASK			(0x7ff << 0)

/* IODFT_ADDR_MISR_RSLT */
#define OMAP44XX_REG_ADDR_TLMR_SHIFT			0
#define OMAP44XX_REG_ADDR_TLMR_MASK			(0x1fffff << 0)

/* IODFT_DATA_MISR_RSLT_1 */
#define OMAP44XX_REG_DATA_TLMR_31_0_SHIFT		0
#define OMAP44XX_REG_DATA_TLMR_31_0_MASK		(0xffffffff << 0)

/* IODFT_DATA_MISR_RSLT_2 */
#define OMAP44XX_REG_DATA_TLMR_63_32_SHIFT		0
#define OMAP44XX_REG_DATA_TLMR_63_32_MASK		(0xffffffff << 0)

/* IODFT_DATA_MISR_RSLT_3 */
#define OMAP44XX_REG_DATA_TLMR_66_64_SHIFT		0
#define OMAP44XX_REG_DATA_TLMR_66_64_MASK		(0x7 << 0)

/* PERF_CNT_1 */
#define OMAP44XX_REG_COUNTER1_SHIFT			0
#define OMAP44XX_REG_COUNTER1_MASK			(0xffffffff << 0)

/* PERF_CNT_2 */
#define OMAP44XX_REG_COUNTER2_SHIFT			0
#define OMAP44XX_REG_COUNTER2_MASK			(0xffffffff << 0)

/* PERF_CNT_CFG */
#define OMAP44XX_REG_CNTR2_MCONNID_EN_SHIFT		31
#define OMAP44XX_REG_CNTR2_MCONNID_EN_MASK		(1 << 31)
#define OMAP44XX_REG_CNTR2_REGION_EN_SHIFT		30
#define OMAP44XX_REG_CNTR2_REGION_EN_MASK		(1 << 30)
#define OMAP44XX_REG_CNTR2_CFG_SHIFT			16
#define OMAP44XX_REG_CNTR2_CFG_MASK			(0xf << 16)
#define OMAP44XX_REG_CNTR1_MCONNID_EN_SHIFT		15
#define OMAP44XX_REG_CNTR1_MCONNID_EN_MASK		(1 << 15)
#define OMAP44XX_REG_CNTR1_REGION_EN_SHIFT		14
#define OMAP44XX_REG_CNTR1_REGION_EN_MASK		(1 << 14)
#define OMAP44XX_REG_CNTR1_CFG_SHIFT			0
#define OMAP44XX_REG_CNTR1_CFG_MASK			(0xf << 0)

/* PERF_CNT_SEL */
#define OMAP44XX_REG_MCONNID2_SHIFT			24
#define OMAP44XX_REG_MCONNID2_MASK			(0xff << 24)
#define OMAP44XX_REG_REGION_SEL2_SHIFT			16
#define OMAP44XX_REG_REGION_SEL2_MASK			(0x3 << 16)
#define OMAP44XX_REG_MCONNID1_SHIFT			8
#define OMAP44XX_REG_MCONNID1_MASK			(0xff << 8)
#define OMAP44XX_REG_REGION_SEL1_SHIFT			0
#define OMAP44XX_REG_REGION_SEL1_MASK			(0x3 << 0)

/* PERF_CNT_TIM */
#define OMAP44XX_REG_TOTAL_TIME_SHIFT			0
#define OMAP44XX_REG_TOTAL_TIME_MASK			(0xffffffff << 0)

/* READ_IDLE_CTRL */
#define OMAP44XX_REG_READ_IDLE_LEN_SHIFT		16
#define OMAP44XX_REG_READ_IDLE_LEN_MASK			(0xf << 16)
#define OMAP44XX_REG_READ_IDLE_INTERVAL_SHIFT		0
#define OMAP44XX_REG_READ_IDLE_INTERVAL_MASK		(0x1ff << 0)

/* READ_IDLE_CTRL_SHDW */
#define OMAP44XX_REG_READ_IDLE_LEN_SHDW_SHIFT		16
#define OMAP44XX_REG_READ_IDLE_LEN_SHDW_MASK		(0xf << 16)
#define OMAP44XX_REG_READ_IDLE_INTERVAL_SHDW_SHIFT	0
#define OMAP44XX_REG_READ_IDLE_INTERVAL_SHDW_MASK	(0x1ff << 0)

/* IRQ_EOI */
#define OMAP44XX_REG_EOI_SHIFT				0
#define OMAP44XX_REG_EOI_MASK				(1 << 0)

/* IRQSTATUS_RAW_SYS */
#define OMAP44XX_REG_DNV_SYS_SHIFT			2
#define OMAP44XX_REG_DNV_SYS_MASK			(1 << 2)
#define OMAP44XX_REG_TA_SYS_SHIFT			1
#define OMAP44XX_REG_TA_SYS_MASK			(1 << 1)
#define OMAP44XX_REG_ERR_SYS_SHIFT			0
#define OMAP44XX_REG_ERR_SYS_MASK			(1 << 0)

/* IRQSTATUS_RAW_LL */
#define OMAP44XX_REG_DNV_LL_SHIFT			2
#define OMAP44XX_REG_DNV_LL_MASK			(1 << 2)
#define OMAP44XX_REG_TA_LL_SHIFT			1
#define OMAP44XX_REG_TA_LL_MASK				(1 << 1)
#define OMAP44XX_REG_ERR_LL_SHIFT			0
#define OMAP44XX_REG_ERR_LL_MASK			(1 << 0)

/* IRQSTATUS_SYS */

/* IRQSTATUS_LL */

/* IRQENABLE_SET_SYS */
#define OMAP44XX_REG_EN_DNV_SYS_SHIFT			2
#define OMAP44XX_REG_EN_DNV_SYS_MASK			(1 << 2)
#define OMAP44XX_REG_EN_TA_SYS_SHIFT			1
#define OMAP44XX_REG_EN_TA_SYS_MASK			(1 << 1)
#define OMAP44XX_REG_EN_ERR_SYS_SHIFT			0
#define OMAP44XX_REG_EN_ERR_SYS_MASK			(1 << 0)

/* IRQENABLE_SET_LL */
#define OMAP44XX_REG_EN_DNV_LL_SHIFT			2
#define OMAP44XX_REG_EN_DNV_LL_MASK			(1 << 2)
#define OMAP44XX_REG_EN_TA_LL_SHIFT			1
#define OMAP44XX_REG_EN_TA_LL_MASK			(1 << 1)
#define OMAP44XX_REG_EN_ERR_LL_SHIFT			0
#define OMAP44XX_REG_EN_ERR_LL_MASK			(1 << 0)

/* IRQENABLE_CLR_SYS */

/* IRQENABLE_CLR_LL */

/* ZQ_CONFIG */
#define OMAP44XX_REG_ZQ_CS1EN_SHIFT			31
#define OMAP44XX_REG_ZQ_CS1EN_MASK			(1 << 31)
#define OMAP44XX_REG_ZQ_CS0EN_SHIFT			30
#define OMAP44XX_REG_ZQ_CS0EN_MASK			(1 << 30)
#define OMAP44XX_REG_ZQ_DUALCALEN_SHIFT			29
#define OMAP44XX_REG_ZQ_DUALCALEN_MASK			(1 << 29)
#define OMAP44XX_REG_ZQ_SFEXITEN_SHIFT			28
#define OMAP44XX_REG_ZQ_SFEXITEN_MASK			(1 << 28)
#define OMAP44XX_REG_ZQ_ZQINIT_MULT_SHIFT		18
#define OMAP44XX_REG_ZQ_ZQINIT_MULT_MASK		(0x3 << 18)
#define OMAP44XX_REG_ZQ_ZQCL_MULT_SHIFT			16
#define OMAP44XX_REG_ZQ_ZQCL_MULT_MASK			(0x3 << 16)
#define OMAP44XX_REG_ZQ_REFINTERVAL_SHIFT		0
#define OMAP44XX_REG_ZQ_REFINTERVAL_MASK		(0xffff << 0)

/* TEMP_ALERT_CONFIG */
#define OMAP44XX_REG_TA_CS1EN_SHIFT			31
#define OMAP44XX_REG_TA_CS1EN_MASK			(1 << 31)
#define OMAP44XX_REG_TA_CS0EN_SHIFT			30
#define OMAP44XX_REG_TA_CS0EN_MASK			(1 << 30)
#define OMAP44XX_REG_TA_SFEXITEN_SHIFT			28
#define OMAP44XX_REG_TA_SFEXITEN_MASK			(1 << 28)
#define OMAP44XX_REG_TA_DEVWDT_SHIFT			26
#define OMAP44XX_REG_TA_DEVWDT_MASK			(0x3 << 26)
#define OMAP44XX_REG_TA_DEVCNT_SHIFT			24
#define OMAP44XX_REG_TA_DEVCNT_MASK			(0x3 << 24)
#define OMAP44XX_REG_TA_REFINTERVAL_SHIFT		0
#define OMAP44XX_REG_TA_REFINTERVAL_MASK		(0x3fffff << 0)

/* OCP_ERR_LOG */
#define OMAP44XX_REG_MADDRSPACE_SHIFT			14
#define OMAP44XX_REG_MADDRSPACE_MASK			(0x3 << 14)
#define OMAP44XX_REG_MBURSTSEQ_SHIFT			11
#define OMAP44XX_REG_MBURSTSEQ_MASK			(0x7 << 11)
#define OMAP44XX_REG_MCMD_SHIFT				8
#define OMAP44XX_REG_MCMD_MASK				(0x7 << 8)
#define OMAP44XX_REG_MCONNID_SHIFT			0
#define OMAP44XX_REG_MCONNID_MASK			(0xff << 0)

/* DDR_PHY_CTRL_1 */
#define OMAP44XX_REG_DDR_PHY_CTRL_1_SHIFT		4
#define OMAP44XX_REG_DDR_PHY_CTRL_1_MASK		(0xfffffff << 4)
#define OMAP44XX_REG_READ_LATENCY_SHIFT			0
#define OMAP44XX_REG_READ_LATENCY_MASK			(0xf << 0)
#define OMAP44XX_REG_DLL_SLAVE_DLY_CTRL_SHIFT		4
#define OMAP44XX_REG_DLL_SLAVE_DLY_CTRL_MASK		(0xFF << 4)
#define OMAP44XX_EMIF_DDR_PHY_CTRL_1_BASE_VAL_SHIFT	12
#define OMAP44XX_EMIF_DDR_PHY_CTRL_1_BASE_VAL_MASK	(0xFFFFF << 12)

/* DDR_PHY_CTRL_1_SHDW */
#define OMAP44XX_REG_DDR_PHY_CTRL_1_SHDW_SHIFT		4
#define OMAP44XX_REG_DDR_PHY_CTRL_1_SHDW_MASK		(0xfffffff << 4)
#define OMAP44XX_REG_READ_LATENCY_SHDW_SHIFT		0
#define OMAP44XX_REG_READ_LATENCY_SHDW_MASK		(0xf << 0)
#define OMAP44XX_REG_DLL_SLAVE_DLY_CTRL_SHDW_SHIFT	4
#define OMAP44XX_REG_DLL_SLAVE_DLY_CTRL_SHDW_MASK	(0xFF << 4)
#define OMAP44XX_EMIF_DDR_PHY_CTRL_1_BASE_VAL_SHDW_SHIFT 12
#define OMAP44XX_EMIF_DDR_PHY_CTRL_1_BASE_VAL_SHDW_MASK	(0xFFFFF << 12)

/* DDR_PHY_CTRL_2 */
#define OMAP44XX_REG_DDR_PHY_CTRL_2_SHIFT		0
#define OMAP44XX_REG_DDR_PHY_CTRL_2_MASK		(0xffffffff << 0)

/* DMM */
#define OMAP44XX_DMM_LISA_MAP_BASE	0x4E000040

/* Memory Adapter (4460 onwards) */
#define OMAP44XX_MA_LISA_MAP_BASE		0x482AF040

/* DMM_LISA_MAP */
#define OMAP44XX_SYS_ADDR_SHIFT		24
#define OMAP44XX_SYS_ADDR_MASK		(0xff << 24)
#define OMAP44XX_SYS_SIZE_SHIFT		20
#define OMAP44XX_SYS_SIZE_MASK		(0x7 << 20)
#define OMAP44XX_SDRC_INTL_SHIFT	18
#define OMAP44XX_SDRC_INTL_MASK		(0x3 << 18)
#define OMAP44XX_SDRC_ADDRSPC_SHIFT	16
#define OMAP44XX_SDRC_ADDRSPC_MASK	(0x3 << 16)
#define OMAP44XX_SDRC_MAP_SHIFT		8
#define OMAP44XX_SDRC_MAP_MASK		(0x3 << 8)
#define OMAP44XX_SDRC_ADDR_SHIFT	0
#define OMAP44XX_SDRC_ADDR_MASK		(0xff << 0)

/* DMM_LISA_MAP fields */
#define DMM_SDRC_MAP_UNMAPPED		0
#define DMM_SDRC_MAP_EMIF1_ONLY		1
#define DMM_SDRC_MAP_EMIF2_ONLY		2
#define DMM_SDRC_MAP_EMIF1_AND_EMIF2	3

#define DMM_SDRC_INTL_NONE		0
#define DMM_SDRC_INTL_128B		1
#define DMM_SDRC_INTL_256B		2
#define DMM_SDRC_INTL_512		3

#define DMM_SDRC_ADDR_SPC_SDRAM		0
#define DMM_SDRC_ADDR_SPC_NVM		1
#define DMM_SDRC_ADDR_SPC_INVALID	2

#define DMM_LISA_MAP_INTERLEAVED_BASE_VAL		(\
	(DMM_SDRC_MAP_EMIF1_AND_EMIF2 << OMAP44XX_SDRC_MAP_SHIFT) |\
	(DMM_SDRC_ADDR_SPC_SDRAM << OMAP44XX_SDRC_ADDRSPC_SHIFT) |\
	(DMM_SDRC_INTL_128B << OMAP44XX_SDRC_INTL_SHIFT) |\
	(CONFIG_SYS_SDRAM_BASE << OMAP44XX_SYS_ADDR_SHIFT))

#define DMM_LISA_MAP_EMIF1_ONLY_BASE_VAL	(\
	(DMM_SDRC_MAP_EMIF1_ONLY << OMAP44XX_SDRC_MAP_SHIFT)|\
	(DMM_SDRC_ADDR_SPC_SDRAM << OMAP44XX_SDRC_ADDRSPC_SHIFT)|\
	(DMM_SDRC_INTL_NONE << OMAP44XX_SDRC_INTL_SHIFT))

#define DMM_LISA_MAP_EMIF2_ONLY_BASE_VAL	(\
	(DMM_SDRC_MAP_EMIF2_ONLY << OMAP44XX_SDRC_MAP_SHIFT)|\
	(DMM_SDRC_ADDR_SPC_SDRAM << OMAP44XX_SDRC_ADDRSPC_SHIFT)|\
	(DMM_SDRC_INTL_NONE << OMAP44XX_SDRC_INTL_SHIFT))

/* Trap for invalid TILER PAT entries */
#define DMM_LISA_MAP_0_INVAL_ADDR_TRAP		(\
	(0  << OMAP44XX_SDRC_ADDR_SHIFT) |\
	(DMM_SDRC_MAP_EMIF1_ONLY << OMAP44XX_SDRC_MAP_SHIFT)|\
	(DMM_SDRC_ADDR_SPC_INVALID << OMAP44XX_SDRC_ADDRSPC_SHIFT)|\
	(DMM_SDRC_INTL_NONE << OMAP44XX_SDRC_INTL_SHIFT)|\
	(0xFF << OMAP44XX_SYS_ADDR_SHIFT))


/* Reg mapping structure */
struct emif_reg_struct {
	u32 emif_mod_id_rev;
	u32 emif_status;
	u32 emif_sdram_config;
	u32 emif_lpddr2_nvm_config;
	u32 emif_sdram_ref_ctrl;
	u32 emif_sdram_ref_ctrl_shdw;
	u32 emif_sdram_tim_1;
	u32 emif_sdram_tim_1_shdw;
	u32 emif_sdram_tim_2;
	u32 emif_sdram_tim_2_shdw;
	u32 emif_sdram_tim_3;
	u32 emif_sdram_tim_3_shdw;
	u32 emif_lpddr2_nvm_tim;
	u32 emif_lpddr2_nvm_tim_shdw;
	u32 emif_pwr_mgmt_ctrl;
	u32 emif_pwr_mgmt_ctrl_shdw;
	u32 emif_lpddr2_mode_reg_data;
	u32 padding1[1];
	u32 emif_lpddr2_mode_reg_data_es2;
	u32 padding11[1];
	u32 emif_lpddr2_mode_reg_cfg;
	u32 emif_l3_config;
	u32 emif_l3_cfg_val_1;
	u32 emif_l3_cfg_val_2;
	u32 emif_iodft_tlgc;
	u32 padding2[7];
	u32 emif_perf_cnt_1;
	u32 emif_perf_cnt_2;
	u32 emif_perf_cnt_cfg;
	u32 emif_perf_cnt_sel;
	u32 emif_perf_cnt_tim;
	u32 padding3;
	u32 emif_read_idlectrl;
	u32 emif_read_idlectrl_shdw;
	u32 padding4;
	u32 emif_irqstatus_raw_sys;
	u32 emif_irqstatus_raw_ll;
	u32 emif_irqstatus_sys;
	u32 emif_irqstatus_ll;
	u32 emif_irqenable_set_sys;
	u32 emif_irqenable_set_ll;
	u32 emif_irqenable_clr_sys;
	u32 emif_irqenable_clr_ll;
	u32 padding5;
	u32 emif_zq_config;
	u32 emif_temp_alert_config;
	u32 emif_l3_err_log;
	u32 padding6[4];
	u32 emif_ddr_phy_ctrl_1;
	u32 emif_ddr_phy_ctrl_1_shdw;
	u32 emif_ddr_phy_ctrl_2;
};

struct dmm_lisa_map_regs {
	u32 dmm_lisa_map_0;
	u32 dmm_lisa_map_1;
	u32 dmm_lisa_map_2;
	u32 dmm_lisa_map_3;
};

struct control_lpddr2io_regs {
	u32 control_lpddr2io1_0;
	u32 control_lpddr2io1_1;
	u32 control_lpddr2io1_2;
	u32 control_lpddr2io1_3;
	u32 control_lpddr2io2_0;
	u32 control_lpddr2io2_1;
	u32 control_lpddr2io2_2;
	u32 control_lpddr2io2_3;
};

#define CS0	0
#define CS1	1
/* The maximum frequency at which the LPDDR2 interface can operate in Hz*/
#define MAX_LPDDR2_FREQ	400000000	/* 400 MHz */

/*
 * The period of DDR clk is represented as numerator and denominator for
 * better accuracy in integer based calculations. However, if the numerator
 * and denominator are very huge there may be chances of overflow in
 * calculations. So, as a trade-off keep denominator(and consequently
 * numerator) within a limit sacrificing some accuracy - but not much
 * If denominator and numerator are already small (such as at 400 MHz)
 * no adjustment is needed
 */
#define EMIF_PERIOD_DEN_LIMIT	1000
/*
 * Maximum number of different frequencies supported by EMIF driver
 * Determines the number of entries in the pointer array for register
 * cache
 */
#define EMIF_MAX_NUM_FREQUENCIES	6
/*
 * Indices into the Addressing Table array.
 * One entry each for all the different types of devices with different
 * addressing schemes
 */
#define ADDR_TABLE_INDEX64M	0
#define ADDR_TABLE_INDEX128M	1
#define ADDR_TABLE_INDEX256M	2
#define ADDR_TABLE_INDEX512M	3
#define ADDR_TABLE_INDEX1GS4	4
#define ADDR_TABLE_INDEX2GS4	5
#define ADDR_TABLE_INDEX4G	6
#define ADDR_TABLE_INDEX8G	7
#define ADDR_TABLE_INDEX1GS2	8
#define ADDR_TABLE_INDEX2GS2	9
#define ADDR_TABLE_INDEXMAX	10

/* Number of Row bits */
#define ROW_9  0
#define ROW_10 1
#define ROW_11 2
#define ROW_12 3
#define ROW_13 4
#define ROW_14 5
#define ROW_15 6
#define ROW_16 7

/* Number of Column bits */
#define COL_8   0
#define COL_9   1
#define COL_10  2
#define COL_11  3
#define COL_7   4 /*Not supported by OMAP included for completeness */

/* Number of Banks*/
#define BANKS1 0
#define BANKS2 1
#define BANKS4 2
#define BANKS8 3

/* Refresh rate in micro seconds x 10 */
#define T_REFI_15_6	156
#define T_REFI_7_8	78
#define T_REFI_3_9	39

#define EBANK_CS1_DIS	0
#define EBANK_CS1_EN	1

/* Read Latency used by the device at reset */
#define RL_BOOT		3
/* Read Latency for the highest frequency you want to use */
#define RL_FINAL	6

/* Interleaving policies at EMIF level- between banks and Chip Selects */
#define EMIF_INTERLEAVING_POLICY_MAX_INTERLEAVING	0
#define EMIF_INTERLEAVING_POLICY_NO_BANK_INTERLEAVING	3

/*
 * Interleaving policy to be used
 * Currently set to MAX interleaving for better performance
 */
#define EMIF_INTERLEAVING_POLICY EMIF_INTERLEAVING_POLICY_MAX_INTERLEAVING

/* State of the core voltage:
 * This is important for some parameters such as read idle control and
 * ZQ calibration timings. Timings are much stricter when voltage ramp
 * is happening compared to when the voltage is stable.
 * We need to calculate two sets of values for these parameters and use
 * them accordingly
 */
#define LPDDR2_VOLTAGE_STABLE	0
#define LPDDR2_VOLTAGE_RAMPING	1

/* Length of the forced read idle period in terms of cycles */
#define EMIF_REG_READ_IDLE_LEN_VAL	5

/* Interval between forced 'read idles' */
/* To be used when voltage is changed for DPS/DVFS - 1us */
#define READ_IDLE_INTERVAL_DVFS		(1*1000)
/*
 * To be used when voltage is not scaled except by Smart Reflex
 * 50us - or maximum value will do
 */
#define READ_IDLE_INTERVAL_NORMAL	(50*1000)


/*
 * Unless voltage is changing due to DVFS one ZQCS command every 50ms should
 * be enough. This shoule be enough also in the case when voltage is changing
 * due to smart-reflex.
 */
#define EMIF_ZQCS_INTERVAL_NORMAL_IN_US	(50*1000)
/*
 * If voltage is changing due to DVFS ZQCS should be performed more
 * often(every 50us)
 */
#define EMIF_ZQCS_INTERVAL_DVFS_IN_US	50

/* The interval between ZQCL commands as a multiple of ZQCS interval */
#define REG_ZQ_ZQCL_MULT		4
/* The interval between ZQINIT commands as a multiple of ZQCL interval */
#define REG_ZQ_ZQINIT_MULT		3
/* Enable ZQ Calibration on exiting Self-refresh */
#define REG_ZQ_SFEXITEN_ENABLE		1
/*
 * ZQ Calibration simultaneously on both chip-selects:
 * Needs one calibration resistor per CS
 * None of the boards that we know of have this capability
 * So disabled by default
 */
#define REG_ZQ_DUALCALEN_DISABLE	0
/*
 * Enable ZQ Calibration by default on CS0. If we are asked to program
 * the EMIF there will be something connected to CS0 for sure
 */
#define REG_ZQ_CS0EN_ENABLE		1

/* EMIF_PWR_MGMT_CTRL register */
/* Low power modes */
#define LP_MODE_DISABLE		0
#define LP_MODE_CLOCK_STOP	1
#define LP_MODE_SELF_REFRESH	2
#define LP_MODE_PWR_DN		3

/* REG_DPD_EN */
#define DPD_DISABLE	0
#define DPD_ENABLE	1

/* Maximum delay before Low Power Modes */
#define REG_CS_TIM		0xF
#define REG_SR_TIM		0xF
#define REG_PD_TIM		0xF

/* EMIF_PWR_MGMT_CTRL register */
#define EMIF_PWR_MGMT_CTRL (\
	((REG_CS_TIM << OMAP44XX_REG_CS_TIM_SHIFT) & OMAP44XX_REG_CS_TIM_MASK)|\
	((REG_SR_TIM << OMAP44XX_REG_SR_TIM_SHIFT) & OMAP44XX_REG_SR_TIM_MASK)|\
	((REG_PD_TIM << OMAP44XX_REG_PD_TIM_SHIFT) & OMAP44XX_REG_PD_TIM_MASK)|\
	((REG_PD_TIM << OMAP44XX_REG_PD_TIM_SHIFT) & OMAP44XX_REG_PD_TIM_MASK)|\
	((LP_MODE_DISABLE << OMAP44XX_REG_LP_MODE_SHIFT)\
			& OMAP44XX_REG_LP_MODE_MASK) |\
	((DPD_DISABLE << OMAP44XX_REG_DPD_EN_SHIFT)\
			& OMAP44XX_REG_DPD_EN_MASK))\

#define EMIF_PWR_MGMT_CTRL_SHDW (\
	((REG_CS_TIM << OMAP44XX_REG_CS_TIM_SHDW_SHIFT)\
			& OMAP44XX_REG_CS_TIM_SHDW_MASK) |\
	((REG_SR_TIM << OMAP44XX_REG_SR_TIM_SHDW_SHIFT)\
			& OMAP44XX_REG_SR_TIM_SHDW_MASK) |\
	((REG_PD_TIM << OMAP44XX_REG_PD_TIM_SHDW_SHIFT)\
			& OMAP44XX_REG_PD_TIM_SHDW_MASK) |\
	((REG_PD_TIM << OMAP44XX_REG_PD_TIM_SHDW_SHIFT)\
			& OMAP44XX_REG_PD_TIM_SHDW_MASK))

/* EMIF_L3_CONFIG register value */
#define EMIF_L3_CONFIG_VAL_SYS_10_LL_0	0x0A0000FF
#define EMIF_L3_CONFIG_VAL_SYS_10_MPU_3_LL_0	0x0A300000
/*
 * Value of bits 12:31 of DDR_PHY_CTRL_1 register:
 * All these fields have magic values dependent on frequency and
 * determined by PHY and DLL integration with EMIF. Setting the magic
 * values suggested by hw team.
 */
#define EMIF_DDR_PHY_CTRL_1_BASE_VAL			0x049FF
#define EMIF_DLL_SLAVE_DLY_CTRL_400_MHZ			0x41
#define EMIF_DLL_SLAVE_DLY_CTRL_200_MHZ			0x80
#define EMIF_DLL_SLAVE_DLY_CTRL_100_MHZ_AND_LESS	0xFF

/*
* MR1 value:
* Burst length	: 8
* Burst type	: sequential
* Wrap		: enabled
* nWR		: 3(default). EMIF does not do pre-charge.
*		: So nWR is don't care
*/
#define MR1_BL_8_BT_SEQ_WRAP_EN_NWR_3	0x23

/* MR2 */
#define MR2_RL3_WL1			1
#define MR2_RL4_WL2			2
#define MR2_RL5_WL2			3
#define MR2_RL6_WL3			4

/* MR10: ZQ calibration codes */
#define MR10_ZQ_ZQCS		0x56
#define MR10_ZQ_ZQCL		0xAB
#define MR10_ZQ_ZQINIT		0xFF
#define MR10_ZQ_ZQRESET		0xC3

/* TEMP_ALERT_CONFIG */
#define TEMP_ALERT_POLL_INTERVAL_MS	360 /* for temp gradient - 5 C/s */
#define TEMP_ALERT_CONFIG_DEVCT_1	0
#define TEMP_ALERT_CONFIG_DEVWDT_32	2

/* MR16 value: refresh full array(no partial array self refresh) */
#define MR16_REF_FULL_ARRAY	0

/* LPDDR2 IO regs */
#define CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN	0x1C1C1C1C
#define CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER	0x9E9E9E9E

/* CONTROL_EFUSE_2 */
#define CONTROL_EFUSE_2_NMOS_PMOS_PTV_CODE_1		0x00ffc000

/*
 * Maximum number of entries we keep in our array of timing tables
 * We need not keep all the speed bins supported by the device
 * We need to keep timing tables for only the speed bins that we
 * are interested in
 */
#define MAX_NUM_SPEEDBINS	4

/* LPDDR2 Densities */
#define LPDDR2_DENSITY_64Mb	0
#define LPDDR2_DENSITY_128Mb	1
#define LPDDR2_DENSITY_256Mb	2
#define LPDDR2_DENSITY_512Mb	3
#define LPDDR2_DENSITY_1Gb	4
#define LPDDR2_DENSITY_2Gb	5
#define LPDDR2_DENSITY_4Gb	6
#define LPDDR2_DENSITY_8Gb	7
#define LPDDR2_DENSITY_16Gb	8
#define LPDDR2_DENSITY_32Gb	9

/* LPDDR2 type */
#define	LPDDR2_TYPE_S4	0
#define	LPDDR2_TYPE_S2	1
#define	LPDDR2_TYPE_NVM	2

/* LPDDR2 IO width */
#define	LPDDR2_IO_WIDTH_32	0
#define	LPDDR2_IO_WIDTH_16	1
#define	LPDDR2_IO_WIDTH_8	2

/* Mode register numbers */
#define LPDDR2_MR0	0
#define LPDDR2_MR1	1
#define LPDDR2_MR2	2
#define LPDDR2_MR3	3
#define LPDDR2_MR4	4
#define LPDDR2_MR5	5
#define LPDDR2_MR6	6
#define LPDDR2_MR7	7
#define LPDDR2_MR8	8
#define LPDDR2_MR9	9
#define LPDDR2_MR10	10
#define LPDDR2_MR11	11
#define LPDDR2_MR16	16
#define LPDDR2_MR17	17
#define LPDDR2_MR18	18

/* MR0 */
#define LPDDR2_MR0_DAI_SHIFT	0
#define LPDDR2_MR0_DAI_MASK	1
#define LPDDR2_MR0_DI_SHIFT	1
#define LPDDR2_MR0_DI_MASK	(1 << 1)
#define LPDDR2_MR0_DNVI_SHIFT	2
#define LPDDR2_MR0_DNVI_MASK	(1 << 2)

/* MR4 */
#define MR4_SDRAM_REF_RATE_SHIFT	0
#define MR4_SDRAM_REF_RATE_MASK		7
#define MR4_TUF_SHIFT			7
#define MR4_TUF_MASK			(1 << 7)

/* MR4 SDRAM Refresh Rate field values */
#define SDRAM_TEMP_LESS_LOW_SHUTDOWN			0x0
#define SDRAM_TEMP_LESS_4X_REFRESH_AND_TIMINGS		0x1
#define SDRAM_TEMP_LESS_2X_REFRESH_AND_TIMINGS		0x2
#define SDRAM_TEMP_NOMINAL				0x3
#define SDRAM_TEMP_RESERVED_4				0x4
#define SDRAM_TEMP_HIGH_DERATE_REFRESH			0x5
#define SDRAM_TEMP_HIGH_DERATE_REFRESH_AND_TIMINGS	0x6
#define SDRAM_TEMP_VERY_HIGH_SHUTDOWN			0x7

#define LPDDR2_MANUFACTURER_SAMSUNG	1
#define LPDDR2_MANUFACTURER_QIMONDA	2
#define LPDDR2_MANUFACTURER_ELPIDA	3
#define LPDDR2_MANUFACTURER_ETRON	4
#define LPDDR2_MANUFACTURER_NANYA	5
#define LPDDR2_MANUFACTURER_HYNIX	6
#define LPDDR2_MANUFACTURER_MOSEL	7
#define LPDDR2_MANUFACTURER_WINBOND	8
#define LPDDR2_MANUFACTURER_ESMT	9
#define LPDDR2_MANUFACTURER_SPANSION 11
#define LPDDR2_MANUFACTURER_SST		12
#define LPDDR2_MANUFACTURER_ZMOS	13
#define LPDDR2_MANUFACTURER_INTEL	14
#define LPDDR2_MANUFACTURER_NUMONYX	254
#define LPDDR2_MANUFACTURER_MICRON	255

/* MR8 register fields */
#define MR8_TYPE_SHIFT		0x0
#define MR8_TYPE_MASK		0x3
#define MR8_DENSITY_SHIFT	0x2
#define MR8_DENSITY_MASK	(0xF << 0x2)
#define MR8_IO_WIDTH_SHIFT	0x6
#define MR8_IO_WIDTH_MASK	(0x3 << 0x6)

struct lpddr2_addressing {
	u8	num_banks;
	u8	t_REFI_us_x10;
	u8	row_sz[2]; /* One entry each for x32 and x16 */
	u8	col_sz[2]; /* One entry each for x32 and x16 */
};

/* Structure for timings from the DDR datasheet */
struct lpddr2_ac_timings {
	u32 max_freq;
	u8 RL;
	u8 tRPab;
	u8 tRCD;
	u8 tWR;
	u8 tRASmin;
	u8 tRRD;
	u8 tWTRx2;
	u8 tXSR;
	u8 tXPx2;
	u8 tRFCab;
	u8 tRTPx2;
	u8 tCKE;
	u8 tCKESR;
	u8 tZQCS;
	u32 tZQCL;
	u32 tZQINIT;
	u8 tDQSCKMAXx2;
	u8 tRASmax;
	u8 tFAW;

};

/*
 * Min tCK values for some of the parameters:
 * If the calculated clock cycles for the respective parameter is
 * less than the corresponding min tCK value, we need to set the min
 * tCK value. This may happen at lower frequencies.
 */
struct lpddr2_min_tck {
	u32 tRL;
	u32 tRP_AB;
	u32 tRCD;
	u32 tWR;
	u32 tRAS_MIN;
	u32 tRRD;
	u32 tWTR;
	u32 tXP;
	u32 tRTP;
	u8  tCKE;
	u32 tCKESR;
	u32 tFAW;
};

struct lpddr2_device_details {
	u8	type;
	u8	density;
	u8	io_width;
	u8	manufacturer;
};

struct lpddr2_device_timings {
	const struct lpddr2_ac_timings **ac_timings;
	const struct lpddr2_min_tck *min_tck;
};

/* Details of the devices connected to each chip-select of an EMIF instance */
struct emif_device_details {
	const struct lpddr2_device_details *cs0_device_details;
	const struct lpddr2_device_details *cs1_device_details;
	const struct lpddr2_device_timings *cs0_device_timings;
	const struct lpddr2_device_timings *cs1_device_timings;
};

/*
 * Structure containing shadow of important registers in EMIF
 * The calculation function fills in this structure to be later used for
 * initialization and DVFS
 */
struct emif_regs {
	u32 freq;
	u32 sdram_config_init;
	u32 sdram_config;
	u32 ref_ctrl;
	u32 sdram_tim1;
	u32 sdram_tim2;
	u32 sdram_tim3;
	u32 read_idle_ctrl;
	u32 zq_config;
	u32 temp_alert_config;
	u32 emif_ddr_phy_ctlr_1_init;
	u32 emif_ddr_phy_ctlr_1;
};

/* assert macros */
#if defined(DEBUG)
#define emif_assert(c)	({ if (!(c)) for (;;); })
#else
#define emif_assert(c)	({ if (0) hang(); })
#endif

#ifdef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs);
void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs);
#else
void emif_get_device_details(u32 emif_nr,
		struct lpddr2_device_details *cs0_device_details,
		struct lpddr2_device_details *cs1_device_details);
void emif_get_device_timings(u32 emif_nr,
		const struct lpddr2_device_timings **cs0_device_timings,
		const struct lpddr2_device_timings **cs1_device_timings);
#endif

#endif
