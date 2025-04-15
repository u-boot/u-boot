// SPDX-License-Identifier: GPL-2.0
/*
 * R8A77995 processor support - PFC hardware block.
 *
 * Copyright (C) 2017 Renesas Electronics Corp.
 *
 * This file is based on the drivers/pinctrl/renesas/pfc-r8a7796.c
 *
 * R-Car Gen3 processor support - PFC hardware block.
 *
 * Copyright (C) 2015  Renesas Electronics Corporation
 */

#include <dm.h>
#include <errno.h>
#include <dm/pinctrl.h>
#include <linux/kernel.h>

#include "sh_pfc.h"

#define CPU_ALL_GP(fn, sfx)						\
	PORT_GP_CFG_9(0,  fn, sfx, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PORT_GP_CFG_32(1, fn, sfx, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PORT_GP_CFG_32(2, fn, sfx, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PORT_GP_CFG_10(3, fn, sfx, SH_PFC_PIN_CFG_IO_VOLTAGE_18_33 | SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PORT_GP_CFG_32(4, fn, sfx, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PORT_GP_CFG_21(5, fn, sfx, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PORT_GP_CFG_14(6, fn, sfx, SH_PFC_PIN_CFG_PULL_UP_DOWN)

#define CPU_ALL_NOGP(fn)						\
	PIN_NOGP_CFG(DU_DOTCLKIN0, "DU_DOTCLKIN0", fn, SH_PFC_PIN_CFG_PULL_DOWN),	\
	PIN_NOGP_CFG(FSCLKST_N, "FSCLKST#", fn, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PIN_NOGP_CFG(MLB_REF, "MLB_REF", fn, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PIN_NOGP_CFG(PRESETOUT_N, "PRESETOUT#", fn, SH_PFC_PIN_CFG_PULL_UP_DOWN),	\
	PIN_NOGP_CFG(TCK, "TCK", fn, SH_PFC_PIN_CFG_PULL_UP),		\
	PIN_NOGP_CFG(TDI, "TDI", fn, SH_PFC_PIN_CFG_PULL_UP),		\
	PIN_NOGP_CFG(TMS, "TMS", fn, SH_PFC_PIN_CFG_PULL_UP),		\
	PIN_NOGP_CFG(TRST_N, "TRST#", fn, SH_PFC_PIN_CFG_PULL_UP),	\
	PIN_NOGP_CFG(VDDQ_AVB0, "VDDQ_AVB0", fn, SH_PFC_PIN_CFG_IO_VOLTAGE_25_33)

/*
 * F_() : just information
 * FM() : macro for FN_xxx / xxx_MARK
 */

/* GPSR0 */
#define GPSR0_8		F_(MLB_SIG,		IP0_27_24)
#define GPSR0_7		F_(MLB_DAT,		IP0_23_20)
#define GPSR0_6		F_(MLB_CLK,		IP0_19_16)
#define GPSR0_5		F_(MSIOF2_RXD,		IP0_15_12)
#define GPSR0_4		F_(MSIOF2_TXD,		IP0_11_8)
#define GPSR0_3		F_(MSIOF2_SCK,		IP0_7_4)
#define GPSR0_2		F_(IRQ0_A,		IP0_3_0)
#define GPSR0_1		FM(USB0_OVC)
#define GPSR0_0		FM(USB0_PWEN)

/* GPSR1 */
#define GPSR1_31	F_(QPOLB,		IP4_27_24)
#define GPSR1_30	F_(QPOLA,		IP4_23_20)
#define GPSR1_29	F_(DU_CDE,		IP4_19_16)
#define GPSR1_28	F_(DU_DISP_CDE,		IP4_15_12)
#define GPSR1_27	F_(DU_DISP,		IP4_11_8)
#define GPSR1_26	F_(DU_VSYNC,		IP4_7_4)
#define GPSR1_25	F_(DU_HSYNC,		IP4_3_0)
#define GPSR1_24	F_(DU_DOTCLKOUT0,	IP3_31_28)
#define GPSR1_23	F_(DU_DR7,		IP3_27_24)
#define GPSR1_22	F_(DU_DR6,		IP3_23_20)
#define GPSR1_21	F_(DU_DR5,		IP3_19_16)
#define GPSR1_20	F_(DU_DR4,		IP3_15_12)
#define GPSR1_19	F_(DU_DR3,		IP3_11_8)
#define GPSR1_18	F_(DU_DR2,		IP3_7_4)
#define GPSR1_17	F_(DU_DR1,		IP3_3_0)
#define GPSR1_16	F_(DU_DR0,		IP2_31_28)
#define GPSR1_15	F_(DU_DG7,		IP2_27_24)
#define GPSR1_14	F_(DU_DG6,		IP2_23_20)
#define GPSR1_13	F_(DU_DG5,		IP2_19_16)
#define GPSR1_12	F_(DU_DG4,		IP2_15_12)
#define GPSR1_11	F_(DU_DG3,		IP2_11_8)
#define GPSR1_10	F_(DU_DG2,		IP2_7_4)
#define GPSR1_9		F_(DU_DG1,		IP2_3_0)
#define GPSR1_8		F_(DU_DG0,		IP1_31_28)
#define GPSR1_7		F_(DU_DB7,		IP1_27_24)
#define GPSR1_6		F_(DU_DB6,		IP1_23_20)
#define GPSR1_5		F_(DU_DB5,		IP1_19_16)
#define GPSR1_4		F_(DU_DB4,		IP1_15_12)
#define GPSR1_3		F_(DU_DB3,		IP1_11_8)
#define GPSR1_2		F_(DU_DB2,		IP1_7_4)
#define GPSR1_1		F_(DU_DB1,		IP1_3_0)
#define GPSR1_0		F_(DU_DB0,		IP0_31_28)

/* GPSR2 */
#define GPSR2_31	F_(NFCE_N,		IP8_19_16)
#define GPSR2_30	F_(NFCLE,		IP8_15_12)
#define GPSR2_29	F_(NFALE,		IP8_11_8)
#define GPSR2_28	F_(VI4_CLKENB,		IP8_7_4)
#define GPSR2_27	F_(VI4_FIELD,		IP8_3_0)
#define GPSR2_26	F_(VI4_HSYNC_N,		IP7_31_28)
#define GPSR2_25	F_(VI4_VSYNC_N,		IP7_27_24)
#define GPSR2_24	F_(VI4_DATA23,		IP7_23_20)
#define GPSR2_23	F_(VI4_DATA22,		IP7_19_16)
#define GPSR2_22	F_(VI4_DATA21,		IP7_15_12)
#define GPSR2_21	F_(VI4_DATA20,		IP7_11_8)
#define GPSR2_20	F_(VI4_DATA19,		IP7_7_4)
#define GPSR2_19	F_(VI4_DATA18,		IP7_3_0)
#define GPSR2_18	F_(VI4_DATA17,		IP6_31_28)
#define GPSR2_17	F_(VI4_DATA16,		IP6_27_24)
#define GPSR2_16	F_(VI4_DATA15,		IP6_23_20)
#define GPSR2_15	F_(VI4_DATA14,		IP6_19_16)
#define GPSR2_14	F_(VI4_DATA13,		IP6_15_12)
#define GPSR2_13	F_(VI4_DATA12,		IP6_11_8)
#define GPSR2_12	F_(VI4_DATA11,		IP6_7_4)
#define GPSR2_11	F_(VI4_DATA10,		IP6_3_0)
#define GPSR2_10	F_(VI4_DATA9,		IP5_31_28)
#define GPSR2_9		F_(VI4_DATA8,		IP5_27_24)
#define GPSR2_8		F_(VI4_DATA7,		IP5_23_20)
#define GPSR2_7		F_(VI4_DATA6,		IP5_19_16)
#define GPSR2_6		F_(VI4_DATA5,		IP5_15_12)
#define GPSR2_5		FM(VI4_DATA4)
#define GPSR2_4		F_(VI4_DATA3,		IP5_11_8)
#define GPSR2_3		F_(VI4_DATA2,		IP5_7_4)
#define GPSR2_2		F_(VI4_DATA1,		IP5_3_0)
#define GPSR2_1		F_(VI4_DATA0,		IP4_31_28)
#define GPSR2_0		FM(VI4_CLK)

/* GPSR3 */
#define GPSR3_9		F_(NFDATA7,		IP9_31_28)
#define GPSR3_8		F_(NFDATA6,		IP9_27_24)
#define GPSR3_7		F_(NFDATA5,		IP9_23_20)
#define GPSR3_6		F_(NFDATA4,		IP9_19_16)
#define GPSR3_5		F_(NFDATA3,		IP9_15_12)
#define GPSR3_4		F_(NFDATA2,		IP9_11_8)
#define GPSR3_3		F_(NFDATA1,		IP9_7_4)
#define GPSR3_2		F_(NFDATA0,		IP9_3_0)
#define GPSR3_1		F_(NFWE_N,		IP8_31_28)
#define GPSR3_0		F_(NFRE_N,		IP8_27_24)

/* GPSR4 */
#define GPSR4_31	F_(CAN0_RX_A,		IP12_27_24)
#define GPSR4_30	F_(CAN1_TX_A,		IP13_7_4)
#define GPSR4_29	F_(CAN1_RX_A,		IP13_3_0)
#define GPSR4_28	F_(CAN0_TX_A,		IP12_31_28)
#define GPSR4_27	FM(TX2)
#define GPSR4_26	FM(RX2)
#define GPSR4_25	F_(SCK2,		IP12_11_8)
#define GPSR4_24	F_(TX1_A,		IP12_7_4)
#define GPSR4_23	F_(RX1_A,		IP12_3_0)
#define GPSR4_22	F_(SCK1_A,		IP11_31_28)
#define GPSR4_21	F_(TX0_A,		IP11_27_24)
#define GPSR4_20	F_(RX0_A,		IP11_23_20)
#define GPSR4_19	F_(SCK0_A,		IP11_19_16)
#define GPSR4_18	F_(MSIOF1_RXD,		IP11_15_12)
#define GPSR4_17	F_(MSIOF1_TXD,		IP11_11_8)
#define GPSR4_16	F_(MSIOF1_SCK,		IP11_7_4)
#define GPSR4_15	FM(MSIOF0_RXD)
#define GPSR4_14	FM(MSIOF0_TXD)
#define GPSR4_13	FM(MSIOF0_SYNC)
#define GPSR4_12	FM(MSIOF0_SCK)
#define GPSR4_11	F_(SDA1,		IP11_3_0)
#define GPSR4_10	F_(SCL1,		IP10_31_28)
#define GPSR4_9		FM(SDA0)
#define GPSR4_8		FM(SCL0)
#define GPSR4_7		F_(SSI_WS4_A,		IP10_27_24)
#define GPSR4_6		F_(SSI_SDATA4_A,	IP10_23_20)
#define GPSR4_5		F_(SSI_SCK4_A,		IP10_19_16)
#define GPSR4_4		F_(SSI_WS34,		IP10_15_12)
#define GPSR4_3		F_(SSI_SDATA3,		IP10_11_8)
#define GPSR4_2		F_(SSI_SCK34,		IP10_7_4)
#define GPSR4_1		F_(AUDIO_CLKA,		IP10_3_0)
#define GPSR4_0		F_(NFRB_N,		IP8_23_20)

/* GPSR5 */
#define GPSR5_20	FM(AVB0_LINK)
#define GPSR5_19	FM(AVB0_PHY_INT)
#define GPSR5_18	FM(AVB0_MAGIC)
#define GPSR5_17	FM(AVB0_MDC)
#define GPSR5_16	FM(AVB0_MDIO)
#define GPSR5_15	FM(AVB0_TXCREFCLK)
#define GPSR5_14	FM(AVB0_TD3)
#define GPSR5_13	FM(AVB0_TD2)
#define GPSR5_12	FM(AVB0_TD1)
#define GPSR5_11	FM(AVB0_TD0)
#define GPSR5_10	FM(AVB0_TXC)
#define GPSR5_9		FM(AVB0_TX_CTL)
#define GPSR5_8		FM(AVB0_RD3)
#define GPSR5_7		FM(AVB0_RD2)
#define GPSR5_6		FM(AVB0_RD1)
#define GPSR5_5		FM(AVB0_RD0)
#define GPSR5_4		FM(AVB0_RXC)
#define GPSR5_3		FM(AVB0_RX_CTL)
#define GPSR5_2		F_(CAN_CLK,		IP12_23_20)
#define GPSR5_1		F_(TPU0TO1_A,		IP12_19_16)
#define GPSR5_0		F_(TPU0TO0_A,		IP12_15_12)

/* GPSR6 */
#define GPSR6_13	FM(RPC_INT_N)
#define GPSR6_12	FM(RPC_RESET_N)
#define GPSR6_11	FM(QSPI1_SSL)
#define GPSR6_10	FM(QSPI1_IO3)
#define GPSR6_9		FM(QSPI1_IO2)
#define GPSR6_8		FM(QSPI1_MISO_IO1)
#define GPSR6_7		FM(QSPI1_MOSI_IO0)
#define GPSR6_6		FM(QSPI1_SPCLK)
#define GPSR6_5		FM(QSPI0_SSL)
#define GPSR6_4		FM(QSPI0_IO3)
#define GPSR6_3		FM(QSPI0_IO2)
#define GPSR6_2		FM(QSPI0_MISO_IO1)
#define GPSR6_1		FM(QSPI0_MOSI_IO0)
#define GPSR6_0		FM(QSPI0_SPCLK)

/* IPSRx */		/* 0 */			/* 1 */			/* 2 */			/* 3 */		/* 4 */			/* 5 */		/* 6  - F */
#define IP0_3_0		FM(IRQ0_A)		FM(MSIOF2_SYNC_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_7_4		FM(MSIOF2_SCK)		F_(0, 0)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_11_8	FM(MSIOF2_TXD)		FM(SCL3_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_15_12	FM(MSIOF2_RXD)		FM(SDA3_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_19_16	FM(MLB_CLK)		FM(MSIOF2_SYNC_A)	FM(SCK5_A)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_23_20	FM(MLB_DAT)		FM(MSIOF2_SS1)		FM(RX5_A)		FM(SCL3_B)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_27_24	FM(MLB_SIG)		FM(MSIOF2_SS2)		FM(TX5_A)		FM(SDA3_B)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP0_31_28	FM(DU_DB0)		FM(LCDOUT0)		FM(MSIOF3_TXD_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_3_0		FM(DU_DB1)		FM(LCDOUT1)		FM(MSIOF3_RXD_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_7_4		FM(DU_DB2)		FM(LCDOUT2)		FM(IRQ0_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_11_8	FM(DU_DB3)		FM(LCDOUT3)		FM(SCK5_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_15_12	FM(DU_DB4)		FM(LCDOUT4)		FM(RX5_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_19_16	FM(DU_DB5)		FM(LCDOUT5)		FM(TX5_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_23_20	FM(DU_DB6)		FM(LCDOUT6)		FM(MSIOF3_SS1_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_27_24	FM(DU_DB7)		FM(LCDOUT7)		FM(MSIOF3_SS2_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP1_31_28	FM(DU_DG0)		FM(LCDOUT8)		FM(MSIOF3_SCK_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_3_0		FM(DU_DG1)		FM(LCDOUT9)		FM(MSIOF3_SYNC_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_7_4		FM(DU_DG2)		FM(LCDOUT10)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_11_8	FM(DU_DG3)		FM(LCDOUT11)		FM(IRQ1_A)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_15_12	FM(DU_DG4)		FM(LCDOUT12)		FM(HSCK3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_19_16	FM(DU_DG5)		FM(LCDOUT13)		FM(HTX3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_23_20	FM(DU_DG6)		FM(LCDOUT14)		FM(HRX3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_27_24	FM(DU_DG7)		FM(LCDOUT15)		FM(SCK4_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP2_31_28	FM(DU_DR0)		FM(LCDOUT16)		FM(RX4_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_3_0		FM(DU_DR1)		FM(LCDOUT17)		FM(TX4_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_7_4		FM(DU_DR2)		FM(LCDOUT18)		FM(PWM0_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_11_8	FM(DU_DR3)		FM(LCDOUT19)		FM(PWM1_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_15_12	FM(DU_DR4)		FM(LCDOUT20)		FM(TCLK2_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_19_16	FM(DU_DR5)		FM(LCDOUT21)		FM(NMI)			F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_23_20	FM(DU_DR6)		FM(LCDOUT22)		FM(PWM2_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_27_24	FM(DU_DR7)		FM(LCDOUT23)		FM(TCLK1_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP3_31_28	FM(DU_DOTCLKOUT0)	FM(QCLK)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* IPSRx */		/* 0 */			/* 1 */			/* 2 */			/* 3 */		/* 4 */			/* 5 */		/* 6  - F */
#define IP4_3_0		FM(DU_HSYNC)		FM(QSTH_QHS)		FM(IRQ3_A)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_7_4		FM(DU_VSYNC)		FM(QSTVA_QVS)		FM(IRQ4_A)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_11_8	FM(DU_DISP)		FM(QSTVB_QVE)		FM(PWM3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_15_12	FM(DU_DISP_CDE)		FM(QCPV_QDE)		FM(IRQ2_B)		FM(DU_DOTCLKIN1)F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_19_16	FM(DU_CDE)		FM(QSTB_QHE)		FM(SCK3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_23_20	FM(QPOLA)		F_(0, 0)		FM(RX3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_27_24	FM(QPOLB)		F_(0, 0)		FM(TX3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP4_31_28	FM(VI4_DATA0)		FM(PWM0_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_3_0		FM(VI4_DATA1)		FM(PWM1_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_7_4		FM(VI4_DATA2)		FM(PWM2_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_11_8	FM(VI4_DATA3)		FM(PWM3_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_15_12	FM(VI4_DATA5)		FM(SCK4_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_19_16	FM(VI4_DATA6)		FM(IRQ2_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_23_20	FM(VI4_DATA7)		FM(TCLK2_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_27_24	FM(VI4_DATA8)		F_(0, 0)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP5_31_28	FM(VI4_DATA9)		FM(MSIOF3_SS2_A)	FM(IRQ1_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_3_0		FM(VI4_DATA10)		FM(RX4_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_7_4		FM(VI4_DATA11)		FM(TX4_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_11_8	FM(VI4_DATA12)		FM(TCLK1_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_15_12	FM(VI4_DATA13)		FM(MSIOF3_SS1_A)	FM(HCTS3_N)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_19_16	FM(VI4_DATA14)		FM(SSI_SCK4_B)		FM(HRTS3_N)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_23_20	FM(VI4_DATA15)		FM(SSI_SDATA4_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_27_24	FM(VI4_DATA16)		FM(HRX3_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP6_31_28	FM(VI4_DATA17)		FM(HTX3_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_3_0		FM(VI4_DATA18)		FM(HSCK3_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_7_4		FM(VI4_DATA19)		FM(SSI_WS4_B)		F_(0, 0)		F_(0, 0)	FM(NFDATA15)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_11_8	FM(VI4_DATA20)		FM(MSIOF3_SYNC_A)	F_(0, 0)		F_(0, 0)	FM(NFDATA14)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_15_12	FM(VI4_DATA21)		FM(MSIOF3_TXD_A)	F_(0, 0)		F_(0, 0)	FM(NFDATA13)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_19_16	FM(VI4_DATA22)		FM(MSIOF3_RXD_A)	F_(0, 0)		F_(0, 0)	FM(NFDATA12)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_23_20	FM(VI4_DATA23)		FM(MSIOF3_SCK_A)	F_(0, 0)		F_(0, 0)	FM(NFDATA11)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_27_24	FM(VI4_VSYNC_N)		FM(SCK1_B)		F_(0, 0)		F_(0, 0)	FM(NFDATA10)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP7_31_28	FM(VI4_HSYNC_N)		FM(RX1_B)		F_(0, 0)		F_(0, 0)	FM(NFDATA9)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* IPSRx */		/* 0 */			/* 1 */			/* 2 */			/* 3 */		/* 4 */			/* 5 */		/* 6  - F */
#define IP8_3_0		FM(VI4_FIELD)		FM(AUDIO_CLKB)		FM(IRQ5_A)		FM(SCIF_CLK)	FM(NFDATA8)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_7_4		FM(VI4_CLKENB)		FM(TX1_B)		F_(0, 0)		F_(0, 0)	FM(NFWP_N)		FM(DVC_MUTE_A)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_11_8	FM(NFALE)		FM(SCL2_B)		FM(IRQ3_B)		FM(PWM0_C)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_15_12	FM(NFCLE)		FM(SDA2_B)		FM(SCK3_A)		FM(PWM1_C)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_19_16	FM(NFCE_N)		F_(0, 0)		FM(RX3_A)		FM(PWM2_C)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_23_20	FM(NFRB_N)		F_(0, 0)		FM(TX3_A)		FM(PWM3_C)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_27_24	FM(NFRE_N)		FM(MMC_CMD)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP8_31_28	FM(NFWE_N)		FM(MMC_CLK)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_3_0		FM(NFDATA0)		FM(MMC_D0)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_7_4		FM(NFDATA1)		FM(MMC_D1)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_11_8	FM(NFDATA2)		FM(MMC_D2)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_15_12	FM(NFDATA3)		FM(MMC_D3)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_19_16	FM(NFDATA4)		FM(MMC_D4)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_23_20	FM(NFDATA5)		FM(MMC_D5)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_27_24	FM(NFDATA6)		FM(MMC_D6)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP9_31_28	FM(NFDATA7)		FM(MMC_D7)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_3_0	FM(AUDIO_CLKA)		F_(0, 0)		F_(0, 0)		F_(0, 0)	F_(0, 0)		FM(DVC_MUTE_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_7_4	FM(SSI_SCK34)		FM(FSO_CFE_0_N_A)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_11_8	FM(SSI_SDATA3)		FM(FSO_CFE_1_N_A)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_15_12	FM(SSI_WS34)		FM(FSO_TOE_N_A)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_19_16	FM(SSI_SCK4_A)		FM(HSCK0)		FM(AUDIO_CLKOUT)	FM(CAN0_RX_B)	FM(IRQ4_B)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_23_20	FM(SSI_SDATA4_A)	FM(HTX0)		FM(SCL2_A)		FM(CAN1_RX_B)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_27_24	FM(SSI_WS4_A)		FM(HRX0)		FM(SDA2_A)		FM(CAN1_TX_B)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP10_31_28	FM(SCL1)		FM(CTS1_N)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_3_0	FM(SDA1)		FM(RTS1_N)		F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_7_4	FM(MSIOF1_SCK)		FM(AVB0_AVTP_PPS_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_11_8	FM(MSIOF1_TXD)		FM(AVB0_AVTP_CAPTURE_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_15_12	FM(MSIOF1_RXD)		FM(AVB0_AVTP_MATCH_B)	F_(0, 0)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_19_16	FM(SCK0_A)		FM(MSIOF1_SYNC)		FM(FSO_CFE_0_N_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_23_20	FM(RX0_A)		FM(MSIOF0_SS1)		FM(FSO_CFE_1_N_B)	F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_27_24	FM(TX0_A)		FM(MSIOF0_SS2)		FM(FSO_TOE_N_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP11_31_28	FM(SCK1_A)		FM(MSIOF1_SS2)		FM(TPU0TO2_B)		FM(CAN0_TX_B)	FM(AUDIO_CLKOUT1)	F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* IPSRx */		/* 0 */			/* 1 */			/* 2 */			/* 3 */		/* 4 */			/* 5 */		/* 6  - F */
#define IP12_3_0	FM(RX1_A)		FM(CTS0_N)		FM(TPU0TO0_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_7_4	FM(TX1_A)		FM(RTS0_N)		FM(TPU0TO1_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_11_8	FM(SCK2)		FM(MSIOF1_SS1)		FM(TPU0TO3_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_15_12	FM(TPU0TO0_A)		FM(AVB0_AVTP_CAPTURE_A)	FM(HCTS0_N)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_19_16	FM(TPU0TO1_A)		FM(AVB0_AVTP_MATCH_A)	FM(HRTS0_N)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_23_20	FM(CAN_CLK)		FM(AVB0_AVTP_PPS_A)	FM(SCK0_B)		FM(IRQ5_B)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_27_24	FM(CAN0_RX_A)		FM(CANFD0_RX)		FM(RX0_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP12_31_28	FM(CAN0_TX_A)		FM(CANFD0_TX)		FM(TX0_B)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_3_0	FM(CAN1_RX_A)		FM(CANFD1_RX)		FM(TPU0TO2_A)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define IP13_7_4	FM(CAN1_TX_A)		FM(CANFD1_TX)		FM(TPU0TO3_A)		F_(0, 0)	F_(0, 0)		F_(0, 0)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

#define PINMUX_GPSR	\
\
		GPSR1_31	GPSR2_31			GPSR4_31		 \
		GPSR1_30	GPSR2_30			GPSR4_30		 \
		GPSR1_29	GPSR2_29			GPSR4_29		 \
		GPSR1_28	GPSR2_28			GPSR4_28		 \
		GPSR1_27	GPSR2_27			GPSR4_27		 \
		GPSR1_26	GPSR2_26			GPSR4_26		 \
		GPSR1_25	GPSR2_25			GPSR4_25		 \
		GPSR1_24	GPSR2_24			GPSR4_24		 \
		GPSR1_23	GPSR2_23			GPSR4_23		 \
		GPSR1_22	GPSR2_22			GPSR4_22		 \
		GPSR1_21	GPSR2_21			GPSR4_21		 \
		GPSR1_20	GPSR2_20			GPSR4_20	GPSR5_20 \
		GPSR1_19	GPSR2_19			GPSR4_19	GPSR5_19 \
		GPSR1_18	GPSR2_18			GPSR4_18	GPSR5_18 \
		GPSR1_17	GPSR2_17			GPSR4_17	GPSR5_17 \
		GPSR1_16	GPSR2_16			GPSR4_16	GPSR5_16 \
		GPSR1_15	GPSR2_15			GPSR4_15	GPSR5_15 \
		GPSR1_14	GPSR2_14			GPSR4_14	GPSR5_14 \
		GPSR1_13	GPSR2_13			GPSR4_13	GPSR5_13	GPSR6_13 \
		GPSR1_12	GPSR2_12			GPSR4_12	GPSR5_12	GPSR6_12 \
		GPSR1_11	GPSR2_11			GPSR4_11	GPSR5_11	GPSR6_11 \
		GPSR1_10	GPSR2_10			GPSR4_10	GPSR5_10	GPSR6_10 \
		GPSR1_9		GPSR2_9		GPSR3_9		GPSR4_9		GPSR5_9		GPSR6_9 \
GPSR0_8		GPSR1_8		GPSR2_8		GPSR3_8		GPSR4_8		GPSR5_8		GPSR6_8 \
GPSR0_7		GPSR1_7		GPSR2_7		GPSR3_7		GPSR4_7		GPSR5_7		GPSR6_7 \
GPSR0_6		GPSR1_6		GPSR2_6		GPSR3_6		GPSR4_6		GPSR5_6		GPSR6_6 \
GPSR0_5		GPSR1_5		GPSR2_5		GPSR3_5		GPSR4_5		GPSR5_5		GPSR6_5 \
GPSR0_4		GPSR1_4		GPSR2_4		GPSR3_4		GPSR4_4		GPSR5_4		GPSR6_4 \
GPSR0_3		GPSR1_3		GPSR2_3		GPSR3_3		GPSR4_3		GPSR5_3		GPSR6_3 \
GPSR0_2		GPSR1_2		GPSR2_2		GPSR3_2		GPSR4_2		GPSR5_2		GPSR6_2 \
GPSR0_1		GPSR1_1		GPSR2_1		GPSR3_1		GPSR4_1		GPSR5_1		GPSR6_1 \
GPSR0_0		GPSR1_0		GPSR2_0		GPSR3_0		GPSR4_0		GPSR5_0		GPSR6_0

#define PINMUX_IPSR				\
\
FM(IP0_3_0)	IP0_3_0		FM(IP1_3_0)	IP1_3_0		FM(IP2_3_0)	IP2_3_0		FM(IP3_3_0)	IP3_3_0 \
FM(IP0_7_4)	IP0_7_4		FM(IP1_7_4)	IP1_7_4		FM(IP2_7_4)	IP2_7_4		FM(IP3_7_4)	IP3_7_4 \
FM(IP0_11_8)	IP0_11_8	FM(IP1_11_8)	IP1_11_8	FM(IP2_11_8)	IP2_11_8	FM(IP3_11_8)	IP3_11_8 \
FM(IP0_15_12)	IP0_15_12	FM(IP1_15_12)	IP1_15_12	FM(IP2_15_12)	IP2_15_12	FM(IP3_15_12)	IP3_15_12 \
FM(IP0_19_16)	IP0_19_16	FM(IP1_19_16)	IP1_19_16	FM(IP2_19_16)	IP2_19_16	FM(IP3_19_16)	IP3_19_16 \
FM(IP0_23_20)	IP0_23_20	FM(IP1_23_20)	IP1_23_20	FM(IP2_23_20)	IP2_23_20	FM(IP3_23_20)	IP3_23_20 \
FM(IP0_27_24)	IP0_27_24	FM(IP1_27_24)	IP1_27_24	FM(IP2_27_24)	IP2_27_24	FM(IP3_27_24)	IP3_27_24 \
FM(IP0_31_28)	IP0_31_28	FM(IP1_31_28)	IP1_31_28	FM(IP2_31_28)	IP2_31_28	FM(IP3_31_28)	IP3_31_28 \
\
FM(IP4_3_0)	IP4_3_0		FM(IP5_3_0)	IP5_3_0		FM(IP6_3_0)	IP6_3_0		FM(IP7_3_0)	IP7_3_0 \
FM(IP4_7_4)	IP4_7_4		FM(IP5_7_4)	IP5_7_4		FM(IP6_7_4)	IP6_7_4		FM(IP7_7_4)	IP7_7_4 \
FM(IP4_11_8)	IP4_11_8	FM(IP5_11_8)	IP5_11_8	FM(IP6_11_8)	IP6_11_8	FM(IP7_11_8)	IP7_11_8 \
FM(IP4_15_12)	IP4_15_12	FM(IP5_15_12)	IP5_15_12	FM(IP6_15_12)	IP6_15_12	FM(IP7_15_12)	IP7_15_12 \
FM(IP4_19_16)	IP4_19_16	FM(IP5_19_16)	IP5_19_16	FM(IP6_19_16)	IP6_19_16	FM(IP7_19_16)	IP7_19_16 \
FM(IP4_23_20)	IP4_23_20	FM(IP5_23_20)	IP5_23_20	FM(IP6_23_20)	IP6_23_20	FM(IP7_23_20)	IP7_23_20 \
FM(IP4_27_24)	IP4_27_24	FM(IP5_27_24)	IP5_27_24	FM(IP6_27_24)	IP6_27_24	FM(IP7_27_24)	IP7_27_24 \
FM(IP4_31_28)	IP4_31_28	FM(IP5_31_28)	IP5_31_28	FM(IP6_31_28)	IP6_31_28	FM(IP7_31_28)	IP7_31_28 \
\
FM(IP8_3_0)	IP8_3_0		FM(IP9_3_0)	IP9_3_0		FM(IP10_3_0)	IP10_3_0	FM(IP11_3_0)	IP11_3_0 \
FM(IP8_7_4)	IP8_7_4		FM(IP9_7_4)	IP9_7_4		FM(IP10_7_4)	IP10_7_4	FM(IP11_7_4)	IP11_7_4 \
FM(IP8_11_8)	IP8_11_8	FM(IP9_11_8)	IP9_11_8	FM(IP10_11_8)	IP10_11_8	FM(IP11_11_8)	IP11_11_8 \
FM(IP8_15_12)	IP8_15_12	FM(IP9_15_12)	IP9_15_12	FM(IP10_15_12)	IP10_15_12	FM(IP11_15_12)	IP11_15_12 \
FM(IP8_19_16)	IP8_19_16	FM(IP9_19_16)	IP9_19_16	FM(IP10_19_16)	IP10_19_16	FM(IP11_19_16)	IP11_19_16 \
FM(IP8_23_20)	IP8_23_20	FM(IP9_23_20)	IP9_23_20	FM(IP10_23_20)	IP10_23_20	FM(IP11_23_20)	IP11_23_20 \
FM(IP8_27_24)	IP8_27_24	FM(IP9_27_24)	IP9_27_24	FM(IP10_27_24)	IP10_27_24	FM(IP11_27_24)	IP11_27_24 \
FM(IP8_31_28)	IP8_31_28	FM(IP9_31_28)	IP9_31_28	FM(IP10_31_28)	IP10_31_28	FM(IP11_31_28)	IP11_31_28 \
\
FM(IP12_3_0)	IP12_3_0	FM(IP13_3_0)	IP13_3_0 \
FM(IP12_7_4)	IP12_7_4	FM(IP13_7_4)	IP13_7_4 \
FM(IP12_11_8)	IP12_11_8 \
FM(IP12_15_12)	IP12_15_12 \
FM(IP12_19_16)	IP12_19_16 \
FM(IP12_23_20)	IP12_23_20 \
FM(IP12_27_24)	IP12_27_24 \
FM(IP12_31_28)	IP12_31_28 \

/* The bit numbering in MOD_SEL fields is reversed */
#define REV4(f0, f1, f2, f3)			f0 f2 f1 f3

/* MOD_SEL0 */			/* 0 */			/* 1 */			/* 2 */			/* 3 */
#define MOD_SEL0_30		FM(SEL_MSIOF2_0)	FM(SEL_MSIOF2_1)
#define MOD_SEL0_29		FM(SEL_I2C3_0)		FM(SEL_I2C3_1)
#define MOD_SEL0_28		FM(SEL_SCIF5_0)		FM(SEL_SCIF5_1)
#define MOD_SEL0_27		FM(SEL_MSIOF3_0)	FM(SEL_MSIOF3_1)
#define MOD_SEL0_26		FM(SEL_HSCIF3_0)	FM(SEL_HSCIF3_1)
#define MOD_SEL0_25		FM(SEL_SCIF4_0)		FM(SEL_SCIF4_1)
#define MOD_SEL0_24_23	   REV4(FM(SEL_PWM0_0),		FM(SEL_PWM0_1),		FM(SEL_PWM0_2),		F_(0, 0))
#define MOD_SEL0_22_21	   REV4(FM(SEL_PWM1_0),		FM(SEL_PWM1_1),		FM(SEL_PWM1_2),		F_(0, 0))
#define MOD_SEL0_20_19	   REV4(FM(SEL_PWM2_0),		FM(SEL_PWM2_1),		FM(SEL_PWM2_2),		F_(0, 0))
#define MOD_SEL0_18_17	   REV4(FM(SEL_PWM3_0),		FM(SEL_PWM3_1),		FM(SEL_PWM3_2),		F_(0, 0))
#define MOD_SEL0_15		FM(SEL_IRQ_0_0)		FM(SEL_IRQ_0_1)
#define MOD_SEL0_14		FM(SEL_IRQ_1_0)		FM(SEL_IRQ_1_1)
#define MOD_SEL0_13		FM(SEL_IRQ_2_0)		FM(SEL_IRQ_2_1)
#define MOD_SEL0_12		FM(SEL_IRQ_3_0)		FM(SEL_IRQ_3_1)
#define MOD_SEL0_11		FM(SEL_IRQ_4_0)		FM(SEL_IRQ_4_1)
#define MOD_SEL0_10		FM(SEL_IRQ_5_0)		FM(SEL_IRQ_5_1)
#define MOD_SEL0_5		FM(SEL_TMU_0_0)		FM(SEL_TMU_0_1)
#define MOD_SEL0_4		FM(SEL_TMU_1_0)		FM(SEL_TMU_1_1)
#define MOD_SEL0_3		FM(SEL_SCIF3_0)		FM(SEL_SCIF3_1)
#define MOD_SEL0_2		FM(SEL_SCIF1_0)		FM(SEL_SCIF1_1)
#define MOD_SEL0_1		FM(SEL_SCU_0)		FM(SEL_SCU_1)
#define MOD_SEL0_0		FM(SEL_RFSO_0)		FM(SEL_RFSO_1)

#define MOD_SEL1_31		FM(SEL_CAN0_0)		FM(SEL_CAN0_1)
#define MOD_SEL1_30		FM(SEL_CAN1_0)		FM(SEL_CAN1_1)
#define MOD_SEL1_29		FM(SEL_I2C2_0)		FM(SEL_I2C2_1)
#define MOD_SEL1_28		FM(SEL_ETHERAVB_0)	FM(SEL_ETHERAVB_1)
#define MOD_SEL1_27		FM(SEL_SCIF0_0)		FM(SEL_SCIF0_1)
#define MOD_SEL1_26		FM(SEL_SSIF4_0)		FM(SEL_SSIF4_1)

#define PINMUX_MOD_SELS	\
\
		MOD_SEL1_31 \
MOD_SEL0_30	MOD_SEL1_30 \
MOD_SEL0_29	MOD_SEL1_29 \
MOD_SEL0_28	MOD_SEL1_28 \
MOD_SEL0_27	MOD_SEL1_27 \
MOD_SEL0_26	MOD_SEL1_26 \
MOD_SEL0_25 \
MOD_SEL0_24_23 \
MOD_SEL0_22_21 \
MOD_SEL0_20_19 \
MOD_SEL0_18_17 \
MOD_SEL0_15 \
MOD_SEL0_14 \
MOD_SEL0_13 \
MOD_SEL0_12 \
MOD_SEL0_11 \
MOD_SEL0_10 \
MOD_SEL0_5 \
MOD_SEL0_4 \
MOD_SEL0_3 \
MOD_SEL0_2 \
MOD_SEL0_1 \
MOD_SEL0_0

enum {
	PINMUX_RESERVED = 0,

	PINMUX_DATA_BEGIN,
	GP_ALL(DATA),
	PINMUX_DATA_END,

#define F_(x, y)
#define FM(x)	FN_##x,
	PINMUX_FUNCTION_BEGIN,
	GP_ALL(FN),
	PINMUX_GPSR
	PINMUX_IPSR
	PINMUX_MOD_SELS
	PINMUX_FUNCTION_END,
#undef F_
#undef FM

#define F_(x, y)
#define FM(x)	x##_MARK,
	PINMUX_MARK_BEGIN,
	PINMUX_GPSR
	PINMUX_IPSR
	PINMUX_MOD_SELS
	PINMUX_MARK_END,
#undef F_
#undef FM
};

static const u16 pinmux_data[] = {
	PINMUX_DATA_GP_ALL(),

	PINMUX_SINGLE(USB0_OVC),
	PINMUX_SINGLE(USB0_PWEN),
	PINMUX_SINGLE(VI4_DATA4),
	PINMUX_SINGLE(VI4_CLK),
	PINMUX_SINGLE(TX2),
	PINMUX_SINGLE(RX2),
	PINMUX_SINGLE(AVB0_LINK),
	PINMUX_SINGLE(AVB0_PHY_INT),
	PINMUX_SINGLE(AVB0_MAGIC),
	PINMUX_SINGLE(AVB0_MDC),
	PINMUX_SINGLE(AVB0_MDIO),
	PINMUX_SINGLE(AVB0_TXCREFCLK),
	PINMUX_SINGLE(AVB0_TD3),
	PINMUX_SINGLE(AVB0_TD2),
	PINMUX_SINGLE(AVB0_TD1),
	PINMUX_SINGLE(AVB0_TD0),
	PINMUX_SINGLE(AVB0_TXC),
	PINMUX_SINGLE(AVB0_TX_CTL),
	PINMUX_SINGLE(AVB0_RD3),
	PINMUX_SINGLE(AVB0_RD2),
	PINMUX_SINGLE(AVB0_RD1),
	PINMUX_SINGLE(AVB0_RD0),
	PINMUX_SINGLE(AVB0_RXC),
	PINMUX_SINGLE(AVB0_RX_CTL),
	PINMUX_SINGLE(RPC_INT_N),
	PINMUX_SINGLE(RPC_RESET_N),
	PINMUX_SINGLE(QSPI1_SSL),
	PINMUX_SINGLE(QSPI1_IO3),
	PINMUX_SINGLE(QSPI1_IO2),
	PINMUX_SINGLE(QSPI1_MISO_IO1),
	PINMUX_SINGLE(QSPI1_MOSI_IO0),
	PINMUX_SINGLE(QSPI1_SPCLK),
	PINMUX_SINGLE(QSPI0_SSL),
	PINMUX_SINGLE(QSPI0_IO3),
	PINMUX_SINGLE(QSPI0_IO2),
	PINMUX_SINGLE(QSPI0_MISO_IO1),
	PINMUX_SINGLE(QSPI0_MOSI_IO0),
	PINMUX_SINGLE(QSPI0_SPCLK),
	PINMUX_SINGLE(SCL0),
	PINMUX_SINGLE(SDA0),
	PINMUX_SINGLE(MSIOF0_RXD),
	PINMUX_SINGLE(MSIOF0_TXD),
	PINMUX_SINGLE(MSIOF0_SYNC),
	PINMUX_SINGLE(MSIOF0_SCK),

	/* IPSR0 */
	PINMUX_IPSR_MSEL(IP0_3_0,	IRQ0_A, SEL_IRQ_0_0),
	PINMUX_IPSR_MSEL(IP0_3_0,	MSIOF2_SYNC_B, SEL_MSIOF2_1),

	PINMUX_IPSR_GPSR(IP0_7_4,	MSIOF2_SCK),

	PINMUX_IPSR_GPSR(IP0_11_8,	MSIOF2_TXD),
	PINMUX_IPSR_MSEL(IP0_11_8,	SCL3_A, SEL_I2C3_0),

	PINMUX_IPSR_GPSR(IP0_15_12,	MSIOF2_RXD),
	PINMUX_IPSR_MSEL(IP0_15_12,	SDA3_A, SEL_I2C3_0),

	PINMUX_IPSR_GPSR(IP0_19_16,	MLB_CLK),
	PINMUX_IPSR_MSEL(IP0_19_16,	MSIOF2_SYNC_A, SEL_MSIOF2_0),
	PINMUX_IPSR_MSEL(IP0_19_16,	SCK5_A, SEL_SCIF5_0),

	PINMUX_IPSR_GPSR(IP0_23_20,	MLB_DAT),
	PINMUX_IPSR_GPSR(IP0_23_20,	MSIOF2_SS1),
	PINMUX_IPSR_MSEL(IP0_23_20,	RX5_A, SEL_SCIF5_0),
	PINMUX_IPSR_MSEL(IP0_23_20,	SCL3_B, SEL_I2C3_1),

	PINMUX_IPSR_GPSR(IP0_27_24,	MLB_SIG),
	PINMUX_IPSR_GPSR(IP0_27_24,	MSIOF2_SS2),
	PINMUX_IPSR_MSEL(IP0_27_24,	TX5_A, SEL_SCIF5_0),
	PINMUX_IPSR_MSEL(IP0_27_24,	SDA3_B, SEL_I2C3_1),

	PINMUX_IPSR_GPSR(IP0_31_28,	DU_DB0),
	PINMUX_IPSR_GPSR(IP0_31_28,	LCDOUT0),
	PINMUX_IPSR_MSEL(IP0_31_28,	MSIOF3_TXD_B, SEL_MSIOF3_1),

	/* IPSR1 */
	PINMUX_IPSR_GPSR(IP1_3_0,	DU_DB1),
	PINMUX_IPSR_GPSR(IP1_3_0,	LCDOUT1),
	PINMUX_IPSR_MSEL(IP1_3_0,	MSIOF3_RXD_B, SEL_MSIOF3_1),

	PINMUX_IPSR_GPSR(IP1_7_4,	DU_DB2),
	PINMUX_IPSR_GPSR(IP1_7_4,	LCDOUT2),
	PINMUX_IPSR_MSEL(IP1_7_4,	IRQ0_B, SEL_IRQ_0_1),

	PINMUX_IPSR_GPSR(IP1_11_8,	DU_DB3),
	PINMUX_IPSR_GPSR(IP1_11_8,	LCDOUT3),
	PINMUX_IPSR_MSEL(IP1_11_8,	SCK5_B, SEL_SCIF5_1),

	PINMUX_IPSR_GPSR(IP1_15_12,	DU_DB4),
	PINMUX_IPSR_GPSR(IP1_15_12,	LCDOUT4),
	PINMUX_IPSR_MSEL(IP1_15_12,	RX5_B, SEL_SCIF5_1),

	PINMUX_IPSR_GPSR(IP1_19_16,	DU_DB5),
	PINMUX_IPSR_GPSR(IP1_19_16,	LCDOUT5),
	PINMUX_IPSR_MSEL(IP1_19_16,	TX5_B, SEL_SCIF5_1),

	PINMUX_IPSR_GPSR(IP1_23_20,	DU_DB6),
	PINMUX_IPSR_GPSR(IP1_23_20,	LCDOUT6),
	PINMUX_IPSR_MSEL(IP1_23_20,	MSIOF3_SS1_B, SEL_MSIOF3_1),

	PINMUX_IPSR_GPSR(IP1_27_24,	DU_DB7),
	PINMUX_IPSR_GPSR(IP1_27_24,	LCDOUT7),
	PINMUX_IPSR_MSEL(IP1_27_24,	MSIOF3_SS2_B, SEL_MSIOF3_1),

	PINMUX_IPSR_GPSR(IP1_31_28,	DU_DG0),
	PINMUX_IPSR_GPSR(IP1_31_28,	LCDOUT8),
	PINMUX_IPSR_MSEL(IP1_31_28,	MSIOF3_SCK_B, SEL_MSIOF3_1),

	/* IPSR2 */
	PINMUX_IPSR_GPSR(IP2_3_0,	DU_DG1),
	PINMUX_IPSR_GPSR(IP2_3_0,	LCDOUT9),
	PINMUX_IPSR_MSEL(IP2_3_0,	MSIOF3_SYNC_B, SEL_MSIOF3_1),

	PINMUX_IPSR_GPSR(IP2_7_4,	DU_DG2),
	PINMUX_IPSR_GPSR(IP2_7_4,	LCDOUT10),

	PINMUX_IPSR_GPSR(IP2_11_8,	DU_DG3),
	PINMUX_IPSR_GPSR(IP2_11_8,	LCDOUT11),
	PINMUX_IPSR_MSEL(IP2_11_8,	IRQ1_A, SEL_IRQ_1_0),

	PINMUX_IPSR_GPSR(IP2_15_12,	DU_DG4),
	PINMUX_IPSR_GPSR(IP2_15_12,	LCDOUT12),
	PINMUX_IPSR_MSEL(IP2_15_12,	HSCK3_B, SEL_HSCIF3_1),

	PINMUX_IPSR_GPSR(IP2_19_16,	DU_DG5),
	PINMUX_IPSR_GPSR(IP2_19_16,	LCDOUT13),
	PINMUX_IPSR_MSEL(IP2_19_16,	HTX3_B, SEL_HSCIF3_1),

	PINMUX_IPSR_GPSR(IP2_23_20,	DU_DG6),
	PINMUX_IPSR_GPSR(IP2_23_20,	LCDOUT14),
	PINMUX_IPSR_MSEL(IP2_23_20,	HRX3_B, SEL_HSCIF3_1),

	PINMUX_IPSR_GPSR(IP2_27_24,	DU_DG7),
	PINMUX_IPSR_GPSR(IP2_27_24,	LCDOUT15),
	PINMUX_IPSR_MSEL(IP2_27_24,	SCK4_B, SEL_SCIF4_1),

	PINMUX_IPSR_GPSR(IP2_31_28,	DU_DR0),
	PINMUX_IPSR_GPSR(IP2_31_28,	LCDOUT16),
	PINMUX_IPSR_MSEL(IP2_31_28,	RX4_B, SEL_SCIF4_1),

	/* IPSR3 */
	PINMUX_IPSR_GPSR(IP3_3_0,	DU_DR1),
	PINMUX_IPSR_GPSR(IP3_3_0,	LCDOUT17),
	PINMUX_IPSR_MSEL(IP3_3_0,	TX4_B, SEL_SCIF4_1),

	PINMUX_IPSR_GPSR(IP3_7_4,	DU_DR2),
	PINMUX_IPSR_GPSR(IP3_7_4,	LCDOUT18),
	PINMUX_IPSR_MSEL(IP3_7_4,	PWM0_B, SEL_PWM0_2),

	PINMUX_IPSR_GPSR(IP3_11_8,	DU_DR3),
	PINMUX_IPSR_GPSR(IP3_11_8,	LCDOUT19),
	PINMUX_IPSR_MSEL(IP3_11_8,	PWM1_B, SEL_PWM1_2),

	PINMUX_IPSR_GPSR(IP3_15_12,	DU_DR4),
	PINMUX_IPSR_GPSR(IP3_15_12,	LCDOUT20),
	PINMUX_IPSR_MSEL(IP3_15_12,	TCLK2_B, SEL_TMU_0_1),

	PINMUX_IPSR_GPSR(IP3_19_16,	DU_DR5),
	PINMUX_IPSR_GPSR(IP3_19_16,	LCDOUT21),
	PINMUX_IPSR_GPSR(IP3_19_16,	NMI),

	PINMUX_IPSR_GPSR(IP3_23_20,	DU_DR6),
	PINMUX_IPSR_GPSR(IP3_23_20,	LCDOUT22),
	PINMUX_IPSR_MSEL(IP3_23_20,	PWM2_B, SEL_PWM2_2),

	PINMUX_IPSR_GPSR(IP3_27_24,	DU_DR7),
	PINMUX_IPSR_GPSR(IP3_27_24,	LCDOUT23),
	PINMUX_IPSR_MSEL(IP3_27_24,	TCLK1_B, SEL_TMU_1_1),

	PINMUX_IPSR_GPSR(IP3_31_28,	DU_DOTCLKOUT0),
	PINMUX_IPSR_GPSR(IP3_31_28,	QCLK),

	/* IPSR4 */
	PINMUX_IPSR_GPSR(IP4_3_0,	DU_HSYNC),
	PINMUX_IPSR_GPSR(IP4_3_0,	QSTH_QHS),
	PINMUX_IPSR_MSEL(IP4_3_0,	IRQ3_A, SEL_IRQ_3_0),

	PINMUX_IPSR_GPSR(IP4_7_4,	DU_VSYNC),
	PINMUX_IPSR_GPSR(IP4_7_4,	QSTVA_QVS),
	PINMUX_IPSR_MSEL(IP4_7_4,	IRQ4_A, SEL_IRQ_4_0),

	PINMUX_IPSR_GPSR(IP4_11_8,	DU_DISP),
	PINMUX_IPSR_GPSR(IP4_11_8,	QSTVB_QVE),
	PINMUX_IPSR_MSEL(IP4_11_8,	PWM3_B, SEL_PWM3_2),

	PINMUX_IPSR_GPSR(IP4_15_12,	DU_DISP_CDE),
	PINMUX_IPSR_GPSR(IP4_15_12,	QCPV_QDE),
	PINMUX_IPSR_MSEL(IP4_15_12,	IRQ2_B, SEL_IRQ_2_1),
	PINMUX_IPSR_GPSR(IP4_15_12,	DU_DOTCLKIN1),

	PINMUX_IPSR_GPSR(IP4_19_16,	DU_CDE),
	PINMUX_IPSR_GPSR(IP4_19_16,	QSTB_QHE),
	PINMUX_IPSR_MSEL(IP4_19_16,	SCK3_B, SEL_SCIF3_1),

	PINMUX_IPSR_GPSR(IP4_23_20,	QPOLA),
	PINMUX_IPSR_MSEL(IP4_23_20,	RX3_B, SEL_SCIF3_1),

	PINMUX_IPSR_GPSR(IP4_27_24,	QPOLB),
	PINMUX_IPSR_MSEL(IP4_27_24,	TX3_B, SEL_SCIF3_1),

	PINMUX_IPSR_GPSR(IP4_31_28,	VI4_DATA0),
	PINMUX_IPSR_MSEL(IP4_31_28,	PWM0_A, SEL_PWM0_0),

	/* IPSR5 */
	PINMUX_IPSR_GPSR(IP5_3_0,	VI4_DATA1),
	PINMUX_IPSR_MSEL(IP5_3_0,	PWM1_A, SEL_PWM1_0),

	PINMUX_IPSR_GPSR(IP5_7_4,	VI4_DATA2),
	PINMUX_IPSR_MSEL(IP5_7_4,	PWM2_A, SEL_PWM2_0),

	PINMUX_IPSR_GPSR(IP5_11_8,	VI4_DATA3),
	PINMUX_IPSR_MSEL(IP5_11_8,	PWM3_A, SEL_PWM3_0),

	PINMUX_IPSR_GPSR(IP5_15_12,	VI4_DATA5),
	PINMUX_IPSR_MSEL(IP5_15_12,	SCK4_A, SEL_SCIF4_0),

	PINMUX_IPSR_GPSR(IP5_19_16,	VI4_DATA6),
	PINMUX_IPSR_MSEL(IP5_19_16,	IRQ2_A, SEL_IRQ_2_0),

	PINMUX_IPSR_GPSR(IP5_23_20,	VI4_DATA7),
	PINMUX_IPSR_MSEL(IP5_23_20,	TCLK2_A, SEL_TMU_0_0),

	PINMUX_IPSR_GPSR(IP5_27_24,	VI4_DATA8),

	PINMUX_IPSR_GPSR(IP5_31_28,	VI4_DATA9),
	PINMUX_IPSR_MSEL(IP5_31_28,	MSIOF3_SS2_A, SEL_MSIOF3_0),
	PINMUX_IPSR_MSEL(IP5_31_28,	IRQ1_B, SEL_IRQ_1_1),

	/* IPSR6 */
	PINMUX_IPSR_GPSR(IP6_3_0,	VI4_DATA10),
	PINMUX_IPSR_MSEL(IP6_3_0,	RX4_A, SEL_SCIF4_0),

	PINMUX_IPSR_GPSR(IP6_7_4,	VI4_DATA11),
	PINMUX_IPSR_MSEL(IP6_7_4,	TX4_A, SEL_SCIF4_0),

	PINMUX_IPSR_GPSR(IP6_11_8,	VI4_DATA12),
	PINMUX_IPSR_MSEL(IP6_11_8,	TCLK1_A, SEL_TMU_1_0),

	PINMUX_IPSR_GPSR(IP6_15_12,	VI4_DATA13),
	PINMUX_IPSR_MSEL(IP6_15_12,	MSIOF3_SS1_A, SEL_MSIOF3_0),
	PINMUX_IPSR_GPSR(IP6_15_12,	HCTS3_N),

	PINMUX_IPSR_GPSR(IP6_19_16,	VI4_DATA14),
	PINMUX_IPSR_MSEL(IP6_19_16,	SSI_SCK4_B, SEL_SSIF4_1),
	PINMUX_IPSR_GPSR(IP6_19_16,	HRTS3_N),

	PINMUX_IPSR_GPSR(IP6_23_20,	VI4_DATA15),
	PINMUX_IPSR_MSEL(IP6_23_20,	SSI_SDATA4_B, SEL_SSIF4_1),

	PINMUX_IPSR_GPSR(IP6_27_24,	VI4_DATA16),
	PINMUX_IPSR_MSEL(IP6_27_24,	HRX3_A, SEL_HSCIF3_0),

	PINMUX_IPSR_GPSR(IP6_31_28,	VI4_DATA17),
	PINMUX_IPSR_MSEL(IP6_31_28,	HTX3_A, SEL_HSCIF3_0),

	/* IPSR7 */
	PINMUX_IPSR_GPSR(IP7_3_0,	VI4_DATA18),
	PINMUX_IPSR_MSEL(IP7_3_0,	HSCK3_A, SEL_HSCIF3_0),

	PINMUX_IPSR_GPSR(IP7_7_4,	VI4_DATA19),
	PINMUX_IPSR_MSEL(IP7_7_4,	SSI_WS4_B, SEL_SSIF4_1),
	PINMUX_IPSR_GPSR(IP7_7_4,	NFDATA15),

	PINMUX_IPSR_GPSR(IP7_11_8,	VI4_DATA20),
	PINMUX_IPSR_MSEL(IP7_11_8,	MSIOF3_SYNC_A, SEL_MSIOF3_0),
	PINMUX_IPSR_GPSR(IP7_11_8,	NFDATA14),

	PINMUX_IPSR_GPSR(IP7_15_12,	VI4_DATA21),
	PINMUX_IPSR_MSEL(IP7_15_12,	MSIOF3_TXD_A, SEL_MSIOF3_0),

	PINMUX_IPSR_GPSR(IP7_15_12,	NFDATA13),
	PINMUX_IPSR_GPSR(IP7_19_16,	VI4_DATA22),
	PINMUX_IPSR_MSEL(IP7_19_16,	MSIOF3_RXD_A, SEL_MSIOF3_0),

	PINMUX_IPSR_GPSR(IP7_19_16,	NFDATA12),
	PINMUX_IPSR_GPSR(IP7_23_20,	VI4_DATA23),
	PINMUX_IPSR_MSEL(IP7_23_20,	MSIOF3_SCK_A, SEL_MSIOF3_0),

	PINMUX_IPSR_GPSR(IP7_23_20,	NFDATA11),

	PINMUX_IPSR_GPSR(IP7_27_24,	VI4_VSYNC_N),
	PINMUX_IPSR_MSEL(IP7_27_24,	SCK1_B, SEL_SCIF1_1),
	PINMUX_IPSR_GPSR(IP7_27_24,	NFDATA10),

	PINMUX_IPSR_GPSR(IP7_31_28,	VI4_HSYNC_N),
	PINMUX_IPSR_MSEL(IP7_31_28,	RX1_B, SEL_SCIF1_1),
	PINMUX_IPSR_GPSR(IP7_31_28,	NFDATA9),

	/* IPSR8 */
	PINMUX_IPSR_GPSR(IP8_3_0,	VI4_FIELD),
	PINMUX_IPSR_GPSR(IP8_3_0,	AUDIO_CLKB),
	PINMUX_IPSR_MSEL(IP8_3_0,	IRQ5_A, SEL_IRQ_5_0),
	PINMUX_IPSR_GPSR(IP8_3_0,	SCIF_CLK),
	PINMUX_IPSR_GPSR(IP8_3_0,	NFDATA8),

	PINMUX_IPSR_GPSR(IP8_7_4,	VI4_CLKENB),
	PINMUX_IPSR_MSEL(IP8_7_4,	TX1_B, SEL_SCIF1_1),
	PINMUX_IPSR_GPSR(IP8_7_4,	NFWP_N),
	PINMUX_IPSR_MSEL(IP8_7_4,	DVC_MUTE_A, SEL_SCU_0),

	PINMUX_IPSR_GPSR(IP8_11_8,	NFALE),
	PINMUX_IPSR_MSEL(IP8_11_8,	SCL2_B, SEL_I2C2_1),
	PINMUX_IPSR_MSEL(IP8_11_8,	IRQ3_B, SEL_IRQ_3_1),
	PINMUX_IPSR_MSEL(IP8_11_8,	PWM0_C, SEL_PWM0_1),

	PINMUX_IPSR_GPSR(IP8_15_12,	NFCLE),
	PINMUX_IPSR_MSEL(IP8_15_12,	SDA2_B, SEL_I2C2_1),
	PINMUX_IPSR_MSEL(IP8_15_12,	SCK3_A, SEL_SCIF3_0),
	PINMUX_IPSR_MSEL(IP8_15_12,	PWM1_C, SEL_PWM1_1),

	PINMUX_IPSR_GPSR(IP8_19_16,	NFCE_N),
	PINMUX_IPSR_MSEL(IP8_19_16,	RX3_A, SEL_SCIF3_0),
	PINMUX_IPSR_MSEL(IP8_19_16,	PWM2_C, SEL_PWM2_1),

	PINMUX_IPSR_GPSR(IP8_23_20,	NFRB_N),
	PINMUX_IPSR_MSEL(IP8_23_20,	TX3_A, SEL_SCIF3_0),
	PINMUX_IPSR_MSEL(IP8_23_20,	PWM3_C, SEL_PWM3_1),

	PINMUX_IPSR_GPSR(IP8_27_24,	NFRE_N),
	PINMUX_IPSR_GPSR(IP8_27_24,	MMC_CMD),

	PINMUX_IPSR_GPSR(IP8_31_28,	NFWE_N),
	PINMUX_IPSR_GPSR(IP8_31_28,	MMC_CLK),

	/* IPSR9 */
	PINMUX_IPSR_GPSR(IP9_3_0,	NFDATA0),
	PINMUX_IPSR_GPSR(IP9_3_0,	MMC_D0),

	PINMUX_IPSR_GPSR(IP9_7_4,	NFDATA1),
	PINMUX_IPSR_GPSR(IP9_7_4,	MMC_D1),

	PINMUX_IPSR_GPSR(IP9_11_8,	NFDATA2),
	PINMUX_IPSR_GPSR(IP9_11_8,	MMC_D2),

	PINMUX_IPSR_GPSR(IP9_15_12,	NFDATA3),
	PINMUX_IPSR_GPSR(IP9_15_12,	MMC_D3),

	PINMUX_IPSR_GPSR(IP9_19_16,	NFDATA4),
	PINMUX_IPSR_GPSR(IP9_19_16,	MMC_D4),

	PINMUX_IPSR_GPSR(IP9_23_20,	NFDATA5),
	PINMUX_IPSR_GPSR(IP9_23_20,	MMC_D5),

	PINMUX_IPSR_GPSR(IP9_27_24,	NFDATA6),
	PINMUX_IPSR_GPSR(IP9_27_24,	MMC_D6),

	PINMUX_IPSR_GPSR(IP9_31_28,	NFDATA7),
	PINMUX_IPSR_GPSR(IP9_31_28,	MMC_D7),

	/* IPSR10 */
	PINMUX_IPSR_GPSR(IP10_3_0,	AUDIO_CLKA),
	PINMUX_IPSR_MSEL(IP10_3_0,	DVC_MUTE_B, SEL_SCU_1),

	PINMUX_IPSR_GPSR(IP10_7_4,	SSI_SCK34),
	PINMUX_IPSR_MSEL(IP10_7_4,	FSO_CFE_0_N_A, SEL_RFSO_0),

	PINMUX_IPSR_GPSR(IP10_11_8,	SSI_SDATA3),
	PINMUX_IPSR_MSEL(IP10_11_8,	FSO_CFE_1_N_A, SEL_RFSO_0),

	PINMUX_IPSR_GPSR(IP10_15_12,	SSI_WS34),
	PINMUX_IPSR_MSEL(IP10_15_12,	FSO_TOE_N_A, SEL_RFSO_0),

	PINMUX_IPSR_MSEL(IP10_19_16,	SSI_SCK4_A, SEL_SSIF4_0),
	PINMUX_IPSR_GPSR(IP10_19_16,	HSCK0),
	PINMUX_IPSR_GPSR(IP10_19_16,	AUDIO_CLKOUT),
	PINMUX_IPSR_MSEL(IP10_19_16,	CAN0_RX_B, SEL_CAN0_1),
	PINMUX_IPSR_MSEL(IP10_19_16,	IRQ4_B, SEL_IRQ_4_1),

	PINMUX_IPSR_MSEL(IP10_23_20,	SSI_SDATA4_A, SEL_SSIF4_0),
	PINMUX_IPSR_GPSR(IP10_23_20,	HTX0),
	PINMUX_IPSR_MSEL(IP10_23_20,	SCL2_A, SEL_I2C2_0),
	PINMUX_IPSR_MSEL(IP10_23_20,	CAN1_RX_B, SEL_CAN1_1),

	PINMUX_IPSR_MSEL(IP10_27_24,	SSI_WS4_A, SEL_SSIF4_0),
	PINMUX_IPSR_GPSR(IP10_27_24,	HRX0),
	PINMUX_IPSR_MSEL(IP10_27_24,	SDA2_A, SEL_I2C2_0),
	PINMUX_IPSR_MSEL(IP10_27_24,	CAN1_TX_B, SEL_CAN1_1),

	PINMUX_IPSR_GPSR(IP10_31_28,	SCL1),
	PINMUX_IPSR_GPSR(IP10_31_28,	CTS1_N),

	/* IPSR11 */
	PINMUX_IPSR_GPSR(IP11_3_0,	SDA1),
	PINMUX_IPSR_GPSR(IP11_3_0,	RTS1_N),

	PINMUX_IPSR_GPSR(IP11_7_4,	MSIOF1_SCK),
	PINMUX_IPSR_MSEL(IP11_7_4,	AVB0_AVTP_PPS_B, SEL_ETHERAVB_1),

	PINMUX_IPSR_GPSR(IP11_11_8,	MSIOF1_TXD),
	PINMUX_IPSR_MSEL(IP11_11_8,	AVB0_AVTP_CAPTURE_B, SEL_ETHERAVB_1),

	PINMUX_IPSR_GPSR(IP11_15_12,	MSIOF1_RXD),
	PINMUX_IPSR_MSEL(IP11_15_12,	AVB0_AVTP_MATCH_B, SEL_ETHERAVB_1),

	PINMUX_IPSR_MSEL(IP11_19_16,	SCK0_A, SEL_SCIF0_0),
	PINMUX_IPSR_GPSR(IP11_19_16,	MSIOF1_SYNC),
	PINMUX_IPSR_MSEL(IP11_19_16,	FSO_CFE_0_N_B, SEL_RFSO_1),

	PINMUX_IPSR_MSEL(IP11_23_20,	RX0_A, SEL_SCIF0_0),
	PINMUX_IPSR_GPSR(IP11_23_20,	MSIOF0_SS1),
	PINMUX_IPSR_MSEL(IP11_23_20,	FSO_CFE_1_N_B, SEL_RFSO_1),

	PINMUX_IPSR_MSEL(IP11_27_24,	TX0_A, SEL_SCIF0_0),
	PINMUX_IPSR_GPSR(IP11_27_24,	MSIOF0_SS2),
	PINMUX_IPSR_MSEL(IP11_27_24,	FSO_TOE_N_B, SEL_RFSO_1),

	PINMUX_IPSR_MSEL(IP11_31_28,	SCK1_A, SEL_SCIF1_0),
	PINMUX_IPSR_GPSR(IP11_31_28,	MSIOF1_SS2),
	PINMUX_IPSR_GPSR(IP11_31_28,	TPU0TO2_B),
	PINMUX_IPSR_MSEL(IP11_31_28,	CAN0_TX_B, SEL_CAN0_1),
	PINMUX_IPSR_GPSR(IP11_31_28,	AUDIO_CLKOUT1),

	/* IPSR12 */
	PINMUX_IPSR_MSEL(IP12_3_0,	RX1_A, SEL_SCIF1_0),
	PINMUX_IPSR_GPSR(IP12_3_0,	CTS0_N),
	PINMUX_IPSR_GPSR(IP12_3_0,	TPU0TO0_B),

	PINMUX_IPSR_MSEL(IP12_7_4,	TX1_A, SEL_SCIF1_0),
	PINMUX_IPSR_GPSR(IP12_7_4,	RTS0_N),
	PINMUX_IPSR_GPSR(IP12_7_4,	TPU0TO1_B),

	PINMUX_IPSR_GPSR(IP12_11_8,	SCK2),
	PINMUX_IPSR_GPSR(IP12_11_8,	MSIOF1_SS1),
	PINMUX_IPSR_GPSR(IP12_11_8,	TPU0TO3_B),

	PINMUX_IPSR_GPSR(IP12_15_12,	TPU0TO0_A),
	PINMUX_IPSR_MSEL(IP12_15_12,	AVB0_AVTP_CAPTURE_A, SEL_ETHERAVB_0),
	PINMUX_IPSR_GPSR(IP12_15_12,	HCTS0_N),

	PINMUX_IPSR_GPSR(IP12_19_16,	TPU0TO1_A),
	PINMUX_IPSR_MSEL(IP12_19_16,	AVB0_AVTP_MATCH_A, SEL_ETHERAVB_0),
	PINMUX_IPSR_GPSR(IP12_19_16,	HRTS0_N),

	PINMUX_IPSR_GPSR(IP12_23_20,	CAN_CLK),
	PINMUX_IPSR_MSEL(IP12_23_20,	AVB0_AVTP_PPS_A, SEL_ETHERAVB_0),
	PINMUX_IPSR_MSEL(IP12_23_20,	SCK0_B, SEL_SCIF0_1),
	PINMUX_IPSR_MSEL(IP12_23_20,	IRQ5_B, SEL_IRQ_5_1),

	PINMUX_IPSR_MSEL(IP12_27_24,	CAN0_RX_A, SEL_CAN0_0),
	PINMUX_IPSR_GPSR(IP12_27_24,	CANFD0_RX),
	PINMUX_IPSR_MSEL(IP12_27_24,	RX0_B, SEL_SCIF0_1),

	PINMUX_IPSR_MSEL(IP12_31_28,	CAN0_TX_A, SEL_CAN0_0),
	PINMUX_IPSR_GPSR(IP12_31_28,	CANFD0_TX),
	PINMUX_IPSR_MSEL(IP12_31_28,	TX0_B, SEL_SCIF0_1),

	/* IPSR13 */
	PINMUX_IPSR_MSEL(IP13_3_0,	CAN1_RX_A, SEL_CAN1_0),
	PINMUX_IPSR_GPSR(IP13_3_0,	CANFD1_RX),
	PINMUX_IPSR_GPSR(IP13_3_0,	TPU0TO2_A),

	PINMUX_IPSR_MSEL(IP13_7_4,	CAN1_TX_A, SEL_CAN1_0),
	PINMUX_IPSR_GPSR(IP13_7_4,	CANFD1_TX),
	PINMUX_IPSR_GPSR(IP13_7_4,	TPU0TO3_A),
};

/*
 * Pins not associated with a GPIO port.
 */
enum {
	GP_ASSIGN_LAST(),
	NOGP_ALL(),
};

static const struct sh_pfc_pin pinmux_pins[] = {
	PINMUX_GPIO_GP_ALL(),
	PINMUX_NOGP_ALL(),
};

#ifdef CONFIG_PINCTRL_PFC_FULL
/* - AUDIO CLOCK ------------------------------------------------------------- */
static const unsigned int audio_clk_a_pins[] = {
	/* CLK A */
	RCAR_GP_PIN(4, 1),
};
static const unsigned int audio_clk_a_mux[] = {
	AUDIO_CLKA_MARK,
};
static const unsigned int audio_clk_b_pins[] = {
	/* CLK B */
	RCAR_GP_PIN(2, 27),
};
static const unsigned int audio_clk_b_mux[] = {
	AUDIO_CLKB_MARK,
};
static const unsigned int audio_clkout_pins[] = {
	/* CLKOUT */
	RCAR_GP_PIN(4, 5),
};
static const unsigned int audio_clkout_mux[] = {
	AUDIO_CLKOUT_MARK,
};
static const unsigned int audio_clkout1_pins[] = {
	/* CLKOUT1 */
	RCAR_GP_PIN(4, 22),
};
static const unsigned int audio_clkout1_mux[] = {
	AUDIO_CLKOUT1_MARK,
};
#endif

/* - EtherAVB --------------------------------------------------------------- */
static const unsigned int avb0_link_pins[] = {
	/* AVB0_LINK */
	RCAR_GP_PIN(5, 20),
};
static const unsigned int avb0_link_mux[] = {
	AVB0_LINK_MARK,
};
static const unsigned int avb0_magic_pins[] = {
	/* AVB0_MAGIC */
	RCAR_GP_PIN(5, 18),
};
static const unsigned int avb0_magic_mux[] = {
	AVB0_MAGIC_MARK,
};
static const unsigned int avb0_phy_int_pins[] = {
	/* AVB0_PHY_INT */
	RCAR_GP_PIN(5, 19),
};
static const unsigned int avb0_phy_int_mux[] = {
	AVB0_PHY_INT_MARK,
};
static const unsigned int avb0_mdio_pins[] = {
	/* AVB0_MDC, AVB0_MDIO */
	RCAR_GP_PIN(5, 17), RCAR_GP_PIN(5, 16),
};
static const unsigned int avb0_mdio_mux[] = {
	AVB0_MDC_MARK, AVB0_MDIO_MARK,
};
static const unsigned int avb0_mii_pins[] = {
	/*
	 * AVB0_TX_CTL, AVB0_TXC, AVB0_TD0,
	 * AVB0_TD1, AVB0_TD2, AVB0_TD3,
	 * AVB0_RX_CTL, AVB0_RXC, AVB0_RD0,
	 * AVB0_RD1, AVB0_RD2, AVB0_RD3,
	 * AVB0_TXCREFCLK
	 */
	RCAR_GP_PIN(5, 9), RCAR_GP_PIN(5, 10), RCAR_GP_PIN(5, 11),
	RCAR_GP_PIN(5, 12), RCAR_GP_PIN(5, 13), RCAR_GP_PIN(5, 14),
	RCAR_GP_PIN(5, 3), RCAR_GP_PIN(5, 4), RCAR_GP_PIN(5, 5),
	RCAR_GP_PIN(5, 6), RCAR_GP_PIN(5, 7), RCAR_GP_PIN(5, 8),
	RCAR_GP_PIN(5, 15),
};
static const unsigned int avb0_mii_mux[] = {
	AVB0_TX_CTL_MARK, AVB0_TXC_MARK, AVB0_TD0_MARK,
	AVB0_TD1_MARK, AVB0_TD2_MARK, AVB0_TD3_MARK,
	AVB0_RX_CTL_MARK, AVB0_RXC_MARK, AVB0_RD0_MARK,
	AVB0_RD1_MARK, AVB0_RD2_MARK, AVB0_RD3_MARK,
	AVB0_TXCREFCLK_MARK,
};
static const unsigned int avb0_avtp_pps_a_pins[] = {
	/* AVB0_AVTP_PPS_A */
	RCAR_GP_PIN(5, 2),
};
static const unsigned int avb0_avtp_pps_a_mux[] = {
	AVB0_AVTP_PPS_A_MARK,
};
static const unsigned int avb0_avtp_match_a_pins[] = {
	/* AVB0_AVTP_MATCH_A */
	RCAR_GP_PIN(5, 1),
};
static const unsigned int avb0_avtp_match_a_mux[] = {
	AVB0_AVTP_MATCH_A_MARK,
};
static const unsigned int avb0_avtp_capture_a_pins[] = {
	/* AVB0_AVTP_CAPTURE_A */
	RCAR_GP_PIN(5, 0),
};
static const unsigned int avb0_avtp_capture_a_mux[] = {
	AVB0_AVTP_CAPTURE_A_MARK,
};
static const unsigned int avb0_avtp_pps_b_pins[] = {
	/* AVB0_AVTP_PPS_B */
	RCAR_GP_PIN(4, 16),
};
static const unsigned int avb0_avtp_pps_b_mux[] = {
	AVB0_AVTP_PPS_B_MARK,
};
static const unsigned int avb0_avtp_match_b_pins[] = {
	/*  AVB0_AVTP_MATCH_B */
	RCAR_GP_PIN(4, 18),
};
static const unsigned int avb0_avtp_match_b_mux[] = {
	AVB0_AVTP_MATCH_B_MARK,
};
static const unsigned int avb0_avtp_capture_b_pins[] = {
	/* AVB0_AVTP_CAPTURE_B */
	RCAR_GP_PIN(4, 17),
};
static const unsigned int avb0_avtp_capture_b_mux[] = {
	AVB0_AVTP_CAPTURE_B_MARK,
};

#ifdef CONFIG_PINCTRL_PFC_FULL
/* - CAN ------------------------------------------------------------------ */
static const unsigned int can0_data_a_pins[] = {
	/* TX, RX */
	RCAR_GP_PIN(4, 28), RCAR_GP_PIN(4, 31),
};
static const unsigned int can0_data_a_mux[] = {
	CAN0_TX_A_MARK, CAN0_RX_A_MARK,
};
static const unsigned int can0_data_b_pins[] = {
	/* TX, RX */
	RCAR_GP_PIN(4, 22), RCAR_GP_PIN(4, 5),
};
static const unsigned int can0_data_b_mux[] = {
	CAN0_TX_B_MARK, CAN0_RX_B_MARK,
};
static const unsigned int can1_data_a_pins[] = {
	/* TX, RX */
	RCAR_GP_PIN(4, 30), RCAR_GP_PIN(4, 29),
};
static const unsigned int can1_data_a_mux[] = {
	CAN1_TX_A_MARK, CAN1_RX_A_MARK,
};
static const unsigned int can1_data_b_pins[] = {
	/* TX, RX */
	RCAR_GP_PIN(4, 7), RCAR_GP_PIN(4, 6),
};
static const unsigned int can1_data_b_mux[] = {
	CAN1_TX_B_MARK, CAN1_RX_B_MARK,
};

/* - CAN Clock -------------------------------------------------------------- */
static const unsigned int can_clk_pins[] = {
	/* CLK */
	RCAR_GP_PIN(5, 2),
};
static const unsigned int can_clk_mux[] = {
	CAN_CLK_MARK,
};

/* - CAN FD ----------------------------------------------------------------- */
static const unsigned int canfd0_data_pins[] = {
	/* TX, RX */
	RCAR_GP_PIN(4, 28), RCAR_GP_PIN(4, 31),
};
static const unsigned int canfd0_data_mux[] = {
	CANFD0_TX_MARK, CANFD0_RX_MARK,
};
static const unsigned int canfd1_data_pins[] = {
	/* TX, RX */
	RCAR_GP_PIN(4, 30), RCAR_GP_PIN(4, 29),
};
static const unsigned int canfd1_data_mux[] = {
	CANFD1_TX_MARK, CANFD1_RX_MARK,
};

/* - DU --------------------------------------------------------------------- */
static const unsigned int du_rgb666_pins[] = {
	/* R[7:2], G[7:2], B[7:2] */
	RCAR_GP_PIN(1, 23), RCAR_GP_PIN(1, 22), RCAR_GP_PIN(1, 21),
	RCAR_GP_PIN(1, 20), RCAR_GP_PIN(1, 19), RCAR_GP_PIN(1, 18),
	RCAR_GP_PIN(1, 15), RCAR_GP_PIN(1, 14), RCAR_GP_PIN(1, 13),
	RCAR_GP_PIN(1, 12), RCAR_GP_PIN(1, 11), RCAR_GP_PIN(1, 10),
	RCAR_GP_PIN(1, 7),  RCAR_GP_PIN(1, 6),  RCAR_GP_PIN(1, 5),
	RCAR_GP_PIN(1, 4),  RCAR_GP_PIN(1, 3),  RCAR_GP_PIN(1, 2),
};
static const unsigned int du_rgb666_mux[] = {
	DU_DR7_MARK, DU_DR6_MARK, DU_DR5_MARK, DU_DR4_MARK,
	DU_DR3_MARK, DU_DR2_MARK,
	DU_DG7_MARK, DU_DG6_MARK, DU_DG5_MARK, DU_DG4_MARK,
	DU_DG3_MARK, DU_DG2_MARK,
	DU_DB7_MARK, DU_DB6_MARK, DU_DB5_MARK, DU_DB4_MARK,
	DU_DB3_MARK, DU_DB2_MARK,
};
static const unsigned int du_rgb888_pins[] = {
	/* R[7:0], G[7:0], B[7:0] */
	RCAR_GP_PIN(1, 23), RCAR_GP_PIN(1, 22), RCAR_GP_PIN(1, 21),
	RCAR_GP_PIN(1, 20), RCAR_GP_PIN(1, 19), RCAR_GP_PIN(1, 18),
	RCAR_GP_PIN(1, 17), RCAR_GP_PIN(1, 16),
	RCAR_GP_PIN(1, 15), RCAR_GP_PIN(1, 14), RCAR_GP_PIN(1, 13),
	RCAR_GP_PIN(1, 12), RCAR_GP_PIN(1, 11), RCAR_GP_PIN(1, 10),
	RCAR_GP_PIN(1, 9),  RCAR_GP_PIN(1, 8),
	RCAR_GP_PIN(1, 7),  RCAR_GP_PIN(1, 6),  RCAR_GP_PIN(1, 5),
	RCAR_GP_PIN(1, 4),  RCAR_GP_PIN(1, 3),  RCAR_GP_PIN(1, 2),
	RCAR_GP_PIN(1, 1),  RCAR_GP_PIN(1, 0),
};
static const unsigned int du_rgb888_mux[] = {
	DU_DR7_MARK, DU_DR6_MARK, DU_DR5_MARK, DU_DR4_MARK,
	DU_DR3_MARK, DU_DR2_MARK, DU_DR1_MARK, DU_DR0_MARK,
	DU_DG7_MARK, DU_DG6_MARK, DU_DG5_MARK, DU_DG4_MARK,
	DU_DG3_MARK, DU_DG2_MARK, DU_DG1_MARK, DU_DG0_MARK,
	DU_DB7_MARK, DU_DB6_MARK, DU_DB5_MARK, DU_DB4_MARK,
	DU_DB3_MARK, DU_DB2_MARK, DU_DB1_MARK, DU_DB0_MARK,
};
static const unsigned int du_clk_in_1_pins[] = {
	/* CLKIN */
	RCAR_GP_PIN(1, 28),
};
static const unsigned int du_clk_in_1_mux[] = {
	DU_DOTCLKIN1_MARK
};
static const unsigned int du_clk_out_0_pins[] = {
	/* CLKOUT */
	RCAR_GP_PIN(1, 24),
};
static const unsigned int du_clk_out_0_mux[] = {
	DU_DOTCLKOUT0_MARK
};
static const unsigned int du_sync_pins[] = {
	/* VSYNC, HSYNC */
	RCAR_GP_PIN(1, 26), RCAR_GP_PIN(1, 25),
};
static const unsigned int du_sync_mux[] = {
	DU_VSYNC_MARK, DU_HSYNC_MARK
};
static const unsigned int du_disp_cde_pins[] = {
	/* DISP_CDE */
	RCAR_GP_PIN(1, 28),
};
static const unsigned int du_disp_cde_mux[] = {
	DU_DISP_CDE_MARK,
};
static const unsigned int du_cde_pins[] = {
	/* CDE */
	RCAR_GP_PIN(1, 29),
};
static const unsigned int du_cde_mux[] = {
	DU_CDE_MARK,
};
static const unsigned int du_disp_pins[] = {
	/* DISP */
	RCAR_GP_PIN(1, 27),
};
static const unsigned int du_disp_mux[] = {
	DU_DISP_MARK,
};
#endif

/* - I2C -------------------------------------------------------------------- */
static const unsigned int i2c0_pins[] = {
	/* SCL, SDA */
	RCAR_GP_PIN(4, 8), RCAR_GP_PIN(4, 9),
};
static const unsigned int i2c0_mux[] = {
	SCL0_MARK, SDA0_MARK,
};
static const unsigned int i2c1_pins[] = {
	/* SCL, SDA */
	RCAR_GP_PIN(4, 10), RCAR_GP_PIN(4, 11),
};
static const unsigned int i2c1_mux[] = {
	SCL1_MARK, SDA1_MARK,
};
static const unsigned int i2c2_a_pins[] = {
	/* SCL, SDA */
	RCAR_GP_PIN(4, 6), RCAR_GP_PIN(4, 7),
};
static const unsigned int i2c2_a_mux[] = {
	SCL2_A_MARK, SDA2_A_MARK,
};
static const unsigned int i2c2_b_pins[] = {
	/* SCL, SDA */
	RCAR_GP_PIN(2, 29), RCAR_GP_PIN(2, 30),
};
static const unsigned int i2c2_b_mux[] = {
	SCL2_B_MARK, SDA2_B_MARK,
};
static const unsigned int i2c3_a_pins[] = {
	/* SCL, SDA */
	RCAR_GP_PIN(0, 4), RCAR_GP_PIN(0, 5),
};
static const unsigned int i2c3_a_mux[] = {
	SCL3_A_MARK, SDA3_A_MARK,
};
static const unsigned int i2c3_b_pins[] = {
	/* SCL, SDA */
	RCAR_GP_PIN(0, 7), RCAR_GP_PIN(0, 8),
};
static const unsigned int i2c3_b_mux[] = {
	SCL3_B_MARK, SDA3_B_MARK,
};

/* - MLB+ ------------------------------------------------------------------- */
static const unsigned int mlb_3pin_pins[] = {
	RCAR_GP_PIN(0, 6), RCAR_GP_PIN(0, 8), RCAR_GP_PIN(0, 7),
};
static const unsigned int mlb_3pin_mux[] = {
	MLB_CLK_MARK, MLB_SIG_MARK, MLB_DAT_MARK,
};

/* - MMC ------------------------------------------------------------------- */
static const unsigned int mmc_data_pins[] = {
	/* D[0:7] */
	RCAR_GP_PIN(3, 2), RCAR_GP_PIN(3, 3),
	RCAR_GP_PIN(3, 4), RCAR_GP_PIN(3, 5),
	RCAR_GP_PIN(3, 6), RCAR_GP_PIN(3, 7),
	RCAR_GP_PIN(3, 8), RCAR_GP_PIN(3, 9),
};
static const unsigned int mmc_data_mux[] = {
	MMC_D0_MARK, MMC_D1_MARK,
	MMC_D2_MARK, MMC_D3_MARK,
	MMC_D4_MARK, MMC_D5_MARK,
	MMC_D6_MARK, MMC_D7_MARK,
};
static const unsigned int mmc_ctrl_pins[] = {
	/* CLK, CMD */
	RCAR_GP_PIN(3, 1), RCAR_GP_PIN(3, 0),
};
static const unsigned int mmc_ctrl_mux[] = {
	MMC_CLK_MARK, MMC_CMD_MARK,
};

#ifdef CONFIG_PINCTRL_PFC_FULL
/* - MSIOF0 ----------------------------------------------------------------- */
static const unsigned int msiof0_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(4, 12),
};

static const unsigned int msiof0_clk_mux[] = {
	MSIOF0_SCK_MARK,
};

static const unsigned int msiof0_sync_pins[] = {
	/* SYNC */
	RCAR_GP_PIN(4, 13),
};

static const unsigned int msiof0_sync_mux[] = {
	MSIOF0_SYNC_MARK,
};

static const unsigned int msiof0_ss1_pins[] = {
	/* SS1 */
	RCAR_GP_PIN(4, 20),
};

static const unsigned int msiof0_ss1_mux[] = {
	MSIOF0_SS1_MARK,
};

static const unsigned int msiof0_ss2_pins[] = {
	/* SS2 */
	RCAR_GP_PIN(4, 21),
};

static const unsigned int msiof0_ss2_mux[] = {
	MSIOF0_SS2_MARK,
};

static const unsigned int msiof0_txd_pins[] = {
	/* TXD */
	RCAR_GP_PIN(4, 14),
};

static const unsigned int msiof0_txd_mux[] = {
	MSIOF0_TXD_MARK,
};

static const unsigned int msiof0_rxd_pins[] = {
	/* RXD */
	RCAR_GP_PIN(4, 15),
};

static const unsigned int msiof0_rxd_mux[] = {
	MSIOF0_RXD_MARK,
};

/* - MSIOF1 ----------------------------------------------------------------- */
static const unsigned int msiof1_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(4, 16),
};

static const unsigned int msiof1_clk_mux[] = {
	MSIOF1_SCK_MARK,
};

static const unsigned int msiof1_sync_pins[] = {
	/* SYNC */
	RCAR_GP_PIN(4, 19),
};

static const unsigned int msiof1_sync_mux[] = {
	MSIOF1_SYNC_MARK,
};

static const unsigned int msiof1_ss1_pins[] = {
	/* SS1 */
	RCAR_GP_PIN(4, 25),
};

static const unsigned int msiof1_ss1_mux[] = {
	MSIOF1_SS1_MARK,
};

static const unsigned int msiof1_ss2_pins[] = {
	/* SS2 */
	RCAR_GP_PIN(4, 22),
};

static const unsigned int msiof1_ss2_mux[] = {
	MSIOF1_SS2_MARK,
};

static const unsigned int msiof1_txd_pins[] = {
	/* TXD */
	RCAR_GP_PIN(4, 17),
};

static const unsigned int msiof1_txd_mux[] = {
	MSIOF1_TXD_MARK,
};

static const unsigned int msiof1_rxd_pins[] = {
	/* RXD */
	RCAR_GP_PIN(4, 18),
};

static const unsigned int msiof1_rxd_mux[] = {
	MSIOF1_RXD_MARK,
};

/* - MSIOF2 ----------------------------------------------------------------- */
static const unsigned int msiof2_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(0, 3),
};

static const unsigned int msiof2_clk_mux[] = {
	MSIOF2_SCK_MARK,
};

static const unsigned int msiof2_sync_a_pins[] = {
	/* SYNC */
	RCAR_GP_PIN(0, 6),
};

static const unsigned int msiof2_sync_a_mux[] = {
	MSIOF2_SYNC_A_MARK,
};

static const unsigned int msiof2_sync_b_pins[] = {
	/* SYNC */
	RCAR_GP_PIN(0, 2),
};

static const unsigned int msiof2_sync_b_mux[] = {
	MSIOF2_SYNC_B_MARK,
};

static const unsigned int msiof2_ss1_pins[] = {
	/* SS1 */
	RCAR_GP_PIN(0, 7),
};

static const unsigned int msiof2_ss1_mux[] = {
	MSIOF2_SS1_MARK,
};

static const unsigned int msiof2_ss2_pins[] = {
	/* SS2 */
	RCAR_GP_PIN(0, 8),
};

static const unsigned int msiof2_ss2_mux[] = {
	MSIOF2_SS2_MARK,
};

static const unsigned int msiof2_txd_pins[] = {
	/* TXD */
	RCAR_GP_PIN(0, 4),
};

static const unsigned int msiof2_txd_mux[] = {
	MSIOF2_TXD_MARK,
};

static const unsigned int msiof2_rxd_pins[] = {
	/* RXD */
	RCAR_GP_PIN(0, 5),
};

static const unsigned int msiof2_rxd_mux[] = {
	MSIOF2_RXD_MARK,
};

/* - MSIOF3 ----------------------------------------------------------------- */
static const unsigned int msiof3_clk_a_pins[] = {
	/* SCK */
	RCAR_GP_PIN(2, 24),
};

static const unsigned int msiof3_clk_a_mux[] = {
	MSIOF3_SCK_A_MARK,
};

static const unsigned int msiof3_sync_a_pins[] = {
	/* SYNC */
	RCAR_GP_PIN(2, 21),
};

static const unsigned int msiof3_sync_a_mux[] = {
	MSIOF3_SYNC_A_MARK,
};

static const unsigned int msiof3_ss1_a_pins[] = {
	/* SS1 */
	RCAR_GP_PIN(2, 14),
};

static const unsigned int msiof3_ss1_a_mux[] = {
	MSIOF3_SS1_A_MARK,
};

static const unsigned int msiof3_ss2_a_pins[] = {
	/* SS2 */
	RCAR_GP_PIN(2, 10),
};

static const unsigned int msiof3_ss2_a_mux[] = {
	MSIOF3_SS2_A_MARK,
};

static const unsigned int msiof3_txd_a_pins[] = {
	/* TXD */
	RCAR_GP_PIN(2, 22),
};

static const unsigned int msiof3_txd_a_mux[] = {
	MSIOF3_TXD_A_MARK,
};

static const unsigned int msiof3_rxd_a_pins[] = {
	/* RXD */
	RCAR_GP_PIN(2, 23),
};

static const unsigned int msiof3_rxd_a_mux[] = {
	MSIOF3_RXD_A_MARK,
};

static const unsigned int msiof3_clk_b_pins[] = {
	/* SCK */
	RCAR_GP_PIN(1, 8),
};

static const unsigned int msiof3_clk_b_mux[] = {
	MSIOF3_SCK_B_MARK,
};

static const unsigned int msiof3_sync_b_pins[] = {
	/* SYNC */
	RCAR_GP_PIN(1, 9),
};

static const unsigned int msiof3_sync_b_mux[] = {
	MSIOF3_SYNC_B_MARK,
};

static const unsigned int msiof3_ss1_b_pins[] = {
	/* SS1 */
	RCAR_GP_PIN(1, 6),
};

static const unsigned int msiof3_ss1_b_mux[] = {
	MSIOF3_SS1_B_MARK,
};

static const unsigned int msiof3_ss2_b_pins[] = {
	/* SS2 */
	RCAR_GP_PIN(1, 7),
};

static const unsigned int msiof3_ss2_b_mux[] = {
	MSIOF3_SS2_B_MARK,
};

static const unsigned int msiof3_txd_b_pins[] = {
	/* TXD */
	RCAR_GP_PIN(1, 0),
};

static const unsigned int msiof3_txd_b_mux[] = {
	MSIOF3_TXD_B_MARK,
};

static const unsigned int msiof3_rxd_b_pins[] = {
	/* RXD */
	RCAR_GP_PIN(1, 1),
};

static const unsigned int msiof3_rxd_b_mux[] = {
	MSIOF3_RXD_B_MARK,
};

/* - PWM0 ------------------------------------------------------------------ */
static const unsigned int pwm0_a_pins[] = {
	/* PWM */
	RCAR_GP_PIN(2, 1),
};

static const unsigned int pwm0_a_mux[] = {
	PWM0_A_MARK,
};

static const unsigned int pwm0_b_pins[] = {
	/* PWM */
	RCAR_GP_PIN(1, 18),
};

static const unsigned int pwm0_b_mux[] = {
	PWM0_B_MARK,
};

static const unsigned int pwm0_c_pins[] = {
	/* PWM */
	RCAR_GP_PIN(2, 29),
};

static const unsigned int pwm0_c_mux[] = {
	PWM0_C_MARK,
};

/* - PWM1 ------------------------------------------------------------------ */
static const unsigned int pwm1_a_pins[] = {
	/* PWM */
	RCAR_GP_PIN(2, 2),
};

static const unsigned int pwm1_a_mux[] = {
	PWM1_A_MARK,
};

static const unsigned int pwm1_b_pins[] = {
	/* PWM */
	RCAR_GP_PIN(1, 19),
};

static const unsigned int pwm1_b_mux[] = {
	PWM1_B_MARK,
};

static const unsigned int pwm1_c_pins[] = {
	/* PWM */
	RCAR_GP_PIN(2, 30),
};

static const unsigned int pwm1_c_mux[] = {
	PWM1_C_MARK,
};

/* - PWM2 ------------------------------------------------------------------ */
static const unsigned int pwm2_a_pins[] = {
	/* PWM */
	RCAR_GP_PIN(2, 3),
};

static const unsigned int pwm2_a_mux[] = {
	PWM2_A_MARK,
};

static const unsigned int pwm2_b_pins[] = {
	/* PWM */
	RCAR_GP_PIN(1, 22),
};

static const unsigned int pwm2_b_mux[] = {
	PWM2_B_MARK,
};

static const unsigned int pwm2_c_pins[] = {
	/* PWM */
	RCAR_GP_PIN(2, 31),
};

static const unsigned int pwm2_c_mux[] = {
	PWM2_C_MARK,
};

/* - PWM3 ------------------------------------------------------------------ */
static const unsigned int pwm3_a_pins[] = {
	/* PWM */
	RCAR_GP_PIN(2, 4),
};

static const unsigned int pwm3_a_mux[] = {
	PWM3_A_MARK,
};

static const unsigned int pwm3_b_pins[] = {
	/* PWM */
	RCAR_GP_PIN(1, 27),
};

static const unsigned int pwm3_b_mux[] = {
	PWM3_B_MARK,
};

static const unsigned int pwm3_c_pins[] = {
	/* PWM */
	RCAR_GP_PIN(4, 0),
};

static const unsigned int pwm3_c_mux[] = {
	PWM3_C_MARK,
};
#endif

/* - QSPI0 ------------------------------------------------------------------ */
static const unsigned int qspi0_ctrl_pins[] = {
	/* QSPI0_SPCLK, QSPI0_SSL */
	RCAR_GP_PIN(6, 0), RCAR_GP_PIN(6, 5),
};
static const unsigned int qspi0_ctrl_mux[] = {
	QSPI0_SPCLK_MARK, QSPI0_SSL_MARK,
};
/* - QSPI1 ------------------------------------------------------------------ */
static const unsigned int qspi1_ctrl_pins[] = {
	/* QSPI1_SPCLK, QSPI1_SSL */
	RCAR_GP_PIN(6, 6), RCAR_GP_PIN(6, 11),
};
static const unsigned int qspi1_ctrl_mux[] = {
	QSPI1_SPCLK_MARK, QSPI1_SSL_MARK,
};

/* - RPC -------------------------------------------------------------------- */
static const unsigned int rpc_clk_pins[] = {
	/* Octal-SPI flash: C/SCLK */
	/* HyperFlash: CK, CK# */
	RCAR_GP_PIN(6, 0), RCAR_GP_PIN(6, 6),
};
static const unsigned int rpc_clk_mux[] = {
	QSPI0_SPCLK_MARK, QSPI1_SPCLK_MARK,
};
static const unsigned int rpc_ctrl_pins[] = {
	/* Octal-SPI flash: S#/CS, DQS */
	/* HyperFlash: CS#, RDS */
	RCAR_GP_PIN(6, 5), RCAR_GP_PIN(6, 11),
};
static const unsigned int rpc_ctrl_mux[] = {
	QSPI0_SSL_MARK, QSPI1_SSL_MARK,
};
static const unsigned int rpc_data_pins[] = {
	/* DQ[0:7] */
	RCAR_GP_PIN(6, 1), RCAR_GP_PIN(6, 2),
	RCAR_GP_PIN(6, 3), RCAR_GP_PIN(6, 4),
	RCAR_GP_PIN(6, 7), RCAR_GP_PIN(6, 8),
	RCAR_GP_PIN(6, 9), RCAR_GP_PIN(6, 10),
};
static const unsigned int rpc_data_mux[] = {
	QSPI0_MOSI_IO0_MARK, QSPI0_MISO_IO1_MARK,
	QSPI0_IO2_MARK, QSPI0_IO3_MARK,
	QSPI1_MOSI_IO0_MARK, QSPI1_MISO_IO1_MARK,
	QSPI1_IO2_MARK, QSPI1_IO3_MARK,
};
static const unsigned int rpc_reset_pins[] = {
	/* RPC_RESET# */
	RCAR_GP_PIN(6, 12),
};
static const unsigned int rpc_reset_mux[] = {
	RPC_RESET_N_MARK,
};
static const unsigned int rpc_int_pins[] = {
	/* RPC_INT# */
	RCAR_GP_PIN(6, 13),
};
static const unsigned int rpc_int_mux[] = {
	RPC_INT_N_MARK,
};

/* - SCIF0 ------------------------------------------------------------------ */
static const unsigned int scif0_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(4, 20), RCAR_GP_PIN(4, 21),
};
static const unsigned int scif0_data_a_mux[] = {
	RX0_A_MARK, TX0_A_MARK,
};
static const unsigned int scif0_clk_a_pins[] = {
	/* SCK */
	RCAR_GP_PIN(4, 19),
};
static const unsigned int scif0_clk_a_mux[] = {
	SCK0_A_MARK,
};
static const unsigned int scif0_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(4, 31), RCAR_GP_PIN(4, 28),
};
static const unsigned int scif0_data_b_mux[] = {
	RX0_B_MARK, TX0_B_MARK,
};
static const unsigned int scif0_clk_b_pins[] = {
	/* SCK */
	RCAR_GP_PIN(5, 2),
};
static const unsigned int scif0_clk_b_mux[] = {
	SCK0_B_MARK,
};
static const unsigned int scif0_ctrl_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(4, 24), RCAR_GP_PIN(4, 23),
};
static const unsigned int scif0_ctrl_mux[] = {
	RTS0_N_MARK, CTS0_N_MARK,
};
/* - SCIF1 ------------------------------------------------------------------ */
static const unsigned int scif1_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(4, 23), RCAR_GP_PIN(4, 24),
};
static const unsigned int scif1_data_a_mux[] = {
	RX1_A_MARK, TX1_A_MARK,
};
static const unsigned int scif1_clk_a_pins[] = {
	/* SCK */
	RCAR_GP_PIN(4, 22),
};
static const unsigned int scif1_clk_a_mux[] = {
	SCK1_A_MARK,
};
static const unsigned int scif1_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(2, 26), RCAR_GP_PIN(2, 28),
};
static const unsigned int scif1_data_b_mux[] = {
	RX1_B_MARK, TX1_B_MARK,
};
static const unsigned int scif1_clk_b_pins[] = {
	/* SCK */
	RCAR_GP_PIN(2, 25),
};
static const unsigned int scif1_clk_b_mux[] = {
	SCK1_B_MARK,
};
static const unsigned int scif1_ctrl_pins[] = {
	/* RTS, CTS */
	RCAR_GP_PIN(4, 11), RCAR_GP_PIN(4, 10),
};
static const unsigned int scif1_ctrl_mux[] = {
	RTS1_N_MARK, CTS1_N_MARK,
};

