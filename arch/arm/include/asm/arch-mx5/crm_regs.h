/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
 */

#ifndef __ARCH_ARM_MACH_MX51_CRM_REGS_H__
#define __ARCH_ARM_MACH_MX51_CRM_REGS_H__

#define MXC_CCM_BASE	CCM_BASE_ADDR

/* DPLL register mapping structure */
struct mxc_pll_reg {
	u32 ctrl;
	u32 config;
	u32 op;
	u32 mfd;
	u32 mfn;
	u32 mfn_minus;
	u32 mfn_plus;
	u32 hfs_op;
	u32 hfs_mfd;
	u32 hfs_mfn;
	u32 mfn_togc;
	u32 destat;
};

/* Register maping of CCM*/
struct mxc_ccm_reg {
	u32 ccr;	/* 0x0000 */
	u32 ccdr;
	u32 csr;
	u32 ccsr;
	u32 cacrr;	/* 0x0010*/
	u32 cbcdr;
	u32 cbcmr;
	u32 cscmr1;
	u32 cscmr2;	/* 0x0020 */
	u32 cscdr1;
	u32 cs1cdr;
	u32 cs2cdr;
	u32 cdcdr;	/* 0x0030 */
	u32 chscdr;
	u32 cscdr2;
	u32 cscdr3;
	u32 cscdr4;	/* 0x0040 */
	u32 cwdr;
	u32 cdhipr;
	u32 cdcr;
	u32 ctor;	/* 0x0050 */
	u32 clpcr;
	u32 cisr;
	u32 cimr;
	u32 ccosr;	/* 0x0060 */
	u32 cgpr;
	u32 CCGR0;
	u32 CCGR1;
	u32 CCGR2;	/* 0x0070 */
	u32 CCGR3;
	u32 CCGR4;
	u32 CCGR5;
	u32 CCGR6;	/* 0x0080 */
	u32 cmeor;
};

/* Define the bits in register CACRR */
#define MXC_CCM_CACRR_ARM_PODF_OFFSET		0
#define MXC_CCM_CACRR_ARM_PODF_MASK		0x7

/* Define the bits in register CBCDR */
#define MXC_CCM_CBCDR_EMI_CLK_SEL		(0x1 << 26)
#define MXC_CCM_CBCDR_PERIPH_CLK_SEL		(0x1 << 25)
#define MXC_CCM_CBCDR_EMI_PODF_OFFSET		22
#define MXC_CCM_CBCDR_EMI_PODF_MASK		(0x7 << 22)
#define MXC_CCM_CBCDR_AXI_B_PODF_OFFSET		19
#define MXC_CCM_CBCDR_AXI_B_PODF_MASK		(0x7 << 19)
#define MXC_CCM_CBCDR_AXI_A_PODF_OFFSET		16
#define MXC_CCM_CBCDR_AXI_A_PODF_MASK		(0x7 << 16)
#define MXC_CCM_CBCDR_NFC_PODF_OFFSET		13
#define MXC_CCM_CBCDR_NFC_PODF_MASK		(0x7 << 13)
#define MXC_CCM_CBCDR_AHB_PODF_OFFSET		10
#define MXC_CCM_CBCDR_AHB_PODF_MASK		(0x7 << 10)
#define MXC_CCM_CBCDR_IPG_PODF_OFFSET		8
#define MXC_CCM_CBCDR_IPG_PODF_MASK		(0x3 << 8)
#define MXC_CCM_CBCDR_PERCLK_PRED1_OFFSET	6
#define MXC_CCM_CBCDR_PERCLK_PRED1_MASK		(0x3 << 6)
#define MXC_CCM_CBCDR_PERCLK_PRED2_OFFSET	3
#define MXC_CCM_CBCDR_PERCLK_PRED2_MASK		(0x7 << 3)
#define MXC_CCM_CBCDR_PERCLK_PODF_OFFSET	0
#define MXC_CCM_CBCDR_PERCLK_PODF_MASK		0x7

/* Define the bits in register CSCMR1 */
#define MXC_CCM_CSCMR1_SSI_EXT2_CLK_SEL_OFFSET		30
#define MXC_CCM_CSCMR1_SSI_EXT2_CLK_SEL_MASK		(0x3 << 30)
#define MXC_CCM_CSCMR1_SSI_EXT1_CLK_SEL_OFFSET		28
#define MXC_CCM_CSCMR1_SSI_EXT1_CLK_SEL_MASK		(0x3 << 28)
#define MXC_CCM_CSCMR1_USB_PHY_CLK_SEL_OFFSET		26
#define MXC_CCM_CSCMR1_USB_PHY_CLK_SEL			(0x1 << 26)
#define MXC_CCM_CSCMR1_UART_CLK_SEL_OFFSET		24
#define MXC_CCM_CSCMR1_UART_CLK_SEL_MASK		(0x3 << 24)
#define MXC_CCM_CSCMR1_USBOH3_CLK_SEL_OFFSET		22
#define MXC_CCM_CSCMR1_USBOH3_CLK_SEL_MASK		(0x3 << 22)
#define MXC_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL_OFFSET	20
#define MXC_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL_MASK	(0x3 << 20)
#define MXC_CCM_CSCMR1_ESDHC3_CLK_SEL			(0x1 << 19)
#define MXC_CCM_CSCMR1_ESDHC4_CLK_SEL			(0x1 << 18)
#define MXC_CCM_CSCMR1_ESDHC2_MSHC2_CLK_SEL_OFFSET	16
#define MXC_CCM_CSCMR1_ESDHC2_MSHC2_CLK_SEL_MASK	(0x3 << 16)
#define MXC_CCM_CSCMR1_SSI1_CLK_SEL_OFFSET		14
#define MXC_CCM_CSCMR1_SSI1_CLK_SEL_MASK		(0x3 << 14)
#define MXC_CCM_CSCMR1_SSI2_CLK_SEL_OFFSET		12
#define MXC_CCM_CSCMR1_SSI2_CLK_SEL_MASK		(0x3 << 12)
#define MXC_CCM_CSCMR1_SSI3_CLK_SEL			(0x1 << 11)
#define MXC_CCM_CSCMR1_VPU_RCLK_SEL			(0x1 << 10)
#define MXC_CCM_CSCMR1_SSI_APM_CLK_SEL_OFFSET		8
#define MXC_CCM_CSCMR1_SSI_APM_CLK_SEL_MASK		(0x3 << 8)
#define MXC_CCM_CSCMR1_TVE_CLK_SEL			(0x1 << 7)
#define MXC_CCM_CSCMR1_TVE_EXT_CLK_SEL			(0x1 << 6)
#define MXC_CCM_CSCMR1_CSPI_CLK_SEL_OFFSET		4
#define MXC_CCM_CSCMR1_CSPI_CLK_SEL_MASK		(0x3 << 4)
#define MXC_CCM_CSCMR1_SPDIF_CLK_SEL_OFFSET		2
#define MXC_CCM_CSCMR1_SPDIF_CLK_SEL_MASK		(0x3 << 2)
#define MXC_CCM_CSCMR1_SSI_EXT2_COM_CLK_SEL		(0x1 << 1)
#define MXC_CCM_CSCMR1_SSI_EXT1_COM_CLK_SEL		0x1

