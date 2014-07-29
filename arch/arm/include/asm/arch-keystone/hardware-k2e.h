/*
 * K2E: SoC definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __ASM_ARCH_HARDWARE_K2E_H
#define __ASM_ARCH_HARDWARE_K2E_H

/* PA SS Registers */
#define KS2_PASS_BASE			0x24000000

/* Power and Sleep Controller (PSC) Domains */
#define KS2_LPSC_MOD_RST		0
#define KS2_LPSC_USB_1			1
#define KS2_LPSC_USB			2
#define KS2_LPSC_EMIF25_SPI		3
#define KS2_LPSC_TSIP			4
#define KS2_LPSC_DEBUGSS_TRC		5
#define KS2_LPSC_TETB_TRC		6
#define KS2_LPSC_PKTPROC		7
#define KS2_LPSC_PA			KS2_LPSC_PKTPROC
#define KS2_LPSC_SGMII			8
#define KS2_LPSC_CPGMAC			KS2_LPSC_SGMII
#define KS2_LPSC_CRYPTO			9
#define KS2_LPSC_PCIE			10
#define KS2_LPSC_VUSR0			12
#define KS2_LPSC_CHIP_SRSS		13
#define KS2_LPSC_MSMC			14
#define KS2_LPSC_EMIF4F_DDR3		23
#define KS2_LPSC_PCIE_1			27
#define KS2_LPSC_XGE			50

/* Chip Interrupt Controller */
#define KS2_CIC2_DDR3_ECC_IRQ_NUM	-1	/* not defined in K2E */
#define KS2_CIC2_DDR3_ECC_CHAN_NUM	-1	/* not defined in K2E */

/* Number of DSP cores */
#define KS2_NUM_DSPS			1

#endif