/* - SCIF2 ------------------------------------------------------------------ */
static const unsigned int scif2_data_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(4, 26), RCAR_GP_PIN(4, 27),
};
static const unsigned int scif2_data_mux[] = {
	RX2_MARK, TX2_MARK,
};
static const unsigned int scif2_clk_pins[] = {
	/* SCK */
	RCAR_GP_PIN(4, 25),
};
static const unsigned int scif2_clk_mux[] = {
	SCK2_MARK,
};
/* - SCIF3 ------------------------------------------------------------------ */
static const unsigned int scif3_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(2, 31), RCAR_GP_PIN(4, 00),
};
static const unsigned int scif3_data_a_mux[] = {
	RX3_A_MARK, TX3_A_MARK,
};
static const unsigned int scif3_clk_a_pins[] = {
	/* SCK */
	RCAR_GP_PIN(2, 30),
};
static const unsigned int scif3_clk_a_mux[] = {
	SCK3_A_MARK,
};
static const unsigned int scif3_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(1, 30), RCAR_GP_PIN(1, 31),
};
static const unsigned int scif3_data_b_mux[] = {
	RX3_B_MARK, TX3_B_MARK,
};
static const unsigned int scif3_clk_b_pins[] = {
	/* SCK */
	RCAR_GP_PIN(1, 29),
};
static const unsigned int scif3_clk_b_mux[] = {
	SCK3_B_MARK,
};
/* - SCIF4 ------------------------------------------------------------------ */
static const unsigned int scif4_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(2, 11), RCAR_GP_PIN(2, 12),
};
static const unsigned int scif4_data_a_mux[] = {
	RX4_A_MARK, TX4_A_MARK,
};
static const unsigned int scif4_clk_a_pins[] = {
	/* SCK */
	RCAR_GP_PIN(2, 6),
};
static const unsigned int scif4_clk_a_mux[] = {
	SCK4_A_MARK,
};
static const unsigned int scif4_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(1, 16), RCAR_GP_PIN(1, 17),
};
static const unsigned int scif4_data_b_mux[] = {
	RX4_B_MARK, TX4_B_MARK,
};
static const unsigned int scif4_clk_b_pins[] = {
	/* SCK */
	RCAR_GP_PIN(1, 15),
};
static const unsigned int scif4_clk_b_mux[] = {
	SCK4_B_MARK,
};
/* - SCIF5 ------------------------------------------------------------------ */
static const unsigned int scif5_data_a_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(0, 7), RCAR_GP_PIN(0, 8),
};
static const unsigned int scif5_data_a_mux[] = {
	RX5_A_MARK, TX5_A_MARK,
};
static const unsigned int scif5_clk_a_pins[] = {
	/* SCK */
	RCAR_GP_PIN(0, 6),
};
static const unsigned int scif5_clk_a_mux[] = {
	SCK5_A_MARK,
};
static const unsigned int scif5_data_b_pins[] = {
	/* RX, TX */
	RCAR_GP_PIN(1, 4), RCAR_GP_PIN(1, 5),
};
static const unsigned int scif5_data_b_mux[] = {
	RX5_B_MARK, TX5_B_MARK,
};
static const unsigned int scif5_clk_b_pins[] = {
	/* SCK */
	RCAR_GP_PIN(1, 3),
};
static const unsigned int scif5_clk_b_mux[] = {
	SCK5_B_MARK,
};
/* - SCIF Clock ------------------------------------------------------------- */
static const unsigned int scif_clk_pins[] = {
	/* SCIF_CLK */
	RCAR_GP_PIN(2, 27),
};
static const unsigned int scif_clk_mux[] = {
	SCIF_CLK_MARK,
};