/* Define the bits in register CSCDR2 */
#define MXC_CCM_CSCDR2_CSPI_CLK_PRED_OFFSET		25
#define MXC_CCM_CSCDR2_CSPI_CLK_PRED_MASK		(0x7 << 25)
#define MXC_CCM_CSCDR2_CSPI_CLK_PODF_OFFSET		19
#define MXC_CCM_CSCDR2_CSPI_CLK_PODF_MASK		(0x3F << 19)
#define MXC_CCM_CSCDR2_SIM_CLK_PRED_OFFSET		16
#define MXC_CCM_CSCDR2_SIM_CLK_PRED_MASK		(0x7 << 16)
#define MXC_CCM_CSCDR2_SIM_CLK_PODF_OFFSET		9
#define MXC_CCM_CSCDR2_SIM_CLK_PODF_MASK		(0x3F << 9)
#define MXC_CCM_CSCDR2_SLIMBUS_CLK_PRED_OFFSET		6
#define MXC_CCM_CSCDR2_SLIMBUS_PRED_MASK		(0x7 << 6)
#define MXC_CCM_CSCDR2_SLIMBUS_PODF_OFFSET		0
#define MXC_CCM_CSCDR2_SLIMBUS_PODF_MASK		0x3F

/* Define the bits in register CBCMR */
#define MXC_CCM_CBCMR_VPU_AXI_CLK_SEL_OFFSET		14
#define MXC_CCM_CBCMR_VPU_AXI_CLK_SEL_MASK		(0x3 << 14)
#define MXC_CCM_CBCMR_PERIPH_CLK_SEL_OFFSET		12
#define MXC_CCM_CBCMR_PERIPH_CLK_SEL_MASK		(0x3 << 12)
#define MXC_CCM_CBCMR_DDR_CLK_SEL_OFFSET		10
#define MXC_CCM_CBCMR_DDR_CLK_SEL_MASK			(0x3 << 10)
#define MXC_CCM_CBCMR_ARM_AXI_CLK_SEL_OFFSET		8
#define MXC_CCM_CBCMR_ARM_AXI_CLK_SEL_MASK		(0x3 << 8)
#define MXC_CCM_CBCMR_IPU_HSP_CLK_SEL_OFFSET		6
#define MXC_CCM_CBCMR_IPU_HSP_CLK_SEL_MASK		(0x3 << 6)
#define MXC_CCM_CBCMR_GPU_CLK_SEL_OFFSET		4
#define MXC_CCM_CBCMR_GPU_CLK_SEL_MASK			(0x3 << 4)
#define MXC_CCM_CBCMR_PERCLK_LP_APM_CLK_SEL		(0x1 << 1)
#define MXC_CCM_CBCMR_PERCLK_IPG_CLK_SEL		(0x1 << 0)

/* Define the bits in register CSCDR1 */
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PRED_OFFSET	22
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PRED_MASK	(0x7 << 22)
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PODF_OFFSET	19
#define MXC_CCM_CSCDR1_ESDHC2_MSHC2_CLK_PODF_MASK	(0x7 << 19)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED_OFFSET	16
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED_MASK	(0x7 << 16)
#define MXC_CCM_CSCDR1_PGC_CLK_PODF_OFFSET		14
#define MXC_CCM_CSCDR1_PGC_CLK_PODF_MASK		(0x3 << 14)
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF_OFFSET	11
#define MXC_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF_MASK	(0x7 << 11)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PRED_OFFSET		8
#define MXC_CCM_CSCDR1_USBOH3_CLK_PRED_MASK		(0x7 << 8)
#define MXC_CCM_CSCDR1_USBOH3_CLK_PODF_OFFSET		6
#define MXC_CCM_CSCDR1_USBOH3_CLK_PODF_MASK		(0x3 << 6)
#define MXC_CCM_CSCDR1_UART_CLK_PRED_OFFSET		3
#define MXC_CCM_CSCDR1_UART_CLK_PRED_MASK		(0x7 << 3)
#define MXC_CCM_CSCDR1_UART_CLK_PODF_OFFSET		0
#define MXC_CCM_CSCDR1_UART_CLK_PODF_MASK		0x7

#endif				/* __ARCH_ARM_MACH_MX51_CRM_REGS_H__ */
