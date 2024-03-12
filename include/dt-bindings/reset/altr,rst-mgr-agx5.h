/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024 Intel Corporation. All rights reserved
 */

#ifndef _DT_BINDINGS_RESET_ALTR_RST_MGR_AGX_EDGE_H
#define _DT_BINDINGS_RESET_ALTR_RST_MGR_AGX_EDGE_H

/* PER0MODRST */
#define EMAC0_RESET		0
#define EMAC1_RESET		1
#define EMAC2_RESET		2
#define USB0_RESET		3
#define USB1_RESET		4
#define NAND_RESET		5
#define COMBOPHY_RESET		6
#define SDMMC_RESET		7
#define EMAC0_OCP_RESET		8
#define EMAC1_OCP_RESET		9
#define EMAC2_OCP_RESET		10
#define USB0_OCP_RESET		11
#define USB1_OCP_RESET		12
#define NAND_OCP_RESET		13
/* 14 is empty */
#define SDMMC_OCP_RESET		15
#define DMA_RESET		16
#define SPIM0_RESET		17
#define SPIM1_RESET		18
#define SPIS0_RESET		19
#define SPIS1_RESET		20
#define DMA_OCP_RESET		21
#define EMAC_PTP_RESET		22
/* 23 is empty*/
#define DMAIF0_RESET		24
#define DMAIF1_RESET		25
#define DMAIF2_RESET		26
#define DMAIF3_RESET		27
#define DMAIF4_RESET		28
#define DMAIF5_RESET		29
#define DMAIF6_RESET		30
#define DMAIF7_RESET		31

/* PER1MODRST */
#define WATCHDOG0_RESET		32
#define WATCHDOG1_RESET		33
#define WATCHDOG2_RESET		34
#define WATCHDOG3_RESET		35
#define L4SYSTIMER0_RESET	36
#define L4SYSTIMER1_RESET	37
#define SPTIMER0_RESET		38
#define SPTIMER1_RESET		39
#define I2C0_RESET		40
#define I2C1_RESET		41
#define I2C2_RESET		42
#define I2C3_RESET		43
#define I2C4_RESET		44
#define I3C0_RESET		45
#define I3C1_RESET		46
/* 47 is empty */
#define UART0_RESET		48
#define UART1_RESET		49
/* 50-55 is empty */
#define GPIO0_RESET		56
#define GPIO1_RESET		57
#define WATCHDOG4_RESET		58
/* 59-63 is empty */

/* BRGMODRST */
#define SOC2FPGA_RESET		64
#define LWHPS2FPGA_RESET	65
#define FPGA2SOC_RESET		66
#define F2SSDRAM_RESET		67
/* 68-69 is empty */
#define DDRSCH_RESET		70
/* 71-95 is empty */

/* DBGMODRST */
#define DBG_RESET		192

#endif