#ifdef CONFIG_PINCTRL_PFC_FULL
/* - SSI ---------------------------------------------------------------*/
static const unsigned int ssi3_data_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(4, 3),
};
static const unsigned int ssi3_data_mux[] = {
	SSI_SDATA3_MARK,
};
static const unsigned int ssi34_ctrl_pins[] = {
	/* SCK,  WS */
	RCAR_GP_PIN(4, 2), RCAR_GP_PIN(4, 4),
};
static const unsigned int ssi34_ctrl_mux[] = {
	SSI_SCK34_MARK, SSI_WS34_MARK,
};
static const unsigned int ssi4_ctrl_a_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(4, 5), RCAR_GP_PIN(4, 7),
};
static const unsigned int ssi4_ctrl_a_mux[] = {
	SSI_SCK4_A_MARK, SSI_WS4_A_MARK,
};
static const unsigned int ssi4_data_a_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(4, 6),
};
static const unsigned int ssi4_data_a_mux[] = {
	SSI_SDATA4_A_MARK,
};
static const unsigned int ssi4_ctrl_b_pins[] = {
	/* SCK, WS */
	RCAR_GP_PIN(2, 15), RCAR_GP_PIN(2, 20),
};
static const unsigned int ssi4_ctrl_b_mux[] = {
	SSI_SCK4_B_MARK, SSI_WS4_B_MARK,
};
static const unsigned int ssi4_data_b_pins[] = {
	/* SDATA */
	RCAR_GP_PIN(2, 16),
};
static const unsigned int ssi4_data_b_mux[] = {
	SSI_SDATA4_B_MARK,
};
#endif

