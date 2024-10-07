/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#ifndef __ASM_AST2700_SLI_H__
#define __ASM_AST2700_SLI_H__

#define SLI_CPU_ADRBASE			0x12c17000
#define SLI_IOD_ADRBASE			0x14c1e000
#define SLIM_CPU_BASE			(SLI_CPU_ADRBASE + 0x000)
#define SLIH_CPU_BASE			(SLI_CPU_ADRBASE + 0x200)
#define SLIV_CPU_BASE			(SLI_CPU_ADRBASE + 0x400)
#define SLIM_IOD_BASE			(SLI_IOD_ADRBASE + 0x000)
#define SLIH_IOD_BASE			(SLI_IOD_ADRBASE + 0x200)
#define SLIV_IOD_BASE			(SLI_IOD_ADRBASE + 0x400)

#define SLI_CTRL_I			0x00
#define   SLIV_RAW_MODE			BIT(15)
#define   SLI_TX_MODE			BIT(14)
#define   SLI_RX_PHY_LAH_SEL_REV	BIT(13)
#define   SLI_RX_PHY_LAH_SEL_NEG	BIT(12)
#define   SLI_AUTO_SEND_TRN_OFF		BIT(8)
#define   SLI_CLEAR_BUS			BIT(6)
#define   SLI_TRANS_EN			BIT(5)
#define   SLI_CLEAR_RX			BIT(2)
#define   SLI_CLEAR_TX			BIT(1)
#define   SLI_RESET_TRIGGER		BIT(0)
#define SLI_CTRL_II			0x04
#define SLI_CTRL_III			0x08
#define   SLI_CLK_SEL			GENMASK(31, 28)
#define     SLI_CLK_500M		0x6
#define     SLI_CLK_200M		0x3
#define   SLI_PHYCLK_SEL		GENMASK(27, 24)
#define     SLI_PHYCLK_25M		0x0
#define     SLI_PHYCLK_800M		0x1
#define     SLI_PHYCLK_400M		0x2
#define     SLI_PHYCLK_200M		0x3
#define     SLI_PHYCLK_788M		0x5
#define     SLI_PHYCLK_500M		0x6
#define     SLI_PHYCLK_250M		0x7
#define   SLIH_PAD_DLY_TX1		GENMASK(23, 18)
#define   SLIH_PAD_DLY_TX0		GENMASK(17, 12)
#define   SLIH_PAD_DLY_RX1		GENMASK(11, 6)
#define   SLIH_PAD_DLY_RX0		GENMASK(5, 0)
#define   SLIM_PAD_DLY_RX3		GENMASK(23, 18)
#define   SLIM_PAD_DLY_RX2		GENMASK(17, 12)
#define   SLIM_PAD_DLY_RX1		GENMASK(11, 6)
#define   SLIM_PAD_DLY_RX0		GENMASK(5, 0)
#define SLI_CTRL_IV			0x0c
#define   SLIM_PAD_DLY_TX3		GENMASK(23, 18)
#define   SLIM_PAD_DLY_TX2		GENMASK(17, 12)
#define   SLIM_PAD_DLY_TX1		GENMASK(11, 6)
#define   SLIM_PAD_DLY_TX0		GENMASK(5, 0)
#define SLI_INTR_EN			0x10
#define SLI_INTR_STATUS			0x14
#define   SLI_INTR_RX_SYNC		BIT(15)
#define   SLI_INTR_RX_ERR		BIT(13)
#define   SLI_INTR_RX_NACK		BIT(12)
#define   SLI_INTR_RX_TRAIN_PKT		BIT(10)
#define   SLI_INTR_RX_DISCONN		BIT(6)
#define   SLI_INTR_TX_SUSPEND		BIT(4)
#define   SLI_INTR_TX_TRAIN		BIT(3)
#define   SLI_INTR_TX_IDLE		BIT(2)
#define   SLI_INTR_RX_SUSPEND		BIT(1)
#define   SLI_INTR_RX_IDLE		BIT(0)
#define   SLI_INTR_RX_ERRORS                                                     \
	  (SLI_INTR_RX_ERR | SLI_INTR_RX_NACK | SLI_INTR_RX_DISCONN)

#define SLIM_MARB_FUNC_I		0x60
#define   SLIM_SLI_MARB_RR		BIT(0)

#define SLI_TARGET_PHYCLK		SLI_PHYCLK_400M
#define SLIH_DEFAULT_DELAY		11
#if (SLI_TARGET_PHYCLK == SLI_PHYCLK_800M) || (SLI_TARGET_PHYCLK == SLI_PHYCLK_788M)
#define SLIM_DEFAULT_DELAY		5
#define SLIM_LAH_CONFIG			1
#else
#define SLIM_DEFAULT_DELAY		12
#define SLIM_LAH_CONFIG			0
#endif
#endif
int sli_init(void);
