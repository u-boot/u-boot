/******************************************************************************
*
* (c) Copyright 2009 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
******************************************************************************/
/*****************************************************************************/
/**
* @file xparameters_ps.h
*
* This file contains the address definitions for the hard peripherals
* attached to the Cortex A9 core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  02/01/09 Initial version,
* </pre>
*
* @note	None.
*
******************************************************************************/

#ifndef _XPARAMETERS_PSS_H_
#define _XPARAMETERS_PSS_H_

/***************************** Include Files *********************************/

#ifdef __cplusplus
extern "C" {
#endif
/***************** Macros (Inline Functions) Definitions *********************/
/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

#define RTL_20	/* Palladium now */

/*
 * This block contains constant declarations for the peripherals
 * within the hardblock
 */

#define XPSS_PERIPHERAL_BASEADDR	0xE0000000

#define XPSS_UART0_BASEADDR			0xE0000000
#define XPSS_UART1_BASEADDR			0xE0001000


#define XPSS_USB0_BASEADDR			0xE0002000
#define XPSS_USB1_BASEADDR			0xE0003000
#define XPSS_I2C0_BASEADDR			0xE0004000
#define XPSS_I2C1_BASEADDR			0xE0005000
#define XPSS_SPI0_BASEADDR			0xE0006000
#define XPSS_SPI1_BASEADDR			0xE0007000

#define XPSS_CAN0_BASEADDR			0xE0008000
#define XPSS_CAN1_BASEADDR			0xE0009000
#define XPSS_GPIO_BASEADDR			0xE000A000
#define XPSS_GEM0_BASEADDR			0xE000B000
#define XPSS_GEM1_BASEADDR			0xE000C000
#define XPSS_QSPI_BASEADDR			0xE000D000
#define XPSS_CRTL_PARPORT_BASEADDR	0xE000E000

#define XPSS_USB0_SLAVE_BASEADDR	0xE0100000
#define XPSS_USB1_SLAVE_BASEADDR	0xE0110000

#define XPSS_SDIO0_BASEADDR			0xE0100000
#define XPSS_SDIO1_BASEADDR			0xE0101000

#define XPSS_IOU_BUS_CFG_BASEADDR	0xE0200000

#define XPSS_NAND_BASEADDR			0xE1000000

#define XPSS_PARPORT0_BASEADDR		0xE2000000
#define XPSS_PARPORT1_BASEADDR		0xE4000000

#define RTL_30
#ifdef RTL_30
#define XPSS_QSPI_LIN_BASEADDR		0xFC000000

#define XPSS_SYS_CTRL_BASEADDR		0xF8000000	/* AKA SLCR */

#define XPSS_TTC0_BASEADDR			0xF8001000
#define XPSS_TTC1_BASEADDR			0xF8002000

#define XPSS_DMAC0_BASEADDR			0xF8003000
#define XPSS_DMAC1_BASEADDR			0xF8004000

#define XPSS_WDT_BASEADDR			0xF8005000
#define XPSS_DDR_CTRL_BASEADDR		0xF8006000

#define XPSS_DEV_CFG_APB_BASEADDR	0xF8007000

#define XPSS_L2CC_BASEADDR			0xF8F02000


#define XPSS_SCU_BASEADDR			0xF8F00000

#else

#define XPSS_QSPI_LIN_BASEADDR		0xE6000000

#define XPSS_SYS_CTRL_BASEADDR		0xFE000000	/* AKA SLCR */

#define XPSS_TTC0_BASEADDR			0xFE001000
#define XPSS_TTC1_BASEADDR			0xFE002000

#define XPSS_DMAC0_BASEADDR			0xFE003000
#define XPSS_DMAC1_BASEADDR			0xFE004000

#define XPSS_WDT_BASEADDR			0xFE005000
#define XPSS_DDR_CTRL_BASEADDR		0xFE006000

#define XPSS_DEV_CFG_APB_BASEADDR	0xFE007000

#define XPSS_L2CC_BASEADDR			0xFFE00000


#define XPSS_SCU_BASEADDR			0xFEF00000

#endif

/* Interrupt Controller bit assignments */

/* Shared Peripheral Interrupts (SPI) */
#define XPSS_L2CC_INT_ID		32
#define XPSS_DEV_CFG_INT_ID		33
#define XPSS_WDT_INT_ID			34
#define XPSS_TTC0_0_INT_ID		35
#define XPSS_TTC0_1_INT_ID		36
#define XPSS_TTC0_2_INT_ID		37
#define XPSS_TOP_DMA_ABORT_INT_ID	38
#define XPSS_TOP_DMA0_INT_ID		39
#define XPSS_TOP_DMA1_INT_ID		40
#define XPSS_TOP_DMA2_INT_ID		41
#define XPSS_TOP_DMA3_INT_ID		42
//#define XPSS_SAM_SEC_INT_ID		41
#define XPSS_SMC_INT_ID			43
#define XPSS_QSPI_INT_ID		44
//#define XPSS_CTI_INT_ID			44
#define XPSS_GPIO_INT_ID		45
#define XPSS_USB0_INT_ID		46
#define XPSS_GEM0_INT_ID		47
#define XPSS_GEM0_WAKE_INT_ID		48
#define XPSS_SDIO0_INT_ID		49
#define XPSS_I2C0_INT_ID		50
#define XPSS_SPI0_INT_ID		51
#define XPSS_UART0_INT_ID		52
#define XPSS_CAN0_INT_ID		53
#define XPSS_FPGA0_INT_ID		54
#define XPSS_FPGA1_INT_ID		55
#define XPSS_FPGA2_INT_ID		56
#define XPSS_FPGA3_INT_ID		57
#define XPSS_FPGA4_INT_ID		58
#define XPSS_FPGA5_INT_ID		59
#define XPSS_FPGA6_INT_ID		60
#define XPSS_FPGA7_INT_ID		61
#define XPSS_TTC1_0_INT_ID		62
#define XPSS_TTC1_1_INT_ID		63
#define XPSS_TTC1_2_INT_ID		64
//#define XPSS_BOT_DMA_ABOR_INT_ID	65
#define XPSS_BOT_DMA0_INT_ID		65
#define XPSS_BOT_DMA1_INT_ID		66
#define XPSS_BOT_DMA2_INT_ID		67
#define XPSS_BOT_DMA3_INT_ID		68
//#define XPSS_SAM_NOSEC_INT_ID		70
#define XPSS_USB1_INT_ID		69
#define XPSS_GEM1_INT_ID		70
#define XPSS_GEM1_WAKE_INT_ID		71
#define XPSS_SDIO1_INT_ID		72
#define XPSS_I2C1_INT_ID		73
#define XPSS_SPI1_INT_ID		74
#define XPSS_UART1_INT_ID		75
#define XPSS_CAN1_INT_ID		76
#define XPSS_FPGA8_INT_ID		77
#define XPSS_FPGA9_INT_ID		78
#define XPSS_FPGA10_INT_ID		79
#define XPSS_FPGA11_INT_ID		80
#define XPSS_FPGA12_INT_ID		81
#define XPSS_FPGA13_INT_ID		82
#define XPSS_FPGA14_INT_ID		83
#define XPSS_FPGA15_INT_ID		84

/* Private Peripheral Interrupts (PPI) */
#define XPSS_GLOBAL_TMR_INT_ID		27
#define XPSS_CPU_TMR_INT_ID		29
#define XPSS_SCU_WDT_INT_ID		30

/* L2CC Register Offsets */
#define XPSS_L2CC_ID_OFFSET			0x0000
#define XPSS_L2CC_TYPE_OFFSET			0x0004
#define XPSS_L2CC_CNTRL_OFFSET			0x0100
#define XPSS_L2CC_AUX_CNTRL_OFFSET		0x0104
#define XPSS_L2CC_TAG_RAM_CNTRL_OFFSET		0x0108
#define XPSS_L2CC_DATA_RAM_CNTRL_OFFSET		0x010C

#define XPSS_L2CC_EVNT_CNTRL_OFFSET		0x0200
#define XPSS_L2CC_EVNT_CNT1_CTRL_OFFSET		0x0204
#define XPSS_L2CC_EVNT_CNT0_CTRL_OFFSET		0x0208
#define XPSS_L2CC_EVNT_CNT1_VAL_OFFSET		0x020C
#define XPSS_L2CC_EVNT_CNT0_VAL_OFFSET		0x0210

#define XPSS_L2CC_IER_OFFSET			0x0214	/* Interrupt Mask */
#define XPSS_L2CC_IPR_OFFSET			0x0218	/* Masked interrupt status */
#define XPSS_L2CC_ISR_OFFSET			0x021C	/* Raw Interrupt Status */
#define XPSS_L2CC_IAR_OFFSET			0x0220	/* Interrupt Clear */

#define XPSS_L2CC_CACHE_SYNC_OFFSET		0x0730	/* Cache Sync */
#define XPSS_L2CC_CACHE_INVLD_PA_OFFSET		0x0770	/* Cache Invalid by PA */
#define XPSS_L2CC_CACHE_INVLD_WAY_OFFSET	0x077C	/* Cache Invalid by Way */
#define XPSS_L2CC_CACHE_CLEAN_PA_OFFSET		0x07B0	/* Cache Clean by PA */
#define XPSS_L2CC_CACHE_CLEAN_INDX_OFFSET	0x07B8	/* Cache Clean by Index */
#define XPSS_L2CC_CACHE_CLEAN_WAY_OFFSET	0x07BC	/* Cache Clean by Way */
#define XPSS_L2CC_CACHE_INV_CLN_PA_OFFSET	0x07F0	/* Cache Invalidate and Clean by PA */
#define XPSS_L2CC_CACHE_INV_CLN_INDX_OFFSET	0x07F8	/* Cache Invalidate and Clean by Index */
#define XPSS_L2CC_CACHE_INV_CLN_WAY_OFFSET	0x07FC	/* Cache Invalidate and Clean by Way */


#define XPSS_L2CC_CACHE_DLCKDWN_0_WAY_OFFSET	0x0900	/* Cache Data Lockdown 0 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_0_WAY_OFFSET	0x0904	/* Cache Instruction Lockdown 0 by Way */
#define XPSS_L2CC_CACHE_DLCKDWN_1_WAY_OFFSET	0x0908	/* Cache Data Lockdown 1 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_1_WAY_OFFSET	0x090C	/* Cache Instruction Lockdown 1 by Way */
#define XPSS_L2CC_CACHE_DLCKDWN_2_WAY_OFFSET	0x0910	/* Cache Data Lockdown 2 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_2_WAY_OFFSET	0x0914	/* Cache Instruction Lockdown 2 by Way */
#define XPSS_L2CC_CACHE_DLCKDWN_3_WAY_OFFSET	0x0918	/* Cache Data Lockdown 3 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_3_WAY_OFFSET	0x091C	/* Cache Instruction Lockdown 3 by Way */
#define XPSS_L2CC_CACHE_DLCKDWN_4_WAY_OFFSET	0x0920	/* Cache Data Lockdown 4 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_4_WAY_OFFSET	0x0924	/* Cache Instruction Lockdown 4 by Way */
#define XPSS_L2CC_CACHE_DLCKDWN_5_WAY_OFFSET	0x0928	/* Cache Data Lockdown 5 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_5_WAY_OFFSET	0x092C	/* Cache Instruction Lockdown 5 by Way */
#define XPSS_L2CC_CACHE_DLCKDWN_6_WAY_OFFSET	0x0930	/* Cache Data Lockdown 6 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_6_WAY_OFFSET	0x0934	/* Cache Instruction Lockdown 6 by Way */
#define XPSS_L2CC_CACHE_DLCKDWN_7_WAY_OFFSET	0x0938	/* Cache Data Lockdown 7 by Way */
#define XPSS_L2CC_CACHE_ILCKDWN_7_WAY_OFFSET	0x093C	/* Cache Instruction Lockdown 7 by Way */

#define XPSS_L2CC_CACHE_LCKDWN_LINE_ENABLE_OFFSET	0x0950	/* Cache Lockdown Line Enable */
#define XPSS_L2CC_CACHE_UUNLOCK_ALL_WAY_OFFSET		0x0954	/* Cache Unlock All Lines by Way */

#define XPSS_L2CC_ADDR_FILTER_START_OFFSET		0x0C00	/* Start of address filtering */
#define XPSS_L2CC_ADDR_FILTER_END_OFFSET		0x0C04	/* Start of address filtering */

#define XPSS_L2CC_DEBUG_CTRL_OFFSET			0x0F40	/* Debug Control Register */

/* XPSS_L2CC_CNTRL_OFFSET bit position */
#define XPSS_L2CC_ENABLE_MASK				0x00000001	/* enables the L2CC */

/* XPSS_L2CC_AUX_CNTRL_OFFSET bit positions */
#define XPSS_L2CC_AUX_IPFE_MASK				0x20000000	/* Instruction Prefetch Enable */
#define XPSS_L2CC_AUX_DPFE_MASK				0x10000000	/* Data Prefetch Enable */
#define XPSS_L2CC_AUX_NSIC_MASK				0x08000000	/* Non-secure interrupt access control */
#define XPSS_L2CC_AUX_NSLE_MASK				0x04000000	/* Non-secure lockdown enable */
#define XPSS_L2CC_AUX_FWE_MASK				0x01800000	/* Force write allocate */
#define XPSS_L2CC_AUX_SAOE_MASK				0x00400000	/* Shared attribute override enable */
#define XPSS_L2CC_AUX_PE_MASK				0x00200000	/* Parity enable */
#define XPSS_L2CC_AUX_EMBE_MASK				0x00100000	/* Event monitor bus enable */
#define XPSS_L2CC_AUX_WAY_SIZE_MASK			0x000E0000	/* Way-size  - s/b b100 for DF */
#define XPSS_L2CC_AUX_ASSOC_MASK			0x00010000	/* Associativity */
#define XPSS_L2CC_AUX_EXCL_CACHE_MASK			0x00001000	/* Exclusive cache configuration */

#define XPSS_L2CC_AUX_REG_DEFAULT_MASK			0x02020000	/* 16k*/
#define XPSS_L2CC_AUX_REG_ZERO_MASK			0xFDF1FEFF	/* */

#define XPSS_L2CC_TAG_RAM_DEFAULT_MASK			0x00000666	/* 7 Cycles of latency for TAG RAM  s/b 0x00000111 for 2 */
#define XPSS_L2CC_DATA_RAM_DEFAULT_MASK			0x00000666	/* 7 Cycles of latency for DATA RAM s/b 0x00000111 for 2 */

/* Interrupt bit positions */
#define XPSS_L2CC_IXR_DECERR_MASK			0x00000100	/* DECERR from L3 */
#define XPSS_L2CC_IXR_SLVERR_MASK			0x00000080	/* SLVERR from L3 */
#define XPSS_L2CC_IXR_ERRRD_MASK			0x00000040	/* Error on L2 data RAM (Read) */
#define XPSS_L2CC_IXR_ERRRT_MASK			0x00000020	/* Error on L2 tag RAM (Read) */
#define XPSS_L2CC_IXR_ERRWD_MASK			0x00000010	/* Error on L2 data RAM (Write) */
#define XPSS_L2CC_IXR_ERRWT_MASK			0x00000008	/* Error on L2 tag RAM (Write) */
#define XPSS_L2CC_IXR_PARRD_MASK			0x00000004	/* Parity Error on L2 data RAM (Read) */
#define XPSS_L2CC_IXR_PARRT_MASK			0x00000002	/* Parity Error on L2 tag RAM (Read) */
#define XPSS_L2CC_IXR_ECNTR_MASK			0x00000001	/* Event Counter1/0 Overflow Increment */

/* Address filtering mask and enable bit */
#define XPSS_L2CC_ADDR_FILTER_VALID_MASK		0xFFF00000	/* Address filtering valid bits*/
#define XPSS_L2CC_ADDR_FILTER_ENABLE_MASK		0x00000001	/* Address filtering enable bit*/

/* Debug control bits */
#define XPSS_L2CC_DEBUG_SPIDEN_MASK			0x00000004	/* Debug SPIDEN bit */
#define XPSS_L2CC_DEBUG_DWB_MASK			0x00000002	/* Debug DWB bit, forces write through */
#define XPSS_L2CC_DEBUG_DCL_MASK			0x00000002	/* Debug DCL bit, disables cache line fill */


/*	SCU register offsets */

#define XPSS_SCU_CONTROL_OFFSET	 		0x000
#define XPSS_SCU_CONFIG_OFFSET 			0x004
#define XPSS_SCU_FILTER_START_OFFSET	0x040
#define XPSS_SCU_FILTER_END_OFFSET 		0x044
#define XPSS_SCU_NON_SECURE_ACCESS_OFFSET 0x054

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* protection macro */