/* - USB0 ------------------------------------------------------------------- */
static const unsigned int usb0_pins[] = {
	/* PWEN, OVC */
	RCAR_GP_PIN(0, 0), RCAR_GP_PIN(0, 1),
};
static const unsigned int usb0_mux[] = {
	USB0_PWEN_MARK, USB0_OVC_MARK,
};

#ifdef CONFIG_PINCTRL_PFC_FULL
/* - VIN4 ------------------------------------------------------------------- */
static const unsigned int vin4_data18_pins[] = {
	RCAR_GP_PIN(2, 3), RCAR_GP_PIN(2, 4),
	RCAR_GP_PIN(2, 5), RCAR_GP_PIN(2, 6),
	RCAR_GP_PIN(2, 7), RCAR_GP_PIN(2, 8),
	RCAR_GP_PIN(2, 11), RCAR_GP_PIN(2, 12),
	RCAR_GP_PIN(2, 13), RCAR_GP_PIN(2, 14),
	RCAR_GP_PIN(2, 15), RCAR_GP_PIN(2, 16),
	RCAR_GP_PIN(2, 19), RCAR_GP_PIN(2, 20),
	RCAR_GP_PIN(2, 21), RCAR_GP_PIN(2, 22),
	RCAR_GP_PIN(2, 23), RCAR_GP_PIN(2, 24),
};
static const unsigned int vin4_data18_mux[] = {
	VI4_DATA2_MARK, VI4_DATA3_MARK,
	VI4_DATA4_MARK, VI4_DATA5_MARK,
	VI4_DATA6_MARK, VI4_DATA7_MARK,
	VI4_DATA10_MARK, VI4_DATA11_MARK,
	VI4_DATA12_MARK, VI4_DATA13_MARK,
	VI4_DATA14_MARK, VI4_DATA15_MARK,
	VI4_DATA18_MARK, VI4_DATA19_MARK,
	VI4_DATA20_MARK, VI4_DATA21_MARK,
	VI4_DATA22_MARK, VI4_DATA23_MARK,
};
static const unsigned int vin4_data_pins[] = {
	RCAR_GP_PIN(2, 1), RCAR_GP_PIN(2, 2),
	RCAR_GP_PIN(2, 3), RCAR_GP_PIN(2, 4),
	RCAR_GP_PIN(2, 5), RCAR_GP_PIN(2, 6),
	RCAR_GP_PIN(2, 7), RCAR_GP_PIN(2, 8),
	RCAR_GP_PIN(2, 9), RCAR_GP_PIN(2, 10),
	RCAR_GP_PIN(2, 11), RCAR_GP_PIN(2, 12),
	RCAR_GP_PIN(2, 13), RCAR_GP_PIN(2, 14),
	RCAR_GP_PIN(2, 15), RCAR_GP_PIN(2, 16),
	RCAR_GP_PIN(2, 17), RCAR_GP_PIN(2, 18),
	RCAR_GP_PIN(2, 19), RCAR_GP_PIN(2, 20),
	RCAR_GP_PIN(2, 21), RCAR_GP_PIN(2, 22),
	RCAR_GP_PIN(2, 23), RCAR_GP_PIN(2, 24),
};
static const unsigned int vin4_data_mux[] = {
	VI4_DATA0_MARK, VI4_DATA1_MARK,
	VI4_DATA2_MARK, VI4_DATA3_MARK,
	VI4_DATA4_MARK, VI4_DATA5_MARK,
	VI4_DATA6_MARK, VI4_DATA7_MARK,
	VI4_DATA8_MARK,  VI4_DATA9_MARK,
	VI4_DATA10_MARK, VI4_DATA11_MARK,
	VI4_DATA12_MARK, VI4_DATA13_MARK,
	VI4_DATA14_MARK, VI4_DATA15_MARK,
	VI4_DATA16_MARK, VI4_DATA17_MARK,
	VI4_DATA18_MARK, VI4_DATA19_MARK,
	VI4_DATA20_MARK, VI4_DATA21_MARK,
	VI4_DATA22_MARK, VI4_DATA23_MARK,
};
static const unsigned int vin4_sync_pins[] = {
	/* HSYNC#, VSYNC# */
	RCAR_GP_PIN(2, 26), RCAR_GP_PIN(2, 25),
};
static const unsigned int vin4_sync_mux[] = {
	VI4_HSYNC_N_MARK, VI4_VSYNC_N_MARK,
};
static const unsigned int vin4_field_pins[] = {
	/* FIELD */
	RCAR_GP_PIN(2, 27),
};
static const unsigned int vin4_field_mux[] = {
	VI4_FIELD_MARK,
};
static const unsigned int vin4_clkenb_pins[] = {
	/* CLKENB */
	RCAR_GP_PIN(2, 28),
};
static const unsigned int vin4_clkenb_mux[] = {
	VI4_CLKENB_MARK,
};
static const unsigned int vin4_clk_pins[] = {
	/* CLK */
	RCAR_GP_PIN(2, 0),
};
static const unsigned int vin4_clk_mux[] = {
	VI4_CLK_MARK,
};
#endif

