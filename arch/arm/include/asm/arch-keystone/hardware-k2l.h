/*
 * K2L: SoC definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __ASM_ARCH_HARDWARE_K2L_H
#define __ASM_ARCH_HARDWARE_K2L_H

#define KS2_ARM_PLL_EN			BIT(13)

/* PA SS Registers */
#define KS2_PASS_BASE			0x26000000

/* Power and Sleep Controller (PSC) Domains */
#define KS2_LPSC_MOD			0
#define KS2_LPSC_DFE_IQN_SYS		1
#define KS2_LPSC_USB			2
#define KS2_LPSC_EMIF25_SPI		3
#define KS2_LPSC_TSIP                   4
#define KS2_LPSC_DEBUGSS_TRC		5
#define KS2_LPSC_TETB_TRC		6
#define KS2_LPSC_PKTPROC		7
#define KS2_LPSC_PA			KS2_LPSC_PKTPROC
#define KS2_LPSC_SGMII			8
#define KS2_LPSC_CPGMAC			KS2_LPSC_SGMII
#define KS2_LPSC_CRYPTO			9
#define KS2_LPSC_PCIE0			10
#define KS2_LPSC_PCIE1			11
#define KS2_LPSC_JESD_MISC		12
#define KS2_LPSC_CHIP_SRSS		13
#define KS2_LPSC_MSMC			14
#define KS2_LPSC_GEM_1			16
#define KS2_LPSC_GEM_2			17
#define KS2_LPSC_GEM_3			18
#define KS2_LPSC_EMIF4F_DDR3		23
#define KS2_LPSC_TAC			25
#define KS2_LPSC_RAC			26
#define KS2_LPSC_DDUC4X_CFR2X_BB	27
#define KS2_LPSC_FFTC_A			28
#define KS2_LPSC_OSR			34
#define KS2_LPSC_TCP3D_0		35
#define KS2_LPSC_TCP3D_1		37
#define KS2_LPSC_VCP2X4_A		39
#define KS2_LPSC_VCP2X4_B		40
#define KS2_LPSC_VCP2X4_C		41
#define KS2_LPSC_VCP2X4_D		42
#define KS2_LPSC_BCP			47
#define KS2_LPSC_DPD4X			48
#define KS2_LPSC_FFTC_B			49
#define KS2_LPSC_IQN_AIL		50

/* MSMC */
#define KS2_MSMC_SEGMENT_PCIE1		14

/* Chip Interrupt Controller */
#define KS2_CIC2_DDR3_ECC_IRQ_NUM	0x0D3
#define KS2_CIC2_DDR3_ECC_CHAN_NUM	0x01D

/* Number of DSP cores */
#define KS2_NUM_DSPS			4

/* NETCP pktdma */
#define KS2_NETCP_PDMA_CTRL_BASE	0x26186000
#define KS2_NETCP_PDMA_TX_BASE		0x26187000
#define KS2_NETCP_PDMA_TX_CH_NUM	21
#define KS2_NETCP_PDMA_RX_BASE		0x26188000
#define KS2_NETCP_PDMA_RX_CH_NUM	91
#define KS2_NETCP_PDMA_SCHED_BASE	0x26186100
#define KS2_NETCP_PDMA_RX_FLOW_BASE	0x26189000
#define KS2_NETCP_PDMA_RX_FLOW_NUM	96
#define KS2_NETCP_PDMA_TX_SND_QUEUE	896

#endif /* __ASM_ARCH_HARDWARE_K2L_H */
