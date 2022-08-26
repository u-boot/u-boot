/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Qualcomm QCS404 sysmap
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */
#ifndef _MACH_SYSMAP_QCS404_H
#define _MACH_SYSMAP_QCS404_H

#define GICD_BASE			(0x0b000000)
#define GICC_BASE			(0x0b002000)

/* Clocks: (from CLK_CTL_BASE)  */
#define GPLL0_STATUS			(0x21000)
#define APCS_GPLL_ENA_VOTE		(0x45000)
#define APCS_CLOCK_BRANCH_ENA_VOTE	(0x45004)

/* BLSP1 AHB clock (root clock for BLSP) */
#define BLSP1_AHB_CBCR			0x1008

/* Uart clock control registers */
#define BLSP1_UART2_BCR			(0x3028)
#define BLSP1_UART2_APPS_CBCR		(0x302C)
#define BLSP1_UART2_APPS_CMD_RCGR	(0x3034)
#define BLSP1_UART2_APPS_CFG_RCGR	(0x3038)
#define BLSP1_UART2_APPS_M		(0x303C)
#define BLSP1_UART2_APPS_N		(0x3040)
#define BLSP1_UART2_APPS_D		(0x3044)

/* SD controller clock control registers */
#define SDCC_BCR(n)			(((n) * 0x1000) + 0x41000)
#define SDCC_CMD_RCGR(n)		(((n) * 0x1000) + 0x41004)
#define SDCC_CFG_RCGR(n)		(((n) * 0x1000) + 0x41008)
#define SDCC_M(n)			(((n) * 0x1000) + 0x4100C)
#define SDCC_N(n)			(((n) * 0x1000) + 0x41010)
#define SDCC_D(n)			(((n) * 0x1000) + 0x41014)
#define SDCC_APPS_CBCR(n)		(((n) * 0x1000) + 0x41018)
#define SDCC_AHB_CBCR(n)		(((n) * 0x1000) + 0x4101C)

/* USB-3.0 controller clock control registers */
#define SYS_NOC_USB3_CBCR		(0x26014)
#define USB30_BCR			(0x39000)
#define USB3PHY_BCR			(0x39008)
#define USB30_MASTER_CBCR		(0x3900C)
#define USB30_SLEEP_CBCR		(0x39010)
#define USB30_MOCK_UTMI_CBCR		(0x39014)
#define USB30_MOCK_UTMI_CMD_RCGR	(0x3901C)
#define USB30_MOCK_UTMI_CFG_RCGR	(0x39020)
#define USB30_MASTER_CMD_RCGR		(0x39028)
#define USB30_MASTER_CFG_RCGR		(0x3902C)
#define USB30_MASTER_M			(0x39030)
#define USB30_MASTER_N			(0x39034)
#define USB30_MASTER_D			(0x39038)
#define USB2A_PHY_SLEEP_CBCR		(0x4102C)
#define USB_HS_PHY_CFG_AHB_CBCR		(0x41030)

#endif