static const struct sh_pfc_pin_group pinmux_groups[] = {
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_PIN_GROUP(audio_clk_a),
	SH_PFC_PIN_GROUP(audio_clk_b),
	SH_PFC_PIN_GROUP(audio_clkout),
	SH_PFC_PIN_GROUP(audio_clkout1),
#endif
	SH_PFC_PIN_GROUP(avb0_link),
	SH_PFC_PIN_GROUP(avb0_magic),
	SH_PFC_PIN_GROUP(avb0_phy_int),
	SH_PFC_PIN_GROUP_ALIAS(avb0_mdc, avb0_mdio),	/* Deprecated */
	SH_PFC_PIN_GROUP(avb0_mdio),
	SH_PFC_PIN_GROUP(avb0_mii),
	SH_PFC_PIN_GROUP(avb0_avtp_pps_a),
	SH_PFC_PIN_GROUP(avb0_avtp_match_a),
	SH_PFC_PIN_GROUP(avb0_avtp_capture_a),
	SH_PFC_PIN_GROUP(avb0_avtp_pps_b),
	SH_PFC_PIN_GROUP(avb0_avtp_match_b),
	SH_PFC_PIN_GROUP(avb0_avtp_capture_b),
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_PIN_GROUP(can0_data_a),
	SH_PFC_PIN_GROUP(can0_data_b),
	SH_PFC_PIN_GROUP(can1_data_a),
	SH_PFC_PIN_GROUP(can1_data_b),
	SH_PFC_PIN_GROUP(can_clk),
	SH_PFC_PIN_GROUP(canfd0_data),
	SH_PFC_PIN_GROUP(canfd1_data),
	SH_PFC_PIN_GROUP(du_rgb666),
	SH_PFC_PIN_GROUP(du_rgb888),
	SH_PFC_PIN_GROUP(du_clk_in_1),
	SH_PFC_PIN_GROUP(du_clk_out_0),
	SH_PFC_PIN_GROUP(du_sync),
	SH_PFC_PIN_GROUP(du_disp_cde),
	SH_PFC_PIN_GROUP(du_cde),
	SH_PFC_PIN_GROUP(du_disp),
#endif
	SH_PFC_PIN_GROUP(i2c0),
	SH_PFC_PIN_GROUP(i2c1),
	SH_PFC_PIN_GROUP(i2c2_a),
	SH_PFC_PIN_GROUP(i2c2_b),
	SH_PFC_PIN_GROUP(i2c3_a),
	SH_PFC_PIN_GROUP(i2c3_b),
	SH_PFC_PIN_GROUP(mlb_3pin),
	BUS_DATA_PIN_GROUP(mmc_data, 1),
	BUS_DATA_PIN_GROUP(mmc_data, 4),
	BUS_DATA_PIN_GROUP(mmc_data, 8),
	SH_PFC_PIN_GROUP(mmc_ctrl),
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_PIN_GROUP(msiof0_clk),
	SH_PFC_PIN_GROUP(msiof0_sync),
	SH_PFC_PIN_GROUP(msiof0_ss1),
	SH_PFC_PIN_GROUP(msiof0_ss2),
	SH_PFC_PIN_GROUP(msiof0_txd),
	SH_PFC_PIN_GROUP(msiof0_rxd),
	SH_PFC_PIN_GROUP(msiof1_clk),
	SH_PFC_PIN_GROUP(msiof1_sync),
	SH_PFC_PIN_GROUP(msiof1_ss1),
	SH_PFC_PIN_GROUP(msiof1_ss2),
	SH_PFC_PIN_GROUP(msiof1_txd),
	SH_PFC_PIN_GROUP(msiof1_rxd),
	SH_PFC_PIN_GROUP(msiof2_clk),
	SH_PFC_PIN_GROUP(msiof2_sync_a),
	SH_PFC_PIN_GROUP(msiof2_sync_b),
	SH_PFC_PIN_GROUP(msiof2_ss1),
	SH_PFC_PIN_GROUP(msiof2_ss2),
	SH_PFC_PIN_GROUP(msiof2_txd),
	SH_PFC_PIN_GROUP(msiof2_rxd),
	SH_PFC_PIN_GROUP(msiof3_clk_a),
	SH_PFC_PIN_GROUP(msiof3_sync_a),
	SH_PFC_PIN_GROUP(msiof3_ss1_a),
	SH_PFC_PIN_GROUP(msiof3_ss2_a),
	SH_PFC_PIN_GROUP(msiof3_txd_a),
	SH_PFC_PIN_GROUP(msiof3_rxd_a),
	SH_PFC_PIN_GROUP(msiof3_clk_b),
	SH_PFC_PIN_GROUP(msiof3_sync_b),
	SH_PFC_PIN_GROUP(msiof3_ss1_b),
	SH_PFC_PIN_GROUP(msiof3_ss2_b),
	SH_PFC_PIN_GROUP(msiof3_txd_b),
	SH_PFC_PIN_GROUP(msiof3_rxd_b),
	SH_PFC_PIN_GROUP(pwm0_a),
	SH_PFC_PIN_GROUP(pwm0_b),
	SH_PFC_PIN_GROUP(pwm0_c),
	SH_PFC_PIN_GROUP(pwm1_a),
	SH_PFC_PIN_GROUP(pwm1_b),
	SH_PFC_PIN_GROUP(pwm1_c),
	SH_PFC_PIN_GROUP(pwm2_a),
	SH_PFC_PIN_GROUP(pwm2_b),
	SH_PFC_PIN_GROUP(pwm2_c),
	SH_PFC_PIN_GROUP(pwm3_a),
	SH_PFC_PIN_GROUP(pwm3_b),
	SH_PFC_PIN_GROUP(pwm3_c),
#endif
	SH_PFC_PIN_GROUP(qspi0_ctrl),
	SH_PFC_PIN_GROUP_SUBSET(qspi0_data2, rpc_data, 0, 2),
	SH_PFC_PIN_GROUP_SUBSET(qspi0_data4, rpc_data, 0, 4),
	SH_PFC_PIN_GROUP(qspi1_ctrl),
	SH_PFC_PIN_GROUP_SUBSET(qspi1_data2, rpc_data, 4, 2),
	SH_PFC_PIN_GROUP_SUBSET(qspi1_data4, rpc_data, 4, 4),
	BUS_DATA_PIN_GROUP(rpc_clk, 1),
	BUS_DATA_PIN_GROUP(rpc_clk, 2),
	SH_PFC_PIN_GROUP(rpc_ctrl),
	SH_PFC_PIN_GROUP(rpc_data),
	SH_PFC_PIN_GROUP(rpc_reset),
	SH_PFC_PIN_GROUP(rpc_int),
	SH_PFC_PIN_GROUP(scif0_data_a),
	SH_PFC_PIN_GROUP(scif0_clk_a),
	SH_PFC_PIN_GROUP(scif0_data_b),
	SH_PFC_PIN_GROUP(scif0_clk_b),
	SH_PFC_PIN_GROUP(scif0_ctrl),
	SH_PFC_PIN_GROUP(scif1_data_a),
	SH_PFC_PIN_GROUP(scif1_clk_a),
	SH_PFC_PIN_GROUP(scif1_data_b),
	SH_PFC_PIN_GROUP(scif1_clk_b),
	SH_PFC_PIN_GROUP(scif1_ctrl),
	SH_PFC_PIN_GROUP(scif2_data),
	SH_PFC_PIN_GROUP(scif2_clk),
	SH_PFC_PIN_GROUP(scif3_data_a),
	SH_PFC_PIN_GROUP(scif3_clk_a),
	SH_PFC_PIN_GROUP(scif3_data_b),
	SH_PFC_PIN_GROUP(scif3_clk_b),
	SH_PFC_PIN_GROUP(scif4_data_a),
	SH_PFC_PIN_GROUP(scif4_clk_a),
	SH_PFC_PIN_GROUP(scif4_data_b),
	SH_PFC_PIN_GROUP(scif4_clk_b),
	SH_PFC_PIN_GROUP(scif5_data_a),
	SH_PFC_PIN_GROUP(scif5_clk_a),
	SH_PFC_PIN_GROUP(scif5_data_b),
	SH_PFC_PIN_GROUP(scif5_clk_b),
	SH_PFC_PIN_GROUP(scif_clk),
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_PIN_GROUP(ssi3_data),
	SH_PFC_PIN_GROUP(ssi34_ctrl),
	SH_PFC_PIN_GROUP(ssi4_ctrl_a),
	SH_PFC_PIN_GROUP(ssi4_data_a),
	SH_PFC_PIN_GROUP(ssi4_ctrl_b),
	SH_PFC_PIN_GROUP(ssi4_data_b),
#endif
	SH_PFC_PIN_GROUP(usb0),
#ifdef CONFIG_PINCTRL_PFC_FULL
	BUS_DATA_PIN_GROUP(vin4_data, 8),
	BUS_DATA_PIN_GROUP(vin4_data, 10),
	BUS_DATA_PIN_GROUP(vin4_data, 12),
	BUS_DATA_PIN_GROUP(vin4_data, 16),
	SH_PFC_PIN_GROUP(vin4_data18),
	BUS_DATA_PIN_GROUP(vin4_data, 20),
	BUS_DATA_PIN_GROUP(vin4_data, 24),
	SH_PFC_PIN_GROUP(vin4_sync),
	SH_PFC_PIN_GROUP(vin4_field),
	SH_PFC_PIN_GROUP(vin4_clkenb),
	SH_PFC_PIN_GROUP(vin4_clk),
#endif
};

#ifdef CONFIG_PINCTRL_PFC_FULL
static const char * const audio_clk_groups[] = {
	"audio_clk_a",
	"audio_clk_b",
	"audio_clkout",
	"audio_clkout1",
};
#endif

static const char * const avb0_groups[] = {
	"avb0_link",
	"avb0_magic",
	"avb0_phy_int",
	"avb0_mdc",	/* Deprecated, please use "avb0_mdio" instead */
	"avb0_mdio",
	"avb0_mii",
	"avb0_avtp_pps_a",
	"avb0_avtp_match_a",
	"avb0_avtp_capture_a",
	"avb0_avtp_pps_b",
	"avb0_avtp_match_b",
	"avb0_avtp_capture_b",
};

#ifdef CONFIG_PINCTRL_PFC_FULL
static const char * const can0_groups[] = {
	"can0_data_a",
	"can0_data_b",
};
static const char * const can1_groups[] = {
	"can1_data_a",
	"can1_data_b",
};
static const char * const can_clk_groups[] = {
	"can_clk",
};

static const char * const canfd0_groups[] = {
	"canfd0_data",
};
static const char * const canfd1_groups[] = {
	"canfd1_data",
};

static const char * const du_groups[] = {
	"du_rgb666",
	"du_rgb888",
	"du_clk_in_1",
	"du_clk_out_0",
	"du_sync",
	"du_disp_cde",
	"du_cde",
	"du_disp",
};
#endif

static const char * const i2c0_groups[] = {
	"i2c0",
};
static const char * const i2c1_groups[] = {
	"i2c1",
};

static const char * const i2c2_groups[] = {
	"i2c2_a",
	"i2c2_b",
};

static const char * const i2c3_groups[] = {
	"i2c3_a",
	"i2c3_b",
};

static const char * const mlb_3pin_groups[] = {
	"mlb_3pin",
};

static const char * const mmc_groups[] = {
	"mmc_data1",
	"mmc_data4",
	"mmc_data8",
	"mmc_ctrl",
};

#ifdef CONFIG_PINCTRL_PFC_FULL
static const char * const msiof0_groups[] = {
	"msiof0_clk",
	"msiof0_sync",
	"msiof0_ss1",
	"msiof0_ss2",
	"msiof0_txd",
	"msiof0_rxd",
};

static const char * const msiof1_groups[] = {
	"msiof1_clk",
	"msiof1_sync",
	"msiof1_ss1",
	"msiof1_ss2",
	"msiof1_txd",
	"msiof1_rxd",
};

static const char * const msiof2_groups[] = {
	"msiof2_clk",
	"msiof2_sync_a",
	"msiof2_sync_b",
	"msiof2_ss1",
	"msiof2_ss2",
	"msiof2_txd",
	"msiof2_rxd",
};

static const char * const msiof3_groups[] = {
	"msiof3_clk_a",
	"msiof3_sync_a",
	"msiof3_ss1_a",
	"msiof3_ss2_a",
	"msiof3_txd_a",
	"msiof3_rxd_a",
	"msiof3_clk_b",
	"msiof3_sync_b",
	"msiof3_ss1_b",
	"msiof3_ss2_b",
	"msiof3_txd_b",
	"msiof3_rxd_b",
};

static const char * const pwm0_groups[] = {
	"pwm0_a",
	"pwm0_b",
	"pwm0_c",
};

static const char * const pwm1_groups[] = {
	"pwm1_a",
	"pwm1_b",
	"pwm1_c",
};

static const char * const pwm2_groups[] = {
	"pwm2_a",
	"pwm2_b",
	"pwm2_c",
};

static const char * const pwm3_groups[] = {
	"pwm3_a",
	"pwm3_b",
	"pwm3_c",
};
#endif

static const char * const qspi0_groups[] = {
	"qspi0_ctrl",
	"qspi0_data2",
	"qspi0_data4",
};

static const char * const qspi1_groups[] = {
	"qspi1_ctrl",
	"qspi1_data2",
	"qspi1_data4",
};

static const char * const rpc_groups[] = {
	"rpc_clk1",
	"rpc_clk2",
	"rpc_ctrl",
	"rpc_data",
	"rpc_reset",
	"rpc_int",
};

static const char * const scif0_groups[] = {
	"scif0_data_a",
	"scif0_clk_a",
	"scif0_data_b",
	"scif0_clk_b",
	"scif0_ctrl",
};

static const char * const scif1_groups[] = {
	"scif1_data_a",
	"scif1_clk_a",
	"scif1_data_b",
	"scif1_clk_b",
	"scif1_ctrl",
};

static const char * const scif2_groups[] = {
	"scif2_data",
	"scif2_clk",
};

static const char * const scif3_groups[] = {
	"scif3_data_a",
	"scif3_clk_a",
	"scif3_data_b",
	"scif3_clk_b",
};

static const char * const scif4_groups[] = {
	"scif4_data_a",
	"scif4_clk_a",
	"scif4_data_b",
	"scif4_clk_b",
};

static const char * const scif5_groups[] = {
	"scif5_data_a",
	"scif5_clk_a",
	"scif5_data_b",
	"scif5_clk_b",
};

static const char * const scif_clk_groups[] = {
	"scif_clk",
};

#ifdef CONFIG_PINCTRL_PFC_FULL
static const char * const ssi_groups[] = {
	"ssi3_data",
	"ssi34_ctrl",
	"ssi4_ctrl_a",
	"ssi4_data_a",
	"ssi4_ctrl_b",
	"ssi4_data_b",
};
#endif

static const char * const usb0_groups[] = {
	"usb0",
};

#ifdef CONFIG_PINCTRL_PFC_FULL
static const char * const vin4_groups[] = {
	"vin4_data8",
	"vin4_data10",
	"vin4_data12",
	"vin4_data16",
	"vin4_data18",
	"vin4_data20",
	"vin4_data24",
	"vin4_sync",
	"vin4_field",
	"vin4_clkenb",
	"vin4_clk",
};
#endif

static const struct sh_pfc_function pinmux_functions[] = {
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_FUNCTION(audio_clk),
#endif
	SH_PFC_FUNCTION(avb0),
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_FUNCTION(can0),
	SH_PFC_FUNCTION(can1),
	SH_PFC_FUNCTION(can_clk),
	SH_PFC_FUNCTION(canfd0),
	SH_PFC_FUNCTION(canfd1),
	SH_PFC_FUNCTION(du),
#endif
	SH_PFC_FUNCTION(i2c0),
	SH_PFC_FUNCTION(i2c1),
	SH_PFC_FUNCTION(i2c2),
	SH_PFC_FUNCTION(i2c3),
	SH_PFC_FUNCTION(mlb_3pin),
	SH_PFC_FUNCTION(mmc),
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_FUNCTION(msiof0),
	SH_PFC_FUNCTION(msiof1),
	SH_PFC_FUNCTION(msiof2),
	SH_PFC_FUNCTION(msiof3),
	SH_PFC_FUNCTION(pwm0),
	SH_PFC_FUNCTION(pwm1),
	SH_PFC_FUNCTION(pwm2),
	SH_PFC_FUNCTION(pwm3),
#endif
	SH_PFC_FUNCTION(qspi0),
	SH_PFC_FUNCTION(qspi1),
	SH_PFC_FUNCTION(rpc),
	SH_PFC_FUNCTION(scif0),
	SH_PFC_FUNCTION(scif1),
	SH_PFC_FUNCTION(scif2),
	SH_PFC_FUNCTION(scif3),
	SH_PFC_FUNCTION(scif4),
	SH_PFC_FUNCTION(scif5),
	SH_PFC_FUNCTION(scif_clk),
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_FUNCTION(ssi),
#endif
	SH_PFC_FUNCTION(usb0),
#ifdef CONFIG_PINCTRL_PFC_FULL
	SH_PFC_FUNCTION(vin4),
#endif
};

static const struct pinmux_cfg_reg pinmux_config_regs[] = {
#define F_(x, y)	FN_##y
#define FM(x)		FN_##x
	{ PINMUX_CFG_REG_VAR("GPSR0", 0xe6060100, 32,
			     GROUP(-23, 1, 1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP0_31_9 RESERVED */
		GP_0_8_FN,	GPSR0_8,
		GP_0_7_FN,	GPSR0_7,
		GP_0_6_FN,	GPSR0_6,
		GP_0_5_FN,	GPSR0_5,
		GP_0_4_FN,	GPSR0_4,
		GP_0_3_FN,	GPSR0_3,
		GP_0_2_FN,	GPSR0_2,
		GP_0_1_FN,	GPSR0_1,
		GP_0_0_FN,	GPSR0_0, ))
	},
	{ PINMUX_CFG_REG("GPSR1", 0xe6060104, 32, 1, GROUP(
		GP_1_31_FN,	GPSR1_31,
		GP_1_30_FN,	GPSR1_30,
		GP_1_29_FN,	GPSR1_29,
		GP_1_28_FN,	GPSR1_28,
		GP_1_27_FN,	GPSR1_27,
		GP_1_26_FN,	GPSR1_26,
		GP_1_25_FN,	GPSR1_25,
		GP_1_24_FN,	GPSR1_24,
		GP_1_23_FN,	GPSR1_23,
		GP_1_22_FN,	GPSR1_22,
		GP_1_21_FN,	GPSR1_21,
		GP_1_20_FN,	GPSR1_20,
		GP_1_19_FN,	GPSR1_19,
		GP_1_18_FN,	GPSR1_18,
		GP_1_17_FN,	GPSR1_17,
		GP_1_16_FN,	GPSR1_16,
		GP_1_15_FN,	GPSR1_15,
		GP_1_14_FN,	GPSR1_14,
		GP_1_13_FN,	GPSR1_13,
		GP_1_12_FN,	GPSR1_12,
		GP_1_11_FN,	GPSR1_11,
		GP_1_10_FN,	GPSR1_10,
		GP_1_9_FN,	GPSR1_9,
		GP_1_8_FN,	GPSR1_8,
		GP_1_7_FN,	GPSR1_7,
		GP_1_6_FN,	GPSR1_6,
		GP_1_5_FN,	GPSR1_5,
		GP_1_4_FN,	GPSR1_4,
		GP_1_3_FN,	GPSR1_3,
		GP_1_2_FN,	GPSR1_2,
		GP_1_1_FN,	GPSR1_1,
		GP_1_0_FN,	GPSR1_0, ))
	},
	{ PINMUX_CFG_REG("GPSR2", 0xe6060108, 32, 1, GROUP(
		GP_2_31_FN,	GPSR2_31,
		GP_2_30_FN,	GPSR2_30,
		GP_2_29_FN,	GPSR2_29,
		GP_2_28_FN,	GPSR2_28,
		GP_2_27_FN,	GPSR2_27,
		GP_2_26_FN,	GPSR2_26,
		GP_2_25_FN,	GPSR2_25,
		GP_2_24_FN,	GPSR2_24,
		GP_2_23_FN,	GPSR2_23,
		GP_2_22_FN,	GPSR2_22,
		GP_2_21_FN,	GPSR2_21,
		GP_2_20_FN,	GPSR2_20,
		GP_2_19_FN,	GPSR2_19,
		GP_2_18_FN,	GPSR2_18,
		GP_2_17_FN,	GPSR2_17,
		GP_2_16_FN,	GPSR2_16,
		GP_2_15_FN,	GPSR2_15,
		GP_2_14_FN,	GPSR2_14,
		GP_2_13_FN,	GPSR2_13,
		GP_2_12_FN,	GPSR2_12,
		GP_2_11_FN,	GPSR2_11,
		GP_2_10_FN,	GPSR2_10,
		GP_2_9_FN,	GPSR2_9,
		GP_2_8_FN,	GPSR2_8,
		GP_2_7_FN,	GPSR2_7,
		GP_2_6_FN,	GPSR2_6,
		GP_2_5_FN,	GPSR2_5,
		GP_2_4_FN,	GPSR2_4,
		GP_2_3_FN,	GPSR2_3,
		GP_2_2_FN,	GPSR2_2,
		GP_2_1_FN,	GPSR2_1,
		GP_2_0_FN,	GPSR2_0, ))
	},
	{ PINMUX_CFG_REG_VAR("GPSR3", 0xe606010c, 32,
			     GROUP(-22, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP3_31_10 RESERVED */
		GP_3_9_FN,	GPSR3_9,
		GP_3_8_FN,	GPSR3_8,
		GP_3_7_FN,	GPSR3_7,
		GP_3_6_FN,	GPSR3_6,
		GP_3_5_FN,	GPSR3_5,
		GP_3_4_FN,	GPSR3_4,
		GP_3_3_FN,	GPSR3_3,
		GP_3_2_FN,	GPSR3_2,
		GP_3_1_FN,	GPSR3_1,
		GP_3_0_FN,	GPSR3_0, ))
	},
	{ PINMUX_CFG_REG("GPSR4", 0xe6060110, 32, 1, GROUP(
		GP_4_31_FN,	GPSR4_31,
		GP_4_30_FN,	GPSR4_30,
		GP_4_29_FN,	GPSR4_29,
		GP_4_28_FN,	GPSR4_28,
		GP_4_27_FN,	GPSR4_27,
		GP_4_26_FN,	GPSR4_26,
		GP_4_25_FN,	GPSR4_25,
		GP_4_24_FN,	GPSR4_24,
		GP_4_23_FN,	GPSR4_23,
		GP_4_22_FN,	GPSR4_22,
		GP_4_21_FN,	GPSR4_21,
		GP_4_20_FN,	GPSR4_20,
		GP_4_19_FN,	GPSR4_19,
		GP_4_18_FN,	GPSR4_18,
		GP_4_17_FN,	GPSR4_17,
		GP_4_16_FN,	GPSR4_16,
		GP_4_15_FN,	GPSR4_15,
		GP_4_14_FN,	GPSR4_14,
		GP_4_13_FN,	GPSR4_13,
		GP_4_12_FN,	GPSR4_12,
		GP_4_11_FN,	GPSR4_11,
		GP_4_10_FN,	GPSR4_10,
		GP_4_9_FN,	GPSR4_9,
		GP_4_8_FN,	GPSR4_8,
		GP_4_7_FN,	GPSR4_7,
		GP_4_6_FN,	GPSR4_6,
		GP_4_5_FN,	GPSR4_5,
		GP_4_4_FN,	GPSR4_4,
		GP_4_3_FN,	GPSR4_3,
		GP_4_2_FN,	GPSR4_2,
		GP_4_1_FN,	GPSR4_1,
		GP_4_0_FN,	GPSR4_0, ))
	},
	{ PINMUX_CFG_REG_VAR("GPSR5", 0xe6060114, 32,
			     GROUP(-11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP5_31_21 RESERVED */
		GP_5_20_FN,	GPSR5_20,
		GP_5_19_FN,	GPSR5_19,
		GP_5_18_FN,	GPSR5_18,
		GP_5_17_FN,	GPSR5_17,
		GP_5_16_FN,	GPSR5_16,
		GP_5_15_FN,	GPSR5_15,
		GP_5_14_FN,	GPSR5_14,
		GP_5_13_FN,	GPSR5_13,
		GP_5_12_FN,	GPSR5_12,
		GP_5_11_FN,	GPSR5_11,
		GP_5_10_FN,	GPSR5_10,
		GP_5_9_FN,	GPSR5_9,
		GP_5_8_FN,	GPSR5_8,
		GP_5_7_FN,	GPSR5_7,
		GP_5_6_FN,	GPSR5_6,
		GP_5_5_FN,	GPSR5_5,
		GP_5_4_FN,	GPSR5_4,
		GP_5_3_FN,	GPSR5_3,
		GP_5_2_FN,	GPSR5_2,
		GP_5_1_FN,	GPSR5_1,
		GP_5_0_FN,	GPSR5_0, ))
	},
	{ PINMUX_CFG_REG_VAR("GPSR6", 0xe6060118, 32,
			     GROUP(-18, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1),
			     GROUP(
		/* GP6_31_14 RESERVED */
		GP_6_13_FN,	GPSR6_13,
		GP_6_12_FN,	GPSR6_12,
		GP_6_11_FN,	GPSR6_11,
		GP_6_10_FN,	GPSR6_10,
		GP_6_9_FN,	GPSR6_9,
		GP_6_8_FN,	GPSR6_8,
		GP_6_7_FN,	GPSR6_7,
		GP_6_6_FN,	GPSR6_6,
		GP_6_5_FN,	GPSR6_5,
		GP_6_4_FN,	GPSR6_4,
		GP_6_3_FN,	GPSR6_3,
		GP_6_2_FN,	GPSR6_2,
		GP_6_1_FN,	GPSR6_1,
		GP_6_0_FN,	GPSR6_0, ))
	},
#undef F_
#undef FM

#define F_(x, y)	x,
#define FM(x)		FN_##x,
	{ PINMUX_CFG_REG("IPSR0", 0xe6060200, 32, 4, GROUP(
		IP0_31_28
		IP0_27_24
		IP0_23_20
		IP0_19_16
		IP0_15_12
		IP0_11_8
		IP0_7_4
		IP0_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR1", 0xe6060204, 32, 4, GROUP(
		IP1_31_28
		IP1_27_24
		IP1_23_20
		IP1_19_16
		IP1_15_12
		IP1_11_8
		IP1_7_4
		IP1_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR2", 0xe6060208, 32, 4, GROUP(
		IP2_31_28
		IP2_27_24
		IP2_23_20
		IP2_19_16
		IP2_15_12
		IP2_11_8
		IP2_7_4
		IP2_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR3", 0xe606020c, 32, 4, GROUP(
		IP3_31_28
		IP3_27_24
		IP3_23_20
		IP3_19_16
		IP3_15_12
		IP3_11_8
		IP3_7_4
		IP3_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR4", 0xe6060210, 32, 4, GROUP(
		IP4_31_28
		IP4_27_24
		IP4_23_20
		IP4_19_16
		IP4_15_12
		IP4_11_8
		IP4_7_4
		IP4_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR5", 0xe6060214, 32, 4, GROUP(
		IP5_31_28
		IP5_27_24
		IP5_23_20
		IP5_19_16
		IP5_15_12
		IP5_11_8
		IP5_7_4
		IP5_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR6", 0xe6060218, 32, 4, GROUP(
		IP6_31_28
		IP6_27_24
		IP6_23_20
		IP6_19_16
		IP6_15_12
		IP6_11_8
		IP6_7_4
		IP6_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR7", 0xe606021c, 32, 4, GROUP(
		IP7_31_28
		IP7_27_24
		IP7_23_20
		IP7_19_16
		IP7_15_12
		IP7_11_8
		IP7_7_4
		IP7_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR8", 0xe6060220, 32, 4, GROUP(
		IP8_31_28
		IP8_27_24
		IP8_23_20
		IP8_19_16
		IP8_15_12
		IP8_11_8
		IP8_7_4
		IP8_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR9", 0xe6060224, 32, 4, GROUP(
		IP9_31_28
		IP9_27_24
		IP9_23_20
		IP9_19_16
		IP9_15_12
		IP9_11_8
		IP9_7_4
		IP9_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR10", 0xe6060228, 32, 4, GROUP(
		IP10_31_28
		IP10_27_24
		IP10_23_20
		IP10_19_16
		IP10_15_12
		IP10_11_8
		IP10_7_4
		IP10_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR11", 0xe606022c, 32, 4, GROUP(
		IP11_31_28
		IP11_27_24
		IP11_23_20
		IP11_19_16
		IP11_15_12
		IP11_11_8
		IP11_7_4
		IP11_3_0 ))
	},
	{ PINMUX_CFG_REG("IPSR12", 0xe6060230, 32, 4, GROUP(
		IP12_31_28
		IP12_27_24
		IP12_23_20
		IP12_19_16
		IP12_15_12
		IP12_11_8
		IP12_7_4
		IP12_3_0 ))
	},
	{ PINMUX_CFG_REG_VAR("IPSR13", 0xe6060234, 32,
			     GROUP(-24, 4, 4),
			     GROUP(
		/* IP13_31_8 RESERVED */
		IP13_7_4
		IP13_3_0 ))
	},
#undef F_
#undef FM

#define F_(x, y)	x,
#define FM(x)		FN_##x,
	{ PINMUX_CFG_REG_VAR("MOD_SEL0", 0xe6060500, 32,
			     GROUP(-1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, -1,
				   1, 1, 1, 1, 1, 1, -4, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* RESERVED 31 */
		MOD_SEL0_30
		MOD_SEL0_29
		MOD_SEL0_28
		MOD_SEL0_27
		MOD_SEL0_26
		MOD_SEL0_25
		MOD_SEL0_24_23
		MOD_SEL0_22_21
		MOD_SEL0_20_19
		MOD_SEL0_18_17
		/* RESERVED 16 */
		MOD_SEL0_15
		MOD_SEL0_14
		MOD_SEL0_13
		MOD_SEL0_12
		MOD_SEL0_11
		MOD_SEL0_10
		/* RESERVED 9, 8, 7, 6 */
		MOD_SEL0_5
		MOD_SEL0_4
		MOD_SEL0_3
		MOD_SEL0_2
		MOD_SEL0_1
		MOD_SEL0_0 ))
	},
	{ PINMUX_CFG_REG_VAR("MOD_SEL1", 0xe6060504, 32,
			     GROUP(1, 1, 1, 1, 1, 1, -26),
			     GROUP(
		MOD_SEL1_31
		MOD_SEL1_30
		MOD_SEL1_29
		MOD_SEL1_28
		MOD_SEL1_27
		MOD_SEL1_26
		/* RESERVED 25-0 */ ))
	},
	{ /* sentinel */ }
};

enum ioctrl_regs {
	POCCTRL0,
	POCCTRL2,
	TDSELCTRL,
};

static const struct pinmux_ioctrl_reg pinmux_ioctrl_regs[] = {
	[POCCTRL0] = { 0xe6060380, },
	[POCCTRL2] = { 0xe6060388, },
	[TDSELCTRL] = { 0xe60603c0, },
	{ /* sentinel */ }
};

static int r8a77995_pin_to_pocctrl(unsigned int pin, u32 *pocctrl)
{
	switch (pin) {
	case RCAR_GP_PIN(3, 0) ... RCAR_GP_PIN(3, 9):
		*pocctrl = pinmux_ioctrl_regs[POCCTRL0].reg;
		return 29 - (pin - RCAR_GP_PIN(3, 0));

	case PIN_VDDQ_AVB0:
		*pocctrl = pinmux_ioctrl_regs[POCCTRL2].reg;
		return 0;

	default:
		return -EINVAL;
	}
}

static const struct pinmux_bias_reg pinmux_bias_regs[] = {
	{ PINMUX_BIAS_REG("PUEN0", 0xe6060400, "PUD0", 0xe6060440) {
		[ 0] = RCAR_GP_PIN(1, 9),	/* DU_DG1 */
		[ 1] = RCAR_GP_PIN(1, 8),	/* DU_DG0 */
		[ 2] = RCAR_GP_PIN(1, 7),	/* DU_DB7 */
		[ 3] = RCAR_GP_PIN(1, 6),	/* DU_DB6 */
		[ 4] = RCAR_GP_PIN(1, 5),	/* DU_DB5 */
		[ 5] = RCAR_GP_PIN(1, 4),	/* DU_DB4 */
		[ 6] = RCAR_GP_PIN(1, 3),	/* DU_DB3 */
		[ 7] = RCAR_GP_PIN(1, 2),	/* DU_DB2 */
		[ 8] = RCAR_GP_PIN(1, 1),	/* DU_DB1 */
		[ 9] = RCAR_GP_PIN(1, 0),	/* DU_DB0 */
		[10] = PIN_MLB_REF,		/* MLB_REF */
		[11] = RCAR_GP_PIN(0, 8),	/* MLB_SIG */
		[12] = RCAR_GP_PIN(0, 7),	/* MLB_DAT */
		[13] = RCAR_GP_PIN(0, 6),	/* MLB_CLK */
		[14] = RCAR_GP_PIN(0, 5),	/* MSIOF2_RXD */
		[15] = RCAR_GP_PIN(0, 4),	/* MSIOF2_TXD */
		[16] = RCAR_GP_PIN(0, 3),	/* MSIOF2_SCK */
		[17] = RCAR_GP_PIN(0, 2),	/* IRQ0_A */
		[18] = RCAR_GP_PIN(0, 1),	/* USB0_OVC */
		[19] = RCAR_GP_PIN(0, 0),	/* USB0_PWEN */
		[20] = PIN_PRESETOUT_N,		/* PRESETOUT# */
		[21] = PIN_DU_DOTCLKIN0,	/* DU_DOTCLKIN0 */
		[22] = PIN_FSCLKST_N,		/* FSCLKST# */
		[23] = SH_PFC_PIN_NONE,
		[24] = SH_PFC_PIN_NONE,
		[25] = SH_PFC_PIN_NONE,
		[26] = SH_PFC_PIN_NONE,
		[27] = SH_PFC_PIN_NONE,
		[28] = PIN_TDI,			/* TDI */
		[29] = PIN_TMS,			/* TMS */
		[30] = PIN_TCK,			/* TCK */
		[31] = PIN_TRST_N,		/* TRST# */
	} },
	{ PINMUX_BIAS_REG("PUEN1", 0xe6060404, "PUD1", 0xe6060444) {
		[ 0] = RCAR_GP_PIN(2, 9),	/* VI4_DATA8 */
		[ 1] = RCAR_GP_PIN(2, 8),	/* VI4_DATA7 */
		[ 2] = RCAR_GP_PIN(2, 7),	/* VI4_DATA6 */
		[ 3] = RCAR_GP_PIN(2, 6),	/* VI4_DATA5 */
		[ 4] = RCAR_GP_PIN(2, 5),	/* VI4_DATA4 */
		[ 5] = RCAR_GP_PIN(2, 4),	/* VI4_DATA3 */
		[ 6] = RCAR_GP_PIN(2, 3),	/* VI4_DATA2 */
		[ 7] = RCAR_GP_PIN(2, 2),	/* VI4_DATA1 */
		[ 8] = RCAR_GP_PIN(2, 1),	/* VI4_DATA0 */
		[ 9] = RCAR_GP_PIN(2, 0),	/* VI4_CLK */
		[10] = RCAR_GP_PIN(1, 31),	/* QPOLB */
		[11] = RCAR_GP_PIN(1, 30),	/* QPOLA */
		[12] = RCAR_GP_PIN(1, 29),	/* DU_CDE */
		[13] = RCAR_GP_PIN(1, 28),	/* DU_DISP/CDE */
		[14] = RCAR_GP_PIN(1, 27),	/* DU_DISP */
		[15] = RCAR_GP_PIN(1, 26),	/* DU_VSYNC */
		[16] = RCAR_GP_PIN(1, 25),	/* DU_HSYNC */
		[17] = RCAR_GP_PIN(1, 24),	/* DU_DOTCLKOUT0 */
		[18] = RCAR_GP_PIN(1, 23),	/* DU_DR7 */
		[19] = RCAR_GP_PIN(1, 22),	/* DU_DR6 */
		[20] = RCAR_GP_PIN(1, 21),	/* DU_DR5 */
		[21] = RCAR_GP_PIN(1, 20),	/* DU_DR4 */
		[22] = RCAR_GP_PIN(1, 19),	/* DU_DR3 */
		[23] = RCAR_GP_PIN(1, 18),	/* DU_DR2 */
		[24] = RCAR_GP_PIN(1, 17),	/* DU_DR1 */
		[25] = RCAR_GP_PIN(1, 16),	/* DU_DR0 */
		[26] = RCAR_GP_PIN(1, 15),	/* DU_DG7 */
		[27] = RCAR_GP_PIN(1, 14),	/* DU_DG6 */
		[28] = RCAR_GP_PIN(1, 13),	/* DU_DG5 */
		[29] = RCAR_GP_PIN(1, 12),	/* DU_DG4 */
		[30] = RCAR_GP_PIN(1, 11),	/* DU_DG3 */
		[31] = RCAR_GP_PIN(1, 10),	/* DU_DG2 */
	} },
	{ PINMUX_BIAS_REG("PUEN2", 0xe6060408, "PUD2", 0xe6060448) {
		[ 0] = RCAR_GP_PIN(3, 8),	/* NFDATA6 */
		[ 1] = RCAR_GP_PIN(3, 7),	/* NFDATA5 */
		[ 2] = RCAR_GP_PIN(3, 6),	/* NFDATA4 */
		[ 3] = RCAR_GP_PIN(3, 5),	/* NFDATA3 */
		[ 4] = RCAR_GP_PIN(3, 4),	/* NFDATA2 */
		[ 5] = RCAR_GP_PIN(3, 3),	/* NFDATA1 */
		[ 6] = RCAR_GP_PIN(3, 2),	/* NFDATA0 */
		[ 7] = RCAR_GP_PIN(3, 1),	/* NFWE# (PUEN) / NFRE# (PUD) */
		[ 8] = RCAR_GP_PIN(3, 0),	/* NFRE# (PUEN) / NFWE# (PUD) */
		[ 9] = RCAR_GP_PIN(4, 0),	/* NFRB# */
		[10] = RCAR_GP_PIN(2, 31),	/* NFCE# */
		[11] = RCAR_GP_PIN(2, 30),	/* NFCLE */
		[12] = RCAR_GP_PIN(2, 29),	/* NFALE */
		[13] = RCAR_GP_PIN(2, 28),	/* VI4_CLKENB */
		[14] = RCAR_GP_PIN(2, 27),	/* VI4_FIELD */
		[15] = RCAR_GP_PIN(2, 26),	/* VI4_HSYNC# */
		[16] = RCAR_GP_PIN(2, 25),	/* VI4_VSYNC# */
		[17] = RCAR_GP_PIN(2, 24),	/* VI4_DATA23 */
		[18] = RCAR_GP_PIN(2, 23),	/* VI4_DATA22 */
		[19] = RCAR_GP_PIN(2, 22),	/* VI4_DATA21 */
		[20] = RCAR_GP_PIN(2, 21),	/* VI4_DATA20 */
		[21] = RCAR_GP_PIN(2, 20),	/* VI4_DATA19 */
		[22] = RCAR_GP_PIN(2, 19),	/* VI4_DATA18 */
		[23] = RCAR_GP_PIN(2, 18),	/* VI4_DATA17 */
		[24] = RCAR_GP_PIN(2, 17),	/* VI4_DATA16 */
		[25] = RCAR_GP_PIN(2, 16),	/* VI4_DATA15 */
		[26] = RCAR_GP_PIN(2, 15),	/* VI4_DATA14 */
		[27] = RCAR_GP_PIN(2, 14),	/* VI4_DATA13 */
		[28] = RCAR_GP_PIN(2, 13),	/* VI4_DATA12 */
		[29] = RCAR_GP_PIN(2, 12),	/* VI4_DATA11 */
		[30] = RCAR_GP_PIN(2, 11),	/* VI4_DATA10 */
		[31] = RCAR_GP_PIN(2, 10),	/* VI4_DATA9 */
	} },
	{ PINMUX_BIAS_REG("PUEN3", 0xe606040c, "PUD3", 0xe606044c) {
		[ 0] = RCAR_GP_PIN(4, 31),	/* CAN0_RX_A */
		[ 1] = RCAR_GP_PIN(5, 2),	/* CAN_CLK */
		[ 2] = RCAR_GP_PIN(5, 1),	/* TPU0TO1_A */
		[ 3] = RCAR_GP_PIN(5, 0),	/* TPU0TO0_A */
		[ 4] = RCAR_GP_PIN(4, 27),	/* TX2 */
		[ 5] = RCAR_GP_PIN(4, 26),	/* RX2 */
		[ 6] = RCAR_GP_PIN(4, 25),	/* SCK2 */
		[ 7] = RCAR_GP_PIN(4, 24),	/* TX1_A */
		[ 8] = RCAR_GP_PIN(4, 23),	/* RX1_A */
		[ 9] = RCAR_GP_PIN(4, 22),	/* SCK1_A */
		[10] = RCAR_GP_PIN(4, 21),	/* TX0_A */
		[11] = RCAR_GP_PIN(4, 20),	/* RX0_A */
		[12] = RCAR_GP_PIN(4, 19),	/* SCK0_A */
		[13] = RCAR_GP_PIN(4, 18),	/* MSIOF1_RXD */
		[14] = RCAR_GP_PIN(4, 17),	/* MSIOF1_TXD */
		[15] = RCAR_GP_PIN(4, 16),	/* MSIOF1_SCK */
		[16] = RCAR_GP_PIN(4, 15),	/* MSIOF0_RXD */
		[17] = RCAR_GP_PIN(4, 14),	/* MSIOF0_TXD */
		[18] = RCAR_GP_PIN(4, 13),	/* MSIOF0_SYNC */
		[19] = RCAR_GP_PIN(4, 12),	/* MSIOF0_SCK */
		[20] = RCAR_GP_PIN(4, 11),	/* SDA1 */
		[21] = RCAR_GP_PIN(4, 10),	/* SCL1 */
		[22] = RCAR_GP_PIN(4, 9),	/* SDA0 */
		[23] = RCAR_GP_PIN(4, 8),	/* SCL0 */
		[24] = RCAR_GP_PIN(4, 7),	/* SSI_WS4_A */
		[25] = RCAR_GP_PIN(4, 6),	/* SSI_SDATA4_A */
		[26] = RCAR_GP_PIN(4, 5),	/* SSI_SCK4_A */
		[27] = RCAR_GP_PIN(4, 4),	/* SSI_WS34 */
		[28] = RCAR_GP_PIN(4, 3),	/* SSI_SDATA3 */
		[29] = RCAR_GP_PIN(4, 2),	/* SSI_SCK34 */
		[30] = RCAR_GP_PIN(4, 1),	/* AUDIO_CLKA */
		[31] = RCAR_GP_PIN(3, 9),	/* NFDATA7 */
	} },
	{ PINMUX_BIAS_REG("PUEN4", 0xe6060410, "PUD4", 0xe6060450) {
		[ 0] = RCAR_GP_PIN(6, 10),	/* QSPI1_IO3 */
		[ 1] = RCAR_GP_PIN(6, 9),	/* QSPI1_IO2 */
		[ 2] = RCAR_GP_PIN(6, 8),	/* QSPI1_MISO_IO1 */
		[ 3] = RCAR_GP_PIN(6, 7),	/* QSPI1_MOSI_IO0 */
		[ 4] = RCAR_GP_PIN(6, 6),	/* QSPI1_SPCLK */
		[ 5] = RCAR_GP_PIN(6, 5),	/* QSPI0_SSL */
		[ 6] = RCAR_GP_PIN(6, 4),	/* QSPI0_IO3 */
		[ 7] = RCAR_GP_PIN(6, 3),	/* QSPI0_IO2 */
		[ 8] = RCAR_GP_PIN(6, 2),	/* QSPI0_MISO_IO1 */
		[ 9] = RCAR_GP_PIN(6, 1),	/* QSPI0_MOSI_IO0 */
		[10] = RCAR_GP_PIN(6, 0),	/* QSPI0_SPCLK */
		[11] = RCAR_GP_PIN(5, 20),	/* AVB0_LINK */
		[12] = RCAR_GP_PIN(5, 19),	/* AVB0_PHY_INT */
		[13] = RCAR_GP_PIN(5, 18),	/* AVB0_MAGIC */
		[14] = RCAR_GP_PIN(5, 17),	/* AVB0_MDC */
		[15] = RCAR_GP_PIN(5, 16),	/* AVB0_MDIO */
		[16] = RCAR_GP_PIN(5, 15),	/* AVB0_TXCREFCLK */
		[17] = RCAR_GP_PIN(5, 14),	/* AVB0_TD3 */
		[18] = RCAR_GP_PIN(5, 13),	/* AVB0_TD2 */
		[19] = RCAR_GP_PIN(5, 12),	/* AVB0_TD1 */
		[20] = RCAR_GP_PIN(5, 11),	/* AVB0_TD0 */
		[21] = RCAR_GP_PIN(5, 10),	/* AVB0_TXC */
		[22] = RCAR_GP_PIN(5, 9),	/* AVB0_TX_CTL */
		[23] = RCAR_GP_PIN(5, 8),	/* AVB0_RD3 */
		[24] = RCAR_GP_PIN(5, 7),	/* AVB0_RD2 */
		[25] = RCAR_GP_PIN(5, 6),	/* AVB0_RD1 */
		[26] = RCAR_GP_PIN(5, 5),	/* AVB0_RD0 */
		[27] = RCAR_GP_PIN(5, 4),	/* AVB0_RXC */
		[28] = RCAR_GP_PIN(5, 3),	/* AVB0_RX_CTL */
		[29] = RCAR_GP_PIN(4, 30),	/* CAN1_TX_A */
		[30] = RCAR_GP_PIN(4, 29),	/* CAN1_RX_A */
		[31] = RCAR_GP_PIN(4, 28),	/* CAN0_TX_A */
	} },
	{ PINMUX_BIAS_REG("PUEN5", 0xe6060414, "PUD4", 0xe6060454) {
		[ 0] = SH_PFC_PIN_NONE,
		[ 1] = SH_PFC_PIN_NONE,
		[ 2] = SH_PFC_PIN_NONE,
		[ 3] = SH_PFC_PIN_NONE,
		[ 4] = SH_PFC_PIN_NONE,
		[ 5] = SH_PFC_PIN_NONE,
		[ 6] = SH_PFC_PIN_NONE,
		[ 7] = SH_PFC_PIN_NONE,
		[ 8] = SH_PFC_PIN_NONE,
		[ 9] = SH_PFC_PIN_NONE,
		[10] = SH_PFC_PIN_NONE,
		[11] = SH_PFC_PIN_NONE,
		[12] = SH_PFC_PIN_NONE,
		[13] = SH_PFC_PIN_NONE,
		[14] = SH_PFC_PIN_NONE,
		[15] = SH_PFC_PIN_NONE,
		[16] = SH_PFC_PIN_NONE,
		[17] = SH_PFC_PIN_NONE,
		[18] = SH_PFC_PIN_NONE,
		[19] = SH_PFC_PIN_NONE,
		[20] = SH_PFC_PIN_NONE,
		[21] = SH_PFC_PIN_NONE,
		[22] = SH_PFC_PIN_NONE,
		[23] = SH_PFC_PIN_NONE,
		[24] = SH_PFC_PIN_NONE,
		[25] = SH_PFC_PIN_NONE,
		[26] = SH_PFC_PIN_NONE,
		[27] = SH_PFC_PIN_NONE,
		[28] = SH_PFC_PIN_NONE,
		[29] = RCAR_GP_PIN(6, 13),	/* RPC_INT# */
		[30] = RCAR_GP_PIN(6, 12),	/* RPC_RESET# */
		[31] = RCAR_GP_PIN(6, 11),	/* QSPI1_SSL */
	} },
	{ /* sentinel */ }
};

static const struct pinmux_bias_reg *
r8a77995_pin_to_bias_reg(const struct sh_pfc *pfc, unsigned int pin,
			 unsigned int *puen_bit, unsigned int *pud_bit)
{
	const struct pinmux_bias_reg *reg;
	unsigned int bit;

	reg = rcar_pin_to_bias_reg(pfc->info, pin, &bit);
	if (!reg)
		return reg;

	*puen_bit = bit;

	/* NFWE# and NFRE# use different bit positions in PUD2 */
	switch (pin) {
	case RCAR_GP_PIN(3, 0):	/* NFRE# */
		*pud_bit = 7;
		break;

	case RCAR_GP_PIN(3, 1):	/* NFWE# */
		*pud_bit = 8;
		break;

	default:
		*pud_bit = bit;
		break;
	}

	return reg;
}

static unsigned int r8a77995_pinmux_get_bias(struct sh_pfc *pfc,
					     unsigned int pin)
{
	const struct pinmux_bias_reg *reg;
	unsigned int puen_bit, pud_bit;

	reg = r8a77995_pin_to_bias_reg(pfc, pin, &puen_bit, &pud_bit);
	if (!reg)
		return PIN_CONFIG_BIAS_DISABLE;

	if (!(sh_pfc_read(pfc, reg->puen) & BIT(puen_bit)))
		return PIN_CONFIG_BIAS_DISABLE;
	else if (sh_pfc_read(pfc, reg->pud) & BIT(pud_bit))
		return PIN_CONFIG_BIAS_PULL_UP;
	else
		return PIN_CONFIG_BIAS_PULL_DOWN;
}

static void r8a77995_pinmux_set_bias(struct sh_pfc *pfc, unsigned int pin,
				     unsigned int bias)
{
	const struct pinmux_bias_reg *reg;
	unsigned int puen_bit, pud_bit;
	u32 enable, updown;

	reg = r8a77995_pin_to_bias_reg(pfc, pin, &puen_bit, &pud_bit);
	if (!reg)
		return;

	enable = sh_pfc_read(pfc, reg->puen) & ~BIT(puen_bit);
	if (bias != PIN_CONFIG_BIAS_DISABLE) {
		enable |= BIT(puen_bit);

		updown = sh_pfc_read(pfc, reg->pud) & ~BIT(pud_bit);
		if (bias == PIN_CONFIG_BIAS_PULL_UP)
			updown |= BIT(pud_bit);

		sh_pfc_write(pfc, reg->pud, updown);
	}
	sh_pfc_write(pfc, reg->puen, enable);
}

static const struct sh_pfc_soc_operations r8a77995_pfc_ops = {
	.pin_to_pocctrl = r8a77995_pin_to_pocctrl,
	.get_bias = r8a77995_pinmux_get_bias,
	.set_bias = r8a77995_pinmux_set_bias,
};

const struct sh_pfc_soc_info r8a77995_pinmux_info = {
	.name = "r8a77995_pfc",
	.ops = &r8a77995_pfc_ops,
	.unlock_reg = 0xe6060000, /* PMMR */

	.function = { PINMUX_FUNCTION_BEGIN, PINMUX_FUNCTION_END },

	.pins = pinmux_pins,
	.nr_pins = ARRAY_SIZE(pinmux_pins),
	.groups = pinmux_groups,
	.nr_groups = ARRAY_SIZE(pinmux_groups),
	.functions = pinmux_functions,
	.nr_functions = ARRAY_SIZE(pinmux_functions),

	.cfg_regs = pinmux_config_regs,
	.bias_regs = pinmux_bias_regs,
	.ioctrl_regs = pinmux_ioctrl_regs,

	.pinmux_data = pinmux_data,
	.pinmux_data_size = ARRAY_SIZE(pinmux_data),
};
