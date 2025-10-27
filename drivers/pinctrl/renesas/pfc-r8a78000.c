// SPDX-License-Identifier: GPL-2.0-only
/*
 * R8A78000 processor support - PFC hardware block.
 *
 * Copyright (C) 2025 Renesas Electronics Corp.
 *
 */

#include <dm.h>
#include <errno.h>
#include <bitfield.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/kernel.h>

#include "sh_pfc.h"

#define CFG_FLAGS (SH_PFC_PIN_CFG_DRIVE_STRENGTH | SH_PFC_PIN_CFG_PULL_UP_DOWN)

#define CPU_ALL_GP(fn, sfx)				\
	PORT_GP_CFG_28(0,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_22(1,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_29(2,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_17(3,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_16(4,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_23(5,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_31(6,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_31(7,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_16(8,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_1(8, 26,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_1(8, 27,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_1(8, 28,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_1(8, 29,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_1(8, 30,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_1(8, 31,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_17(9,	fn, sfx, CFG_FLAGS),	\
	PORT_GP_CFG_14(10,	fn, sfx, CFG_FLAGS)

/*
 * F_() : just information
 * FM() : macro for FN_xxx / xxx_MARK
 */

/* GPSR0 */
#define GPSR0_27	F_(DP2_HOTPLUG,		GRP0_27_FUNC)
#define GPSR0_26	F_(DP1_HOTPLUG,		GRP0_26_FUNC)
#define GPSR0_25	F_(DP0_HOTPLUG,		GRP0_25_FUNC)
#define GPSR0_24	F_(MSIOF1_SS2_A,	GRP0_24_FUNC)
#define GPSR0_23	F_(MSIOF1_SS1_A,	GRP0_23_FUNC)
#define GPSR0_22	F_(MSIOF1_SYNC_A,	GRP0_22_FUNC)
#define GPSR0_21	F_(MSIOF1_RXD_A,	GRP0_21_FUNC)
#define GPSR0_20	F_(MSIOF1_TXD_A,	GRP0_20_FUNC)
#define GPSR0_19	F_(MSIOF1_SCK_A,	GRP0_19_FUNC)
#define GPSR0_18	F_(MSIOF0_SS2,		GRP0_18_FUNC)
#define GPSR0_17	F_(MSIOF0_SS1,		GRP0_17_FUNC)
#define GPSR0_16	F_(MSIOF0_SYNC,		GRP0_16_FUNC)
#define GPSR0_15	F_(MSIOF0_RXD,		GRP0_15_FUNC)
#define GPSR0_14	F_(MSIOF0_TXD,		GRP0_14_FUNC)
#define GPSR0_13	F_(MSIOF0_SCK,		GRP0_13_FUNC)
#define GPSR0_12	F_(RXDB_EXTFXR_A,	GRP0_12_FUNC)
#define GPSR0_11	F_(FXR_TXENB_N_A,	GRP0_11_FUNC)
#define GPSR0_10	F_(FXR_TXDB_A,		GRP0_10_FUNC)
#define GPSR0_9		F_(RXDA_EXTFXR_A,	GRP0_9_FUNC)
#define GPSR0_8		F_(FXR_TXENA_N_A,	GRP0_8_FUNC)
#define GPSR0_7		F_(FXR_TXDA_A,		GRP0_7_FUNC)
#define GPSR0_6		F_(CLK_EXTFXR_A,	GRP0_6_FUNC)
#define GPSR0_5		F_(FXR_CLKOUT2_A,	GRP0_5_FUNC)
#define GPSR0_4		F_(FXR_CLKOUT1_A,	GRP0_4_FUNC)
#define GPSR0_3		F_(STPWT_EXTFXR_A,	GRP0_3_FUNC)
#define GPSR0_2		F_(GP0_02,		GRP0_2_FUNC)
#define GPSR0_1		F_(GP0_01,		GRP0_1_FUNC)
#define GPSR0_0		F_(GP0_00,		GRP0_0_FUNC)

/* GPSR1 */
#define GPSR1_21	F_(RLIN33TX,		GRP1_21_FUNC)
#define GPSR1_20	F_(RLIN33RX_INTP19,	GRP1_20_FUNC)
#define GPSR1_19	F_(RLIN32TX,		GRP1_19_FUNC)
#define GPSR1_18	F_(RLIN32RX_INTP18,	GRP1_18_FUNC)
#define GPSR1_17	F_(RLIN31TX,		GRP1_17_FUNC)
#define GPSR1_16	F_(RLIN31RX_INTP17,	GRP1_16_FUNC)
#define GPSR1_15	F_(RLIN30TX,		GRP1_15_FUNC)
#define GPSR1_14	F_(RLIN30RX_INTP16,	GRP1_14_FUNC)
#define GPSR1_13	F_(CAN6TX,		GRP1_13_FUNC)
#define GPSR1_12	F_(CAN6RX_INTP6,	GRP1_12_FUNC)
#define GPSR1_11	F_(CAN5TX,		GRP1_11_FUNC)
#define GPSR1_10	F_(CAN5RX_INTP5,	GRP1_10_FUNC)
#define GPSR1_9		F_(CAN4TX,		GRP1_9_FUNC)
#define GPSR1_8		F_(CAN4RX_INTP4,	GRP1_8_FUNC)
#define GPSR1_7		F_(CAN3TX,		GRP1_7_FUNC)
#define GPSR1_6		F_(CAN3RX_INTP3,	GRP1_6_FUNC)
#define GPSR1_5		F_(CAN2TX,		GRP1_5_FUNC)
#define GPSR1_4		F_(CAN2RX_INTP2,	GRP1_4_FUNC)
#define GPSR1_3		F_(CAN1TX,		GRP1_3_FUNC)
#define GPSR1_2		F_(CAN1RX_INTP1,	GRP1_2_FUNC)
#define GPSR1_1		F_(CAN0TX,		GRP1_1_FUNC)
#define GPSR1_0		F_(CAN0RX_INTP0,	GRP1_0_FUNC)

/* GPSR2 */
#define GPSR2_28	F_(INTP34_B,		GRP2_28_FUNC)
#define GPSR2_27	F_(TAUD1O3,		GRP2_27_FUNC)
#define GPSR2_26	F_(TAUD1O2,		GRP2_26_FUNC)
#define GPSR2_25	F_(TAUD1O1,		GRP2_25_FUNC)
#define GPSR2_24	F_(TAUD1O0,		GRP2_24_FUNC)
#define GPSR2_23	F_(EXTCLK0O_B,		GRP2_23_FUNC)
#define GPSR2_22	F_(AVS1,		GRP2_22_FUNC)
#define GPSR2_21	F_(AVS0,		GRP2_21_FUNC)
#define GPSR2_20	F_(SDA0,		GRP2_20_FUNC)
#define GPSR2_19	F_(SCL0,		GRP2_19_FUNC)
#define GPSR2_18	F_(INTP33_B,		GRP2_18_FUNC)
#define GPSR2_17	F_(INTP32_B,		GRP2_17_FUNC)
#define GPSR2_16	F_(CAN_CLK,		GRP2_16_FUNC)
#define GPSR2_15	F_(CAN15TX_B,		GRP2_15_FUNC)
#define GPSR2_14	F_(CAN15RX_INTP15_B,	GRP2_14_FUNC)
#define GPSR2_13	F_(CAN14TX_B,		GRP2_13_FUNC)
#define GPSR2_12	F_(CAN14RX_INTP14_B,	GRP2_12_FUNC)
#define GPSR2_11	F_(CAN13TX_B,		GRP2_11_FUNC)
#define GPSR2_10	F_(CAN13RX_INTP13_B,	GRP2_10_FUNC)
#define GPSR2_9		F_(CAN12TX_B,		GRP2_9_FUNC)
#define GPSR2_8		F_(CAN12RX_INTP12_B,	GRP2_8_FUNC)
#define GPSR2_7		F_(RLIN37TX_B,		GRP2_7_FUNC)
#define GPSR2_6		F_(RLIN37RX_INTP23_B,	GRP2_6_FUNC)
#define GPSR2_5		F_(RLIN36TX_B,		GRP2_5_FUNC)
#define GPSR2_4		F_(RLIN36RX_INTP22_B,	GRP2_4_FUNC)
#define GPSR2_3		F_(RLIN35TX_B,		GRP2_3_FUNC)
#define GPSR2_2		F_(RLIN35RX_INTP21_B,	GRP2_2_FUNC)
#define GPSR2_1		F_(RLIN34TX_B,		GRP2_1_FUNC)
#define GPSR2_0		F_(RLIN34RX_INTP20_B,	GRP2_0_FUNC)

/* GPSR3 */
#define GPSR3_16	F_(ERRORIN_N,		GRP3_16_FUNC)
#define GPSR3_15	F_(ERROROUT_N,		GRP3_15_FUNC)
#define GPSR3_14	F_(QSPI1_SSL,		GRP3_14_FUNC)
#define GPSR3_13	F_(QSPI1_IO3,		GRP3_13_FUNC)
#define GPSR3_12	F_(QSPI1_IO2,		GRP3_12_FUNC)
#define GPSR3_11	F_(QSPI1_MISO_IO1,	GRP3_11_FUNC)
#define GPSR3_10	F_(QSPI1_MOSI_IO0,	GRP3_10_FUNC)
#define GPSR3_9		F_(QSPI1_SPCLK,		GRP3_9_FUNC)
#define GPSR3_8		F_(RPC_INT_N,		GRP3_8_FUNC)
#define GPSR3_7		F_(RPC_WP_N,		GRP3_7_FUNC)
#define GPSR3_6		F_(RPC_RESET_N,		GRP3_6_FUNC)
#define GPSR3_5		F_(QSPI0_SSL,		GRP3_5_FUNC)
#define GPSR3_4		F_(QSPI0_IO3,		GRP3_4_FUNC)
#define GPSR3_3		F_(QSPI0_IO2,		GRP3_3_FUNC)
#define GPSR3_2		F_(QSPI0_MISO_IO1,	GRP3_2_FUNC)
#define GPSR3_1		F_(QSPI0_MOSI_IO0,	GRP3_1_FUNC)
#define GPSR3_0		F_(QSPI0_SPCLK,		GRP3_0_FUNC)

/* GPSR4 */
#define GPSR4_15	F_(PCIE61_CLKREQ_N,	GRP4_15_FUNC)
#define GPSR4_14	F_(PCIE60_CLKREQ_N,	GRP4_14_FUNC)
#define GPSR4_13	F_(ERRORIN_N,		GRP4_13_FUNC)
#define GPSR4_12	F_(SD0_CD,		GRP4_12_FUNC)
#define GPSR4_11	F_(SD0_WP,		GRP4_11_FUNC)
#define GPSR4_10	F_(MMC0_DS,		GRP4_10_FUNC)
#define GPSR4_9		F_(MMC0_D7,		GRP4_9_FUNC)
#define GPSR4_8		F_(MMC0_D6,		GRP4_8_FUNC)
#define GPSR4_7		F_(MMC0_D5,		GRP4_7_FUNC)
#define GPSR4_6		F_(MMC0_D4,		GRP4_6_FUNC)
#define GPSR4_5		F_(MMC0_SD_D3,		GRP4_5_FUNC)
#define GPSR4_4		F_(MMC0_SD_D2,		GRP4_4_FUNC)
#define GPSR4_3		F_(MMC0_SD_D1,		GRP4_3_FUNC)
#define GPSR4_2		F_(MMC0_SD_D0,		GRP4_2_FUNC)
#define GPSR4_1		F_(MMC0_SD_CMD,		GRP4_1_FUNC)
#define GPSR4_0		F_(MMC0_SD_CLK,		GRP4_0_FUNC)

/* GPSR5 */
#define GPSR5_22	F_(TPU0TO3,		GRP5_22_FUNC)
#define GPSR5_21	F_(TPU0TO2,		GRP5_21_FUNC)
#define GPSR5_20	F_(TPU0TO1,		GRP5_20_FUNC)
#define GPSR5_19	F_(TPU0TO0,		GRP5_19_FUNC)
#define GPSR5_18	F_(TCLK4,		GRP5_18_FUNC)
#define GPSR5_17	F_(TCLK3,		GRP5_17_FUNC)
#define GPSR5_16	F_(TCLK2,		GRP5_16_FUNC)
#define GPSR5_15	F_(TCLK1,		GRP5_15_FUNC)
#define GPSR5_14	F_(IRQ3_A,		GRP5_14_FUNC)
#define GPSR5_13	F_(IRQ2_A,		GRP5_13_FUNC)
#define GPSR5_12	F_(IRQ1_A,		GRP5_12_FUNC)
#define GPSR5_11	F_(IRQ0_A,		GRP5_11_FUNC)
#define GPSR5_10	F_(HSCK1,		GRP5_10_FUNC)
#define GPSR5_9		F_(HCTS1_N,		GRP5_9_FUNC)
#define GPSR5_8		F_(HRTS1_N,		GRP5_8_FUNC)
#define GPSR5_7		F_(HRX1,		GRP5_7_FUNC)
#define GPSR5_6		F_(HTX1,		GRP5_6_FUNC)
#define GPSR5_5		F_(SCIF_CLK,		GRP5_5_FUNC)
#define GPSR5_4		F_(HSCK0,		GRP5_4_FUNC)
#define GPSR5_3		F_(HCTS0_N,		GRP5_3_FUNC)
#define GPSR5_2		F_(HRTS0_N,		GRP5_2_FUNC)
#define GPSR5_1		F_(HRX0,		GRP5_1_FUNC)
#define GPSR5_0		F_(HTX0,		GRP5_0_FUNC)

/* GPSR6 */
#define GPSR6_30	F_(AUDIO1_CLKOUT1,	GRP6_30_FUNC)
#define GPSR6_29	F_(AUDIO1_CLKOUT0,	GRP6_29_FUNC)
#define GPSR6_28	F_(SSI2_SD,		GRP6_28_FUNC)
#define GPSR6_27	F_(SSI2_WS,		GRP6_27_FUNC)
#define GPSR6_26	F_(SSI2_SCK,		GRP6_26_FUNC)
#define GPSR6_25	F_(AUDIO0_CLKOUT3,	GRP6_25_FUNC)
#define GPSR6_24	F_(AUDIO0_CLKOUT2,	GRP6_24_FUNC)
#define GPSR6_23	F_(SSI1_SD,		GRP6_23_FUNC)
#define GPSR6_22	F_(SSI1_WS,		GRP6_22_FUNC)
#define GPSR6_21	F_(SSI1_SCK,		GRP6_21_FUNC)
#define GPSR6_20	F_(AUDIO0_CLKOUT1,	GRP6_20_FUNC)
#define GPSR6_19	F_(AUDIO0_CLKOUT0,	GRP6_19_FUNC)
#define GPSR6_18	F_(SSI0_SD,		GRP6_18_FUNC)
#define GPSR6_17	F_(SSI0_WS,		GRP6_17_FUNC)
#define GPSR6_16	F_(SSI0_SCK,		GRP6_16_FUNC)
#define GPSR6_15	F_(MSIOF4_SS2_B,	GRP6_15_FUNC)
#define GPSR6_14	F_(MSIOF4_SS1_B,	GRP6_14_FUNC)
#define GPSR6_13	F_(MSIOF4_SYNC_B,	GRP6_13_FUNC)
#define GPSR6_12	F_(MSIOF4_RXD_B,	GRP6_12_FUNC)
#define GPSR6_11	F_(MSIOF4_TXD_B,	GRP6_11_FUNC)
#define GPSR6_10	F_(MSIOF4_SCK_B,	GRP6_10_FUNC)
#define GPSR6_9		F_(MSIOF7_SS2_A,	GRP6_9_FUNC)
#define GPSR6_8		F_(MSIOF7_SS1_A,	GRP6_8_FUNC)
#define GPSR6_7		F_(MSIOF7_SYNC_A,	GRP6_7_FUNC)
#define GPSR6_6		F_(MSIOF7_RXD_A,	GRP6_6_FUNC)
#define GPSR6_5		F_(MSIOF7_TXD_A,	GRP6_5_FUNC)
#define GPSR6_4		F_(MSIOF7_SCK_A,	GRP6_4_FUNC)
#define GPSR6_3		F_(RIF6_CLK,		GRP6_3_FUNC)
#define GPSR6_2		F_(RIF6_SYNC,		GRP6_2_FUNC)
#define GPSR6_1		F_(RIF6_D1,		GRP6_1_FUNC)
#define GPSR6_0		F_(RIF6_D0,		GRP6_0_FUNC)

/* GPSR7 */
#define GPSR7_30	F_(MSIOF6_SS2_B,	GRP7_30_FUNC)
#define GPSR7_29	F_(MSIOF6_SS1_B,	GRP7_29_FUNC)
#define GPSR7_28	F_(MSIOF6_SYNC_B,	GRP7_28_FUNC)
#define GPSR7_27	F_(MSIOF6_RXD_B,	GRP7_27_FUNC)
#define GPSR7_26	F_(MSIOF6_TXD_B,	GRP7_26_FUNC)
#define GPSR7_25	F_(MSIOF6_SCK_B,	GRP7_25_FUNC)
#define GPSR7_24	F_(MSIOF5_SS2,		GRP7_24_FUNC)
#define GPSR7_23	F_(MSIOF5_SS1,		GRP7_23_FUNC)
#define GPSR7_22	F_(MSIOF5_SYNC,		GRP7_22_FUNC)
#define GPSR7_21	F_(MSIOF5_RXD,		GRP7_21_FUNC)
#define GPSR7_20	F_(MSIOF5_TXD,		GRP7_20_FUNC)
#define GPSR7_19	F_(GP07_19,		GRP7_19_FUNC)
#define GPSR7_18	F_(GP07_18,		GRP7_18_FUNC)
#define GPSR7_17	F_(MSIOF5_SCK,		GRP7_17_FUNC)
#define GPSR7_16	F_(AUDIO_CLKC_A,	GRP7_16_FUNC)
#define GPSR7_15	F_(SSI6_SD,		GRP7_15_FUNC)
#define GPSR7_14	F_(SSI6_WS,		GRP7_14_FUNC)
#define GPSR7_13	F_(SSI6_SCK,		GRP7_13_FUNC)
#define GPSR7_12	F_(AUDIO_CLKB_A,	GRP7_12_FUNC)
#define GPSR7_11	F_(SSI5_SD,		GRP7_11_FUNC)
#define GPSR7_10	F_(SSI5_WS,		GRP7_10_FUNC)
#define GPSR7_9		F_(SSI5_SCK,		GRP7_9_FUNC)
#define GPSR7_8		F_(AUDIO_CLKA_A,	GRP7_8_FUNC)
#define GPSR7_7		F_(SSI4_SD,		GRP7_7_FUNC)
#define GPSR7_6		F_(SSI4_WS,		GRP7_6_FUNC)
#define GPSR7_5		F_(SSI4_SCK,		GRP7_5_FUNC)
#define GPSR7_4		F_(AUDIO1_CLKOUT3,	GRP7_4_FUNC)
#define GPSR7_3		F_(AUDIO1_CLKOUT2,	GRP7_3_FUNC)
#define GPSR7_2		F_(SSI3_SD,		GRP7_2_FUNC)
#define GPSR7_1		F_(SSI3_WS,		GRP7_1_FUNC)
#define GPSR7_0		F_(SSI3_SCK,		GRP7_0_FUNC)

/* GPSR8 */
#define GPSR8_31	F_(S3DA2,		GRP8_31_FUNC)
#define GPSR8_30	F_(S3CL2,		GRP8_30_FUNC)
#define GPSR8_29	F_(S3DA1,		GRP8_29_FUNC)
#define GPSR8_28	F_(S3CL1,		GRP8_28_FUNC)
#define GPSR8_27	F_(S3DA0,		GRP8_27_FUNC)
#define GPSR8_26	F_(S3CL0,		GRP8_26_FUNC)
#define GPSR8_15	F_(SDA8,		GRP8_15_FUNC)
#define GPSR8_14	F_(SCL8,		GRP8_14_FUNC)
#define GPSR8_13	F_(SDA7,		GRP8_13_FUNC)
#define GPSR8_12	F_(SCL7,		GRP8_12_FUNC)
#define GPSR8_11	F_(SDA6,		GRP8_11_FUNC)
#define GPSR8_10	F_(SCL6,		GRP8_10_FUNC)
#define GPSR8_9		F_(SDA5,		GRP8_9_FUNC)
#define GPSR8_8		F_(SCL5,		GRP8_8_FUNC)
#define GPSR8_7		F_(SDA4,		GRP8_7_FUNC)
#define GPSR8_6		F_(SCL4,		GRP8_6_FUNC)
#define GPSR8_5		F_(SDA3,		GRP8_5_FUNC)
#define GPSR8_4		F_(SCL3,		GRP8_4_FUNC)
#define GPSR8_3		F_(SDA2,		GRP8_3_FUNC)
#define GPSR8_2		F_(SCL2,		GRP8_2_FUNC)
#define GPSR8_1		F_(SDA1,		GRP8_1_FUNC)
#define GPSR8_0		F_(SCL1,		GRP8_0_FUNC)

/* GPSR9 */
#define GPSR9_16	F_(RSW3_MATCH,		GRP9_16_FUNC)
#define GPSR9_15	F_(RSW3_CAPTURE,	GRP9_15_FUNC)
#define GPSR9_14	F_(RSW3_PPS,		GRP9_14_FUNC)
#define GPSR9_13	F_(ETH10G0_PHYINT,	GRP9_13_FUNC)
#define GPSR9_12	F_(ETH10G0_LINK,	GRP9_12_FUNC)
#define GPSR9_11	F_(ETH10G0_MDC,		GRP9_11_FUNC)
#define GPSR9_10	F_(ETH10G0_MDIO,	GRP9_10_FUNC)
#define GPSR9_9		F_(ETH25G0_PHYINT,	GRP9_9_FUNC)
#define GPSR9_8		F_(ETH25G0_LINK,	GRP9_8_FUNC)
#define GPSR9_7		F_(ETH25G0_MDC,		GRP9_7_FUNC)
#define GPSR9_6		F_(ETH25G0_MDIO,	GRP9_6_FUNC)
#define GPSR9_5		F_(ETHES4_MATCH,	GRP9_5_FUNC)
#define GPSR9_4		F_(ETHES4_CAPTURE,	GRP9_4_FUNC)
#define GPSR9_3		F_(ETHES4_PPS,		GRP9_3_FUNC)
#define GPSR9_2		F_(ETHES0_MATCH,	GRP9_2_FUNC)
#define GPSR9_1		F_(ETHES0_CAPTURE,	GRP9_1_FUNC)
#define GPSR9_0		F_(ETHES0_PPS,		GRP9_0_FUNC)

/* GPSR10 */
#define GPSR10_13	F_(PCIE41_CLKREQ_N,	GRP10_13_FUNC)
#define GPSR10_12	F_(PCIE40_CLKREQ_N,	GRP10_12_FUNC)
#define GPSR10_11	F_(USB3_VBUS_VALID,	GRP10_11_FUNC)
#define GPSR10_10	F_(USB3_OVC,		GRP10_10_FUNC)
#define GPSR10_9	F_(USB3_PWEN,		GRP10_9_FUNC)
#define GPSR10_8	F_(USB2_VBUS_VALID,	GRP10_8_FUNC)
#define GPSR10_7	F_(USB2_OVC,		GRP10_7_FUNC)
#define GPSR10_6	F_(USB2_PWEN,		GRP10_6_FUNC)
#define GPSR10_5	F_(USB1_VBUS_VALID,	GRP10_5_FUNC)
#define GPSR10_4	F_(USB1_OVC,		GRP10_4_FUNC)
#define GPSR10_3	F_(USB1_PWEN,		GRP10_3_FUNC)
#define GPSR10_2	F_(USB0_VBUS_VALID,	GRP10_2_FUNC)
#define GPSR10_1	F_(USB0_OVC,		GRP10_1_FUNC)
#define GPSR10_0	F_(USB0_PWEN,		GRP10_0_FUNC)

/* Group0 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP0_27_FUNC	FM(DP2_HOTPLUG)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_26_FUNC	FM(DP1_HOTPLUG)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_25_FUNC	FM(DP0_HOTPLUG)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_24_FUNC	FM(MSIOF1_SS2_A)	FM(TAUD0O13)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_23_FUNC	FM(MSIOF1_SS1_A)	FM(TAUD0O12)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_22_FUNC	FM(MSIOF1_SYNC_A)	FM(TAUD0O11)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_21_FUNC	FM(MSIOF1_RXD_A)	FM(TAUD0O10)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_20_FUNC	FM(MSIOF1_TXD_A)	FM(TAUD0O9)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_19_FUNC	FM(MSIOF1_SCK_A)	FM(TAUD0O8)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_18_FUNC	FM(MSIOF0_SS2)		FM(TAUD0O7)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_17_FUNC	FM(MSIOF0_SS1)		FM(TAUD0O6)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_16_FUNC	FM(MSIOF0_SYNC)		FM(TAUD0O5)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_15_FUNC	FM(MSIOF0_RXD)		FM(TAUD0O4)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_14_FUNC	FM(MSIOF0_TXD)		FM(TAUD0O3)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_13_FUNC	FM(MSIOF0_SCK)		FM(TAUD0O2)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_12_FUNC	FM(RXDB_EXTFXR_A)	FM(CAN11TX)		FM(RLIN311TX)		FM(RTCA0OUT_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_11_FUNC	FM(FXR_TXENB_N_A)	FM(CAN11RX_INTP11)	FM(RLIN311RX_INTP27)	FM(EXTCLK0O_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_10_FUNC	FM(FXR_TXDB_A)		FM(CAN10TX)		FM(RLIN310TX)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_9_FUNC	FM(RXDA_EXTFXR_A)	FM(CAN10RX_INTP10)	FM(RLIN310RX_INTP26)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_8_FUNC	FM(FXR_TXENA_N_A)	FM(CAN9TX)		FM(RLIN39TX)		FM(TAUD1O15)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_7_FUNC	FM(FXR_TXDA_A)		FM(CAN9RX_INTP9)	FM(RLIN39RX_INTP25)	FM(TAUD1O14)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_6_FUNC	FM(CLK_EXTFXR_A)	FM(CAN8TX)		FM(RLIN38TX)		FM(TAUD1O13)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_5_FUNC	FM(FXR_CLKOUT2_A)	FM(CAN8RX_INTP8)	FM(RLIN38RX_INTP24)	FM(TAUD1O12)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_4_FUNC	FM(FXR_CLKOUT1_A)	FM(CAN7TX)		FM(RLIN315TX_A)		FM(TAUD1O11)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_3_FUNC	FM(STPWT_EXTFXR_A)	FM(CAN7RX_INTP7)	FM(RLIN315RX_INTP31_A)	FM(TAUD1O10)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_2_FUNC	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_1_FUNC	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP0_0_FUNC	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group1 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP1_21_FUNC	FM(RLIN33TX)		FM(TAUJ1I3_TAUJ1O3)	FM(CAN15TX_A)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_20_FUNC	FM(RLIN33RX_INTP19)	FM(TAUJ1I2_TAUJ1O2)	FM(CAN15RX_INTP15_A)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_19_FUNC	FM(RLIN32TX)		FM(TAUJ1I1_TAUJ1O1)	FM(CAN14TX_A)		FM(NMI1_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_18_FUNC	FM(RLIN32RX_INTP18)	FM(TAUJ1I0_TAUJ1O0)	FM(CAN14RX_INTP14_A)	FM(INTP34_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_17_FUNC	FM(RLIN31TX)		FM(TAUJ3I3_TAUJ3O3)	FM(CAN13TX_A)		FM(INTP33_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_16_FUNC	FM(RLIN31RX_INTP17)	FM(TAUJ3I2_TAUJ3O2)	FM(CAN13RX_INTP13_A)	FM(INTP32_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_15_FUNC	FM(RLIN30TX)		FM(TAUJ3I1_TAUJ3O1)	FM(CAN12TX_A)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_14_FUNC	FM(RLIN30RX_INTP16)	FM(TAUJ3I0_TAUJ3O0)	FM(CAN12RX_INTP12_A)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_13_FUNC	FM(CAN6TX)		FM(RLIN314TX_A)		FM(TAUD1O9)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_12_FUNC	FM(CAN6RX_INTP6)	FM(RLIN314RX_INTP30_A)	FM(TAUD1O8)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_11_FUNC	FM(CAN5TX)		FM(RLIN313TX_A)		FM(MSIOF3_SS2)		FM(RXDB_EXTFXR_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_10_FUNC	FM(CAN5RX_INTP5)	FM(RLIN313RX_INTP29_A)	FM(MSIOF3_SS1)		FM(FXR_TXENB_N_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_9_FUNC	FM(CAN4TX)		FM(RLIN312TX_A)		FM(MSIOF3_SYNC)		FM(FXR_TXDB_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_8_FUNC	FM(CAN4RX_INTP4)	FM(RLIN312RX_INTP28_A)	FM(MSIOF3_RXD)		FM(RXDA_EXTFXR_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_7_FUNC	FM(CAN3TX)		FM(RLIN37TX_A)		FM(MSIOF3_TXD)		FM(FXR_TXENA_N_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_6_FUNC	FM(CAN3RX_INTP3)	FM(RLIN37RX_INTP23_A)	FM(MSIOF3_SCK)		FM(FXR_TXDA_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_5_FUNC	FM(CAN2TX)		FM(RLIN36TX_A)		FM(MSIOF2_SS2)		FM(CLK_EXTFXR_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_4_FUNC	FM(CAN2RX_INTP2)	FM(RLIN36RX_INTP22_A)	FM(MSIOF2_SS1)		FM(FXR_CLKOUT2_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_3_FUNC	FM(CAN1TX)		FM(RLIN35TX_A)		FM(MSIOF2_SYNC)		FM(FXR_CLKOUT1_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_2_FUNC	FM(CAN1RX_INTP1)	FM(RLIN35RX_INTP21_A)	FM(MSIOF2_RXD)		FM(STPWT_EXTFXR_B)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_1_FUNC	FM(CAN0TX)		FM(RLIN34TX_A)		FM(MSIOF2_TXD)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP1_0_FUNC	FM(CAN0RX_INTP0)	FM(RLIN34RX_INTP20_A)	FM(MSIOF2_SCK)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group2 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP2_28_FUNC	FM(INTP34_B)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_27_FUNC	FM(TAUD1O3)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_26_FUNC	FM(TAUD1O2)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_25_FUNC	FM(TAUD1O1)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_24_FUNC	FM(TAUD1O0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_23_FUNC	FM(EXTCLK0O_B)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_22_FUNC	FM(AVS1)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_21_FUNC	FM(AVS0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_20_FUNC	FM(SDA0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_19_FUNC	FM(SCL0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_18_FUNC	FM(INTP33_B)		FM(TAUD0O1)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_17_FUNC	FM(INTP32_B)		FM(TAUD0O0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_16_FUNC	FM(CAN_CLK)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_15_FUNC	FM(CAN15TX_B)		FM(RLIN315TX_B)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_14_FUNC	FM(CAN15RX_B_INTP15)	FM(RLIN315RX_INTP31_B)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_13_FUNC	FM(CAN14TX_B)		FM(RLIN314TX_B)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_12_FUNC	FM(CAN14RX_B_INTP14)	FM(RLIN314RX_INTP30_B)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_11_FUNC	FM(CAN13TX_B)		FM(RLIN313TX)		F_(0, 0)		FM(CANXL1_TX)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_10_FUNC	FM(CAN13RX_B_INTP13)	FM(RLIN313RX_INTP29_B)	F_(0, 0)		FM(CANXL1_RX)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_9_FUNC	FM(CAN12TX_B)		FM(RLIN312TX)		FM(TAUD1O7)		FM(CANXL0_TX)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_8_FUNC	FM(CAN12RX_B_INTP12)	FM(RLIN312RX_INTP28_B)	FM(TAUD1O6)		FM(CANXL0_RX)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_7_FUNC	FM(RLIN37TX_B)		FM(RTCA0OUT_B)		FM(TAUD1O5)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_6_FUNC	FM(RLIN37RX_B_INTP23)	F_(0, 0)		FM(TAUD1O4)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_5_FUNC	FM(RLIN36TX_B)		FM(MSIOF1_SS2_B)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_4_FUNC	FM(RLIN36RX_B_INTP22)	FM(MSIOF1_SS1_B)	F_(0, 0)		FM(CTIACK)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_3_FUNC	FM(RLIN35TX_B)		FM(MSIOF1_SYN_B)	F_(0, 0)		FM(CTIREQ)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_2_FUNC	FM(RLIN35RX_B_INTP21)	FM(MSIOF1_RXD_B)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_1_FUNC	FM(RLIN34TX_B)		FM(MSIOF1_TXD_B)	FM(TAUD0O15)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP2_0_FUNC	FM(RLIN34RX_B_INTP20)	FM(MSIOF1_SCK_B)	FM(TAUD0O14)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)


/* Group3 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP3_16_FUNC	FM(ERRORIN0_N)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_15_FUNC	FM(ERROROUT_N)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_14_FUNC	FM(QSPI1_SSL)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_13_FUNC	FM(QSPI1_IO3)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_12_FUNC	FM(QSPI1_IO2)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_11_FUNC	FM(QSPI1_MISO_IO1)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_10_FUNC	FM(QSPI1_MOSI_IO0)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_9_FUNC	FM(QSPI1_SPCLK)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_8_FUNC	FM(RPC_INT_N)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_7_FUNC	FM(RPC_WP_N)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_6_FUNC	FM(RPC_RESET_N)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_5_FUNC	FM(QSPI0_SSL)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_4_FUNC	FM(QSPI0_IO3)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_3_FUNC	FM(QSPI0_IO2)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_2_FUNC	FM(QSPI0_MISO_IO1)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_1_FUNC	FM(QSPI0_MOSI_IO0)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP3_0_FUNC	FM(QSPI0_SPCLK)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group4 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP4_15_FUNC	FM(PCIE61_CLKREQ_N)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_14_FUNC	FM(PCIE60_CLKREQ_N)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_13_FUNC	FM(ERRORIN1_N)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_12_FUNC	FM(SD0_CD)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_11_FUNC	FM(SD0_WP)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_10_FUNC	FM(MMC0_DS)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_9_FUNC	FM(MMC0_D7)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_8_FUNC	FM(MMC0_D6)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_7_FUNC	FM(MMC0_D5)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_6_FUNC	FM(MMC0_D4)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_5_FUNC	FM(MMC0_SD_D3)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_4_FUNC	FM(MMC0_SD_D2)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_3_FUNC	FM(MMC0_SD_D1)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_2_FUNC	FM(MMC0_SD_D0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_1_FUNC	FM(MMC0_SD_CMD)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP4_0_FUNC	FM(MMC0_SD_CLK)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group5 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP5_22_FUNC	FM(TPU0TO3)		FM(SSI9_WS)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_21_FUNC	FM(TPU0TO2)		FM(SSI9_SCK)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_20_FUNC	FM(TPU0TO1)		FM(PWM5)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_19_FUNC	FM(TPU0TO0)		FM(PWM4)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_18_FUNC	FM(TCLK4)		FM(PWM3)		FM(SSI19_SD)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_17_FUNC	FM(TCLK3)		FM(PWM2)		FM(SSI19_WS)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_16_FUNC	FM(TCLK2)		FM(PWM1)		FM(SSI19_SCK)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_15_FUNC	FM(TCLK1)		FM(PWM0_A)		FM(SSI18_SD)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_14_FUNC	FM(IRQ3_A)		F_(0, 0)		F_(0, 0)		FM(RIF7_D1)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_13_FUNC	FM(IRQ2_A)		FM(SSI17_SD)		F_(0, 0)		FM(RIF7_D0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_12_FUNC	FM(IRQ1_A)		FM(SSI17_WS)		F_(0, 0)		FM(RIF7_SYNC)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_11_FUNC	FM(IRQ0_A)		FM(SSI17_SCK)		F_(0, 0)		FM(RIF7_CLK)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_10_FUNC	FM(HSCK1)		FM(SCK1)		FM(SSI13_SCK)		FM(RIF0_CLK_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_9_FUNC	FM(HCTS1_N)		FM(CTS1_N)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_8_FUNC	FM(HRTS1_N)		FM(RTS1_N)		FM(RIF0_SYNC_B)		FM(SSI16_SD)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_7_FUNC	FM(HRX1)		FM(RX1)			F_(0, 0)		FM(SSI16_WS)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_6_FUNC	FM(HTX1)		FM(TX1)			F_(0, 0)		FM(SSI16_SCK)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_5_FUNC	FM(SCIF_CLK)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_4_FUNC	FM(HSCK0)		FM(SCK0)		F_(0, 0)		FM(SSI15_SD)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_3_FUNC	FM(HCTS0_N)		FM(CTS0_N)		FM(IRQ1_B)		FM(SSI15_WS)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_2_FUNC	FM(HRTS0_N)		FM(RTS0_N)		FM(IRQ0_B)		FM(SSI15_SCK)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_1_FUNC	FM(HRX0)		FM(RX0)			FM(SSI13_SD)		FM(RIF0_D1_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP5_0_FUNC	FM(HTX0)		FM(TX0)			FM(SSI13_WS)		FM(RIF0_D0_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group6 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3				4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP6_30_FUNC	FM(AUDIO1_CLKOUT1)	FM(MSIOF7_RXD_B)	FM(RIF5_CLK)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_29_FUNC	FM(AUDIO1_CLKOUT0)	FM(MSIOF7_TXD_B)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_28_FUNC	FM(SSI2_SD)		FM(MSIOF7_SCK_B)	FM(RIF5_SYNC)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_27_FUNC	FM(SSI2_WS)		F_(0, 0)		FM(RIF1_D1_A)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_26_FUNC	FM(SSI2_SCK)		F_(0, 0)		FM(RIF1_D0_A)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_25_FUNC	FM(AUDIO0_CLKOUT3)	F_(0, 0)		FM(RIF1_CLK_A)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_24_FUNC	FM(AUDIO0_CLKOUT2)	F_(0, 0)		FM(RIF2_D1)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_23_FUNC	FM(SSI1_SD)		F_(0, 0)		FM(HCTS3_N)		FM(CTS3_N)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_22_FUNC	FM(SSI1_WS)		F_(0, 0)		FM(HRTS3_N)		FM(RTS3_N)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_21_FUNC	FM(SSI1_SCK)		FM(MSIOF4_SS2_A)	FM(HSCK3)		FM(SCK3)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_20_FUNC	FM(AUDIO0_CLKOUT1)	FM(MSIOF4_SS1_A)	FM(RIF2_D0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_19_FUNC	FM(AUDIO0_CLKOUT0)	FM(MSIOF4_SYNC_A)	FM(RIF2_SYNC)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_18_FUNC	FM(SSI0_SD)		FM(MSIOF4_RXD_A)	FM(HRX3)		FM(RX3)			F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_17_FUNC	FM(SSI0_WS)		FM(MSIOF4_TXD_A)	FM(HTX3)		FM(TX3)			F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_16_FUNC	FM(SSI0_SCK)		FM(MSIOF4_SCK_A)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_15_FUNC	FM(MSIOF4_SS2_B)	FM(SSI14_SD)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_14_FUNC	FM(MSIOF4_SS1_B)	FM(SSI12_SD)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_13_FUNC	FM(MSIOF4_SYNC_B)	FM(SSI12_WS)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_12_FUNC	FM(MSIOF4_RXD_B)	F_(0, 0)		FM(AUDIO_CLKC_B)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_11_FUNC	FM(MSIOF4_TXD_B)	FM(SSI12_SCK)		F_(0, 0)		FM(RIF1_SYNC_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_10_FUNC	FM(MSIOF4_SCK_B)	F_(0, 0)		FM(AUDIO_CLKB_B)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_9_FUNC	FM(MSIOF7_SS2_A)	FM(SSI14_WS)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_8_FUNC	FM(MSIOF7_SS1_A)	FM(SSI14_SCK)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_7_FUNC	FM(MSIOF7_SYNC_A)	FM(RIF1_D1_B)		FM(SSI11_SD)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_6_FUNC	FM(MSIOF7_RXD_A)	FM(RIF1_D0_B)		FM(SSI11_WS)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_5_FUNC	FM(MSIOF7_TXD_A)	FM(RIF1_SYNC_B)		FM(SSI11_SCK)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_4_FUNC	FM(MSIOF7_SCK_A)	FM(RIF1_CLK_B)		FM(AUDIO_CLKA_B)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_3_FUNC	FM(RIF6_CLK)		FM(SSI10_SD)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_2_FUNC	FM(RIF6_SYNC)		FM(SSI10_WS)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_1_FUNC	FM(RIF6_D1)		FM(SSI10_SCK)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP6_0_FUNC	FM(RIF6_D0)		FM(SSI9_SD)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group7 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP7_30_FUNC	FM(MSIOF6_SS2_B)	FM(HRX2_B)		FM(RX4_B)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_29_FUNC	FM(MSIOF6_SS1_B)	FM(SSI7_SD)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_28_FUNC	FM(MSIOF6_SYNC_B)	FM(SSI7_WS)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_27_FUNC	FM(MSIOF6_RXD_B)	FM(SSI7_SCK)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_26_FUNC	FM(MSIOF6_TXD_B)	FM(HTX2_B)		FM(TX4_B)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_25_FUNC	FM(MSIOF6_SCK_B)	FM(SSI8_SD)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_24_FUNC	FM(MSIOF5_SS2)		FM(HCTS2_N_B)		FM(CTS4_N_B)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_23_FUNC	FM(MSIOF5_SS1)		FM(RIF0_SYNC_A)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_22_FUNC	FM(MSIOF5_SYNC)		FM(HRTS2_N_B)		FM(RTS4_N_B)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_21_FUNC	FM(MSIOF5_RXD)		FM(RIF0_D1_A)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_20_FUNC	FM(MSIOF5_TXD)		FM(HSCK2_B)		FM(SCK4_B)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_19_FUNC	F_(0, 0)		FM(MSIOF6_SS2_A)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_18_FUNC	F_(0, 0)		FM(MSIOF6_SS1_A)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_17_FUNC	FM(MSIOF5_SCK)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_16_FUNC	FM(AUDIO_CLKC_A)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_15_FUNC	FM(SSI6_SD)		FM(MSIOF6_RXD_A)	FM(RIF4_D1)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_14_FUNC	FM(SSI6_WS)		FM(MSIOF6_TXD_A)	FM(RIF4_D0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_13_FUNC	FM(SSI6_SCK)		FM(MSIOF6_SCK_A)	FM(RIF4_SYNC)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_12_FUNC	FM(AUDIO_CLKB_A)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_11_FUNC	FM(SSI5_SD)		FM(MSIOF6_SYNC_A)	FM(RIF4_CLK)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_10_FUNC	FM(SSI5_WS)		FM(RIF3_SYNC)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_9_FUNC	FM(SSI5_SCK)		FM(RIF3_CLK)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_8_FUNC	FM(AUDIO_CLKA_A)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_7_FUNC	FM(SSI4_SD)		FM(RIF3_D1)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_6_FUNC	FM(SSI4_WS)		FM(RIF3_D0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_5_FUNC	FM(SSI4_SCK)		FM(RIF2_CLK)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_4_FUNC	FM(AUDIO1_CLKOUT3)	FM(RIF0_D0_A)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_3_FUNC	FM(AUDIO1_CLKOUT2)	FM(RIF0_CLK_A)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_2_FUNC	FM(SSI3_SD)		FM(MSIOF7_SS2_B)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_1_FUNC	FM(SSI3_WS)		FM(MSIOF7_SS1_B)	FM(RIF5_D1)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP7_0_FUNC	FM(SSI3_SCK)		FM(MSIOF7_SYNC_B)	FM(RIF5_D0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group8 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP8_31_FUNC	FM(S3DA2)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_30_FUNC	FM(S3CL2)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_29_FUNC	FM(S3DA1)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_28_FUNC	FM(S3CL1)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_27_FUNC	FM(S3DA0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_26_FUNC	FM(S3CL0)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_15_FUNC	FM(SDA8)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_14_FUNC	FM(SCL8)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_13_FUNC	FM(SDA7)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_12_FUNC	FM(SCL7)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_11_FUNC	FM(SDA6)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_10_FUNC	FM(SCL6)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_9_FUNC	FM(SDA5)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_8_FUNC	FM(SCL5)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_7_FUNC	FM(SDA4)		FM(HCTS2_N_A)		FM(CTS4_N_A)		FM(PWM7_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_6_FUNC	FM(SCL4)		FM(HRTS2_N_A)		FM(RTS4_N_A)		FM(PWM9_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_5_FUNC	FM(SDA3)		FM(HRX2_A)		FM(RX4_A)		FM(PWM8_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_4_FUNC	FM(SCL3)		FM(HTX2_A)		FM(TX4_A)		FM(PWM6_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_3_FUNC	FM(SDA2)		FM(HSCK2_A)		FM(SCK4_A)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_2_FUNC	FM(SCL2)		FM(PWM0_B)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_1_FUNC	FM(SDA1)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP8_0_FUNC	FM(SCL1)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group9 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP9_16_FUNC	FM(RSW3_MATCH)		F_(0, 0)		F_(0, 0)		FM(PWM9_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_15_FUNC	FM(RSW3_CAPTURE)	F_(0, 0)		F_(0, 0)		FM(PWM8_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_14_FUNC	FM(RSW3_PPS)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_13_FUNC	FM(ETH10G0_PHYINT)	FM(ETH10G1_PHYINT)	F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_12_FUNC	FM(ETH10G0_LINK)	FM(ETH10G1_LINK)	F_(0, 0)		FM(PWM7_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_11_FUNC	FM(ETH10G0_MDC)		FM(ETH10G1_MDC)		F_(0, 0)		FM(IRQ3_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_10_FUNC	FM(ETH10G0_MDIO)	FM(ETH10G1_MDIO)	F_(0, 0)		FM(IRQ2_B)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_9_FUNC	FM(ETH25G0_PHYINT)	FM(ETH25G1_PHYINT)	FM(ETH25G2_PHYINT)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_8_FUNC	FM(ETH25G0_LINK)	FM(ETH25G1_LINK)	FM(ETH25G2_LINK)	FM(PWM6_A)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_7_FUNC	FM(ETH25G0_MDC)		FM(ETH25G1_MDC)		FM(ETH25G2_MDC)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_6_FUNC	FM(ETH25G0_MDIO)	FM(ETH25G1_MDIO)	FM(ETH25G2_MDIO)	F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_5_FUNC	FM(ETHES4_MATCH)	FM(ETHES5_MATCH)	FM(ETHES6_MATCH)	FM(ETHES7_MATCH)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_4_FUNC	FM(ETHES4_CAPTURE)	FM(ETHES5_CAPTURE)	FM(ETHES6_CAPTURE)	FM(ETHES7_CAPTURE)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_3_FUNC	FM(ETHES4_PPS)		FM(ETHES5_PPS)		FM(ETHES6_PPS)		FM(ETHES7_PPS)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_2_FUNC	FM(ETHES0_MATCH)	FM(ETHES1_MATCH)	FM(ETHES2_MATCH)	FM(ETHES3_MATCH)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_1_FUNC	FM(ETHES0_CAPTURE)	FM(ETHES1_CAPTURE)	FM(ETHES2_CAPTURE)	FM(ETHES3_CAPTURE)	F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP9_0_FUNC	FM(ETHES0_PPS)		FM(ETHES1_PPS)		FM(ETHES2_PPS)		FM(ETHES3_PPS)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

/* Group10 Functions */	/* 0 */			/* 1 */			/* 2 */			/* 3			4	 5	  6	   7	    8	     9	      A	       B	C	 D	  E	   F */
#define GRP10_13_FUNC	FM(PCIE41_CLKREQ_N)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_12_FUNC	FM(PCIE40_CLKREQ_N)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_11_FUNC	FM(USB3_VBUS_VALID)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_10_FUNC	FM(USB3_OVC)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_9_FUNC	FM(USB3_PWEN)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_8_FUNC	FM(USB2_VBUS_VALID)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_7_FUNC	FM(USB2_OVC)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_6_FUNC	FM(USB2_PWEN)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_5_FUNC	FM(USB1_VBUS_VALID)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_4_FUNC	FM(USB1_OVC)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_3_FUNC	FM(USB1_PWEN)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_2_FUNC	FM(USB0_VBUS_VALID)	F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_1_FUNC	FM(USB0_OVC)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)
#define GRP10_0_FUNC	FM(USB0_PWEN)		F_(0, 0)		F_(0, 0)		F_(0, 0)		F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0) F_(0, 0)

#define PINMUX_GPSR	\
/* GPSR0 */	/* GPSR1 */	/* GPSR2 */	/* GPSR3 */	/* GPSR4 */	/* GPSR5 */	/* GPSR6 */	/* GPSR7 */	/* GPSR8 */	/* GPSR9 */	/* GPSR10 */	\
																GPSR8_31					\
												GPSR6_30	GPSR7_30	GPSR8_30					\
												GPSR6_29	GPSR7_29	GPSR8_29					\
				GPSR2_28							GPSR6_28	GPSR7_28	GPSR8_28					\
GPSR0_27			GPSR2_27							GPSR6_27	GPSR7_27	GPSR8_27					\
GPSR0_26			GPSR2_26							GPSR6_26	GPSR7_26	GPSR8_26					\
GPSR0_25			GPSR2_25							GPSR6_25	GPSR7_25							\
GPSR0_24			GPSR2_24							GPSR6_24	GPSR7_24							\
GPSR0_23			GPSR2_23							GPSR6_23	GPSR7_23							\
GPSR0_22			GPSR2_22					GPSR5_22	GPSR6_22	GPSR7_22							\
GPSR0_21	GPSR1_21	GPSR2_21					GPSR5_21	GPSR6_21	GPSR7_21							\
GPSR0_20	GPSR1_20	GPSR2_20					GPSR5_20	GPSR6_20	GPSR7_20							\
GPSR0_19	GPSR1_19	GPSR2_19					GPSR5_19	GPSR6_19	GPSR7_19							\
GPSR0_18	GPSR1_18	GPSR2_18					GPSR5_18	GPSR6_18	GPSR7_18							\
GPSR0_17	GPSR1_17	GPSR2_17					GPSR5_17	GPSR6_17	GPSR7_17							\
GPSR0_16	GPSR1_16	GPSR2_16	GPSR3_16			GPSR5_16	GPSR6_16	GPSR7_16			GPSR9_16			\
GPSR0_15	GPSR1_15	GPSR2_15	GPSR3_15	GPSR4_15	GPSR5_15	GPSR6_15	GPSR7_15	GPSR8_15	GPSR9_15			\
GPSR0_14	GPSR1_14	GPSR2_14	GPSR3_14	GPSR4_14	GPSR5_14	GPSR6_14	GPSR7_14	GPSR8_14	GPSR9_14			\
GPSR0_13	GPSR1_13	GPSR2_13	GPSR3_13	GPSR4_13	GPSR5_13	GPSR6_13	GPSR7_13	GPSR8_13	GPSR9_13	GPSR4_13	\
GPSR0_12	GPSR1_12	GPSR2_12	GPSR3_12	GPSR4_12	GPSR5_12	GPSR6_12	GPSR7_12	GPSR8_12	GPSR9_12	GPSR4_12	\
GPSR0_11	GPSR1_11	GPSR2_11	GPSR3_11	GPSR4_11	GPSR5_11	GPSR6_11	GPSR7_11	GPSR8_11	GPSR9_11	GPSR4_11	\
GPSR0_10	GPSR1_10	GPSR2_10	GPSR3_10	GPSR4_10	GPSR5_10	GPSR6_10	GPSR7_10	GPSR8_10	GPSR9_10	GPSR4_10	\
GPSR0_9		GPSR1_9		GPSR2_9		GPSR3_9		GPSR4_9		GPSR5_9		GPSR6_9		GPSR7_9		GPSR8_9		GPSR9_9		GPSR4_9		\
GPSR0_8		GPSR1_8		GPSR2_8		GPSR3_8		GPSR4_8		GPSR5_8		GPSR6_8		GPSR7_8		GPSR8_8		GPSR9_8		GPSR4_8		\
GPSR0_7		GPSR1_7		GPSR2_7		GPSR3_7		GPSR4_7		GPSR5_7		GPSR6_7		GPSR7_7		GPSR8_7		GPSR9_7		GPSR4_7		\
GPSR0_6		GPSR1_6		GPSR2_6		GPSR3_6		GPSR4_6		GPSR5_6		GPSR6_6		GPSR7_6		GPSR8_6		GPSR9_6		GPSR4_6		\
GPSR0_5		GPSR1_5		GPSR2_5		GPSR3_5		GPSR4_5		GPSR5_5		GPSR6_5		GPSR7_5		GPSR8_5		GPSR9_5		GPSR4_5		\
GPSR0_4		GPSR1_4		GPSR2_4		GPSR3_4		GPSR4_4		GPSR5_4		GPSR6_4		GPSR7_4		GPSR8_4		GPSR9_4		GPSR4_4		\
GPSR0_3		GPSR1_3		GPSR2_3		GPSR3_3		GPSR4_3		GPSR5_3		GPSR6_3		GPSR7_3		GPSR8_3		GPSR9_3		GPSR4_3		\
GPSR0_2		GPSR1_2		GPSR2_2		GPSR3_2		GPSR4_2		GPSR5_2		GPSR6_2		GPSR7_2		GPSR8_2		GPSR9_2		GPSR4_2		\
GPSR0_1		GPSR1_1		GPSR2_1		GPSR3_1		GPSR4_1		GPSR5_1		GPSR6_1		GPSR7_1		GPSR8_1		GPSR9_1		GPSR4_1		\
GPSR0_0		GPSR1_0		GPSR2_0		GPSR3_0		GPSR4_0		GPSR5_0		GPSR6_0		GPSR7_0		GPSR8_0		GPSR9_0		GPSR4_0

/* GRP0_FUNC */
#define GRP0_FUNC 			\
FM(GRP0_27_FUNC)	GRP0_27_FUNC	\
FM(GRP0_26_FUNC)	GRP0_26_FUNC	\
FM(GRP0_25_FUNC)	GRP0_25_FUNC	\
FM(GRP0_24_FUNC)	GRP0_24_FUNC	\
FM(GRP0_23_FUNC)	GRP0_23_FUNC	\
FM(GRP0_22_FUNC)	GRP0_22_FUNC	\
FM(GRP0_21_FUNC)	GRP0_21_FUNC	\
FM(GRP0_20_FUNC)	GRP0_20_FUNC	\
FM(GRP0_19_FUNC)	GRP0_19_FUNC	\
FM(GRP0_18_FUNC)	GRP0_18_FUNC	\
FM(GRP0_17_FUNC)	GRP0_17_FUNC	\
FM(GRP0_16_FUNC)	GRP0_16_FUNC	\
FM(GRP0_15_FUNC)	GRP0_15_FUNC	\
FM(GRP0_14_FUNC)	GRP0_14_FUNC	\
FM(GRP0_13_FUNC)	GRP0_13_FUNC	\
FM(GRP0_12_FUNC)	GRP0_12_FUNC	\
FM(GRP0_11_FUNC)	GRP0_11_FUNC	\
FM(GRP0_10_FUNC)	GRP0_10_FUNC	\
FM(GRP0_9_FUNC)		GRP0_9_FUNC	\
FM(GRP0_8_FUNC)		GRP0_8_FUNC	\
FM(GRP0_7_FUNC)		GRP0_7_FUNC	\
FM(GRP0_6_FUNC)		GRP0_6_FUNC	\
FM(GRP0_5_FUNC)		GRP0_5_FUNC	\
FM(GRP0_4_FUNC)		GRP0_4_FUNC	\
FM(GRP0_3_FUNC)		GRP0_3_FUNC	\
FM(GRP0_2_FUNC)		GRP0_2_FUNC	\
FM(GRP0_1_FUNC)		GRP0_1_FUNC	\
FM(GRP0_0_FUNC)		GRP0_0_FUNC

/* GRP1_FUNC */
#define GRP1_FUNC 			\
FM(GRP1_21_FUNC)	GRP1_21_FUNC	\
FM(GRP1_20_FUNC)	GRP1_20_FUNC	\
FM(GRP1_19_FUNC)	GRP1_19_FUNC	\
FM(GRP1_18_FUNC)	GRP1_18_FUNC	\
FM(GRP1_17_FUNC)	GRP1_17_FUNC	\
FM(GRP1_16_FUNC)	GRP1_16_FUNC	\
FM(GRP1_15_FUNC)	GRP1_15_FUNC	\
FM(GRP1_14_FUNC)	GRP1_14_FUNC	\
FM(GRP1_13_FUNC)	GRP1_13_FUNC	\
FM(GRP1_12_FUNC)	GRP1_12_FUNC	\
FM(GRP1_11_FUNC)	GRP1_11_FUNC	\
FM(GRP1_10_FUNC)	GRP1_10_FUNC	\
FM(GRP1_9_FUNC)		GRP1_9_FUNC	\
FM(GRP1_8_FUNC)		GRP1_8_FUNC	\
FM(GRP1_7_FUNC)		GRP1_7_FUNC	\
FM(GRP1_6_FUNC)		GRP1_6_FUNC	\
FM(GRP1_5_FUNC)		GRP1_5_FUNC	\
FM(GRP1_4_FUNC)		GRP1_4_FUNC	\
FM(GRP1_3_FUNC)		GRP1_3_FUNC	\
FM(GRP1_2_FUNC)		GRP1_2_FUNC	\
FM(GRP1_1_FUNC)		GRP1_1_FUNC	\
FM(GRP1_0_FUNC)		GRP1_0_FUNC

/* GRP2_FUNC */
#define GRP2_FUNC 			\
FM(GRP2_28_FUNC)	GRP2_28_FUNC	\
FM(GRP2_27_FUNC)	GRP2_27_FUNC	\
FM(GRP2_26_FUNC)	GRP2_26_FUNC	\
FM(GRP2_25_FUNC)	GRP2_25_FUNC	\
FM(GRP2_24_FUNC)	GRP2_24_FUNC	\
FM(GRP2_23_FUNC)	GRP2_23_FUNC	\
FM(GRP2_22_FUNC)	GRP2_22_FUNC	\
FM(GRP2_21_FUNC)	GRP2_21_FUNC	\
FM(GRP2_20_FUNC)	GRP2_20_FUNC	\
FM(GRP2_19_FUNC)	GRP2_19_FUNC	\
FM(GRP2_18_FUNC)	GRP2_18_FUNC	\
FM(GRP2_17_FUNC)	GRP2_17_FUNC	\
FM(GRP2_16_FUNC)	GRP2_16_FUNC	\
FM(GRP2_15_FUNC)	GRP2_15_FUNC	\
FM(GRP2_14_FUNC)	GRP2_14_FUNC	\
FM(GRP2_13_FUNC)	GRP2_13_FUNC	\
FM(GRP2_12_FUNC)	GRP2_12_FUNC	\
FM(GRP2_11_FUNC)	GRP2_11_FUNC	\
FM(GRP2_10_FUNC)	GRP2_10_FUNC	\
FM(GRP2_9_FUNC)		GRP2_9_FUNC	\
FM(GRP2_8_FUNC)		GRP2_8_FUNC	\
FM(GRP2_7_FUNC)		GRP2_7_FUNC	\
FM(GRP2_6_FUNC)		GRP2_6_FUNC	\
FM(GRP2_5_FUNC)		GRP2_5_FUNC	\
FM(GRP2_4_FUNC)		GRP2_4_FUNC	\
FM(GRP2_3_FUNC)		GRP2_3_FUNC	\
FM(GRP2_2_FUNC)		GRP2_2_FUNC	\
FM(GRP2_1_FUNC)		GRP2_1_FUNC	\
FM(GRP2_0_FUNC)		GRP2_0_FUNC

/* GRP3_FUNC */
#define GRP3_FUNC 			\
FM(GRP3_16_FUNC)	GRP3_16_FUNC	\
FM(GRP3_15_FUNC)	GRP3_15_FUNC	\
FM(GRP3_14_FUNC)	GRP3_14_FUNC	\
FM(GRP3_13_FUNC)	GRP3_13_FUNC	\
FM(GRP3_12_FUNC)	GRP3_12_FUNC	\
FM(GRP3_11_FUNC)	GRP3_11_FUNC	\
FM(GRP3_10_FUNC)	GRP3_10_FUNC	\
FM(GRP3_9_FUNC)		GRP3_9_FUNC	\
FM(GRP3_8_FUNC)		GRP3_8_FUNC	\
FM(GRP3_7_FUNC)		GRP3_7_FUNC	\
FM(GRP3_6_FUNC)		GRP3_6_FUNC	\
FM(GRP3_5_FUNC)		GRP3_5_FUNC	\
FM(GRP3_4_FUNC)		GRP3_4_FUNC	\
FM(GRP3_3_FUNC)		GRP3_3_FUNC	\
FM(GRP3_2_FUNC)		GRP3_2_FUNC	\
FM(GRP3_1_FUNC)		GRP3_1_FUNC	\
FM(GRP3_0_FUNC)		GRP3_0_FUNC

/* GRP4_FUNC */
#define GRP4_FUNC 			\
FM(GRP4_15_FUNC)	GRP4_15_FUNC	\
FM(GRP4_14_FUNC)	GRP4_14_FUNC	\
FM(GRP4_13_FUNC)	GRP4_13_FUNC	\
FM(GRP4_12_FUNC)	GRP4_12_FUNC	\
FM(GRP4_11_FUNC)	GRP4_11_FUNC	\
FM(GRP4_10_FUNC)	GRP4_10_FUNC	\
FM(GRP4_9_FUNC)		GRP4_9_FUNC	\
FM(GRP4_8_FUNC)		GRP4_8_FUNC	\
FM(GRP4_7_FUNC)		GRP4_7_FUNC	\
FM(GRP4_6_FUNC)		GRP4_6_FUNC	\
FM(GRP4_5_FUNC)		GRP4_5_FUNC	\
FM(GRP4_4_FUNC)		GRP4_4_FUNC	\
FM(GRP4_3_FUNC)		GRP4_3_FUNC	\
FM(GRP4_2_FUNC)		GRP4_2_FUNC	\
FM(GRP4_1_FUNC)		GRP4_1_FUNC	\
FM(GRP4_0_FUNC)		GRP4_0_FUNC

/* GRP5_FUNC */
#define GRP5_FUNC 			\
FM(GRP5_22_FUNC)	GRP5_22_FUNC	\
FM(GRP5_21_FUNC)	GRP5_21_FUNC	\
FM(GRP5_20_FUNC)	GRP5_20_FUNC	\
FM(GRP5_19_FUNC)	GRP5_19_FUNC	\
FM(GRP5_18_FUNC)	GRP5_18_FUNC	\
FM(GRP5_17_FUNC)	GRP5_17_FUNC	\
FM(GRP5_16_FUNC)	GRP5_16_FUNC	\
FM(GRP5_15_FUNC)	GRP5_15_FUNC	\
FM(GRP5_14_FUNC)	GRP5_14_FUNC	\
FM(GRP5_13_FUNC)	GRP5_13_FUNC	\
FM(GRP5_12_FUNC)	GRP5_12_FUNC	\
FM(GRP5_11_FUNC)	GRP5_11_FUNC	\
FM(GRP5_10_FUNC)	GRP5_10_FUNC	\
FM(GRP5_9_FUNC)		GRP5_9_FUNC	\
FM(GRP5_8_FUNC)		GRP5_8_FUNC	\
FM(GRP5_7_FUNC)		GRP5_7_FUNC	\
FM(GRP5_6_FUNC)		GRP5_6_FUNC	\
FM(GRP5_5_FUNC)		GRP5_5_FUNC	\
FM(GRP5_4_FUNC)		GRP5_4_FUNC	\
FM(GRP5_3_FUNC)		GRP5_3_FUNC	\
FM(GRP5_2_FUNC)		GRP5_2_FUNC	\
FM(GRP5_1_FUNC)		GRP5_1_FUNC	\
FM(GRP5_0_FUNC)		GRP5_0_FUNC

/* GRP6_FUNC */
#define GRP6_FUNC 			\
FM(GRP6_30_FUNC)	GRP6_30_FUNC	\
FM(GRP6_29_FUNC)	GRP6_29_FUNC	\
FM(GRP6_28_FUNC)	GRP6_28_FUNC	\
FM(GRP6_27_FUNC)	GRP6_27_FUNC	\
FM(GRP6_26_FUNC)	GRP6_26_FUNC	\
FM(GRP6_25_FUNC)	GRP6_25_FUNC	\
FM(GRP6_24_FUNC)	GRP6_24_FUNC	\
FM(GRP6_23_FUNC)	GRP6_23_FUNC	\
FM(GRP6_22_FUNC)	GRP6_22_FUNC	\
FM(GRP6_21_FUNC)	GRP6_21_FUNC	\
FM(GRP6_20_FUNC)	GRP6_20_FUNC	\
FM(GRP6_19_FUNC)	GRP6_19_FUNC	\
FM(GRP6_18_FUNC)	GRP6_18_FUNC	\
FM(GRP6_17_FUNC)	GRP6_17_FUNC	\
FM(GRP6_16_FUNC)	GRP6_16_FUNC	\
FM(GRP6_15_FUNC)	GRP6_15_FUNC	\
FM(GRP6_14_FUNC)	GRP6_14_FUNC	\
FM(GRP6_13_FUNC)	GRP6_13_FUNC	\
FM(GRP6_12_FUNC)	GRP6_12_FUNC	\
FM(GRP6_11_FUNC)	GRP6_11_FUNC	\
FM(GRP6_10_FUNC)	GRP6_10_FUNC	\
FM(GRP6_9_FUNC)		GRP6_9_FUNC	\
FM(GRP6_8_FUNC)		GRP6_8_FUNC	\
FM(GRP6_7_FUNC)		GRP6_7_FUNC	\
FM(GRP6_6_FUNC)		GRP6_6_FUNC	\
FM(GRP6_5_FUNC)		GRP6_5_FUNC	\
FM(GRP6_4_FUNC)		GRP6_4_FUNC	\
FM(GRP6_3_FUNC)		GRP6_3_FUNC	\
FM(GRP6_2_FUNC)		GRP6_2_FUNC	\
FM(GRP6_1_FUNC)		GRP6_1_FUNC	\
FM(GRP6_0_FUNC)		GRP6_0_FUNC

/* GRP7_FUNC */
#define GRP7_FUNC 			\
FM(GRP7_30_FUNC)	GRP7_30_FUNC	\
FM(GRP7_29_FUNC)	GRP7_29_FUNC	\
FM(GRP7_28_FUNC)	GRP7_28_FUNC	\
FM(GRP7_27_FUNC)	GRP7_27_FUNC	\
FM(GRP7_26_FUNC)	GRP7_26_FUNC	\
FM(GRP7_25_FUNC)	GRP7_25_FUNC	\
FM(GRP7_24_FUNC)	GRP7_24_FUNC	\
FM(GRP7_23_FUNC)	GRP7_23_FUNC	\
FM(GRP7_22_FUNC)	GRP7_22_FUNC	\
FM(GRP7_21_FUNC)	GRP7_21_FUNC	\
FM(GRP7_20_FUNC)	GRP7_20_FUNC	\
FM(GRP7_19_FUNC)	GRP7_19_FUNC	\
FM(GRP7_18_FUNC)	GRP7_18_FUNC	\
FM(GRP7_17_FUNC)	GRP7_17_FUNC	\
FM(GRP7_16_FUNC)	GRP7_16_FUNC	\
FM(GRP7_15_FUNC)	GRP7_15_FUNC	\
FM(GRP7_14_FUNC)	GRP7_14_FUNC	\
FM(GRP7_13_FUNC)	GRP7_13_FUNC	\
FM(GRP7_12_FUNC)	GRP7_12_FUNC	\
FM(GRP7_11_FUNC)	GRP7_11_FUNC	\
FM(GRP7_10_FUNC)	GRP7_10_FUNC	\
FM(GRP7_9_FUNC)		GRP7_9_FUNC	\
FM(GRP7_8_FUNC)		GRP7_8_FUNC	\
FM(GRP7_7_FUNC)		GRP7_7_FUNC	\
FM(GRP7_6_FUNC)		GRP7_6_FUNC	\
FM(GRP7_5_FUNC)		GRP7_5_FUNC	\
FM(GRP7_4_FUNC)		GRP7_4_FUNC	\
FM(GRP7_3_FUNC)		GRP7_3_FUNC	\
FM(GRP7_2_FUNC)		GRP7_2_FUNC	\
FM(GRP7_1_FUNC)		GRP7_1_FUNC	\
FM(GRP7_0_FUNC)		GRP7_0_FUNC

/* GRP8_FUNC */
#define GRP8_FUNC 			\
FM(GRP8_31_FUNC)	GRP8_31_FUNC	\
FM(GRP8_30_FUNC)	GRP8_30_FUNC	\
FM(GRP8_29_FUNC)	GRP8_29_FUNC	\
FM(GRP8_28_FUNC)	GRP8_28_FUNC	\
FM(GRP8_27_FUNC)	GRP8_27_FUNC	\
FM(GRP8_26_FUNC)	GRP8_26_FUNC	\
FM(GRP8_15_FUNC)	GRP8_15_FUNC	\
FM(GRP8_14_FUNC)	GRP8_14_FUNC	\
FM(GRP8_13_FUNC)	GRP8_13_FUNC	\
FM(GRP8_12_FUNC)	GRP8_12_FUNC	\
FM(GRP8_11_FUNC)	GRP8_11_FUNC	\
FM(GRP8_10_FUNC)	GRP8_10_FUNC	\
FM(GRP8_9_FUNC)		GRP8_9_FUNC	\
FM(GRP8_8_FUNC)		GRP8_8_FUNC	\
FM(GRP8_7_FUNC)		GRP8_7_FUNC	\
FM(GRP8_6_FUNC)		GRP8_6_FUNC	\
FM(GRP8_5_FUNC)		GRP8_5_FUNC	\
FM(GRP8_4_FUNC)		GRP8_4_FUNC	\
FM(GRP8_3_FUNC)		GRP8_3_FUNC	\
FM(GRP8_2_FUNC)		GRP8_2_FUNC	\
FM(GRP8_1_FUNC)		GRP8_1_FUNC	\
FM(GRP8_0_FUNC)		GRP8_0_FUNC

/* GRP9_FUNC */
#define GRP9_FUNC 			\
FM(GRP9_16_FUNC)	GRP9_16_FUNC	\
FM(GRP9_15_FUNC)	GRP9_15_FUNC	\
FM(GRP9_14_FUNC)	GRP9_14_FUNC	\
FM(GRP9_13_FUNC)	GRP9_13_FUNC	\
FM(GRP9_12_FUNC)	GRP9_12_FUNC	\
FM(GRP9_11_FUNC)	GRP9_11_FUNC	\
FM(GRP9_10_FUNC)	GRP9_10_FUNC	\
FM(GRP9_9_FUNC)		GRP9_9_FUNC	\
FM(GRP9_8_FUNC)		GRP9_8_FUNC	\
FM(GRP9_7_FUNC)		GRP9_7_FUNC	\
FM(GRP9_6_FUNC)		GRP9_6_FUNC	\
FM(GRP9_5_FUNC)		GRP9_5_FUNC	\
FM(GRP9_4_FUNC)		GRP9_4_FUNC	\
FM(GRP9_3_FUNC)		GRP9_3_FUNC	\
FM(GRP9_2_FUNC)		GRP9_2_FUNC	\
FM(GRP9_1_FUNC)		GRP9_1_FUNC	\
FM(GRP9_0_FUNC)		GRP9_0_FUNC

/* GRP10_FUNC */
#define GRP10_FUNC 			\
FM(GRP10_13_FUNC)	GRP10_13_FUNC	\
FM(GRP10_12_FUNC)	GRP10_12_FUNC	\
FM(GRP10_11_FUNC)	GRP10_11_FUNC	\
FM(GRP10_10_FUNC)	GRP10_10_FUNC	\
FM(GRP10_9_FUNC)	GRP10_9_FUNC	\
FM(GRP10_8_FUNC)	GRP10_8_FUNC	\
FM(GRP10_7_FUNC)	GRP10_7_FUNC	\
FM(GRP10_6_FUNC)	GRP10_6_FUNC	\
FM(GRP10_5_FUNC)	GRP10_5_FUNC	\
FM(GRP10_4_FUNC)	GRP10_4_FUNC	\
FM(GRP10_3_FUNC)	GRP10_3_FUNC	\
FM(GRP10_2_FUNC)	GRP10_2_FUNC	\
FM(GRP10_1_FUNC)	GRP10_1_FUNC	\
FM(GRP10_0_FUNC)	GRP10_0_FUNC

/* Mux all groups functions */
#define PINMUX_GRP_FUNC		\
GRP0_FUNC			\
GRP1_FUNC			\
GRP2_FUNC			\
GRP3_FUNC			\
GRP4_FUNC			\
GRP5_FUNC			\
GRP6_FUNC			\
GRP7_FUNC			\
GRP8_FUNC			\
GRP9_FUNC			\
GRP10_FUNC

#define GP_ALTSEL(id, bank, pin)	\
GP##bank##_ALTSEL##id##_##pin##_0,	\
GP##bank##_ALTSEL##id##_##pin##_1,

#define GP_ALTSEL_TYPE(id, bank)	\
	GP_ALTSEL(id, bank, 31)		\
	GP_ALTSEL(id, bank, 30)		\
	GP_ALTSEL(id, bank, 29)		\
	GP_ALTSEL(id, bank, 28)		\
	GP_ALTSEL(id, bank, 27)		\
	GP_ALTSEL(id, bank, 26)		\
	GP_ALTSEL(id, bank, 25)		\
	GP_ALTSEL(id, bank, 24)		\
	GP_ALTSEL(id, bank, 23)		\
	GP_ALTSEL(id, bank, 22)		\
	GP_ALTSEL(id, bank, 21)		\
	GP_ALTSEL(id, bank, 20)		\
	GP_ALTSEL(id, bank, 19)		\
	GP_ALTSEL(id, bank, 18)		\
	GP_ALTSEL(id, bank, 17)		\
	GP_ALTSEL(id, bank, 16)		\
	GP_ALTSEL(id, bank, 15)		\
	GP_ALTSEL(id, bank, 14)		\
	GP_ALTSEL(id, bank, 13)		\
	GP_ALTSEL(id, bank, 12)		\
	GP_ALTSEL(id, bank, 11)		\
	GP_ALTSEL(id, bank, 10)		\
	GP_ALTSEL(id, bank,  9)		\
	GP_ALTSEL(id, bank,  8)		\
	GP_ALTSEL(id, bank,  7)		\
	GP_ALTSEL(id, bank,  6)		\
	GP_ALTSEL(id, bank,  5)		\
	GP_ALTSEL(id, bank,  4)		\
	GP_ALTSEL(id, bank,  3)		\
	GP_ALTSEL(id, bank,  2)		\
	GP_ALTSEL(id, bank,  1)		\
	GP_ALTSEL(id, bank,  0)

#define GP_ALTSEL0(bank)	GP_ALTSEL_TYPE(0, bank)
#define GP_ALTSEL1(bank)	GP_ALTSEL_TYPE(1, bank)
#define GP_ALTSEL2(bank)	GP_ALTSEL_TYPE(2, bank)
#define GP_ALTSEL3(bank)	GP_ALTSEL_TYPE(3, bank)

#define PINMUX_ALTSEL	\
GP_ALTSEL0(0)	GP_ALTSEL0(1)	GP_ALTSEL0(2)	GP_ALTSEL0(3)	GP_ALTSEL0(4)	GP_ALTSEL0(5)	GP_ALTSEL0(6)	GP_ALTSEL0(7)	GP_ALTSEL0(8)	GP_ALTSEL0(9)	GP_ALTSEL0(10)	\
GP_ALTSEL1(0)	GP_ALTSEL1(1)	GP_ALTSEL1(2)	GP_ALTSEL1(3)	GP_ALTSEL1(4)	GP_ALTSEL1(5)	GP_ALTSEL1(6)	GP_ALTSEL1(7)	GP_ALTSEL1(8)	GP_ALTSEL1(9)	GP_ALTSEL1(10)	\
GP_ALTSEL2(0)	GP_ALTSEL2(1)	GP_ALTSEL2(2)	GP_ALTSEL2(3)	GP_ALTSEL2(4)	GP_ALTSEL2(5)	GP_ALTSEL2(6)	GP_ALTSEL2(7)	GP_ALTSEL2(8)	GP_ALTSEL2(9)	GP_ALTSEL2(10)	\
GP_ALTSEL3(0)	GP_ALTSEL3(1)	GP_ALTSEL3(2)	GP_ALTSEL3(3)	GP_ALTSEL3(4)	GP_ALTSEL3(5)	GP_ALTSEL3(6)	GP_ALTSEL3(7)	GP_ALTSEL3(8)	GP_ALTSEL3(9)	GP_ALTSEL3(10)

/* GP1_MODSEL */		/* 0 */			/* 1 */
#define GP1_MODSEL_21		FM(SEL_TAUJ13_OUTPUT)	FM(SEL_TAUJ13_INPUT)
#define GP1_MODSEL_20		FM(SEL_TAUJ12_OUTPUT)	FM(SEL_TAUJ12_INPUT)
#define GP1_MODSEL_19		FM(SEL_TAUJ11_OUTPUT)	FM(SEL_TAUJ11_INPUT)
#define GP1_MODSEL_18		FM(SEL_TAUJ10_OUTPUT)	FM(SEL_TAUJ10_INPUT)
#define GP1_MODSEL_17		FM(SEL_TAUJ33_OUTPUT)	FM(SEL_TAUJ33_INPUT)
#define GP1_MODSEL_16		FM(SEL_TAUJ32_OUTPUT)	FM(SEL_TAUJ32_INPUT)
#define GP1_MODSEL_15		FM(SEL_TAUJ31_OUTPUT)	FM(SEL_TAUJ31_INPUT)
#define GP1_MODSEL_14		FM(SEL_TAUJ30_OUTPUT)	FM(SEL_TAUJ30_INPUT)

#define GP1_MODSEL	\
GP1_MODSEL_21		\
GP1_MODSEL_20		\
GP1_MODSEL_19		\
GP1_MODSEL_18		\
GP1_MODSEL_17		\
GP1_MODSEL_16		\
GP1_MODSEL_15		\
GP1_MODSEL_14

/* GP2_MODSEL */		/* 0 */		/* 1 */
#define GP2_MODSEL_20		FM(SEL_SDA0_0)	FM(SEL_SDA0_1)
#define GP2_MODSEL_19		FM(SEL_SCL0_0)	FM(SEL_SCL0_1)

#define GP2_MODSEL	\
GP2_MODSEL_20		\
GP2_MODSEL_19

/* GP8_MODSEL */		/* 0 */		/* 1 */
#define GP8_MODSEL_15		FM(SEL_SDA8_0)	FM(SEL_SDA8_1)
#define GP8_MODSEL_14		FM(SEL_SCL8_0)	FM(SEL_SCL8_1)
#define GP8_MODSEL_13		FM(SEL_SDA7_0)	FM(SEL_SDA7_1)
#define GP8_MODSEL_12		FM(SEL_SCL7_0)	FM(SEL_SCL7_1)
#define GP8_MODSEL_11		FM(SEL_SDA6_0)	FM(SEL_SDA6_1)
#define GP8_MODSEL_10		FM(SEL_SCL6_0)	FM(SEL_SCL6_1)
#define GP8_MODSEL_9		FM(SEL_SDA5_0)	FM(SEL_SDA5_1)
#define GP8_MODSEL_8		FM(SEL_SCL5_0)	FM(SEL_SCL5_1)
#define GP8_MODSEL_7		FM(SEL_SDA4_0)	FM(SEL_SDA4_1)
#define GP8_MODSEL_6		FM(SEL_SCL4_0)	FM(SEL_SCL4_1)
#define GP8_MODSEL_5		FM(SEL_SDA3_0)	FM(SEL_SDA3_1)
#define GP8_MODSEL_4		FM(SEL_SCL3_0)	FM(SEL_SCL3_1)
#define GP8_MODSEL_3		FM(SEL_SDA2_0)	FM(SEL_SDA2_1)
#define GP8_MODSEL_2		FM(SEL_SCL2_0)	FM(SEL_SCL2_1)
#define GP8_MODSEL_1		FM(SEL_SDA1_0)	FM(SEL_SDA1_1)
#define GP8_MODSEL_0		FM(SEL_SCL1_0)	FM(SEL_SCL1_1)

#define GP8_MODSEL	\
GP8_MODSEL_15	\
GP8_MODSEL_14	\
GP8_MODSEL_13	\
GP8_MODSEL_12	\
GP8_MODSEL_11	\
GP8_MODSEL_10	\
GP8_MODSEL_9	\
GP8_MODSEL_8	\
GP8_MODSEL_7	\
GP8_MODSEL_6	\
GP8_MODSEL_5	\
GP8_MODSEL_4	\
GP8_MODSEL_3	\
GP8_MODSEL_2	\
GP8_MODSEL_1	\
GP8_MODSEL_0

#define PINMUX_MODSELS	\
GP8_MODSEL		\
GP2_MODSEL		\
GP1_MODSEL

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
	PINMUX_GRP_FUNC
	PINMUX_ALTSEL
	PINMUX_MODSELS
	PINMUX_FUNCTION_END,
#undef F_
#undef FM

#define F_(x, y)
#define FM(x)	x##_MARK,
	PINMUX_MARK_BEGIN,
	PINMUX_GPSR
	PINMUX_GRP_FUNC
	PINMUX_MODSELS
	PINMUX_MARK_END,
#undef F_
#undef FM
};

#define GP_ALTSEL_FUNC_0(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_1(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_2(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_3(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_4(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_5(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_6(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_7(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_0

#define GP_ALTSEL_FUNC_8(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC_9(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC_10(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC_11(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_0,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC_12(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC_13(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_0,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC_14(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_0,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC_15(bank, pin)		\
	GP##bank##_ALTSEL0_##pin##_1,		\
	GP##bank##_ALTSEL1_##pin##_1,		\
	GP##bank##_ALTSEL2_##pin##_1,		\
	GP##bank##_ALTSEL3_##pin##_1

#define GP_ALTSEL_FUNC(bank, pin, func)		\
	GP_ALTSEL_FUNC_##func(bank, pin)

/* GP0 ALTSEL function 0 */
#define ALT_DP2_HOTPLUG		GP_ALTSEL_FUNC(0, 27, 0)
#define ALT_DP1_HOTPLUG		GP_ALTSEL_FUNC(0, 26, 0)
#define ALT_DP0_HOTPLUG		GP_ALTSEL_FUNC(0, 25, 0)
#define ALT_MSIOF1_SS2_A	GP_ALTSEL_FUNC(0, 24, 0)
#define ALT_MSIOF1_SS1_A	GP_ALTSEL_FUNC(0, 23, 0)
#define ALT_MSIOF1_SYNC_A	GP_ALTSEL_FUNC(0, 22, 0)
#define ALT_MSIOF1_RXD_A	GP_ALTSEL_FUNC(0, 21, 0)
#define ALT_MSIOF1_TXD_A	GP_ALTSEL_FUNC(0, 20, 0)
#define ALT_MSIOF1_SCK_A	GP_ALTSEL_FUNC(0, 19, 0)
#define ALT_MSIOF0_SS2		GP_ALTSEL_FUNC(0, 18, 0)
#define ALT_MSIOF0_SS1		GP_ALTSEL_FUNC(0, 17, 0)
#define ALT_MSIOF0_SYNC		GP_ALTSEL_FUNC(0, 16, 0)
#define ALT_MSIOF0_RXD		GP_ALTSEL_FUNC(0, 15, 0)
#define ALT_MSIOF0_TXD		GP_ALTSEL_FUNC(0, 14, 0)
#define ALT_MSIOF0_SCK		GP_ALTSEL_FUNC(0, 13, 0)
#define ALT_RXDB_EXTFXR_A	GP_ALTSEL_FUNC(0, 12, 0)
#define ALT_FXR_TXENB_N_A	GP_ALTSEL_FUNC(0, 11, 0)
#define ALT_FXR_TXDB_A		GP_ALTSEL_FUNC(0, 10, 0)
#define ALT_RXDA_EXTFXR_A	GP_ALTSEL_FUNC(0, 9, 0)
#define ALT_FXR_TXENA_N_A	GP_ALTSEL_FUNC(0, 8, 0)
#define ALT_FXR_TXDA_A		GP_ALTSEL_FUNC(0, 7, 0)
#define ALT_CLK_EXTFXR_A	GP_ALTSEL_FUNC(0, 6, 0)
#define ALT_FXR_CLKOUT2_A	GP_ALTSEL_FUNC(0, 5, 0)
#define ALT_FXR_CLKOUT1_A	GP_ALTSEL_FUNC(0, 4, 0)
#define ALT_STPWT_EXTFXR_A	GP_ALTSEL_FUNC(0, 3, 0)

/* GP0 ALTSEL function 1 */
#define ALT_TAUD0O13		GP_ALTSEL_FUNC(0, 24, 1)
#define ALT_TAUD0O12		GP_ALTSEL_FUNC(0, 23, 1)
#define ALT_TAUD0O11		GP_ALTSEL_FUNC(0, 22, 1)
#define ALT_TAUD0O10		GP_ALTSEL_FUNC(0, 21, 1)
#define ALT_TAUD0O9		GP_ALTSEL_FUNC(0, 20, 1)
#define ALT_TAUD0O8		GP_ALTSEL_FUNC(0, 19, 1)
#define ALT_TAUD0O7		GP_ALTSEL_FUNC(0, 18, 1)
#define ALT_TAUD0O6		GP_ALTSEL_FUNC(0, 17, 1)
#define ALT_TAUD0O5		GP_ALTSEL_FUNC(0, 16, 1)
#define ALT_TAUD0O4		GP_ALTSEL_FUNC(0, 15, 1)
#define ALT_TAUD0O3		GP_ALTSEL_FUNC(0, 14, 1)
#define ALT_TAUD0O2		GP_ALTSEL_FUNC(0, 13, 1)
#define ALT_CAN11TX		GP_ALTSEL_FUNC(0, 12, 1)
#define ALT_CAN11RX_INTP11	GP_ALTSEL_FUNC(0, 11, 1)
#define ALT_CAN10TX		GP_ALTSEL_FUNC(0, 10, 1)
#define ALT_CAN10RX_INTP10	GP_ALTSEL_FUNC(0, 9, 1)
#define ALT_CAN9TX		GP_ALTSEL_FUNC(0, 8, 1)
#define ALT_CAN9RX_INTP9	GP_ALTSEL_FUNC(0, 7, 1)
#define ALT_CAN8TX		GP_ALTSEL_FUNC(0, 6, 1)
#define ALT_CAN8RX_INTP8	GP_ALTSEL_FUNC(0, 5, 1)
#define ALT_CAN7TX		GP_ALTSEL_FUNC(0, 4, 1)
#define ALT_CAN7RX_INTP7	GP_ALTSEL_FUNC(0, 3, 1)

/* GP0 ALTSEL function 2 */
#define ALT_RLIN311TX		GP_ALTSEL_FUNC(0, 12, 2)
#define ALT_RLIN311RX_INTP27	GP_ALTSEL_FUNC(0, 11, 2)
#define ALT_RLIN310TX		GP_ALTSEL_FUNC(0, 10, 2)
#define ALT_RLIN310RX_INTP26	GP_ALTSEL_FUNC(0, 9, 2)
#define ALT_RLIN39TX		GP_ALTSEL_FUNC(0, 8, 2)
#define ALT_RLIN39RX_INTP25	GP_ALTSEL_FUNC(0, 7, 2)
#define ALT_RLIN38TX		GP_ALTSEL_FUNC(0, 6, 2)
#define ALT_RLIN38RX_INTP24	GP_ALTSEL_FUNC(0, 5, 2)
#define ALT_RLIN315TX_A		GP_ALTSEL_FUNC(0, 4, 2)
#define ALT_RLIN315RX_INTP31_A	GP_ALTSEL_FUNC(0, 3, 2)

/* GP0 ALTSEL function 3 */
#define ALT_RTCA0OUT_A		GP_ALTSEL_FUNC(0, 12, 3)
#define ALT_EXTCLK0O_A		GP_ALTSEL_FUNC(0, 11, 3)
#define ALT_TAUD1O15		GP_ALTSEL_FUNC(0, 8, 3)
#define ALT_TAUD1O14		GP_ALTSEL_FUNC(0, 7, 3)
#define ALT_TAUD1O13		GP_ALTSEL_FUNC(0, 6, 3)
#define ALT_TAUD1O12		GP_ALTSEL_FUNC(0, 5, 3)
#define ALT_TAUD1O11		GP_ALTSEL_FUNC(0, 4, 3)
#define ALT_TAUD1O10		GP_ALTSEL_FUNC(0, 3, 3)

/* GP1 ALTSEL function 0 */
#define ALT_RLIN33TX		GP_ALTSEL_FUNC(1, 21, 0)
#define ALT_RLIN33RX_INTP19	GP_ALTSEL_FUNC(1, 20, 0)
#define ALT_RLIN32TX		GP_ALTSEL_FUNC(1, 19, 0)
#define ALT_RLIN32RX_INTP18	GP_ALTSEL_FUNC(1, 18, 0)
#define ALT_RLIN31TX		GP_ALTSEL_FUNC(1, 17, 0)
#define ALT_RLIN31RX_INTP17	GP_ALTSEL_FUNC(1, 16, 0)
#define ALT_RLIN30TX		GP_ALTSEL_FUNC(1, 15, 0)
#define ALT_RLIN30RX_INTP16	GP_ALTSEL_FUNC(1, 14, 0)
#define ALT_CAN6TX		GP_ALTSEL_FUNC(1, 13, 0)
#define ALT_CAN6RX_INTP6	GP_ALTSEL_FUNC(1, 12, 0)
#define ALT_CAN5TX		GP_ALTSEL_FUNC(1, 11, 0)
#define ALT_CAN5RX_INTP5	GP_ALTSEL_FUNC(1, 10, 0)
#define ALT_CAN4TX		GP_ALTSEL_FUNC(1, 9, 0)
#define ALT_CAN4RX_INTP4	GP_ALTSEL_FUNC(1, 8, 0)
#define ALT_CAN3TX		GP_ALTSEL_FUNC(1, 7, 0)
#define ALT_CAN3RX_INTP3	GP_ALTSEL_FUNC(1, 6, 0)
#define ALT_CAN2TX		GP_ALTSEL_FUNC(1, 5, 0)
#define ALT_CAN2RX_INTP2	GP_ALTSEL_FUNC(1, 4, 0)
#define ALT_CAN1TX		GP_ALTSEL_FUNC(1, 3, 0)
#define ALT_CAN1RX_INTP1	GP_ALTSEL_FUNC(1, 2, 0)
#define ALT_CAN0TX		GP_ALTSEL_FUNC(1, 1, 0)
#define ALT_CAN0RX_INTP0	GP_ALTSEL_FUNC(1, 0, 0)

/* GP1 ALTSEL function 1 */
#define ALT_TAUJ1I3_TAUJ1O3	GP_ALTSEL_FUNC(1, 21, 1)
#define ALT_TAUJ1I2_TAUJ1O2	GP_ALTSEL_FUNC(1, 20, 1)
#define ALT_TAUJ1I1_TAUJ1O1	GP_ALTSEL_FUNC(1, 19, 1)
#define ALT_TAUJ1I0_TAUJ1O0	GP_ALTSEL_FUNC(1, 18, 1)
#define ALT_TAUJ3I3_TAUJ3O3	GP_ALTSEL_FUNC(1, 17, 1)
#define ALT_TAUJ3I2_TAUJ3O2	GP_ALTSEL_FUNC(1, 16, 1)
#define ALT_TAUJ3I1_TAUJ3O1	GP_ALTSEL_FUNC(1, 15, 1)
#define ALT_TAUJ3I0_TAUJ3O0	GP_ALTSEL_FUNC(1, 14, 1)
#define ALT_RLIN314TX_A		GP_ALTSEL_FUNC(1, 13, 1)
#define ALT_RLIN314RX_INTP30_A	GP_ALTSEL_FUNC(1, 12, 1)
#define ALT_RLIN313TX_A		GP_ALTSEL_FUNC(1, 11, 1)
#define ALT_RLIN313RX_INTP29_A	GP_ALTSEL_FUNC(1, 10, 1)
#define ALT_RLIN312TX_A		GP_ALTSEL_FUNC(1, 9, 1)
#define ALT_RLIN312RX_INTP28_A	GP_ALTSEL_FUNC(1, 8, 1)
#define ALT_RLIN37TX_A		GP_ALTSEL_FUNC(1, 7, 1)
#define ALT_RLIN37RX_INTP23_A	GP_ALTSEL_FUNC(1, 6, 1)
#define ALT_RLIN36TX_A		GP_ALTSEL_FUNC(1, 5, 1)
#define ALT_RLIN36RX_INTP22_A	GP_ALTSEL_FUNC(1, 4, 1)
#define ALT_RLIN35TX_A		GP_ALTSEL_FUNC(1, 3, 1)
#define ALT_RLIN35RX_INTP21_A	GP_ALTSEL_FUNC(1, 2, 1)
#define ALT_RLIN34TX_A		GP_ALTSEL_FUNC(1, 1, 1)
#define ALT_RLIN34RX_INTP20_A	GP_ALTSEL_FUNC(1, 0, 1)

/* GP1 ALTSEL function 2 */
#define ALT_CAN15TX_A		GP_ALTSEL_FUNC(1, 21, 2)
#define ALT_CAN15RX_INTP15_A	GP_ALTSEL_FUNC(1, 20, 2)
#define ALT_CAN14TX_A		GP_ALTSEL_FUNC(1, 19, 2)
#define ALT_CAN14RX_INTP14_A	GP_ALTSEL_FUNC(1, 18, 2)
#define ALT_CAN13TX_A		GP_ALTSEL_FUNC(1, 17, 2)
#define ALT_CAN13RX_INTP13_A	GP_ALTSEL_FUNC(1, 16, 2)
#define ALT_CAN12TX_A		GP_ALTSEL_FUNC(1, 15, 2)
#define ALT_CAN12RX_INTP12_A	GP_ALTSEL_FUNC(1, 14, 2)
#define ALT_TAUD1O9		GP_ALTSEL_FUNC(1, 13, 2)
#define ALT_TAUD1O8		GP_ALTSEL_FUNC(1, 12, 2)
#define ALT_MSIOF3_SS2		GP_ALTSEL_FUNC(1, 11, 2)
#define ALT_MSIOF3_SS1		GP_ALTSEL_FUNC(1, 10, 2)
#define ALT_MSIOF3_SYNC		GP_ALTSEL_FUNC(1, 9, 2)
#define ALT_MSIOF3_RXD		GP_ALTSEL_FUNC(1, 8, 2)
#define ALT_MSIOF3_TXD		GP_ALTSEL_FUNC(1, 7, 2)
#define ALT_MSIOF3_SCK		GP_ALTSEL_FUNC(1, 6, 2)
#define ALT_MSIOF2_SS2		GP_ALTSEL_FUNC(1, 5, 2)
#define ALT_MSIOF2_SS1		GP_ALTSEL_FUNC(1, 4, 2)
#define ALT_MSIOF2_SYNC		GP_ALTSEL_FUNC(1, 3, 2)
#define ALT_MSIOF2_RXD		GP_ALTSEL_FUNC(1, 2, 2)
#define ALT_MSIOF2_TXD		GP_ALTSEL_FUNC(1, 1, 2)
#define ALT_MSIOF2_SCK		GP_ALTSEL_FUNC(1, 0, 2)

/* GP1 ALTSEL function 3 */
#define ALT_NMI1_A		GP_ALTSEL_FUNC(1, 19, 3)
#define ALT_INTP34_A		GP_ALTSEL_FUNC(1, 18, 3)
#define ALT_INTP33_A		GP_ALTSEL_FUNC(1, 17, 3)
#define ALT_INTP32_A		GP_ALTSEL_FUNC(1, 16, 3)
#define ALT_RXDB_EXTFXR_B	GP_ALTSEL_FUNC(1, 11, 3)
#define ALT_FXR_TXENB_N_B	GP_ALTSEL_FUNC(1, 10, 3)
#define ALT_FXR_TXDB_B		GP_ALTSEL_FUNC(1, 9, 3)
#define ALT_RXDA_EXTFXR_B	GP_ALTSEL_FUNC(1, 8, 3)
#define ALT_FXR_TXENA_N_B	GP_ALTSEL_FUNC(1, 7, 3)
#define ALT_FXR_TXDA_B		GP_ALTSEL_FUNC(1, 6, 3)
#define ALT_CLK_EXTFXR_B	GP_ALTSEL_FUNC(1, 5, 3)
#define ALT_FXR_CLKOUT2_B	GP_ALTSEL_FUNC(1, 4, 3)
#define ALT_FXR_CLKOUT1_B	GP_ALTSEL_FUNC(1, 3, 3)
#define ALT_STPWT_EXTFXR_B	GP_ALTSEL_FUNC(1, 2, 3)

/* GP2 ALTSEL function 0 */
#define ALT_INTP34_B		GP_ALTSEL_FUNC(2, 28, 0)
#define ALT_TAUD1O3		GP_ALTSEL_FUNC(2, 27, 0)
#define ALT_TAUD1O2		GP_ALTSEL_FUNC(2, 26, 0)
#define ALT_TAUD1O1		GP_ALTSEL_FUNC(2, 25, 0)
#define ALT_TAUD1O0		GP_ALTSEL_FUNC(2, 24, 0)
#define ALT_EXTCLK0O_B		GP_ALTSEL_FUNC(2, 23, 0)
#define ALT_AVS1		GP_ALTSEL_FUNC(2, 22, 0)
#define ALT_AVS0		GP_ALTSEL_FUNC(2, 21, 0)
#define ALT_SDA0		GP_ALTSEL_FUNC(2, 20, 0)
#define ALT_SCL0		GP_ALTSEL_FUNC(2, 19, 0)
#define ALT_INTP33_B		GP_ALTSEL_FUNC(2, 18, 0)
#define ALT_INTP32_B		GP_ALTSEL_FUNC(2, 17, 0)
#define ALT_CAN_CLK		GP_ALTSEL_FUNC(2, 16, 0)
#define ALT_CAN15TX_B		GP_ALTSEL_FUNC(2, 15, 0)
#define ALT_CAN15RX_B_INTP15	GP_ALTSEL_FUNC(2, 14, 0)
#define ALT_CAN14TX_B		GP_ALTSEL_FUNC(2, 13, 0)
#define ALT_CAN14RX_B_INTP14	GP_ALTSEL_FUNC(2, 12, 0)
#define ALT_CAN13TX_B		GP_ALTSEL_FUNC(2, 11, 0)
#define ALT_CAN13RX_B_INTP13	GP_ALTSEL_FUNC(2, 10, 0)
#define ALT_CAN12TX_B		GP_ALTSEL_FUNC(2, 9, 0)
#define ALT_CAN12RX_B_INTP12	GP_ALTSEL_FUNC(2, 8, 0)
#define ALT_RLIN37TX_B		GP_ALTSEL_FUNC(2, 7, 0)
#define ALT_RLIN37RX_B_INTP23	GP_ALTSEL_FUNC(2, 6, 0)
#define ALT_RLIN36TX_B		GP_ALTSEL_FUNC(2, 5, 0)
#define ALT_RLIN36RX_B_INTP22	GP_ALTSEL_FUNC(2, 4, 0)
#define ALT_RLIN35TX_B		GP_ALTSEL_FUNC(2, 3, 0)
#define ALT_RLIN35RX_B_INTP21	GP_ALTSEL_FUNC(2, 2, 0)
#define ALT_RLIN34TX_B		GP_ALTSEL_FUNC(2, 1, 0)
#define ALT_RLIN34RX_B_INTP20	GP_ALTSEL_FUNC(2, 0, 0)

/* GP2 ALTSEL function 1 */
#define ALT_TAUD0O1		GP_ALTSEL_FUNC(2, 18, 1)
#define ALT_TAUD0O0		GP_ALTSEL_FUNC(2, 17, 1)
#define ALT_RLIN315TX_B		GP_ALTSEL_FUNC(2, 15, 1)
#define ALT_RLIN315RX_INTP31_B	GP_ALTSEL_FUNC(2, 14, 1)
#define ALT_RLIN314TX_B		GP_ALTSEL_FUNC(2, 13, 1)
#define ALT_RLIN314RX_INTP30_B	GP_ALTSEL_FUNC(2, 12, 1)
#define ALT_RLIN313TX		GP_ALTSEL_FUNC(2, 11, 1)
#define ALT_RLIN313RX_INTP29_B	GP_ALTSEL_FUNC(2, 10, 1)
#define ALT_RLIN312TX		GP_ALTSEL_FUNC(2, 9, 1)
#define ALT_RLIN312RX_INTP28_B	GP_ALTSEL_FUNC(2, 8, 1)
#define ALT_RTCA0OUT_B		GP_ALTSEL_FUNC(2, 7, 1)
#define ALT_MSIOF1_SS2_B	GP_ALTSEL_FUNC(2, 5, 1)
#define ALT_MSIOF1_SS1_B	GP_ALTSEL_FUNC(2, 4, 1)
#define ALT_MSIOF1_SYN_B	GP_ALTSEL_FUNC(2, 3, 1)
#define ALT_MSIOF1_RXD_B	GP_ALTSEL_FUNC(2, 2, 1)
#define ALT_MSIOF1_TXD_B	GP_ALTSEL_FUNC(2, 1, 1)
#define ALT_MSIOF1_SCK_B	GP_ALTSEL_FUNC(2, 0, 1)

/* GP2 ALTSEL function 2 */
#define ALT_TAUD1O7		GP_ALTSEL_FUNC(2, 9, 2)
#define ALT_TAUD1O6		GP_ALTSEL_FUNC(2, 8, 2)
#define ALT_TAUD1O5		GP_ALTSEL_FUNC(2, 7, 2)
#define ALT_TAUD1O4		GP_ALTSEL_FUNC(2, 6, 2)
#define ALT_TAUD0O15		GP_ALTSEL_FUNC(2, 1, 2)
#define ALT_TAUD0O14		GP_ALTSEL_FUNC(2, 0, 2)

/* GP2 ALTSEL function 3 */
#define ALT_CANXL1_TX		GP_ALTSEL_FUNC(2, 11, 3)
#define ALT_CANXL1_RX		GP_ALTSEL_FUNC(2, 10, 3)
#define ALT_CANXL0_TX		GP_ALTSEL_FUNC(2, 9, 3)
#define ALT_CANXL0_RX		GP_ALTSEL_FUNC(2, 8, 3)
#define ALT_CTIACK		GP_ALTSEL_FUNC(2, 4, 3)
#define ALT_CTIREQ		GP_ALTSEL_FUNC(2, 3, 3)

/* GP3 ALTSEL function 0 */
#define ALT_ERRORIN0_N		GP_ALTSEL_FUNC(3, 16, 0)
#define ALT_ERROROUT_N		GP_ALTSEL_FUNC(3, 15, 0)
#define ALT_QSPI1_SSL		GP_ALTSEL_FUNC(3, 14, 0)
#define ALT_QSPI1_IO3		GP_ALTSEL_FUNC(3, 13, 0)
#define ALT_QSPI1_IO2		GP_ALTSEL_FUNC(3, 12, 0)
#define ALT_QSPI1_MISO_IO1	GP_ALTSEL_FUNC(3, 11, 0)
#define ALT_QSPI1_MOSI_IO0	GP_ALTSEL_FUNC(3, 10, 0)
#define ALT_QSPI1_SPCLK		GP_ALTSEL_FUNC(3, 9, 0)
#define ALT_RPC_INT_N		GP_ALTSEL_FUNC(3, 8, 0)
#define ALT_RPC_WP_N		GP_ALTSEL_FUNC(3, 7, 0)
#define ALT_RPC_RESET_N		GP_ALTSEL_FUNC(3, 6, 0)
#define ALT_QSPI0_SSL		GP_ALTSEL_FUNC(3, 5, 0)
#define ALT_QSPI0_IO3		GP_ALTSEL_FUNC(3, 4, 0)
#define ALT_QSPI0_IO2		GP_ALTSEL_FUNC(3, 3, 0)
#define ALT_QSPI0_MISO_IO1	GP_ALTSEL_FUNC(3, 2, 0)
#define ALT_QSPI0_MOSI_IO0	GP_ALTSEL_FUNC(3, 1, 0)
#define ALT_QSPI0_SPCLK		GP_ALTSEL_FUNC(3, 0, 0)

/* GP4 ALTSEL function 0 */
#define ALT_PCIE61_CLKREQ_N	GP_ALTSEL_FUNC(4, 15, 0)
#define ALT_PCIE60_CLKREQ_N	GP_ALTSEL_FUNC(4, 14, 0)
#define ALT_ERRORIN1_N		GP_ALTSEL_FUNC(4, 13, 0)
#define ALT_SD0_CD		GP_ALTSEL_FUNC(4, 12, 0)
#define ALT_SD0_WP		GP_ALTSEL_FUNC(4, 11, 0)
#define ALT_MMC0_DS		GP_ALTSEL_FUNC(4, 10, 0)
#define ALT_MMC0_D7		GP_ALTSEL_FUNC(4, 9, 0)
#define ALT_MMC0_D6		GP_ALTSEL_FUNC(4, 8, 0)
#define ALT_MMC0_D5		GP_ALTSEL_FUNC(4, 7, 0)
#define ALT_MMC0_D4		GP_ALTSEL_FUNC(4, 6, 0)
#define ALT_MMC0_SD_D3		GP_ALTSEL_FUNC(4, 5, 0)
#define ALT_MMC0_SD_D2		GP_ALTSEL_FUNC(4, 4, 0)
#define ALT_MMC0_SD_D1		GP_ALTSEL_FUNC(4, 3, 0)
#define ALT_MMC0_SD_D0		GP_ALTSEL_FUNC(4, 2, 0)
#define ALT_MMC0_SD_CMD		GP_ALTSEL_FUNC(4, 1, 0)
#define ALT_MMC0_SD_CLK		GP_ALTSEL_FUNC(4, 0, 0)

/* GP5 ALTSEL function 0 */
#define ALT_TPU0TO3		GP_ALTSEL_FUNC(5, 22, 0)
#define ALT_TPU0TO2		GP_ALTSEL_FUNC(5, 21, 0)
#define ALT_TPU0TO1		GP_ALTSEL_FUNC(5, 20, 0)
#define ALT_TPU0TO0		GP_ALTSEL_FUNC(5, 19, 0)
#define ALT_TCLK4		GP_ALTSEL_FUNC(5, 18, 0)
#define ALT_TCLK3		GP_ALTSEL_FUNC(5, 17, 0)
#define ALT_TCLK2		GP_ALTSEL_FUNC(5, 16, 0)
#define ALT_TCLK1		GP_ALTSEL_FUNC(5, 15, 0)
#define ALT_IRQ3_A		GP_ALTSEL_FUNC(5, 14, 0)
#define ALT_IRQ2_A		GP_ALTSEL_FUNC(5, 13, 0)
#define ALT_IRQ1_A		GP_ALTSEL_FUNC(5, 12, 0)
#define ALT_IRQ0_A		GP_ALTSEL_FUNC(5, 11, 0)
#define ALT_HSCK1		GP_ALTSEL_FUNC(5, 10, 0)
#define ALT_HCTS1_N		GP_ALTSEL_FUNC(5, 9, 0)
#define ALT_HRTS1_N		GP_ALTSEL_FUNC(5, 8, 0)
#define ALT_HRX1		GP_ALTSEL_FUNC(5, 7, 0)
#define ALT_HTX1		GP_ALTSEL_FUNC(5, 6, 0)
#define ALT_SCIF_CLK		GP_ALTSEL_FUNC(5, 5, 0)
#define ALT_HSCK0		GP_ALTSEL_FUNC(5, 4, 0)
#define ALT_HCTS0_N		GP_ALTSEL_FUNC(5, 3, 0)
#define ALT_HRTS0_N		GP_ALTSEL_FUNC(5, 2, 0)
#define ALT_HRX0		GP_ALTSEL_FUNC(5, 1, 0)
#define ALT_HTX0		GP_ALTSEL_FUNC(5, 0, 0)

/* GP5 ALTSEL function 1 */
#define ALT_SSI9_WS		GP_ALTSEL_FUNC(5, 22, 1)
#define ALT_SSI9_SCK		GP_ALTSEL_FUNC(5, 21, 1)
#define ALT_PWM5		GP_ALTSEL_FUNC(5, 20, 1)
#define ALT_PWM4		GP_ALTSEL_FUNC(5, 19, 1)
#define ALT_PWM3		GP_ALTSEL_FUNC(5, 18, 1)
#define ALT_PWM2		GP_ALTSEL_FUNC(5, 17, 1)
#define ALT_PWM1		GP_ALTSEL_FUNC(5, 16, 1)
#define ALT_PWM0_A		GP_ALTSEL_FUNC(5, 15, 1)
#define ALT_SSI17_SD		GP_ALTSEL_FUNC(5, 13, 1)
#define ALT_SSI17_WS		GP_ALTSEL_FUNC(5, 12, 1)
#define ALT_SSI17_SCK		GP_ALTSEL_FUNC(5, 11, 1)
#define ALT_SCK1		GP_ALTSEL_FUNC(5, 10, 1)
#define ALT_CTS1_N		GP_ALTSEL_FUNC(5, 9, 1)
#define ALT_RTS1_N		GP_ALTSEL_FUNC(5, 8, 1)
#define ALT_RX1			GP_ALTSEL_FUNC(5, 7, 1)
#define ALT_TX1			GP_ALTSEL_FUNC(5, 6, 1)
#define ALT_SCK0		GP_ALTSEL_FUNC(5, 4, 1)
#define ALT_CTS0_N		GP_ALTSEL_FUNC(5, 3, 1)
#define ALT_RTS0_N		GP_ALTSEL_FUNC(5, 2, 1)
#define ALT_RX0			GP_ALTSEL_FUNC(5, 1, 1)
#define ALT_TX0			GP_ALTSEL_FUNC(5, 0, 1)

/* GP5 ALTSEL function 2 */
#define ALT_SSI19_SD		GP_ALTSEL_FUNC(5, 18, 2)
#define ALT_SSI19_WS		GP_ALTSEL_FUNC(5, 17, 2)
#define ALT_SSI19_SCK		GP_ALTSEL_FUNC(5, 16, 2)
#define ALT_SSI18_SD		GP_ALTSEL_FUNC(5, 15, 2)
#define ALT_SSI13_SCK		GP_ALTSEL_FUNC(5, 10, 2)
#define ALT_RIF0_SYNC_B		GP_ALTSEL_FUNC(5, 8, 2)
#define ALT_IRQ1_B		GP_ALTSEL_FUNC(5, 3, 2)
#define ALT_IRQ0_B		GP_ALTSEL_FUNC(5, 2, 2)
#define ALT_SSI13_SD		GP_ALTSEL_FUNC(5, 1, 2)
#define ALT_SSI13_WS		GP_ALTSEL_FUNC(5, 0, 2)

/* GP5 ALTSEL function 3*/
#define ALT_RIF7_D1		GP_ALTSEL_FUNC(5, 14, 3)
#define ALT_RIF7_D0		GP_ALTSEL_FUNC(5, 13, 3)
#define ALT_RIF7_SYNC		GP_ALTSEL_FUNC(5, 12, 3)
#define ALT_RIF7_CLK		GP_ALTSEL_FUNC(5, 11, 3)
#define ALT_RIF0_CLK_B		GP_ALTSEL_FUNC(5, 10, 3)
#define ALT_SSI16_SD		GP_ALTSEL_FUNC(5, 8, 3)
#define ALT_SSI16_WS		GP_ALTSEL_FUNC(5, 7, 3)
#define ALT_SSI16_SCK		GP_ALTSEL_FUNC(5, 6, 3)
#define ALT_SSI15_SD		GP_ALTSEL_FUNC(5, 4, 3)
#define ALT_SSI15_WS		GP_ALTSEL_FUNC(5, 3, 3)
#define ALT_SSI15_SCK		GP_ALTSEL_FUNC(5, 2, 3)
#define ALT_RIF0_D1_B		GP_ALTSEL_FUNC(5, 1, 3)
#define ALT_RIF0_D0_B		GP_ALTSEL_FUNC(5, 0, 3)

/* GP6 ALTSEL function 0 */
#define ALT_AUDIO1_CLKOUT1	GP_ALTSEL_FUNC(6, 30, 0)
#define ALT_AUDIO1_CLKOUT0	GP_ALTSEL_FUNC(6, 29, 0)
#define ALT_SSI2_SD		GP_ALTSEL_FUNC(6, 28, 0)
#define ALT_SSI2_WS		GP_ALTSEL_FUNC(6, 27, 0)
#define ALT_SSI2_SCK		GP_ALTSEL_FUNC(6, 26, 0)
#define ALT_AUDIO0_CLKOUT3	GP_ALTSEL_FUNC(6, 25, 0)
#define ALT_AUDIO0_CLKOUT2	GP_ALTSEL_FUNC(6, 24, 0)
#define ALT_SSI1_SD		GP_ALTSEL_FUNC(6, 23, 0)
#define ALT_SSI1_WS		GP_ALTSEL_FUNC(6, 22, 0)
#define ALT_SSI1_SCK		GP_ALTSEL_FUNC(6, 21, 0)
#define ALT_AUDIO0_CLKOUT1	GP_ALTSEL_FUNC(6, 20, 0)
#define ALT_AUDIO0_CLKOUT0	GP_ALTSEL_FUNC(6, 19, 0)
#define ALT_SSI0_SD		GP_ALTSEL_FUNC(6, 18, 0)
#define ALT_SSI0_WS		GP_ALTSEL_FUNC(6, 17, 0)
#define ALT_SSI0_SCK		GP_ALTSEL_FUNC(6, 16, 0)
#define ALT_MSIOF4_SS2_B	GP_ALTSEL_FUNC(6, 15, 0)
#define ALT_MSIOF4_SS1_B	GP_ALTSEL_FUNC(6, 14, 0)
#define ALT_MSIOF4_SYNC_B	GP_ALTSEL_FUNC(6, 13, 0)
#define ALT_MSIOF4_RXD_B	GP_ALTSEL_FUNC(6, 12, 0)
#define ALT_MSIOF4_TXD_B	GP_ALTSEL_FUNC(6, 11, 0)
#define ALT_MSIOF4_SCK_B	GP_ALTSEL_FUNC(6, 10, 0)
#define ALT_MSIOF7_SS2_A	GP_ALTSEL_FUNC(6, 9, 0)
#define ALT_MSIOF7_SS1_A	GP_ALTSEL_FUNC(6, 8, 0)
#define ALT_MSIOF7_SYNC_A	GP_ALTSEL_FUNC(6, 7, 0)
#define ALT_MSIOF7_RXD_A	GP_ALTSEL_FUNC(6, 6, 0)
#define ALT_MSIOF7_TXD_A	GP_ALTSEL_FUNC(6, 5, 0)
#define ALT_MSIOF7_SCK_A	GP_ALTSEL_FUNC(6, 4, 0)
#define ALT_RIF6_CLK		GP_ALTSEL_FUNC(6, 3, 0)
#define ALT_RIF6_SYNC		GP_ALTSEL_FUNC(6, 2, 0)
#define ALT_RIF6_D1		GP_ALTSEL_FUNC(6, 1, 0)
#define ALT_RIF6_D0		GP_ALTSEL_FUNC(6, 0, 0)

/* GP6 ALTSEL function 1 */
#define ALT_MSIOF7_RXD_B	GP_ALTSEL_FUNC(6, 30, 1)
#define ALT_MSIOF7_TXD_B	GP_ALTSEL_FUNC(6, 29, 1)
#define ALT_MSIOF7_SCK_B	GP_ALTSEL_FUNC(6, 28, 1)
#define ALT_MSIOF4_SS2_A	GP_ALTSEL_FUNC(6, 21, 1)
#define ALT_MSIOF4_SS1_A	GP_ALTSEL_FUNC(6, 20, 1)
#define ALT_MSIOF4_SYNC_A	GP_ALTSEL_FUNC(6, 19, 1)
#define ALT_MSIOF4_RXD_A	GP_ALTSEL_FUNC(6, 18, 1)
#define ALT_MSIOF4_TXD_A	GP_ALTSEL_FUNC(6, 17, 1)
#define ALT_MSIOF4_SCK_A	GP_ALTSEL_FUNC(6, 16, 1)
#define ALT_SSI14_SD		GP_ALTSEL_FUNC(6, 15, 1)
#define ALT_SSI12_SD		GP_ALTSEL_FUNC(6, 14, 1)
#define ALT_SSI12_WS		GP_ALTSEL_FUNC(6, 13, 1)
#define ALT_SSI12_SCK		GP_ALTSEL_FUNC(6, 11, 1)
#define ALT_SSI14_WS		GP_ALTSEL_FUNC(6, 9, 1)
#define ALT_SSI14_SCK		GP_ALTSEL_FUNC(6, 8, 1)
#define ALT_RIF1_D1_B		GP_ALTSEL_FUNC(6, 7, 1)
#define ALT_RIF1_D0_B		GP_ALTSEL_FUNC(6, 6, 1)
#define ALT_RIF1_SYNC_B		GP_ALTSEL_FUNC(6, 5, 1)
#define ALT_RIF1_CLK_B		GP_ALTSEL_FUNC(6, 4, 1)
#define ALT_SSI10_SD		GP_ALTSEL_FUNC(6, 3, 1)
#define ALT_SSI10_WS		GP_ALTSEL_FUNC(6, 2, 1)
#define ALT_SSI10_SCK		GP_ALTSEL_FUNC(6, 1, 1)
#define ALT_SSI9_SD		GP_ALTSEL_FUNC(6, 0, 1)

/* GP6 ALTSEL function 2 */
#define ALT_RIF5_CLK		GP_ALTSEL_FUNC(6, 30, 2)
#define ALT_RIF5_SYNC		GP_ALTSEL_FUNC(6, 28, 2)
#define ALT_RIF1_D1_A		GP_ALTSEL_FUNC(6, 27, 2)
#define ALT_RIF1_D0_A		GP_ALTSEL_FUNC(6, 26, 2)
#define ALT_RIF1_CLK_A		GP_ALTSEL_FUNC(6, 25, 2)
#define ALT_RIF2_D1		GP_ALTSEL_FUNC(6, 24, 2)
#define ALT_HCTS3_N		GP_ALTSEL_FUNC(6, 23, 2)
#define ALT_HRTS3_N		GP_ALTSEL_FUNC(6, 22, 2)
#define ALT_HSCK3		GP_ALTSEL_FUNC(6, 21, 2)
#define ALT_RIF2_D0		GP_ALTSEL_FUNC(6, 20, 2)
#define ALT_RIF2_SYNC		GP_ALTSEL_FUNC(6, 19, 2)
#define ALT_HRX3		GP_ALTSEL_FUNC(6, 18, 2)
#define ALT_HTX3		GP_ALTSEL_FUNC(6, 17, 2)
#define ALT_AUDIO_CLKC_B	GP_ALTSEL_FUNC(6, 12, 2)
#define ALT_AUDIO_CLKB_B	GP_ALTSEL_FUNC(6, 10, 2)
#define ALT_SSI11_SD		GP_ALTSEL_FUNC(6, 7, 2)
#define ALT_SSI11_WS		GP_ALTSEL_FUNC(6, 6, 2)
#define ALT_SSI11_SCK		GP_ALTSEL_FUNC(6, 5, 2)
#define ALT_AUDIO_CLKA_B	GP_ALTSEL_FUNC(6, 4, 2)

/* GP6 ALTSEL function 3 */
#define ALT_CTS3_N		GP_ALTSEL_FUNC(6, 22, 3)
#define ALT_RTS3_N		GP_ALTSEL_FUNC(6, 22, 3)
#define ALT_SCK3		GP_ALTSEL_FUNC(6, 21, 3)
#define ALT_RX3			GP_ALTSEL_FUNC(6, 18, 3)
#define ALT_TX3			GP_ALTSEL_FUNC(6, 17, 3)
#define ALT_RIF1_SYNC_A		GP_ALTSEL_FUNC(6, 11, 3)

/* GP7 ALTSEL function 0 */
#define ALT_MSIOF6_SS2_B	GP_ALTSEL_FUNC(7, 30, 0)
#define ALT_MSIOF6_SS1_B	GP_ALTSEL_FUNC(7, 29, 0)
#define ALT_MSIOF6_SYNC_B	GP_ALTSEL_FUNC(7, 28, 0)
#define ALT_MSIOF6_RXD_B	GP_ALTSEL_FUNC(7, 27, 0)
#define ALT_MSIOF6_TXD_B	GP_ALTSEL_FUNC(7, 26, 0)
#define ALT_MSIOF6_SCK_B	GP_ALTSEL_FUNC(7, 25, 0)
#define ALT_MSIOF5_SS2		GP_ALTSEL_FUNC(7, 24, 0)
#define ALT_MSIOF5_SS1		GP_ALTSEL_FUNC(7, 23, 0)
#define ALT_MSIOF5_SYNC		GP_ALTSEL_FUNC(7, 22, 0)
#define ALT_MSIOF5_RXD		GP_ALTSEL_FUNC(7, 21, 0)
#define ALT_MSIOF5_TXD		GP_ALTSEL_FUNC(7, 20, 0)
#define ALT_MSIOF5_SCK		GP_ALTSEL_FUNC(7, 17, 0)
#define ALT_AUDIO_CLKC_A	GP_ALTSEL_FUNC(7, 16, 0)
#define ALT_SSI6_SD		GP_ALTSEL_FUNC(7, 15, 0)
#define ALT_SSI6_WS		GP_ALTSEL_FUNC(7, 14, 0)
#define ALT_SSI6_SCK		GP_ALTSEL_FUNC(7, 13, 0)
#define ALT_AUDIO_CLKB_A	GP_ALTSEL_FUNC(7, 12, 0)
#define ALT_SSI5_SD		GP_ALTSEL_FUNC(7, 11, 0)
#define ALT_SSI5_WS		GP_ALTSEL_FUNC(7, 10, 0)
#define ALT_SSI5_SCK		GP_ALTSEL_FUNC(7, 9, 0)
#define ALT_AUDIO_CLKA_A	GP_ALTSEL_FUNC(7, 8, 0)
#define ALT_SSI4_SD		GP_ALTSEL_FUNC(7, 7, 0)
#define ALT_SSI4_WS		GP_ALTSEL_FUNC(7, 6, 0)
#define ALT_SSI4_SCK		GP_ALTSEL_FUNC(7, 5, 0)
#define ALT_AUDIO1_CLKOUT3	GP_ALTSEL_FUNC(7, 4, 0)
#define ALT_AUDIO1_CLKOUT2	GP_ALTSEL_FUNC(7, 3, 0)
#define ALT_SSI3_SD		GP_ALTSEL_FUNC(7, 2, 0)
#define ALT_SSI3_WS		GP_ALTSEL_FUNC(7, 1, 0)
#define ALT_SSI3_SCK		GP_ALTSEL_FUNC(7, 0, 0)

/* GP7 ALTSEL function 1 */
#define ALT_HRX2_B		GP_ALTSEL_FUNC(7, 30, 1)
#define ALT_SSI7_SD		GP_ALTSEL_FUNC(7, 29, 1)
#define ALT_SSI7_WS		GP_ALTSEL_FUNC(7, 28, 1)
#define ALT_SSI7_SCK		GP_ALTSEL_FUNC(7, 27, 1)
#define ALT_HTX2_B		GP_ALTSEL_FUNC(7, 26, 1)
#define ALT_SSI8_SD		GP_ALTSEL_FUNC(7, 25, 1)
#define ALT_HCTS2_N_B		GP_ALTSEL_FUNC(7, 24, 1)
#define ALT_RIF0_SYNC_A		GP_ALTSEL_FUNC(7, 23, 1)
#define ALT_HRTS2_N_B		GP_ALTSEL_FUNC(7, 22, 1)
#define ALT_RIF0_D1_A		GP_ALTSEL_FUNC(7, 21, 1)
#define ALT_HSCK2_B		GP_ALTSEL_FUNC(7, 20, 1)
#define ALT_MSIOF6_SS2_A	GP_ALTSEL_FUNC(7, 19, 1)
#define ALT_MSIOF6_SS1_A	GP_ALTSEL_FUNC(7, 18, 1)
#define ALT_MSIOF6_RXD_A	GP_ALTSEL_FUNC(7, 15, 1)
#define ALT_MSIOF6_TXD_A	GP_ALTSEL_FUNC(7, 14, 1)
#define ALT_MSIOF6_SCK_A	GP_ALTSEL_FUNC(7, 13, 1)
#define ALT_MSIOF6_SYNC_A	GP_ALTSEL_FUNC(7, 11, 1)
#define ALT_RIF3_SYNC		GP_ALTSEL_FUNC(7, 10, 1)
#define ALT_RIF3_CLK		GP_ALTSEL_FUNC(7, 9, 1)
#define ALT_RIF3_D1		GP_ALTSEL_FUNC(7, 7, 1)
#define ALT_RIF3_D0		GP_ALTSEL_FUNC(7, 6, 1)
#define ALT_RIF2_CLK		GP_ALTSEL_FUNC(7, 5, 1)
#define ALT_RIF0_D0_A		GP_ALTSEL_FUNC(7, 4, 1)
#define ALT_RIF0_CLK_A		GP_ALTSEL_FUNC(7, 3, 1)
#define ALT_MSIOF7_SS2_B	GP_ALTSEL_FUNC(7, 2, 1)
#define ALT_MSIOF7_SS1_B	GP_ALTSEL_FUNC(7, 1, 1)
#define ALT_MSIOF7_SYNC_B	GP_ALTSEL_FUNC(7, 0, 1)

/* GP7 ALTSEL function 2 */
#define ALT_RX4_B		GP_ALTSEL_FUNC(7, 30, 2)
#define ALT_TX4_B		GP_ALTSEL_FUNC(7, 26, 2)
#define ALT_CTS4_N_B		GP_ALTSEL_FUNC(7, 24, 2)
#define ALT_RTS4_N_B		GP_ALTSEL_FUNC(7, 22, 2)
#define ALT_SCK4_B		GP_ALTSEL_FUNC(7, 20, 2)
#define ALT_RIF4_D1		GP_ALTSEL_FUNC(7, 15, 2)
#define ALT_RIF4_D0		GP_ALTSEL_FUNC(7, 14, 2)
#define ALT_RIF4_SYNC		GP_ALTSEL_FUNC(7, 13, 2)
#define ALT_RIF4_CLK		GP_ALTSEL_FUNC(7, 11, 2)
#define ALT_RIF5_D1		GP_ALTSEL_FUNC(7, 1, 2)
#define ALT_RIF5_D0		GP_ALTSEL_FUNC(7, 0, 2)

/* GP8 ALTSEL function 0 */
#define ALT_S3DA2		GP_ALTSEL_FUNC(8, 31, 0)
#define ALT_S3CL2		GP_ALTSEL_FUNC(8, 30, 0)
#define ALT_S3DA1		GP_ALTSEL_FUNC(8, 29, 0)
#define ALT_S3CL1		GP_ALTSEL_FUNC(8, 28, 0)
#define ALT_S3DA0		GP_ALTSEL_FUNC(8, 27, 0)
#define ALT_S3CL0		GP_ALTSEL_FUNC(8, 26, 0)
#define ALT_SDA8		GP_ALTSEL_FUNC(8, 15, 0)
#define ALT_SCL8		GP_ALTSEL_FUNC(8, 14, 0)
#define ALT_SDA7		GP_ALTSEL_FUNC(8, 13, 0)
#define ALT_SCL7		GP_ALTSEL_FUNC(8, 12, 0)
#define ALT_SDA6		GP_ALTSEL_FUNC(8, 11, 0)
#define ALT_SCL6		GP_ALTSEL_FUNC(8, 10, 0)
#define ALT_SDA5		GP_ALTSEL_FUNC(8, 9, 0)
#define ALT_SCL5		GP_ALTSEL_FUNC(8, 8, 0)
#define ALT_SDA4		GP_ALTSEL_FUNC(8, 7, 0)
#define ALT_SCL4		GP_ALTSEL_FUNC(8, 6, 0)
#define ALT_SDA3		GP_ALTSEL_FUNC(8, 5, 0)
#define ALT_SCL3		GP_ALTSEL_FUNC(8, 4, 0)
#define ALT_SDA2		GP_ALTSEL_FUNC(8, 3, 0)
#define ALT_SCL2		GP_ALTSEL_FUNC(8, 2, 0)
#define ALT_SDA1		GP_ALTSEL_FUNC(8, 1, 0)
#define ALT_SCL1		GP_ALTSEL_FUNC(8, 0, 0)

/* GP8 ALTSEL function 1 */
#define ALT_HCTS2_N_A		GP_ALTSEL_FUNC(8, 7, 1)
#define ALT_HRTS2_N_A		GP_ALTSEL_FUNC(8, 6, 1)
#define ALT_HRX2_A		GP_ALTSEL_FUNC(8, 5, 1)
#define ALT_HTX2_A		GP_ALTSEL_FUNC(8, 4, 1)
#define ALT_HSCK2_A		GP_ALTSEL_FUNC(8, 3, 1)
#define ALT_PWM0_B		GP_ALTSEL_FUNC(8, 2, 1)

/* GP8 ALTSEL function 2 */
#define ALT_CTS4_N_A		GP_ALTSEL_FUNC(8, 7, 2)
#define ALT_RTS4_N_A		GP_ALTSEL_FUNC(8, 6, 2)
#define ALT_RX4_A		GP_ALTSEL_FUNC(8, 5, 2)
#define ALT_TX4_A		GP_ALTSEL_FUNC(8, 4, 2)
#define ALT_SCK4_A		GP_ALTSEL_FUNC(8, 3, 2)

/* GP8 ALTSEL function 3 */
#define ALT_PWM7_B		GP_ALTSEL_FUNC(8, 7, 3)
#define ALT_PWM9_B		GP_ALTSEL_FUNC(8, 6, 3)
#define ALT_PWM8_B		GP_ALTSEL_FUNC(8, 5, 3)
#define ALT_PWM6_B		GP_ALTSEL_FUNC(8, 4, 3)

/* GP9 ALTSEL function 0 */
#define ALT_RSW3_MATCH		GP_ALTSEL_FUNC(9, 16, 0)
#define ALT_RSW3_CAPTURE	GP_ALTSEL_FUNC(9, 15, 0)
#define ALT_RSW3_PPS		GP_ALTSEL_FUNC(9, 14, 0)
#define ALT_ETH10G0_PHYINT	GP_ALTSEL_FUNC(9, 13, 0)
#define ALT_ETH10G0_LINK	GP_ALTSEL_FUNC(9, 12, 0)
#define ALT_ETH10G0_MDC		GP_ALTSEL_FUNC(9, 11, 0)
#define ALT_ETH10G0_MDIO	GP_ALTSEL_FUNC(9, 10, 0)
#define ALT_ETH25G0_PHYINT	GP_ALTSEL_FUNC(9, 9, 0)
#define ALT_ETH25G0_LINK	GP_ALTSEL_FUNC(9, 8, 0)
#define ALT_ETH25G0_MDC		GP_ALTSEL_FUNC(9, 7, 0)
#define ALT_ETH25G0_MDIO	GP_ALTSEL_FUNC(9, 6, 0)
#define ALT_ETHES4_MATCH	GP_ALTSEL_FUNC(9, 5, 0)
#define ALT_ETHES4_CAPTURE	GP_ALTSEL_FUNC(9, 4, 0)
#define ALT_ETHES4_PPS		GP_ALTSEL_FUNC(9, 3, 0)
#define ALT_ETHES0_MATCH	GP_ALTSEL_FUNC(9, 2, 0)
#define ALT_ETHES0_CAPTURE	GP_ALTSEL_FUNC(9, 1, 0)
#define ALT_ETHES0_PPS		GP_ALTSEL_FUNC(9, 0, 0)

/* GP9 ALTSEL function 1 */
#define ALT_ETH10G1_PHYINT	GP_ALTSEL_FUNC(9, 13, 1)
#define ALT_ETH10G1_LINK	GP_ALTSEL_FUNC(9, 12, 1)
#define ALT_ETH10G1_MDC		GP_ALTSEL_FUNC(9, 11, 1)
#define ALT_ETH10G1_MDIO	GP_ALTSEL_FUNC(9, 10, 1)
#define ALT_ETH25G1_PHYINT	GP_ALTSEL_FUNC(9, 9, 1)
#define ALT_ETH25G1_LINK	GP_ALTSEL_FUNC(9, 8, 1)
#define ALT_ETH25G1_MDC		GP_ALTSEL_FUNC(9, 7, 1)
#define ALT_ETH25G1_MDIO	GP_ALTSEL_FUNC(9, 6, 1)
#define ALT_ETHES5_MATCH	GP_ALTSEL_FUNC(9, 5, 1)
#define ALT_ETHES5_CAPTURE	GP_ALTSEL_FUNC(9, 4, 1)
#define ALT_ETHES5_PPS		GP_ALTSEL_FUNC(9, 3, 1)
#define ALT_ETHES1_MATCH	GP_ALTSEL_FUNC(9, 2, 1)
#define ALT_ETHES1_CAPTURE	GP_ALTSEL_FUNC(9, 1, 1)
#define ALT_ETHES1_PPS		GP_ALTSEL_FUNC(9, 0, 1)

/* GP9 ALTSEL function 2 */
#define ALT_ETH25G2_PHYINT	GP_ALTSEL_FUNC(9, 9, 2)
#define ALT_ETH25G2_LINK	GP_ALTSEL_FUNC(9, 8, 2)
#define ALT_ETH25G2_MDC		GP_ALTSEL_FUNC(9, 7, 2)
#define ALT_ETH25G2_MDIO	GP_ALTSEL_FUNC(9, 6, 2)
#define ALT_ETHES6_MATCH	GP_ALTSEL_FUNC(9, 5, 2)
#define ALT_ETHES6_CAPTURE	GP_ALTSEL_FUNC(9, 4, 2)
#define ALT_ETHES6_PPS		GP_ALTSEL_FUNC(9, 3, 2)
#define ALT_ETHES2_MATCH	GP_ALTSEL_FUNC(9, 2, 2)
#define ALT_ETHES2_CAPTURE	GP_ALTSEL_FUNC(9, 1, 2)
#define ALT_ETHES2_PPS		GP_ALTSEL_FUNC(9, 0, 2)

/* GP9 ALTSEL function 3 */
#define ALT_PWM9_A		GP_ALTSEL_FUNC(9, 16, 0)
#define ALT_PWM8_A		GP_ALTSEL_FUNC(9, 15, 0)
#define ALT_PWM7_A		GP_ALTSEL_FUNC(9, 12, 0)
#define ALT_IRQ3_B		GP_ALTSEL_FUNC(9, 11, 0)
#define ALT_IRQ2_B		GP_ALTSEL_FUNC(9, 10, 0)
#define ALT_PWM6_A		GP_ALTSEL_FUNC(9, 8, 0)
#define ALT_ETHES7_MATCH	GP_ALTSEL_FUNC(9, 5, 0)
#define ALT_ETHES7_CAPTURE	GP_ALTSEL_FUNC(9, 4, 0)
#define ALT_ETHES7_PPS		GP_ALTSEL_FUNC(9, 3, 0)
#define ALT_ETHES3_MATCH	GP_ALTSEL_FUNC(9, 2, 0)
#define ALT_ETHES3_CAPTURE	GP_ALTSEL_FUNC(9, 1, 0)
#define ALT_ETHES3_PPS		GP_ALTSEL_FUNC(9, 0, 0)

/* GP10 ALTSEL function 0 */
#define ALT_PCIE41_CLKREQ_N	GP_ALTSEL_FUNC(10, 13, 0)
#define ALT_PCIE40_CLKREQ_N	GP_ALTSEL_FUNC(10, 12, 0)
#define ALT_USB3_VBUS_VALID	GP_ALTSEL_FUNC(10, 11, 0)
#define ALT_USB3_OVC		GP_ALTSEL_FUNC(10, 10, 0)
#define ALT_USB3_PWEN		GP_ALTSEL_FUNC(10, 9, 0)
#define ALT_USB2_VBUS_VALID	GP_ALTSEL_FUNC(10, 8, 0)
#define ALT_USB2_OVC		GP_ALTSEL_FUNC(10, 7, 0)
#define ALT_USB2_PWEN		GP_ALTSEL_FUNC(10, 6, 0)
#define ALT_USB1_VBUS_VALID	GP_ALTSEL_FUNC(10, 5, 0)
#define ALT_USB1_OVC		GP_ALTSEL_FUNC(10, 4, 0)
#define ALT_USB1_PWEN		GP_ALTSEL_FUNC(10, 3, 0)
#define ALT_USB0_VBUS_VALID	GP_ALTSEL_FUNC(10, 2, 0)
#define ALT_USB0_OVC		GP_ALTSEL_FUNC(10, 1, 0)
#define ALT_USB0_PWEN		GP_ALTSEL_FUNC(10, 0, 0)

#define PINMUX_GFUNC_GPSR(grp_func, fn)	\
fn##_MARK, ALT_##fn, FN_##grp_func, 0

#define PINMUX_GFUNC_MSEL_GPSR(grp_func, fn, msel)	\
fn##_MARK, ALT_##fn, FN_##msel, FN_##grp_func, 0

#define PINMUX_GFUNC_MSELMARK_GPSR(grp_func, fn, msel)	\
msel##_MARK, ALT_##fn, FN_##msel, FN_##grp_func, 0

static const u16 pinmux_data[] = {
	PINMUX_DATA_GP_ALL(),

	/* Group0 Functions */
	PINMUX_GFUNC_GPSR(GRP0_27_FUNC, DP2_HOTPLUG),

	PINMUX_GFUNC_GPSR(GRP0_26_FUNC, DP1_HOTPLUG),

	PINMUX_GFUNC_GPSR(GRP0_25_FUNC, DP0_HOTPLUG),

	PINMUX_GFUNC_GPSR(GRP0_24_FUNC, MSIOF1_SS2_A),
	PINMUX_GFUNC_GPSR(GRP0_24_FUNC, TAUD0O13),

	PINMUX_GFUNC_GPSR(GRP0_23_FUNC, MSIOF1_SS1_A),
	PINMUX_GFUNC_GPSR(GRP0_23_FUNC, TAUD0O12),

	PINMUX_GFUNC_GPSR(GRP0_22_FUNC, MSIOF1_SYNC_A),
	PINMUX_GFUNC_GPSR(GRP0_22_FUNC, TAUD0O11),

	PINMUX_GFUNC_GPSR(GRP0_21_FUNC, MSIOF1_RXD_A),
	PINMUX_GFUNC_GPSR(GRP0_21_FUNC, TAUD0O10),

	PINMUX_GFUNC_GPSR(GRP0_20_FUNC, MSIOF1_TXD_A),
	PINMUX_GFUNC_GPSR(GRP0_20_FUNC, TAUD0O9),

	PINMUX_GFUNC_GPSR(GRP0_19_FUNC, MSIOF1_SCK_A),
	PINMUX_GFUNC_GPSR(GRP0_19_FUNC, TAUD0O8),

	PINMUX_GFUNC_GPSR(GRP0_18_FUNC, MSIOF0_SS2),
	PINMUX_GFUNC_GPSR(GRP0_18_FUNC, TAUD0O7),

	PINMUX_GFUNC_GPSR(GRP0_17_FUNC, MSIOF0_SS1),
	PINMUX_GFUNC_GPSR(GRP0_17_FUNC, TAUD0O6),

	PINMUX_GFUNC_GPSR(GRP0_16_FUNC, MSIOF0_SYNC),
	PINMUX_GFUNC_GPSR(GRP0_16_FUNC, TAUD0O5),

	PINMUX_GFUNC_GPSR(GRP0_15_FUNC, MSIOF0_RXD),
	PINMUX_GFUNC_GPSR(GRP0_15_FUNC, TAUD0O4),

	PINMUX_GFUNC_GPSR(GRP0_14_FUNC, MSIOF0_TXD),
	PINMUX_GFUNC_GPSR(GRP0_14_FUNC, TAUD0O3),

	PINMUX_GFUNC_GPSR(GRP0_13_FUNC, MSIOF0_SCK),
	PINMUX_GFUNC_GPSR(GRP0_13_FUNC, TAUD0O2),

	PINMUX_GFUNC_GPSR(GRP0_12_FUNC, RXDB_EXTFXR_A),
	PINMUX_GFUNC_GPSR(GRP0_12_FUNC, CAN11TX),
	PINMUX_GFUNC_GPSR(GRP0_12_FUNC, RLIN311TX),
	PINMUX_GFUNC_GPSR(GRP0_12_FUNC, RTCA0OUT_A),

	PINMUX_GFUNC_GPSR(GRP0_11_FUNC, FXR_TXENB_N_A),
	PINMUX_GFUNC_GPSR(GRP0_11_FUNC, CAN11RX_INTP11),
	PINMUX_GFUNC_GPSR(GRP0_11_FUNC, RLIN311RX_INTP27),
	PINMUX_GFUNC_GPSR(GRP0_11_FUNC, EXTCLK0O_A),

	PINMUX_GFUNC_GPSR(GRP0_10_FUNC, FXR_TXDB_A),
	PINMUX_GFUNC_GPSR(GRP0_10_FUNC, CAN10TX),
	PINMUX_GFUNC_GPSR(GRP0_10_FUNC, RLIN310TX),

	PINMUX_GFUNC_GPSR(GRP0_9_FUNC, RXDA_EXTFXR_A),
	PINMUX_GFUNC_GPSR(GRP0_9_FUNC, CAN10RX_INTP10),
	PINMUX_GFUNC_GPSR(GRP0_9_FUNC, RLIN310RX_INTP26),

	PINMUX_GFUNC_GPSR(GRP0_8_FUNC, FXR_TXENA_N_A),
	PINMUX_GFUNC_GPSR(GRP0_8_FUNC, CAN9TX),
	PINMUX_GFUNC_GPSR(GRP0_8_FUNC, RLIN39TX),
	PINMUX_GFUNC_GPSR(GRP0_8_FUNC, TAUD1O15),

	PINMUX_GFUNC_GPSR(GRP0_7_FUNC, FXR_TXDA_A),
	PINMUX_GFUNC_GPSR(GRP0_7_FUNC, CAN9RX_INTP9),
	PINMUX_GFUNC_GPSR(GRP0_7_FUNC, RLIN39RX_INTP25),
	PINMUX_GFUNC_GPSR(GRP0_7_FUNC, TAUD1O14),

	PINMUX_GFUNC_GPSR(GRP0_6_FUNC, CLK_EXTFXR_A),
	PINMUX_GFUNC_GPSR(GRP0_6_FUNC, CAN8TX),
	PINMUX_GFUNC_GPSR(GRP0_6_FUNC, RLIN38TX),
	PINMUX_GFUNC_GPSR(GRP0_6_FUNC, TAUD1O13),

	PINMUX_GFUNC_GPSR(GRP0_5_FUNC, FXR_CLKOUT2_A),
	PINMUX_GFUNC_GPSR(GRP0_5_FUNC, CAN8RX_INTP8),
	PINMUX_GFUNC_GPSR(GRP0_5_FUNC, RLIN38RX_INTP24),
	PINMUX_GFUNC_GPSR(GRP0_5_FUNC, TAUD1O12),

	PINMUX_GFUNC_GPSR(GRP0_4_FUNC, FXR_CLKOUT1_A),
	PINMUX_GFUNC_GPSR(GRP0_4_FUNC, CAN7TX),
	PINMUX_GFUNC_GPSR(GRP0_4_FUNC, RLIN315TX_A),
	PINMUX_GFUNC_GPSR(GRP0_4_FUNC, TAUD1O11),

	PINMUX_GFUNC_GPSR(GRP0_3_FUNC, STPWT_EXTFXR_A),
	PINMUX_GFUNC_GPSR(GRP0_3_FUNC, CAN7RX_INTP7),
	PINMUX_GFUNC_GPSR(GRP0_3_FUNC, RLIN315RX_INTP31_A),
	PINMUX_GFUNC_GPSR(GRP0_3_FUNC, TAUD1O10),

	/* Group1 Functions */
	PINMUX_GFUNC_GPSR(GRP1_21_FUNC, RLIN33TX),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_21_FUNC, TAUJ1I3_TAUJ1O3, SEL_TAUJ13_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_21_FUNC, TAUJ1I3_TAUJ1O3, SEL_TAUJ13_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_21_FUNC, CAN15TX_A),

	PINMUX_GFUNC_GPSR(GRP1_20_FUNC, RLIN33RX_INTP19),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_20_FUNC, TAUJ1I2_TAUJ1O2, SEL_TAUJ12_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_20_FUNC, TAUJ1I2_TAUJ1O2, SEL_TAUJ12_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_20_FUNC, CAN15RX_INTP15_A),

	PINMUX_GFUNC_GPSR(GRP1_19_FUNC, RLIN32TX),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_19_FUNC, TAUJ1I1_TAUJ1O1, SEL_TAUJ11_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_19_FUNC, TAUJ1I1_TAUJ1O1, SEL_TAUJ11_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_19_FUNC, CAN14TX_A),
	PINMUX_GFUNC_GPSR(GRP1_19_FUNC, NMI1_A),

	PINMUX_GFUNC_GPSR(GRP1_18_FUNC, RLIN32RX_INTP18),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_18_FUNC, TAUJ1I0_TAUJ1O0, SEL_TAUJ10_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_18_FUNC, TAUJ1I0_TAUJ1O0, SEL_TAUJ10_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_18_FUNC, CAN14RX_INTP14_A),
	PINMUX_GFUNC_GPSR(GRP1_18_FUNC, INTP34_A),

	PINMUX_GFUNC_GPSR(GRP1_17_FUNC, RLIN31TX),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_17_FUNC, TAUJ3I3_TAUJ3O3, SEL_TAUJ33_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_17_FUNC, TAUJ3I3_TAUJ3O3, SEL_TAUJ33_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_17_FUNC, CAN13TX_A),
	PINMUX_GFUNC_GPSR(GRP1_17_FUNC, INTP33_A),

	PINMUX_GFUNC_GPSR(GRP1_16_FUNC, RLIN31RX_INTP17),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_16_FUNC, TAUJ3I2_TAUJ3O2, SEL_TAUJ32_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_16_FUNC, TAUJ3I2_TAUJ3O2, SEL_TAUJ32_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_16_FUNC, CAN13RX_INTP13_A),
	PINMUX_GFUNC_GPSR(GRP1_16_FUNC, INTP32_A),

	PINMUX_GFUNC_GPSR(GRP1_15_FUNC, RLIN30TX),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_15_FUNC, TAUJ3I1_TAUJ3O1, SEL_TAUJ31_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_15_FUNC, TAUJ3I1_TAUJ3O1, SEL_TAUJ31_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_15_FUNC, CAN12TX_A),

	PINMUX_GFUNC_GPSR(GRP1_14_FUNC, RLIN30RX_INTP16),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_14_FUNC, TAUJ3I0_TAUJ3O0, SEL_TAUJ30_OUTPUT),
	PINMUX_GFUNC_MSELMARK_GPSR(GRP1_14_FUNC, TAUJ3I0_TAUJ3O0, SEL_TAUJ30_INPUT),
	PINMUX_GFUNC_GPSR(GRP1_14_FUNC, CAN12RX_INTP12_A),

	PINMUX_GFUNC_GPSR(GRP1_13_FUNC, CAN6TX),
	PINMUX_GFUNC_GPSR(GRP1_13_FUNC, RLIN314TX_A),
	PINMUX_GFUNC_GPSR(GRP1_13_FUNC, TAUD1O9),

	PINMUX_GFUNC_GPSR(GRP1_12_FUNC, CAN6RX_INTP6),
	PINMUX_GFUNC_GPSR(GRP1_12_FUNC, RLIN314RX_INTP30_A),
	PINMUX_GFUNC_GPSR(GRP1_12_FUNC, TAUD1O8),

	PINMUX_GFUNC_GPSR(GRP1_11_FUNC, CAN5TX),
	PINMUX_GFUNC_GPSR(GRP1_11_FUNC, RLIN313TX_A),
	PINMUX_GFUNC_GPSR(GRP1_11_FUNC, MSIOF3_SS2),
	PINMUX_GFUNC_GPSR(GRP1_11_FUNC, RXDB_EXTFXR_B),

	PINMUX_GFUNC_GPSR(GRP1_10_FUNC, CAN5RX_INTP5),
	PINMUX_GFUNC_GPSR(GRP1_10_FUNC, RLIN313RX_INTP29_A),
	PINMUX_GFUNC_GPSR(GRP1_10_FUNC, MSIOF3_SS1),
	PINMUX_GFUNC_GPSR(GRP1_10_FUNC, FXR_TXENB_N_B),

	PINMUX_GFUNC_GPSR(GRP1_9_FUNC, CAN4TX),
	PINMUX_GFUNC_GPSR(GRP1_9_FUNC, RLIN312TX_A),
	PINMUX_GFUNC_GPSR(GRP1_9_FUNC, MSIOF3_SYNC),
	PINMUX_GFUNC_GPSR(GRP1_9_FUNC, FXR_TXDB_B),

	PINMUX_GFUNC_GPSR(GRP1_8_FUNC, CAN4RX_INTP4),
	PINMUX_GFUNC_GPSR(GRP1_8_FUNC, RLIN312RX_INTP28_A),
	PINMUX_GFUNC_GPSR(GRP1_8_FUNC, MSIOF3_RXD),
	PINMUX_GFUNC_GPSR(GRP1_8_FUNC, RXDA_EXTFXR_B),

	PINMUX_GFUNC_GPSR(GRP1_7_FUNC, CAN3TX),
	PINMUX_GFUNC_GPSR(GRP1_7_FUNC, RLIN37TX_A),
	PINMUX_GFUNC_GPSR(GRP1_7_FUNC, MSIOF3_TXD),
	PINMUX_GFUNC_GPSR(GRP1_7_FUNC, FXR_TXENA_N_B),

	PINMUX_GFUNC_GPSR(GRP1_6_FUNC, CAN3RX_INTP3),
	PINMUX_GFUNC_GPSR(GRP1_6_FUNC, RLIN37RX_INTP23_A),
	PINMUX_GFUNC_GPSR(GRP1_6_FUNC, MSIOF3_SCK),
	PINMUX_GFUNC_GPSR(GRP1_6_FUNC, FXR_TXDA_B),

	PINMUX_GFUNC_GPSR(GRP1_5_FUNC, CAN2TX),
	PINMUX_GFUNC_GPSR(GRP1_5_FUNC, RLIN36TX_A),
	PINMUX_GFUNC_GPSR(GRP1_5_FUNC, MSIOF2_SS2),
	PINMUX_GFUNC_GPSR(GRP1_5_FUNC, CLK_EXTFXR_B),

	PINMUX_GFUNC_GPSR(GRP1_4_FUNC, CAN2RX_INTP2),
	PINMUX_GFUNC_GPSR(GRP1_4_FUNC, RLIN36RX_INTP22_A),
	PINMUX_GFUNC_GPSR(GRP1_4_FUNC, MSIOF2_SS1),
	PINMUX_GFUNC_GPSR(GRP1_4_FUNC, FXR_CLKOUT2_B),

	PINMUX_GFUNC_GPSR(GRP1_3_FUNC, CAN1TX),
	PINMUX_GFUNC_GPSR(GRP1_3_FUNC, RLIN35TX_A),
	PINMUX_GFUNC_GPSR(GRP1_3_FUNC, MSIOF2_SYNC),
	PINMUX_GFUNC_GPSR(GRP1_3_FUNC, FXR_CLKOUT1_B),

	PINMUX_GFUNC_GPSR(GRP1_2_FUNC, CAN1RX_INTP1),
	PINMUX_GFUNC_GPSR(GRP1_2_FUNC, RLIN35RX_INTP21_A),
	PINMUX_GFUNC_GPSR(GRP1_2_FUNC, MSIOF2_RXD),
	PINMUX_GFUNC_GPSR(GRP1_2_FUNC, STPWT_EXTFXR_B),

	PINMUX_GFUNC_GPSR(GRP1_1_FUNC, CAN0TX),
	PINMUX_GFUNC_GPSR(GRP1_1_FUNC, RLIN34TX_A),
	PINMUX_GFUNC_GPSR(GRP1_1_FUNC, MSIOF2_TXD),

	PINMUX_GFUNC_GPSR(GRP1_0_FUNC, CAN0RX_INTP0),
	PINMUX_GFUNC_GPSR(GRP1_0_FUNC, RLIN34RX_INTP20_A),
	PINMUX_GFUNC_GPSR(GRP1_0_FUNC, MSIOF2_SCK),

	/* Group2 Functions */
	PINMUX_GFUNC_GPSR(GRP2_28_FUNC, INTP34_B),

	PINMUX_GFUNC_GPSR(GRP2_27_FUNC, TAUD1O3),

	PINMUX_GFUNC_GPSR(GRP2_26_FUNC, TAUD1O2),

	PINMUX_GFUNC_GPSR(GRP2_25_FUNC, TAUD1O1),

	PINMUX_GFUNC_GPSR(GRP2_24_FUNC, TAUD1O0),

	PINMUX_GFUNC_GPSR(GRP2_23_FUNC, EXTCLK0O_B),

	PINMUX_GFUNC_GPSR(GRP2_22_FUNC, AVS1),

	PINMUX_GFUNC_GPSR(GRP2_21_FUNC, AVS0),

	PINMUX_GFUNC_MSEL_GPSR(GRP2_20_FUNC, SDA0, SEL_SDA0_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP2_19_FUNC, SCL0, SEL_SCL0_1),

	PINMUX_GFUNC_GPSR(GRP2_18_FUNC, INTP33_B),
	PINMUX_GFUNC_GPSR(GRP2_18_FUNC, TAUD0O1),

	PINMUX_GFUNC_GPSR(GRP2_17_FUNC, INTP32_B),
	PINMUX_GFUNC_GPSR(GRP2_17_FUNC, TAUD0O0),

	PINMUX_GFUNC_GPSR(GRP2_16_FUNC, CAN_CLK),

	PINMUX_GFUNC_GPSR(GRP2_15_FUNC, CAN15TX_B),
	PINMUX_GFUNC_GPSR(GRP2_15_FUNC, RLIN315TX_B),

	PINMUX_GFUNC_GPSR(GRP2_14_FUNC, CAN15RX_B_INTP15),
	PINMUX_GFUNC_GPSR(GRP2_14_FUNC, RLIN315RX_INTP31_B),

	PINMUX_GFUNC_GPSR(GRP2_13_FUNC, CAN14TX_B),
	PINMUX_GFUNC_GPSR(GRP2_13_FUNC, RLIN314TX_B),

	PINMUX_GFUNC_GPSR(GRP2_12_FUNC, CAN14RX_B_INTP14),
	PINMUX_GFUNC_GPSR(GRP2_12_FUNC, RLIN314RX_INTP30_B),

	PINMUX_GFUNC_GPSR(GRP2_11_FUNC, CAN13TX_B),
	PINMUX_GFUNC_GPSR(GRP2_11_FUNC, RLIN313TX),
	PINMUX_GFUNC_GPSR(GRP2_11_FUNC, CANXL1_TX),

	PINMUX_GFUNC_GPSR(GRP2_10_FUNC, CAN13RX_B_INTP13),
	PINMUX_GFUNC_GPSR(GRP2_10_FUNC, RLIN313RX_INTP29_B),
	PINMUX_GFUNC_GPSR(GRP2_10_FUNC, CANXL1_RX),

	PINMUX_GFUNC_GPSR(GRP2_9_FUNC, CAN12TX_B),
	PINMUX_GFUNC_GPSR(GRP2_9_FUNC, RLIN312TX),
	PINMUX_GFUNC_GPSR(GRP2_9_FUNC, TAUD1O7),
	PINMUX_GFUNC_GPSR(GRP2_9_FUNC, CANXL0_TX),

	PINMUX_GFUNC_GPSR(GRP2_8_FUNC, CAN12RX_B_INTP12),
	PINMUX_GFUNC_GPSR(GRP2_8_FUNC, RLIN312RX_INTP28_B),
	PINMUX_GFUNC_GPSR(GRP2_8_FUNC, TAUD1O6),
	PINMUX_GFUNC_GPSR(GRP2_8_FUNC, CANXL0_RX),

	PINMUX_GFUNC_GPSR(GRP2_7_FUNC, RLIN37TX_B),
	PINMUX_GFUNC_GPSR(GRP2_7_FUNC, RTCA0OUT_B),
	PINMUX_GFUNC_GPSR(GRP2_7_FUNC, TAUD1O5),

	PINMUX_GFUNC_GPSR(GRP2_6_FUNC, RLIN37RX_B_INTP23),
	PINMUX_GFUNC_GPSR(GRP2_6_FUNC, TAUD1O4),

	PINMUX_GFUNC_GPSR(GRP2_5_FUNC, RLIN36TX_B),
	PINMUX_GFUNC_GPSR(GRP2_5_FUNC, MSIOF1_SS2_B),

	PINMUX_GFUNC_GPSR(GRP2_4_FUNC, RLIN36RX_B_INTP22),
	PINMUX_GFUNC_GPSR(GRP2_4_FUNC, MSIOF1_SS1_B),
	PINMUX_GFUNC_GPSR(GRP2_4_FUNC, CTIACK),

	PINMUX_GFUNC_GPSR(GRP2_3_FUNC, RLIN35TX_B),
	PINMUX_GFUNC_GPSR(GRP2_3_FUNC, MSIOF1_SYN_B),
	PINMUX_GFUNC_GPSR(GRP2_3_FUNC, CTIREQ),

	PINMUX_GFUNC_GPSR(GRP2_2_FUNC, RLIN35RX_B_INTP21),
	PINMUX_GFUNC_GPSR(GRP2_2_FUNC, MSIOF1_RXD_B),

	PINMUX_GFUNC_GPSR(GRP2_1_FUNC, RLIN34TX_B),
	PINMUX_GFUNC_GPSR(GRP2_1_FUNC, MSIOF1_TXD_B),
	PINMUX_GFUNC_GPSR(GRP2_1_FUNC, TAUD0O15),

	PINMUX_GFUNC_GPSR(GRP2_0_FUNC, RLIN34RX_B_INTP20),
	PINMUX_GFUNC_GPSR(GRP2_0_FUNC, MSIOF1_SCK_B),
	PINMUX_GFUNC_GPSR(GRP2_0_FUNC, TAUD0O14),

	/* Group3 Functions */
	PINMUX_GFUNC_GPSR(GRP3_16_FUNC, ERRORIN0_N),

	PINMUX_GFUNC_GPSR(GRP3_15_FUNC, ERROROUT_N),

	PINMUX_GFUNC_GPSR(GRP3_14_FUNC, QSPI1_SSL),

	PINMUX_GFUNC_GPSR(GRP3_13_FUNC, QSPI1_IO3),

	PINMUX_GFUNC_GPSR(GRP3_12_FUNC, QSPI1_IO2),

	PINMUX_GFUNC_GPSR(GRP3_11_FUNC, QSPI1_MISO_IO1),

	PINMUX_GFUNC_GPSR(GRP3_10_FUNC, QSPI1_MOSI_IO0),

	PINMUX_GFUNC_GPSR(GRP3_9_FUNC, QSPI1_SPCLK),

	PINMUX_GFUNC_GPSR(GRP3_8_FUNC, RPC_INT_N),

	PINMUX_GFUNC_GPSR(GRP3_7_FUNC, RPC_WP_N),

	PINMUX_GFUNC_GPSR(GRP3_6_FUNC, RPC_RESET_N),

	PINMUX_GFUNC_GPSR(GRP3_5_FUNC, QSPI0_SSL),

	PINMUX_GFUNC_GPSR(GRP3_4_FUNC, QSPI0_IO3),

	PINMUX_GFUNC_GPSR(GRP3_3_FUNC, QSPI0_IO2),

	PINMUX_GFUNC_GPSR(GRP3_2_FUNC, QSPI0_MISO_IO1),

	PINMUX_GFUNC_GPSR(GRP3_1_FUNC, QSPI0_MOSI_IO0),

	PINMUX_GFUNC_GPSR(GRP3_0_FUNC, QSPI0_SPCLK),

	/* Group4 Functions */
	PINMUX_GFUNC_GPSR(GRP4_15_FUNC, PCIE61_CLKREQ_N),

	PINMUX_GFUNC_GPSR(GRP4_14_FUNC, PCIE60_CLKREQ_N),

	PINMUX_GFUNC_GPSR(GRP4_13_FUNC, ERRORIN1_N),

	PINMUX_GFUNC_GPSR(GRP4_12_FUNC, SD0_CD),

	PINMUX_GFUNC_GPSR(GRP4_11_FUNC, SD0_WP),

	PINMUX_GFUNC_GPSR(GRP4_10_FUNC, MMC0_DS),

	PINMUX_GFUNC_GPSR(GRP4_9_FUNC, MMC0_D7),

	PINMUX_GFUNC_GPSR(GRP4_8_FUNC, MMC0_D6),

	PINMUX_GFUNC_GPSR(GRP4_7_FUNC, MMC0_D5),

	PINMUX_GFUNC_GPSR(GRP4_6_FUNC, MMC0_D4),

	PINMUX_GFUNC_GPSR(GRP4_5_FUNC, MMC0_SD_D3),

	PINMUX_GFUNC_GPSR(GRP4_4_FUNC, MMC0_SD_D2),

	PINMUX_GFUNC_GPSR(GRP4_3_FUNC, MMC0_SD_D1),

	PINMUX_GFUNC_GPSR(GRP4_2_FUNC, MMC0_SD_D0),

	PINMUX_GFUNC_GPSR(GRP4_1_FUNC, MMC0_SD_CMD),

	PINMUX_GFUNC_GPSR(GRP4_0_FUNC, MMC0_SD_CLK),

	/* Group5 Functions */
	PINMUX_GFUNC_GPSR(GRP5_22_FUNC, TPU0TO3),
	PINMUX_GFUNC_GPSR(GRP5_22_FUNC, SSI9_WS),

	PINMUX_GFUNC_GPSR(GRP5_21_FUNC, TPU0TO2),
	PINMUX_GFUNC_GPSR(GRP5_21_FUNC, SSI9_SCK),

	PINMUX_GFUNC_GPSR(GRP5_20_FUNC, TPU0TO1),
	PINMUX_GFUNC_GPSR(GRP5_20_FUNC, PWM5),

	PINMUX_GFUNC_GPSR(GRP5_19_FUNC, TPU0TO0),
	PINMUX_GFUNC_GPSR(GRP5_19_FUNC, PWM4),

	PINMUX_GFUNC_GPSR(GRP5_18_FUNC, TCLK4),
	PINMUX_GFUNC_GPSR(GRP5_18_FUNC, PWM3),
	PINMUX_GFUNC_GPSR(GRP5_18_FUNC, SSI19_SD),

	PINMUX_GFUNC_GPSR(GRP5_17_FUNC, TCLK3),
	PINMUX_GFUNC_GPSR(GRP5_17_FUNC, PWM2),
	PINMUX_GFUNC_GPSR(GRP5_17_FUNC, SSI19_WS),

	PINMUX_GFUNC_GPSR(GRP5_16_FUNC, TCLK2),
	PINMUX_GFUNC_GPSR(GRP5_16_FUNC, PWM1),
	PINMUX_GFUNC_GPSR(GRP5_16_FUNC, SSI19_SCK),

	PINMUX_GFUNC_GPSR(GRP5_15_FUNC, TCLK1),
	PINMUX_GFUNC_GPSR(GRP5_15_FUNC, PWM0_A),
	PINMUX_GFUNC_GPSR(GRP5_15_FUNC, SSI18_SD),

	PINMUX_GFUNC_GPSR(GRP5_14_FUNC, IRQ3_A),
	PINMUX_GFUNC_GPSR(GRP5_14_FUNC, RIF7_D1),

	PINMUX_GFUNC_GPSR(GRP5_13_FUNC, IRQ2_A),
	PINMUX_GFUNC_GPSR(GRP5_13_FUNC, SSI17_SD),
	PINMUX_GFUNC_GPSR(GRP5_13_FUNC, RIF7_D0),

	PINMUX_GFUNC_GPSR(GRP5_12_FUNC, IRQ1_A),
	PINMUX_GFUNC_GPSR(GRP5_12_FUNC, SSI17_WS),
	PINMUX_GFUNC_GPSR(GRP5_12_FUNC, RIF7_SYNC),

	PINMUX_GFUNC_GPSR(GRP5_11_FUNC, IRQ0_A),
	PINMUX_GFUNC_GPSR(GRP5_11_FUNC, SSI17_SCK),
	PINMUX_GFUNC_GPSR(GRP5_11_FUNC, RIF7_CLK),

	PINMUX_GFUNC_GPSR(GRP5_10_FUNC, HSCK1),
	PINMUX_GFUNC_GPSR(GRP5_10_FUNC, SCK1),
	PINMUX_GFUNC_GPSR(GRP5_10_FUNC, SSI13_SCK),
	PINMUX_GFUNC_GPSR(GRP5_10_FUNC, RIF0_CLK_B),

	PINMUX_GFUNC_GPSR(GRP5_9_FUNC, HCTS1_N),
	PINMUX_GFUNC_GPSR(GRP5_9_FUNC, CTS1_N),

	PINMUX_GFUNC_GPSR(GRP5_8_FUNC, HRTS1_N),
	PINMUX_GFUNC_GPSR(GRP5_8_FUNC, RTS1_N),
	PINMUX_GFUNC_GPSR(GRP5_8_FUNC, RIF0_SYNC_B),
	PINMUX_GFUNC_GPSR(GRP5_8_FUNC, SSI16_SD),

	PINMUX_GFUNC_GPSR(GRP5_7_FUNC, HRX1),
	PINMUX_GFUNC_GPSR(GRP5_7_FUNC, RX1),
	PINMUX_GFUNC_GPSR(GRP5_7_FUNC, SSI16_WS),

	PINMUX_GFUNC_GPSR(GRP5_6_FUNC, HTX1),
	PINMUX_GFUNC_GPSR(GRP5_6_FUNC, TX1),
	PINMUX_GFUNC_GPSR(GRP5_6_FUNC, SSI16_SCK),

	PINMUX_GFUNC_GPSR(GRP5_5_FUNC, SCIF_CLK),

	PINMUX_GFUNC_GPSR(GRP5_4_FUNC, HSCK0),
	PINMUX_GFUNC_GPSR(GRP5_4_FUNC, SCK0),
	PINMUX_GFUNC_GPSR(GRP5_4_FUNC, SSI15_SD),

	PINMUX_GFUNC_GPSR(GRP5_3_FUNC, HCTS0_N),
	PINMUX_GFUNC_GPSR(GRP5_3_FUNC, CTS0_N),
	PINMUX_GFUNC_GPSR(GRP5_3_FUNC, IRQ1_B),
	PINMUX_GFUNC_GPSR(GRP5_3_FUNC, SSI15_WS),

	PINMUX_GFUNC_GPSR(GRP5_2_FUNC, HRTS0_N),
	PINMUX_GFUNC_GPSR(GRP5_2_FUNC, RTS0_N),
	PINMUX_GFUNC_GPSR(GRP5_2_FUNC, IRQ0_B),
	PINMUX_GFUNC_GPSR(GRP5_2_FUNC, SSI15_SCK),

	PINMUX_GFUNC_GPSR(GRP5_1_FUNC, HRX0),
	PINMUX_GFUNC_GPSR(GRP5_1_FUNC, RX0),
	PINMUX_GFUNC_GPSR(GRP5_1_FUNC, SSI13_SD),
	PINMUX_GFUNC_GPSR(GRP5_1_FUNC, RIF0_D1_B),

	PINMUX_GFUNC_GPSR(GRP5_0_FUNC, HTX0),
	PINMUX_GFUNC_GPSR(GRP5_0_FUNC, TX0),
	PINMUX_GFUNC_GPSR(GRP5_0_FUNC, SSI13_WS),
	PINMUX_GFUNC_GPSR(GRP5_0_FUNC, RIF0_D0_B),

	/* Group6 Functions */
	PINMUX_GFUNC_GPSR(GRP6_30_FUNC, AUDIO1_CLKOUT1),
	PINMUX_GFUNC_GPSR(GRP6_30_FUNC, MSIOF7_RXD_B),
	PINMUX_GFUNC_GPSR(GRP6_30_FUNC, RIF5_CLK),

	PINMUX_GFUNC_GPSR(GRP6_29_FUNC, AUDIO1_CLKOUT0),
	PINMUX_GFUNC_GPSR(GRP6_29_FUNC, MSIOF7_TXD_B),

	PINMUX_GFUNC_GPSR(GRP6_28_FUNC, SSI2_SD),
	PINMUX_GFUNC_GPSR(GRP6_28_FUNC, MSIOF7_SCK_B),
	PINMUX_GFUNC_GPSR(GRP6_28_FUNC, RIF5_SYNC),

	PINMUX_GFUNC_GPSR(GRP6_27_FUNC, SSI2_WS),
	PINMUX_GFUNC_GPSR(GRP6_27_FUNC, RIF1_D1_A),

	PINMUX_GFUNC_GPSR(GRP6_26_FUNC, SSI2_SCK),
	PINMUX_GFUNC_GPSR(GRP6_26_FUNC, RIF1_D0_A),

	PINMUX_GFUNC_GPSR(GRP6_25_FUNC, AUDIO0_CLKOUT3),
	PINMUX_GFUNC_GPSR(GRP6_25_FUNC, RIF1_CLK_A),

	PINMUX_GFUNC_GPSR(GRP6_24_FUNC, AUDIO0_CLKOUT2),
	PINMUX_GFUNC_GPSR(GRP6_24_FUNC, RIF2_D1),

	PINMUX_GFUNC_GPSR(GRP6_23_FUNC, SSI1_SD),
	PINMUX_GFUNC_GPSR(GRP6_23_FUNC, HCTS3_N),
	PINMUX_GFUNC_GPSR(GRP6_23_FUNC, CTS3_N),

	PINMUX_GFUNC_GPSR(GRP6_22_FUNC, SSI1_WS),
	PINMUX_GFUNC_GPSR(GRP6_22_FUNC, HRTS3_N),
	PINMUX_GFUNC_GPSR(GRP6_22_FUNC, RTS3_N),

	PINMUX_GFUNC_GPSR(GRP6_21_FUNC, SSI1_SCK),
	PINMUX_GFUNC_GPSR(GRP6_21_FUNC, MSIOF4_SS2_A),
	PINMUX_GFUNC_GPSR(GRP6_21_FUNC, HSCK3),
	PINMUX_GFUNC_GPSR(GRP6_21_FUNC, SCK3),

	PINMUX_GFUNC_GPSR(GRP6_20_FUNC, AUDIO0_CLKOUT1),
	PINMUX_GFUNC_GPSR(GRP6_20_FUNC, MSIOF4_SS1_A),
	PINMUX_GFUNC_GPSR(GRP6_20_FUNC, RIF2_D0),

	PINMUX_GFUNC_GPSR(GRP6_19_FUNC, AUDIO0_CLKOUT0),
	PINMUX_GFUNC_GPSR(GRP6_19_FUNC, MSIOF4_SYNC_A),
	PINMUX_GFUNC_GPSR(GRP6_19_FUNC, RIF2_SYNC),

	PINMUX_GFUNC_GPSR(GRP6_18_FUNC, SSI0_SD),
	PINMUX_GFUNC_GPSR(GRP6_18_FUNC, MSIOF4_RXD_A),
	PINMUX_GFUNC_GPSR(GRP6_18_FUNC, HRX3),
	PINMUX_GFUNC_GPSR(GRP6_18_FUNC, RX3),

	PINMUX_GFUNC_GPSR(GRP6_17_FUNC, SSI0_WS),
	PINMUX_GFUNC_GPSR(GRP6_17_FUNC, MSIOF4_TXD_A),
	PINMUX_GFUNC_GPSR(GRP6_17_FUNC, HTX3),
	PINMUX_GFUNC_GPSR(GRP6_17_FUNC, TX3),

	PINMUX_GFUNC_GPSR(GRP6_16_FUNC, SSI0_SCK),
	PINMUX_GFUNC_GPSR(GRP6_16_FUNC, MSIOF4_SCK_A),

	PINMUX_GFUNC_GPSR(GRP6_15_FUNC, MSIOF4_SS2_B),
	PINMUX_GFUNC_GPSR(GRP6_15_FUNC, SSI14_SD),

	PINMUX_GFUNC_GPSR(GRP6_14_FUNC, MSIOF4_SS1_B),
	PINMUX_GFUNC_GPSR(GRP6_14_FUNC, SSI12_SD),

	PINMUX_GFUNC_GPSR(GRP6_13_FUNC, MSIOF4_SYNC_B),
	PINMUX_GFUNC_GPSR(GRP6_13_FUNC, SSI12_WS),

	PINMUX_GFUNC_GPSR(GRP6_12_FUNC, MSIOF4_RXD_B),
	PINMUX_GFUNC_GPSR(GRP6_12_FUNC, AUDIO_CLKC_B),

	PINMUX_GFUNC_GPSR(GRP6_11_FUNC, MSIOF4_TXD_B),
	PINMUX_GFUNC_GPSR(GRP6_11_FUNC, SSI12_SCK),
	PINMUX_GFUNC_GPSR(GRP6_11_FUNC, RIF1_SYNC_A),

	PINMUX_GFUNC_GPSR(GRP6_10_FUNC, MSIOF4_SCK_B),
	PINMUX_GFUNC_GPSR(GRP6_10_FUNC, AUDIO_CLKB_B),

	PINMUX_GFUNC_GPSR(GRP6_9_FUNC, MSIOF7_SS2_A),
	PINMUX_GFUNC_GPSR(GRP6_9_FUNC, SSI14_WS),

	PINMUX_GFUNC_GPSR(GRP6_8_FUNC, MSIOF7_SS1_A),
	PINMUX_GFUNC_GPSR(GRP6_8_FUNC, SSI14_SCK),

	PINMUX_GFUNC_GPSR(GRP6_7_FUNC, MSIOF7_SYNC_A),
	PINMUX_GFUNC_GPSR(GRP6_7_FUNC, RIF1_D1_B),
	PINMUX_GFUNC_GPSR(GRP6_7_FUNC, SSI11_SD),

	PINMUX_GFUNC_GPSR(GRP6_6_FUNC, MSIOF7_RXD_A),
	PINMUX_GFUNC_GPSR(GRP6_6_FUNC, RIF1_D0_B),
	PINMUX_GFUNC_GPSR(GRP6_6_FUNC, SSI11_WS),

	PINMUX_GFUNC_GPSR(GRP6_5_FUNC, MSIOF7_TXD_A),
	PINMUX_GFUNC_GPSR(GRP6_5_FUNC, RIF1_SYNC_B),
	PINMUX_GFUNC_GPSR(GRP6_5_FUNC, SSI11_SCK),

	PINMUX_GFUNC_GPSR(GRP6_4_FUNC, MSIOF7_SCK_A),
	PINMUX_GFUNC_GPSR(GRP6_4_FUNC, RIF1_CLK_B),
	PINMUX_GFUNC_GPSR(GRP6_4_FUNC, AUDIO_CLKA_B),

	PINMUX_GFUNC_GPSR(GRP6_3_FUNC, RIF6_CLK),
	PINMUX_GFUNC_GPSR(GRP6_3_FUNC, SSI10_SD),

	PINMUX_GFUNC_GPSR(GRP6_2_FUNC, RIF6_SYNC),
	PINMUX_GFUNC_GPSR(GRP6_2_FUNC, SSI10_WS),

	PINMUX_GFUNC_GPSR(GRP6_1_FUNC, RIF6_D1),
	PINMUX_GFUNC_GPSR(GRP6_1_FUNC, SSI10_SCK),

	PINMUX_GFUNC_GPSR(GRP6_0_FUNC, RIF6_D0),
	PINMUX_GFUNC_GPSR(GRP6_0_FUNC, SSI9_SD),

	/* Group7 Functions */
	PINMUX_GFUNC_GPSR(GRP7_30_FUNC, MSIOF6_SS2_B),
	PINMUX_GFUNC_GPSR(GRP7_30_FUNC, HRX2_B),
	PINMUX_GFUNC_GPSR(GRP7_30_FUNC, RX4_B),

	PINMUX_GFUNC_GPSR(GRP7_29_FUNC, MSIOF6_SS1_B),
	PINMUX_GFUNC_GPSR(GRP7_29_FUNC, SSI7_SD),

	PINMUX_GFUNC_GPSR(GRP7_28_FUNC, MSIOF6_SYNC_B),
	PINMUX_GFUNC_GPSR(GRP7_28_FUNC, SSI7_WS),

	PINMUX_GFUNC_GPSR(GRP7_27_FUNC, MSIOF6_RXD_B),
	PINMUX_GFUNC_GPSR(GRP7_27_FUNC, SSI7_SCK),

	PINMUX_GFUNC_GPSR(GRP7_26_FUNC, MSIOF6_TXD_B),
	PINMUX_GFUNC_GPSR(GRP7_26_FUNC, HTX2_B),
	PINMUX_GFUNC_GPSR(GRP7_26_FUNC, TX4_B),

	PINMUX_GFUNC_GPSR(GRP7_25_FUNC, MSIOF6_SCK_B),
	PINMUX_GFUNC_GPSR(GRP7_25_FUNC, SSI8_SD),

	PINMUX_GFUNC_GPSR(GRP7_24_FUNC, MSIOF5_SS2),
	PINMUX_GFUNC_GPSR(GRP7_24_FUNC, HCTS2_N_B),
	PINMUX_GFUNC_GPSR(GRP7_24_FUNC, CTS4_N_B),

	PINMUX_GFUNC_GPSR(GRP7_23_FUNC, MSIOF5_SS1),
	PINMUX_GFUNC_GPSR(GRP7_23_FUNC, RIF0_SYNC_A),

	PINMUX_GFUNC_GPSR(GRP7_22_FUNC, MSIOF5_SYNC),
	PINMUX_GFUNC_GPSR(GRP7_22_FUNC, HRTS2_N_B),
	PINMUX_GFUNC_GPSR(GRP7_22_FUNC, RTS4_N_B),

	PINMUX_GFUNC_GPSR(GRP7_21_FUNC, MSIOF5_RXD),
	PINMUX_GFUNC_GPSR(GRP7_21_FUNC, RIF0_D1_A),

	PINMUX_GFUNC_GPSR(GRP7_20_FUNC, MSIOF5_TXD),
	PINMUX_GFUNC_GPSR(GRP7_20_FUNC, HSCK2_B),
	PINMUX_GFUNC_GPSR(GRP7_20_FUNC, SCK4_B),

	PINMUX_GFUNC_GPSR(GRP7_19_FUNC, MSIOF6_SS2_A),

	PINMUX_GFUNC_GPSR(GRP7_18_FUNC, MSIOF6_SS1_A),

	PINMUX_GFUNC_GPSR(GRP7_17_FUNC, MSIOF5_SCK),

	PINMUX_GFUNC_GPSR(GRP7_16_FUNC, AUDIO_CLKC_A),

	PINMUX_GFUNC_GPSR(GRP7_15_FUNC, SSI6_SD),
	PINMUX_GFUNC_GPSR(GRP7_15_FUNC, MSIOF6_RXD_A),
	PINMUX_GFUNC_GPSR(GRP7_15_FUNC, RIF4_D1),

	PINMUX_GFUNC_GPSR(GRP7_14_FUNC, SSI6_WS),
	PINMUX_GFUNC_GPSR(GRP7_14_FUNC, MSIOF6_TXD_A),
	PINMUX_GFUNC_GPSR(GRP7_14_FUNC, RIF4_D0),

	PINMUX_GFUNC_GPSR(GRP7_13_FUNC, SSI6_SCK),
	PINMUX_GFUNC_GPSR(GRP7_13_FUNC, MSIOF6_SCK_A),
	PINMUX_GFUNC_GPSR(GRP7_13_FUNC, RIF4_SYNC),

	PINMUX_GFUNC_GPSR(GRP7_12_FUNC, AUDIO_CLKB_A),

	PINMUX_GFUNC_GPSR(GRP7_11_FUNC, SSI5_SD),
	PINMUX_GFUNC_GPSR(GRP7_11_FUNC, MSIOF6_SYNC_A),
	PINMUX_GFUNC_GPSR(GRP7_11_FUNC, RIF4_CLK),

	PINMUX_GFUNC_GPSR(GRP7_10_FUNC, SSI5_WS),
	PINMUX_GFUNC_GPSR(GRP7_10_FUNC, RIF3_SYNC),

	PINMUX_GFUNC_GPSR(GRP7_9_FUNC, SSI5_SCK),
	PINMUX_GFUNC_GPSR(GRP7_9_FUNC, RIF3_CLK),

	PINMUX_GFUNC_GPSR(GRP7_8_FUNC, AUDIO_CLKA_A),

	PINMUX_GFUNC_GPSR(GRP7_7_FUNC, SSI4_SD),
	PINMUX_GFUNC_GPSR(GRP7_7_FUNC, RIF3_D1),

	PINMUX_GFUNC_GPSR(GRP7_6_FUNC, SSI4_WS),
	PINMUX_GFUNC_GPSR(GRP7_6_FUNC, RIF3_D0),

	PINMUX_GFUNC_GPSR(GRP7_5_FUNC, SSI4_SCK),
	PINMUX_GFUNC_GPSR(GRP7_5_FUNC, RIF2_CLK),

	PINMUX_GFUNC_GPSR(GRP7_4_FUNC, AUDIO1_CLKOUT3),
	PINMUX_GFUNC_GPSR(GRP7_4_FUNC, RIF0_D0_A),

	PINMUX_GFUNC_GPSR(GRP7_3_FUNC, AUDIO1_CLKOUT2),
	PINMUX_GFUNC_GPSR(GRP7_3_FUNC, RIF0_CLK_A),

	PINMUX_GFUNC_GPSR(GRP7_2_FUNC, SSI3_SD),
	PINMUX_GFUNC_GPSR(GRP7_2_FUNC, MSIOF7_SS2_B),

	PINMUX_GFUNC_GPSR(GRP7_1_FUNC, SSI3_WS),
	PINMUX_GFUNC_GPSR(GRP7_1_FUNC, MSIOF7_SS1_B),
	PINMUX_GFUNC_GPSR(GRP7_1_FUNC, RIF5_D1),

	PINMUX_GFUNC_GPSR(GRP7_0_FUNC, SSI3_SCK),
	PINMUX_GFUNC_GPSR(GRP7_0_FUNC, MSIOF7_SYNC_B),
	PINMUX_GFUNC_GPSR(GRP7_0_FUNC, RIF5_D0),

	/* Group8 Functions */
	PINMUX_GFUNC_GPSR(GRP8_31_FUNC, S3DA2),

	PINMUX_GFUNC_GPSR(GRP8_30_FUNC, S3CL2),

	PINMUX_GFUNC_GPSR(GRP8_29_FUNC, S3DA1),

	PINMUX_GFUNC_GPSR(GRP8_28_FUNC, S3CL1),

	PINMUX_GFUNC_GPSR(GRP8_27_FUNC, S3DA0),

	PINMUX_GFUNC_GPSR(GRP8_26_FUNC, S3CL0),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_15_FUNC, SDA8, SEL_SDA8_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_14_FUNC, SCL8, SEL_SCL8_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_13_FUNC, SDA7, SEL_SDA7_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_12_FUNC, SCL7, SEL_SCL7_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_11_FUNC, SDA6, SEL_SDA6_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_10_FUNC, SCL6, SEL_SCL6_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_9_FUNC,  SDA5, SEL_SDA5_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_8_FUNC,  SCL5, SEL_SCL5_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_7_FUNC,  SDA4, SEL_SDA4_1),
	PINMUX_GFUNC_GPSR(GRP8_7_FUNC, HCTS2_N_A),
	PINMUX_GFUNC_GPSR(GRP8_7_FUNC, CTS4_N_A),
	PINMUX_GFUNC_GPSR(GRP8_7_FUNC, PWM7_B),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_6_FUNC,  SCL4, SEL_SCL4_1),
	PINMUX_GFUNC_GPSR(GRP8_6_FUNC, HRTS2_N_A),
	PINMUX_GFUNC_GPSR(GRP8_6_FUNC, RTS4_N_A),
	PINMUX_GFUNC_GPSR(GRP8_6_FUNC, PWM9_B),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_5_FUNC,  SDA3, SEL_SDA3_1),
	PINMUX_GFUNC_GPSR(GRP8_5_FUNC, HRX2_A),
	PINMUX_GFUNC_GPSR(GRP8_5_FUNC, RX4_A),
	PINMUX_GFUNC_GPSR(GRP8_5_FUNC, PWM8_B),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_4_FUNC,  SCL3, SEL_SCL3_1),
	PINMUX_GFUNC_GPSR(GRP8_4_FUNC, HTX2_A),
	PINMUX_GFUNC_GPSR(GRP8_4_FUNC, TX4_A),
	PINMUX_GFUNC_GPSR(GRP8_4_FUNC, PWM6_B),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_3_FUNC,  SDA2, SEL_SDA2_1),
	PINMUX_GFUNC_GPSR(GRP8_3_FUNC, HSCK2_A),
	PINMUX_GFUNC_GPSR(GRP8_3_FUNC, SCK4_A),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_2_FUNC,  SCL2, SEL_SCL2_1),
	PINMUX_GFUNC_GPSR(GRP8_2_FUNC, PWM0_B),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_1_FUNC,  SDA1, SEL_SDA1_1),

	PINMUX_GFUNC_MSEL_GPSR(GRP8_0_FUNC,  SCL1, SEL_SCL1_1),

	/* Group9 Functions */
	PINMUX_GFUNC_GPSR(GRP9_16_FUNC, RSW3_MATCH),
	PINMUX_GFUNC_GPSR(GRP9_16_FUNC, PWM9_A),

	PINMUX_GFUNC_GPSR(GRP9_15_FUNC, RSW3_CAPTURE),
	PINMUX_GFUNC_GPSR(GRP9_15_FUNC, PWM8_A),

	PINMUX_GFUNC_GPSR(GRP9_14_FUNC, RSW3_PPS),

	PINMUX_GFUNC_GPSR(GRP9_13_FUNC, ETH10G0_PHYINT),
	PINMUX_GFUNC_GPSR(GRP9_13_FUNC, ETH10G1_PHYINT),

	PINMUX_GFUNC_GPSR(GRP9_12_FUNC, ETH10G0_LINK),
	PINMUX_GFUNC_GPSR(GRP9_12_FUNC, ETH10G1_LINK),
	PINMUX_GFUNC_GPSR(GRP9_12_FUNC, PWM7_A),

	PINMUX_GFUNC_GPSR(GRP9_11_FUNC, ETH10G0_MDC),
	PINMUX_GFUNC_GPSR(GRP9_11_FUNC, ETH10G1_MDC),
	PINMUX_GFUNC_GPSR(GRP9_11_FUNC, IRQ3_B),

	PINMUX_GFUNC_GPSR(GRP9_10_FUNC, ETH10G0_MDIO),
	PINMUX_GFUNC_GPSR(GRP9_10_FUNC, ETH10G1_MDIO),
	PINMUX_GFUNC_GPSR(GRP9_10_FUNC, IRQ2_B),

	PINMUX_GFUNC_GPSR(GRP9_9_FUNC, ETH25G0_PHYINT),
	PINMUX_GFUNC_GPSR(GRP9_9_FUNC, ETH25G1_PHYINT),
	PINMUX_GFUNC_GPSR(GRP9_9_FUNC, ETH25G2_PHYINT),

	PINMUX_GFUNC_GPSR(GRP9_8_FUNC, ETH25G0_LINK),
	PINMUX_GFUNC_GPSR(GRP9_8_FUNC, ETH25G1_LINK),
	PINMUX_GFUNC_GPSR(GRP9_8_FUNC, ETH25G2_LINK),
	PINMUX_GFUNC_GPSR(GRP9_8_FUNC, PWM6_A),

	PINMUX_GFUNC_GPSR(GRP9_7_FUNC, ETH25G0_MDC),
	PINMUX_GFUNC_GPSR(GRP9_7_FUNC, ETH25G1_MDC),
	PINMUX_GFUNC_GPSR(GRP9_7_FUNC, ETH25G2_MDC),

	PINMUX_GFUNC_GPSR(GRP9_6_FUNC, ETH25G0_MDIO),
	PINMUX_GFUNC_GPSR(GRP9_6_FUNC, ETH25G1_MDIO),
	PINMUX_GFUNC_GPSR(GRP9_6_FUNC, ETH25G2_MDIO),

	PINMUX_GFUNC_GPSR(GRP9_5_FUNC, ETHES4_MATCH),
	PINMUX_GFUNC_GPSR(GRP9_5_FUNC, ETHES5_MATCH),
	PINMUX_GFUNC_GPSR(GRP9_5_FUNC, ETHES6_MATCH),
	PINMUX_GFUNC_GPSR(GRP9_5_FUNC, ETHES7_MATCH),

	PINMUX_GFUNC_GPSR(GRP9_4_FUNC, ETHES4_CAPTURE),
	PINMUX_GFUNC_GPSR(GRP9_4_FUNC, ETHES5_CAPTURE),
	PINMUX_GFUNC_GPSR(GRP9_4_FUNC, ETHES6_CAPTURE),
	PINMUX_GFUNC_GPSR(GRP9_4_FUNC, ETHES7_CAPTURE),

	PINMUX_GFUNC_GPSR(GRP9_3_FUNC, ETHES4_PPS),
	PINMUX_GFUNC_GPSR(GRP9_3_FUNC, ETHES5_PPS),
	PINMUX_GFUNC_GPSR(GRP9_3_FUNC, ETHES6_PPS),
	PINMUX_GFUNC_GPSR(GRP9_3_FUNC, ETHES7_PPS),

	PINMUX_GFUNC_GPSR(GRP9_2_FUNC, ETHES0_MATCH),
	PINMUX_GFUNC_GPSR(GRP9_2_FUNC, ETHES1_MATCH),
	PINMUX_GFUNC_GPSR(GRP9_2_FUNC, ETHES2_MATCH),
	PINMUX_GFUNC_GPSR(GRP9_2_FUNC, ETHES3_MATCH),

	PINMUX_GFUNC_GPSR(GRP9_1_FUNC, ETHES0_CAPTURE),
	PINMUX_GFUNC_GPSR(GRP9_1_FUNC, ETHES1_CAPTURE),
	PINMUX_GFUNC_GPSR(GRP9_1_FUNC, ETHES2_CAPTURE),
	PINMUX_GFUNC_GPSR(GRP9_1_FUNC, ETHES3_CAPTURE),

	PINMUX_GFUNC_GPSR(GRP9_0_FUNC, ETHES0_PPS),
	PINMUX_GFUNC_GPSR(GRP9_0_FUNC, ETHES1_PPS),
	PINMUX_GFUNC_GPSR(GRP9_0_FUNC, ETHES2_PPS),
	PINMUX_GFUNC_GPSR(GRP9_0_FUNC, ETHES3_PPS),

	/* Group10 Functions */
	PINMUX_GFUNC_GPSR(GRP10_13_FUNC, PCIE41_CLKREQ_N),

	PINMUX_GFUNC_GPSR(GRP10_12_FUNC, PCIE40_CLKREQ_N),

	PINMUX_GFUNC_GPSR(GRP10_11_FUNC, USB3_VBUS_VALID),

	PINMUX_GFUNC_GPSR(GRP10_10_FUNC, USB3_OVC),

	PINMUX_GFUNC_GPSR(GRP10_9_FUNC, USB3_PWEN),

	PINMUX_GFUNC_GPSR(GRP10_8_FUNC, USB2_VBUS_VALID),

	PINMUX_GFUNC_GPSR(GRP10_7_FUNC, USB2_OVC),

	PINMUX_GFUNC_GPSR(GRP10_6_FUNC, USB2_PWEN),

	PINMUX_GFUNC_GPSR(GRP10_5_FUNC, USB1_VBUS_VALID),

	PINMUX_GFUNC_GPSR(GRP10_4_FUNC, USB1_OVC),

	PINMUX_GFUNC_GPSR(GRP10_3_FUNC, USB1_PWEN),

	PINMUX_GFUNC_GPSR(GRP10_2_FUNC, USB0_VBUS_VALID),

	PINMUX_GFUNC_GPSR(GRP10_1_FUNC, USB0_OVC),

	PINMUX_GFUNC_GPSR(GRP10_0_FUNC, USB0_PWEN),

};

/*
 * Pins not associated with a GPIO port.
 * TODO: Define for None GPIO pins.
 */
enum {
	GP_ASSIGN_LAST(),
	//NOGP_ALL(),
};

static const struct sh_pfc_pin pinmux_pins[] = {
	PINMUX_GPIO_GP_ALL(),
	// TODO: Define for None GPIO pins.
	//PINMUX_NOGP_ALL(),
};

/* - HSCIF0 ----------------------------------------------------------------- */
static const unsigned int hscif0_data_pins[] = {
	/* HRX0, HTX0 */
	RCAR_GP_PIN(5, 1), RCAR_GP_PIN(5, 0),
};
static const unsigned int hscif0_data_mux[] = {
	HRX0_MARK, HTX0_MARK,
};
static const unsigned int hscif0_clk_pins[] = {
	/* HSCK0 */
	RCAR_GP_PIN(5, 4),
};
static const unsigned int hscif0_clk_mux[] = {
	HSCK0_MARK,
};
static const unsigned int hscif0_ctrl_pins[] = {
	/* HRTS0_N, HCTS0_N */
	RCAR_GP_PIN(5, 2), RCAR_GP_PIN(5, 3),
};
static const unsigned int hscif0_ctrl_mux[] = {
	HRTS0_N_MARK, HCTS0_N_MARK,
};

/* - HSCIF1 ----------------------------------------------------------------- */
static const unsigned int hscif1_data_pins[] = {
	/* HRX1, HTX1 */
	RCAR_GP_PIN(5, 7), RCAR_GP_PIN(5, 6),
};
static const unsigned int hscif1_data_mux[] = {
	HRX1_MARK, HTX1_MARK,
};
static const unsigned int hscif1_clk_pins[] = {
	/* HSCK1 */
	RCAR_GP_PIN(5, 10),
};
static const unsigned int hscif1_clk_mux[] = {
	HSCK1_MARK,
};
static const unsigned int hscif1_ctrl_pins[] = {
	/* HRTS1_N, HCTS1_N */
	RCAR_GP_PIN(5, 8), RCAR_GP_PIN(5, 9),
};
static const unsigned int hscif1_ctrl_mux[] = {
	HRTS1_N_MARK, HCTS1_N_MARK,
};

/* - HSCIF2 ----------------------------------------------------------------- */
static const unsigned int hscif2_data_a_pins[] = {
	/* HRX2_A, HTX2_A */
	RCAR_GP_PIN(8, 5), RCAR_GP_PIN(8, 4),
};
static const unsigned int hscif2_data_a_mux[] = {
	HRX2_A_MARK, HTX2_A_MARK,
};
static const unsigned int hscif2_clk_a_pins[] = {
	/* HSCK2_A */
	RCAR_GP_PIN(8, 3),
};
static const unsigned int hscif2_clk_a_mux[] = {
	HSCK2_A_MARK,
};
static const unsigned int hscif2_ctrl_a_pins[] = {
	/* HRTS2_A_N, HCTS2_A_N */
	RCAR_GP_PIN(8, 6), RCAR_GP_PIN(8, 7),
};
static const unsigned int hscif2_ctrl_a_mux[] = {
	HRTS2_N_A_MARK, HCTS2_N_A_MARK,
};

static const unsigned int hscif2_data_b_pins[] = {
	/* HRX2_B, HTX2_B */
	RCAR_GP_PIN(7, 30), RCAR_GP_PIN(7, 26),
};
static const unsigned int hscif2_data_b_mux[] = {
	HRX2_B_MARK, HTX2_B_MARK,
};
static const unsigned int hscif2_clk_b_pins[] = {
	/* HSCK2_B */
	RCAR_GP_PIN(7, 20),
};
static const unsigned int hscif2_clk_b_mux[] = {
	HSCK2_B_MARK,
};
static const unsigned int hscif2_ctrl_b_pins[] = {
	/* HRTS2_B_N, HCTS2_B_N */
	RCAR_GP_PIN(7, 22), RCAR_GP_PIN(7, 24),
};
static const unsigned int hscif2_ctrl_b_mux[] = {
	HRTS2_N_B_MARK, HCTS2_N_B_MARK,
};

/* - HSCIF3 ----------------------------------------------------------------- */
static const unsigned int hscif3_data_pins[] = {
	/* HRX3, HTX3 */
	RCAR_GP_PIN(6, 18), RCAR_GP_PIN(8, 17),
};
static const unsigned int hscif3_data_mux[] = {
	HRX3_MARK, HTX3_MARK,
};
static const unsigned int hscif3_clk_pins[] = {
	/* HSCK3 */
	RCAR_GP_PIN(6, 21),
};
static const unsigned int hscif3_clk_mux[] = {
	HSCK3_MARK,
};
static const unsigned int hscif3_ctrl_pins[] = {
	/* HRTS3_N, HCTS3_N */
	RCAR_GP_PIN(6, 22), RCAR_GP_PIN(6, 23),
};
static const unsigned int hscif3_ctrl_mux[] = {
	HRTS3_N_MARK, HCTS3_N_MARK,
};

/* - SCIF0 ----------------------------------------------------------------- */
static const unsigned int scif0_data_pins[] = {
	/* RX0, TX0 */
	RCAR_GP_PIN(5, 1), RCAR_GP_PIN(5, 0),
};
static const unsigned int scif0_data_mux[] = {
	RX0_MARK, TX0_MARK,
};
static const unsigned int scif0_clk_pins[] = {
	/* SCK0 */
	RCAR_GP_PIN(5, 4),
};
static const unsigned int scif0_clk_mux[] = {
	SCK0_MARK,
};
static const unsigned int scif0_ctrl_pins[] = {
	/* RTS0_N, CTS0_N */
	RCAR_GP_PIN(5, 2), RCAR_GP_PIN(5, 3),
};
static const unsigned int scif0_ctrl_mux[] = {
	RTS0_N_MARK, CTS0_N_MARK,
};

/* - SCIF1 ----------------------------------------------------------------- */
static const unsigned int scif1_data_pins[] = {
	/* RX1, TX1 */
	RCAR_GP_PIN(5, 7), RCAR_GP_PIN(5, 6),
};
static const unsigned int scif1_data_mux[] = {
	RX1_MARK, TX1_MARK,
};
static const unsigned int scif1_clk_pins[] = {
	/* SCK1 */
	RCAR_GP_PIN(5, 10),
};
static const unsigned int scif1_clk_mux[] = {
	SCK1_MARK,
};
static const unsigned int scif1_ctrl_pins[] = {
	/* RTS1_N, CTS1_N */
	RCAR_GP_PIN(5, 8), RCAR_GP_PIN(5, 9),
};
static const unsigned int scif1_ctrl_mux[] = {
	RTS1_N_MARK, CTS1_N_MARK,
};

/* - SCIF3 ----------------------------------------------------------------- */
static const unsigned int scif3_data_pins[] = {
	/* RX3, TX3 */
	RCAR_GP_PIN(6, 18), RCAR_GP_PIN(8, 17),
};
static const unsigned int scif3_data_mux[] = {
	RX3_MARK, TX3_MARK,
};
static const unsigned int scif3_clk_pins[] = {
	/* SCK3 */
	RCAR_GP_PIN(6, 21),
};
static const unsigned int scif3_clk_mux[] = {
	SCK3_MARK,
};
static const unsigned int scif3_ctrl_pins[] = {
	/* RTS3_N, CTS3_N */
	RCAR_GP_PIN(6, 22), RCAR_GP_PIN(6, 23),
};
static const unsigned int scif3_ctrl_mux[] = {
	RTS3_N_MARK, CTS3_N_MARK,
};

/* - SCIF4 ----------------------------------------------------------------- */
static const unsigned int scif4_data_a_pins[] = {
	/* RX4_A, TX4_A */
	RCAR_GP_PIN(8, 5), RCAR_GP_PIN(8, 4),
};
static const unsigned int scif4_data_a_mux[] = {
	RX4_A_MARK, TX4_A_MARK,
};
static const unsigned int scif4_clk_a_pins[] = {
	/* SCK4_A */
	RCAR_GP_PIN(8, 3),
};
static const unsigned int scif4_clk_a_mux[] = {
	SCK4_A_MARK,
};
static const unsigned int scif4_ctrl_a_pins[] = {
	/* RTS4_A_N, CTS4_A_N */
	RCAR_GP_PIN(8, 6), RCAR_GP_PIN(8, 7),
};
static const unsigned int scif4_ctrl_a_mux[] = {
	RTS4_N_A_MARK, CTS4_N_A_MARK,
};

static const unsigned int scif4_data_b_pins[] = {
	/* RX4_B, TX4_B */
	RCAR_GP_PIN(7, 30), RCAR_GP_PIN(7, 26),
};
static const unsigned int scif4_data_b_mux[] = {
	RX4_B_MARK, TX4_B_MARK,
};
static const unsigned int scif4_clk_b_pins[] = {
	/* SCK4_B */
	RCAR_GP_PIN(7, 20),
};
static const unsigned int scif4_clk_b_mux[] = {
	SCK4_B_MARK,
};
static const unsigned int scif4_ctrl_b_pins[] = {
	/* RTS4_B_N, CTS4_B_N */
	RCAR_GP_PIN(7, 22), RCAR_GP_PIN(7, 24),
};
static const unsigned int scif4_ctrl_b_mux[] = {
	RTS4_N_B_MARK, CTS4_N_B_MARK,
};

/* - SCIF Clock ------------------------------------------------------------- */
static const unsigned int scif_clk_pins[] = {
	/* SCIF_CLK */
	RCAR_GP_PIN(5, 5),
};
static const unsigned int scif_clk_mux[] = {
	SCIF_CLK_MARK,
};

/* - I2C0 ------------------------------------------------------------------- */
static const unsigned int i2c0_pins[] = {
	/* SDA0, SCL0 */
	RCAR_GP_PIN(2, 20), RCAR_GP_PIN(2, 19),
};
static const unsigned int i2c0_mux[] = {
	SDA0_MARK, SCL0_MARK,
};

/* - I2C1 ------------------------------------------------------------------- */
static const unsigned int i2c1_pins[] = {
	/* SDA1, SCL1 */
	RCAR_GP_PIN(8, 1), RCAR_GP_PIN(8, 0),
};
static const unsigned int i2c1_mux[] = {
	SDA1_MARK, SCL1_MARK,
};

/* - I2C2 ------------------------------------------------------------------- */
static const unsigned int i2c2_pins[] = {
	/* SDA2, SCL2 */
	RCAR_GP_PIN(8, 3), RCAR_GP_PIN(8, 2),
};
static const unsigned int i2c2_mux[] = {
	SDA2_MARK, SCL2_MARK,
};

/* - I2C3 ------------------------------------------------------------------- */
static const unsigned int i2c3_pins[] = {
	/* SDA3, SCL3 */
	RCAR_GP_PIN(8, 5), RCAR_GP_PIN(8, 4),
};
static const unsigned int i2c3_mux[] = {
	SDA3_MARK, SCL3_MARK,
};

/* - I2C4 ------------------------------------------------------------------- */
static const unsigned int i2c4_pins[] = {
	/* SDA4, SCL4 */
	RCAR_GP_PIN(8, 7), RCAR_GP_PIN(8, 6),
};
static const unsigned int i2c4_mux[] = {
	SDA4_MARK, SCL4_MARK,
};

/* - I2C5 ------------------------------------------------------------------- */
static const unsigned int i2c5_pins[] = {
	/* SDA5, SCL5 */
	RCAR_GP_PIN(8, 9), RCAR_GP_PIN(8, 8),
};
static const unsigned int i2c5_mux[] = {
	SDA5_MARK, SCL5_MARK,
};

/* - I2C6 ------------------------------------------------------------------- */
static const unsigned int i2c6_pins[] = {
	/* SDA6, SCL6 */
	RCAR_GP_PIN(8, 11), RCAR_GP_PIN(8, 10),
};
static const unsigned int i2c6_mux[] = {
	SDA6_MARK, SCL6_MARK,
};

/* - I2C7 ------------------------------------------------------------------- */
static const unsigned int i2c7_pins[] = {
	/* SDA7, SCL7 */
	RCAR_GP_PIN(8, 13), RCAR_GP_PIN(8, 12),
};
static const unsigned int i2c7_mux[] = {
	SDA7_MARK, SCL7_MARK,
};

/* - I2C8 ------------------------------------------------------------------- */
static const unsigned int i2c8_pins[] = {
	/* SDA8, SCL8 */
	RCAR_GP_PIN(8, 15), RCAR_GP_PIN(8, 14),
};
static const unsigned int i2c8_mux[] = {
	SDA8_MARK, SCL8_MARK,
};

/* - INTC-EX ---------------------------------------------------------------- */
static const unsigned int intc_ex_irq0_pins[] = {
	/* IRQ0_A, IRQ0_B */
	RCAR_GP_PIN(5, 11), RCAR_GP_PIN(5, 2),
};
static const unsigned int intc_ex_irq0_mux[] = {
	IRQ0_A_MARK, IRQ0_B_MARK,
};
static const unsigned int intc_ex_irq1_pins[] = {
	/* IRQ1_A, IRQ1_B */
	RCAR_GP_PIN(5, 12), RCAR_GP_PIN(5, 3),
};
static const unsigned int intc_ex_irq1_mux[] = {
	IRQ1_A_MARK, IRQ1_B_MARK,
};
static const unsigned int intc_ex_irq2_pins[] = {
	/* IRQ2_A, IRQ2_B */
	RCAR_GP_PIN(5, 13), RCAR_GP_PIN(9, 10),
};
static const unsigned int intc_ex_irq2_mux[] = {
	IRQ2_A_MARK, IRQ2_B_MARK,
};
static const unsigned int intc_ex_irq3_pins[] = {
	/* IRQ3_A, IRQ3_B */
	RCAR_GP_PIN(5, 14), RCAR_GP_PIN(9, 11),
};
static const unsigned int intc_ex_irq3_mux[] = {
	IRQ3_A_MARK, IRQ3_B_MARK,
};

/* - PCIE4 ------------------------------------------------------------------- */
static const unsigned int pcie40_clkreq_n_pins[] = {
	/* PCIE40_CLKREQ_N */
	RCAR_GP_PIN(10, 12),
};

static const unsigned int pcie40_clkreq_n_mux[] = {
	PCIE40_CLKREQ_N_MARK,
};

static const unsigned int pcie41_clkreq_n_pins[] = {
	/* PCIE41_CLKREQ_N */
	RCAR_GP_PIN(10, 13),
};

static const unsigned int pcie41_clkreq_n_mux[] = {
	PCIE41_CLKREQ_N_MARK,
};

/* - PCIE6 ------------------------------------------------------------------- */
static const unsigned int pcie60_clkreq_n_pins[] = {
	/* PCIE60_CLKREQ_N */
	RCAR_GP_PIN(4, 14),
};

static const unsigned int pcie60_clkreq_n_mux[] = {
	PCIE60_CLKREQ_N_MARK,
};

static const unsigned int pcie61_clkreq_n_pins[] = {
	/* PCIE61_CLKREQ_N */
	RCAR_GP_PIN(4, 15),
};

static const unsigned int pcie61_clkreq_n_mux[] = {
	PCIE61_CLKREQ_N_MARK,
};

/* - QSPI0 ------------------------------------------------------------------ */
static const unsigned int qspi0_ctrl_pins[] = {
	/* SPCLK, SSL */
	RCAR_GP_PIN(3, 0), RCAR_GP_PIN(3, 5),
};
static const unsigned int qspi0_ctrl_mux[] = {
	QSPI0_SPCLK_MARK, QSPI0_SSL_MARK,
};
static const unsigned int qspi0_data_pins[] = {
	/* MOSI_IO0, MISO_IO1, IO2, IO3 */
	RCAR_GP_PIN(3, 1), RCAR_GP_PIN(3, 2),
	RCAR_GP_PIN(3, 3), RCAR_GP_PIN(3, 4),
};
static const unsigned int qspi0_data_mux[] = {
	QSPI0_MOSI_IO0_MARK, QSPI0_MISO_IO1_MARK,
	QSPI0_IO2_MARK, QSPI0_IO3_MARK,
};

/* - QSPI1 ------------------------------------------------------------------ */
static const unsigned int qspi1_ctrl_pins[] = {
	/* SPCLK, SSL */
	RCAR_GP_PIN(3, 9), RCAR_GP_PIN(3, 14),
};
static const unsigned int qspi1_ctrl_mux[] = {
	QSPI1_SPCLK_MARK, QSPI1_SSL_MARK,
};
static const unsigned int qspi1_data_pins[] = {
	/* MOSI_IO0, MISO_IO1, IO2, IO3 */
	RCAR_GP_PIN(3, 10), RCAR_GP_PIN(3, 11),
	RCAR_GP_PIN(3, 12), RCAR_GP_PIN(3, 13),
};
static const unsigned int qspi1_data_mux[] = {
	QSPI1_MOSI_IO0_MARK, QSPI1_MISO_IO1_MARK,
	QSPI1_IO2_MARK, QSPI1_IO3_MARK,
};

/* - SDHI/MMC0 --------------------------------------------------------------- */
static const unsigned int mmc0_data_pins[] = {
	/* MMC0_SD_D[0:3], MMC0_D[4:7] */
	RCAR_GP_PIN(4, 2), RCAR_GP_PIN(4, 3),
	RCAR_GP_PIN(4, 4), RCAR_GP_PIN(4, 5),
	RCAR_GP_PIN(4, 6), RCAR_GP_PIN(4, 7),
	RCAR_GP_PIN(4, 8), RCAR_GP_PIN(4, 9),
};
static const unsigned int mmc0_data_mux[] = {
	MMC0_SD_D0_MARK, MMC0_SD_D1_MARK,
	MMC0_SD_D2_MARK, MMC0_SD_D3_MARK,
	MMC0_D4_MARK, MMC0_D5_MARK,
	MMC0_D6_MARK, MMC0_D7_MARK,
};
static const unsigned int mmc0_ctrl_pins[] = {
	/* MMC0_SD_CLK, MMC0_SD_CMD */
	RCAR_GP_PIN(4, 0), RCAR_GP_PIN(4, 1),
};
static const unsigned int mmc0_ctrl_mux[] = {
	MMC0_SD_CLK_MARK, MMC0_SD_CMD_MARK,
};
static const unsigned int mmc0_cd_pins[] = {
	/* SD0_CD */
	RCAR_GP_PIN(4, 12),
};
static const unsigned int mmc0_cd_mux[] = {
	SD0_CD_MARK,
};
static const unsigned int mmc0_wp_pins[] = {
	/* SD0_WP */
	RCAR_GP_PIN(4, 11),
};
static const unsigned int mmc0_wp_mux[] = {
	SD0_WP_MARK,
};
static const unsigned int mmc0_ds_pins[] = {
	/* MMC0_DS */
	RCAR_GP_PIN(4, 10),
};
static const unsigned int mmc0_ds_mux[] = {
	MMC0_DS_MARK,
};

/* - RSW3 ------------------------------------------------------------------ */
static const unsigned int rsw3_match_pins[] = {
	/* RSW3_MATCH */
	RCAR_GP_PIN(9, 16),
};
static const unsigned int rsw3_match_mux[] = {
	RSW3_MATCH_MARK,
};
static const unsigned int rsw3_capture_pins[] = {
	/* RSW3_CAPTURE */
	RCAR_GP_PIN(9, 15),
};
static const unsigned int rsw3_capture_mux[] = {
	RSW3_CAPTURE_MARK,
};
static const unsigned int rsw3_pps_pins[] = {
	/* RSW3_PPS */
	RCAR_GP_PIN(9, 14),
};
static const unsigned int rsw3_pps_mux[] = {
	RSW3_PPS_MARK,
};

/* - TSN0 ------------------------------------------------ */
static const unsigned int eth10g0_link_pins[] = {
	/* ETH10G0_LINK */
	RCAR_GP_PIN(9, 12),
};
static const unsigned int eth10g0_link_mux[] = {
	ETH10G0_LINK_MARK,
};
static const unsigned int eth10g0_phyint_pins[] = {
	/* ETH10G0_PHYINT */
	RCAR_GP_PIN(9, 13),
};
static const unsigned int eth10g0_phyint_mux[] = {
	ETH10G0_PHYINT_MARK,
};
static const unsigned int eth10g0_mdio_pins[] = {
	/* ETH10G0_MDC, ETH10G0_MDIO */
	RCAR_GP_PIN(9, 11), RCAR_GP_PIN(9, 10),
};
static const unsigned int eth10g0_mdio_mux[] = {
	ETH10G0_MDC_MARK, ETH10G0_MDIO_MARK,
};

static const unsigned int eth25g0_link_pins[] = {
	/* ETH25G0_LINK */
	RCAR_GP_PIN(9, 8),
};
static const unsigned int eth25g0_link_mux[] = {
	ETH25G0_LINK_MARK,
};
static const unsigned int eth25g0_phyint_pins[] = {
	/* ETH25G0_PHYINT */
	RCAR_GP_PIN(9, 9),
};
static const unsigned int eth25g0_phyint_mux[] = {
	ETH25G0_PHYINT_MARK,
};
static const unsigned int eth25g0_mdio_pins[] = {
	/* ETH25G0_MDC, ETH25G0_MDIO */
	RCAR_GP_PIN(9, 7), RCAR_GP_PIN(9, 6),
};
static const unsigned int eth25g0_mdio_mux[] = {
	ETH25G0_MDC_MARK, ETH25G0_MDIO_MARK,
};

/* - TSN1 ------------------------------------------------ */
static const unsigned int eth10g1_link_pins[] = {
	/* ETH10G1_LINK */
	RCAR_GP_PIN(9, 12),
};
static const unsigned int eth10g1_link_mux[] = {
	ETH10G1_LINK_MARK,
};
static const unsigned int eth10g1_phyint_pins[] = {
	/* ETH10G1_PHYINT */
	RCAR_GP_PIN(9, 13),
};
static const unsigned int eth10g1_phyint_mux[] = {
	ETH10G1_PHYINT_MARK,
};
static const unsigned int eth10g1_mdio_pins[] = {
	/* ETH10G1_MDC, ETH10G1_MDIO */
	RCAR_GP_PIN(9, 11), RCAR_GP_PIN(9, 10),
};
static const unsigned int eth10g1_mdio_mux[] = {
	ETH10G1_MDC_MARK, ETH10G1_MDIO_MARK,
};

static const unsigned int eth25g1_link_pins[] = {
	/* ETH25G1_LINK */
	RCAR_GP_PIN(9, 8),
};
static const unsigned int eth25g1_link_mux[] = {
	ETH25G1_LINK_MARK,
};
static const unsigned int eth25g1_phyint_pins[] = {
	/* ETH25G1_PHYINT */
	RCAR_GP_PIN(9, 9),
};
static const unsigned int eth25g1_phyint_mux[] = {
	ETH25G1_PHYINT_MARK,
};
static const unsigned int eth25g1_mdio_pins[] = {
	/* ETH25G1_MDC, ETH25G1_MDIO */
	RCAR_GP_PIN(9, 7), RCAR_GP_PIN(9, 6),
};
static const unsigned int eth25g1_mdio_mux[] = {
	ETH25G1_MDC_MARK, ETH25G1_MDIO_MARK,
};

/* - TSN2 ------------------------------------------------ */
static const unsigned int eth25g2_link_pins[] = {
	/* ETH25G2_LINK */
	RCAR_GP_PIN(9, 8),
};
static const unsigned int eth25g2_link_mux[] = {
	ETH25G2_LINK_MARK,
};
static const unsigned int eth25g2_phyint_pins[] = {
	/* ETH25G2_PHYINT */
	RCAR_GP_PIN(9, 9),
};
static const unsigned int eth25g2_phyint_mux[] = {
	ETH25G2_PHYINT_MARK,
};
static const unsigned int eth25g2_mdio_pins[] = {
	/* ETH25G2_MDC, ETH25G2_MDIO */
	RCAR_GP_PIN(9, 7), RCAR_GP_PIN(9, 6),
};
static const unsigned int eth25g2_mdio_mux[] = {
	ETH25G2_MDC_MARK, ETH25G2_MDIO_MARK,
};

/* - gPTPa (TSN0) ---------------------------------------- */
static const unsigned int ethes0_pps_pins[] = {
	/* ETHES0_PPS */
	RCAR_GP_PIN(9, 0),
};
static const unsigned int ethes0_pps_mux[] = {
	ETHES0_PPS_MARK,
};
static const unsigned int ethes0_capture_pins[] = {
	/* ETHES0_CAPTURE */
	RCAR_GP_PIN(9, 1),
};
static const unsigned int ethes0_capture_mux[] = {
	ETHES0_CAPTURE_MARK,
};
static const unsigned int ethes0_match_pins[] = {
	/* ETHES0_MATCH */
	RCAR_GP_PIN(9, 2),
};
static const unsigned int ethes0_match_mux[] = {
	ETHES0_MATCH_MARK,
};

/* - gPTPb (TSN1-7) -------------------------------------- */
static const unsigned int ethes1_pps_pins[] = {
	/* ETHES1_PPS */
	RCAR_GP_PIN(9, 0),
};
static const unsigned int ethes1_pps_mux[] = {
	ETHES1_PPS_MARK,
};
static const unsigned int ethes1_capture_pins[] = {
	/* ETHES1_CAPTURE */
	RCAR_GP_PIN(9, 1),
};
static const unsigned int ethes1_capture_mux[] = {
	ETHES1_CAPTURE_MARK,
};
static const unsigned int ethes1_match_pins[] = {
	/* ETHES1_MATCH */
	RCAR_GP_PIN(9, 2),
};
static const unsigned int ethes1_match_mux[] = {
	ETHES1_MATCH_MARK,
};

static const unsigned int ethes2_pps_pins[] = {
	/* ETHES2_PPS */
	RCAR_GP_PIN(9, 0),
};
static const unsigned int ethes2_pps_mux[] = {
	ETHES2_PPS_MARK,
};
static const unsigned int ethes2_capture_pins[] = {
	/* ETHES2_CAPTURE */
	RCAR_GP_PIN(9, 1),
};
static const unsigned int ethes2_capture_mux[] = {
	ETHES2_CAPTURE_MARK,
};
static const unsigned int ethes2_match_pins[] = {
	/* ETHES1_MATCH */
	RCAR_GP_PIN(9, 2),
};
static const unsigned int ethes2_match_mux[] = {
	ETHES2_MATCH_MARK,
};

static const unsigned int ethes3_pps_pins[] = {
	/* ETHES3_PPS */
	RCAR_GP_PIN(9, 0),
};
static const unsigned int ethes3_pps_mux[] = {
	ETHES3_PPS_MARK,
};
static const unsigned int ethes3_capture_pins[] = {
	/* ETHES3_CAPTURE */
	RCAR_GP_PIN(9, 1),
};
static const unsigned int ethes3_capture_mux[] = {
	ETHES3_CAPTURE_MARK,
};
static const unsigned int ethes3_match_pins[] = {
	/* ETHES3_MATCH */
	RCAR_GP_PIN(9, 2),
};
static const unsigned int ethes3_match_mux[] = {
	ETHES3_MATCH_MARK,
};

static const unsigned int ethes4_pps_pins[] = {
	/* ETHES4_PPS */
	RCAR_GP_PIN(9, 3),
};
static const unsigned int ethes4_pps_mux[] = {
	ETHES4_PPS_MARK,
};
static const unsigned int ethes4_capture_pins[] = {
	/* ETHES4_CAPTURE */
	RCAR_GP_PIN(9, 4),
};
static const unsigned int ethes4_capture_mux[] = {
	ETHES4_CAPTURE_MARK,
};
static const unsigned int ethes4_match_pins[] = {
	/* ETHES4_MATCH */
	RCAR_GP_PIN(9, 5),
};
static const unsigned int ethes4_match_mux[] = {
	ETHES4_MATCH_MARK,
};

static const unsigned int ethes5_pps_pins[] = {
	/* ETHES5_PPS */
	RCAR_GP_PIN(9, 3),
};
static const unsigned int ethes5_pps_mux[] = {
	ETHES5_PPS_MARK,
};
static const unsigned int ethes5_capture_pins[] = {
	/* ETHES5_CAPTURE */
	RCAR_GP_PIN(9, 4),
};
static const unsigned int ethes5_capture_mux[] = {
	ETHES5_CAPTURE_MARK,
};
static const unsigned int ethes5_match_pins[] = {
	/* ETHES5_MATCH */
	RCAR_GP_PIN(9, 5),
};
static const unsigned int ethes5_match_mux[] = {
	ETHES5_MATCH_MARK,
};

static const unsigned int ethes6_pps_pins[] = {
	/* ETHES6_PPS */
	RCAR_GP_PIN(9, 3),
};
static const unsigned int ethes6_pps_mux[] = {
	ETHES6_PPS_MARK,
};
static const unsigned int ethes6_capture_pins[] = {
	/* ETHES6_CAPTURE */
	RCAR_GP_PIN(9, 4),
};
static const unsigned int ethes6_capture_mux[] = {
	ETHES6_CAPTURE_MARK,
};
static const unsigned int ethes6_match_pins[] = {
	/* ETHES6_MATCH */
	RCAR_GP_PIN(9, 5),
};
static const unsigned int ethes6_match_mux[] = {
	ETHES6_MATCH_MARK,
};

static const unsigned int ethes7_pps_pins[] = {
	/* ETHES7_PPS */
	RCAR_GP_PIN(9, 3),
};
static const unsigned int ethes7_pps_mux[] = {
	ETHES7_PPS_MARK,
};
static const unsigned int ethes7_capture_pins[] = {
	/* ETHES7_CAPTURE */
	RCAR_GP_PIN(9, 4),
};
static const unsigned int ethes7_capture_mux[] = {
	ETHES7_CAPTURE_MARK,
};
static const unsigned int ethes7_match_pins[] = {
	/* ETHES7_MATCH */
	RCAR_GP_PIN(9, 5),
};
static const unsigned int ethes7_match_mux[] = {
	ETHES7_MATCH_MARK,
};

static const struct sh_pfc_pin_group pinmux_groups[] = {
	SH_PFC_PIN_GROUP(hscif0_data),
	SH_PFC_PIN_GROUP(hscif0_clk),
	SH_PFC_PIN_GROUP(hscif0_ctrl),
	SH_PFC_PIN_GROUP(hscif1_data),
	SH_PFC_PIN_GROUP(hscif1_clk),
	SH_PFC_PIN_GROUP(hscif1_ctrl),
	SH_PFC_PIN_GROUP(hscif2_data_a),
	SH_PFC_PIN_GROUP(hscif2_clk_a),
	SH_PFC_PIN_GROUP(hscif2_ctrl_a),
	SH_PFC_PIN_GROUP(hscif2_data_b),
	SH_PFC_PIN_GROUP(hscif2_clk_b),
	SH_PFC_PIN_GROUP(hscif2_ctrl_b),
	SH_PFC_PIN_GROUP(hscif3_data),
	SH_PFC_PIN_GROUP(hscif3_clk),
	SH_PFC_PIN_GROUP(hscif3_ctrl),

	SH_PFC_PIN_GROUP(scif0_data),
	SH_PFC_PIN_GROUP(scif0_clk),
	SH_PFC_PIN_GROUP(scif0_ctrl),
	SH_PFC_PIN_GROUP(scif1_data),
	SH_PFC_PIN_GROUP(scif1_clk),
	SH_PFC_PIN_GROUP(scif1_ctrl),
	SH_PFC_PIN_GROUP(scif3_data),
	SH_PFC_PIN_GROUP(scif3_clk),
	SH_PFC_PIN_GROUP(scif3_ctrl),
	SH_PFC_PIN_GROUP(scif4_data_a),
	SH_PFC_PIN_GROUP(scif4_clk_a),
	SH_PFC_PIN_GROUP(scif4_ctrl_a),
	SH_PFC_PIN_GROUP(scif4_data_b),
	SH_PFC_PIN_GROUP(scif4_clk_b),
	SH_PFC_PIN_GROUP(scif4_ctrl_b),
	SH_PFC_PIN_GROUP(scif_clk),

	SH_PFC_PIN_GROUP(i2c0),
	SH_PFC_PIN_GROUP(i2c1),
	SH_PFC_PIN_GROUP(i2c2),
	SH_PFC_PIN_GROUP(i2c3),
	SH_PFC_PIN_GROUP(i2c4),
	SH_PFC_PIN_GROUP(i2c5),
	SH_PFC_PIN_GROUP(i2c6),
	SH_PFC_PIN_GROUP(i2c7),
	SH_PFC_PIN_GROUP(i2c8),

	SH_PFC_PIN_GROUP(intc_ex_irq0),
	SH_PFC_PIN_GROUP(intc_ex_irq1),
	SH_PFC_PIN_GROUP(intc_ex_irq2),
	SH_PFC_PIN_GROUP(intc_ex_irq3),

	SH_PFC_PIN_GROUP(pcie40_clkreq_n),
	SH_PFC_PIN_GROUP(pcie41_clkreq_n),
	SH_PFC_PIN_GROUP(pcie60_clkreq_n),
	SH_PFC_PIN_GROUP(pcie61_clkreq_n),

	SH_PFC_PIN_GROUP(qspi0_ctrl),
	BUS_DATA_PIN_GROUP(qspi0_data, 2),
	BUS_DATA_PIN_GROUP(qspi0_data, 4),
	SH_PFC_PIN_GROUP(qspi1_ctrl),
	BUS_DATA_PIN_GROUP(qspi1_data, 2),
	BUS_DATA_PIN_GROUP(qspi1_data, 4),

	BUS_DATA_PIN_GROUP(mmc0_data, 1),
	BUS_DATA_PIN_GROUP(mmc0_data, 4),
	BUS_DATA_PIN_GROUP(mmc0_data, 8),
	SH_PFC_PIN_GROUP(mmc0_ctrl),
	SH_PFC_PIN_GROUP(mmc0_cd),
	SH_PFC_PIN_GROUP(mmc0_wp),
	SH_PFC_PIN_GROUP(mmc0_ds),

	SH_PFC_PIN_GROUP(rsw3_match),
	SH_PFC_PIN_GROUP(rsw3_capture),
	SH_PFC_PIN_GROUP(rsw3_pps),

	SH_PFC_PIN_GROUP(eth10g0_link),
	SH_PFC_PIN_GROUP(eth10g0_phyint),
	SH_PFC_PIN_GROUP(eth10g0_mdio),
	SH_PFC_PIN_GROUP(eth25g0_link),
	SH_PFC_PIN_GROUP(eth25g0_phyint),
	SH_PFC_PIN_GROUP(eth25g0_mdio),

	SH_PFC_PIN_GROUP(eth10g1_link),
	SH_PFC_PIN_GROUP(eth10g1_phyint),
	SH_PFC_PIN_GROUP(eth10g1_mdio),
	SH_PFC_PIN_GROUP(eth25g1_link),
	SH_PFC_PIN_GROUP(eth25g1_phyint),
	SH_PFC_PIN_GROUP(eth25g1_mdio),

	SH_PFC_PIN_GROUP(eth25g2_link),
	SH_PFC_PIN_GROUP(eth25g2_phyint),
	SH_PFC_PIN_GROUP(eth25g2_mdio),

	SH_PFC_PIN_GROUP(ethes0_pps),
	SH_PFC_PIN_GROUP(ethes0_capture),
	SH_PFC_PIN_GROUP(ethes0_match),
	SH_PFC_PIN_GROUP(ethes1_pps),
	SH_PFC_PIN_GROUP(ethes1_capture),
	SH_PFC_PIN_GROUP(ethes1_match),
	SH_PFC_PIN_GROUP(ethes2_pps),
	SH_PFC_PIN_GROUP(ethes2_capture),
	SH_PFC_PIN_GROUP(ethes2_match),
	SH_PFC_PIN_GROUP(ethes3_pps),
	SH_PFC_PIN_GROUP(ethes3_capture),
	SH_PFC_PIN_GROUP(ethes3_match),
	SH_PFC_PIN_GROUP(ethes4_pps),
	SH_PFC_PIN_GROUP(ethes4_capture),
	SH_PFC_PIN_GROUP(ethes4_match),
	SH_PFC_PIN_GROUP(ethes5_pps),
	SH_PFC_PIN_GROUP(ethes5_capture),
	SH_PFC_PIN_GROUP(ethes5_match),
	SH_PFC_PIN_GROUP(ethes6_pps),
	SH_PFC_PIN_GROUP(ethes6_capture),
	SH_PFC_PIN_GROUP(ethes6_match),
	SH_PFC_PIN_GROUP(ethes7_pps),
	SH_PFC_PIN_GROUP(ethes7_capture),
	SH_PFC_PIN_GROUP(ethes7_match),
};

static const char * const hscif0_groups[] = {
	"hscif0_data",
	"hscif0_clk",
	"hscif0_ctrl",
};

static const char * const hscif1_groups[] = {
	"hscif1_data",
	"hscif1_clk",
	"hscif1_ctrl",
};

static const char * const hscif2_groups[] = {
	"hscif2_data_a",
	"hscif2_clk_a",
	"hscif2_ctrl_a",
	"hscif2_data_b",
	"hscif2_clk_b",
	"hscif2_ctrl_b",
};

static const char * const hscif3_groups[] = {
	"hscif3_data",
	"hscif3_clk",
	"hscif3_ctrl",
};

static const char * const scif0_groups[] = {
	"scif0_data",
	"scif0_clk",
	"scif0_ctrl",
};

static const char * const scif1_groups[] = {
	"scif1_data",
	"scif1_clk",
	"scif1_ctrl",
};

static const char * const scif3_groups[] = {
	"scif3_data",
	"scif3_clk",
	"scif3_ctrl",
};

static const char * const scif4_groups[] = {
	"scif4_data_a",
	"scif4_clk_a",
	"scif4_ctrl_a",
	"scif4_data_b",
	"scif4_clk_b",
	"scif4_ctrl_b",
};

static const char * const scif_clk_groups[] = {
	"scif_clk",
};

static const char * const i2c0_groups[] = {
	"i2c0",
};

static const char * const i2c1_groups[] = {
	"i2c1",
};

static const char * const i2c2_groups[] = {
	"i2c2",
};

static const char * const i2c3_groups[] = {
	"i2c3",
};

static const char * const i2c4_groups[] = {
	"i2c4",
};

static const char * const i2c5_groups[] = {
	"i2c5",
};

static const char * const i2c6_groups[] = {
	"i2c6",
};

static const char * const i2c7_groups[] = {
	"i2c7",
};

static const char * const i2c8_groups[] = {
	"i2c8",
};

static const char * const intc_ex_groups[] = {
	"intc_ex_irq0",
	"intc_ex_irq1",
	"intc_ex_irq2",
	"intc_ex_irq3",
};

static const char * const pcie4_groups[] = {
	"pcie40_clkreq_n",
	"pcie41_clkreq_n",
};

static const char * const pcie6_groups[] = {
	"pcie60_clkreq_n",
	"pcie61_clkreq_n",
};

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

static const char * const mmc0_groups[] = {
	"mmc0_data1",
	"mmc0_data4",
	"mmc0_data8",
	"mmc0_ctrl",
	"mmc0_cd",
	"mmc0_wp",
	"mmc0_ds",
};

static const char * const rsw3_groups[] = {
	"rsw3_match",
	"rsw3_capture",
	"rsw3_pps",
};

static const char * const eth10g0_groups[] = {
	"eth10g0_link",
	"eth10g0_phyint",
	"eth10g0_mdio",
};

static const char * const eth25g0_groups[] = {
	"eth25g0_link",
	"eth25g0_phyint",
	"eth25g0_mdio",
};

static const char * const eth10g1_groups[] = {
	"eth10g1_link",
	"eth10g1_phyint",
	"eth10g1_mdio",
};

static const char * const eth25g1_groups[] = {
	"eth25g1_link",
	"eth25g1_phyint",
	"eth25g1_mdio",
};

static const char * const eth25g2_groups[] = {
	"eth25g2_link",
	"eth25g2_phyint",
	"eth25g2_mdio",
};

static const char * const ethes0_groups[] = {
	"ethes0_pps",
	"ethes0_capture",
	"ethes0_match",
};

static const char * const ethes1_groups[] = {
	"ethes1_pps",
	"ethes1_capture",
	"ethes1_match",
};

static const char * const ethes2_groups[] = {
	"ethes2_pps",
	"ethes2_capture",
	"ethes2_match",
};

static const char * const ethes3_groups[] = {
	"ethes3_pps",
	"ethes3_capture",
	"ethes3_match",
};

static const char * const ethes4_groups[] = {
	"ethes4_pps",
	"ethes4_capture",
	"ethes4_match",
};

static const char * const ethes5_groups[] = {
	"ethes5_pps",
	"ethes5_capture",
	"ethes5_match",
};

static const char * const ethes6_groups[] = {
	"ethes6_pps",
	"ethes6_capture",
	"ethes6_match",
};

static const char * const ethes7_groups[] = {
	"ethes7_pps",
	"ethes7_capture",
	"ethes7_match",
};

static const struct sh_pfc_function pinmux_functions[] = {
	SH_PFC_FUNCTION(hscif0),
	SH_PFC_FUNCTION(hscif1),
	SH_PFC_FUNCTION(hscif2),
	SH_PFC_FUNCTION(hscif3),

	SH_PFC_FUNCTION(scif0),
	SH_PFC_FUNCTION(scif1),
	SH_PFC_FUNCTION(scif3),
	SH_PFC_FUNCTION(scif4),
	SH_PFC_FUNCTION(scif_clk),

	SH_PFC_FUNCTION(i2c0),
	SH_PFC_FUNCTION(i2c1),
	SH_PFC_FUNCTION(i2c2),
	SH_PFC_FUNCTION(i2c3),
	SH_PFC_FUNCTION(i2c4),
	SH_PFC_FUNCTION(i2c5),
	SH_PFC_FUNCTION(i2c6),
	SH_PFC_FUNCTION(i2c7),
	SH_PFC_FUNCTION(i2c8),

	SH_PFC_FUNCTION(intc_ex),

	SH_PFC_FUNCTION(pcie4),
	SH_PFC_FUNCTION(pcie6),

	SH_PFC_FUNCTION(qspi0),
	SH_PFC_FUNCTION(qspi1),

	SH_PFC_FUNCTION(mmc0),

	SH_PFC_FUNCTION(rsw3),

	SH_PFC_FUNCTION(eth10g0),
	SH_PFC_FUNCTION(eth25g0),
	SH_PFC_FUNCTION(eth10g1),
	SH_PFC_FUNCTION(eth25g1),
	SH_PFC_FUNCTION(eth25g2),

	SH_PFC_FUNCTION(ethes0),
	SH_PFC_FUNCTION(ethes1),
	SH_PFC_FUNCTION(ethes2),
	SH_PFC_FUNCTION(ethes3),
	SH_PFC_FUNCTION(ethes4),
	SH_PFC_FUNCTION(ethes5),
	SH_PFC_FUNCTION(ethes6),
	SH_PFC_FUNCTION(ethes7),
};

static const struct pinmux_cfg_reg pinmux_config_regs[] = {
#define F_(x, y)	FN_##y
#define FM(x)		FN_##x
	{ PINMUX_CFG_REG_VAR("GPSR0", 0xC1080040, 32,
			     GROUP(-4,
				  1, 1, 1, 1, 1, 1, 1,
				  1, 1, 1, 1, 1, 1, 1,
				  1, 1, 1, 1, 1, 1, 1,
				  1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP0_31_28 RESERVED */
		GP_0_27_FN, 	GPSR0_27,
		GP_0_26_FN, 	GPSR0_26,
		GP_0_25_FN, 	GPSR0_25,
		GP_0_24_FN, 	GPSR0_24,
		GP_0_23_FN, 	GPSR0_23,
		GP_0_22_FN, 	GPSR0_22,
		GP_0_21_FN, 	GPSR0_21,
		GP_0_20_FN, 	GPSR0_20,
		GP_0_19_FN, 	GPSR0_19,
		GP_0_18_FN, 	GPSR0_18,
		GP_0_17_FN, 	GPSR0_17,
		GP_0_16_FN, 	GPSR0_16,
		GP_0_15_FN, 	GPSR0_15,
		GP_0_14_FN, 	GPSR0_14,
		GP_0_13_FN, 	GPSR0_13,
		GP_0_12_FN, 	GPSR0_12,
		GP_0_11_FN, 	GPSR0_11,
		GP_0_10_FN, 	GPSR0_10,
		GP_0_9_FN, 	GPSR0_9,
		GP_0_8_FN, 	GPSR0_8,
		GP_0_7_FN, 	GPSR0_7,
		GP_0_6_FN, 	GPSR0_6,
		GP_0_5_FN, 	GPSR0_5,
		GP_0_4_FN,	GPSR0_4,
		GP_0_3_FN,	GPSR0_3,
		GP_0_2_FN,	GPSR0_2,
		GP_0_1_FN,	GPSR0_1,
		GP_0_0_FN,	GPSR0_0, ))
	},
	{ PINMUX_CFG_REG_VAR("GPSR1", 0xC1080840, 32,
			     GROUP(-10,
				   1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP1_31_22 RESERVED */
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
	{ PINMUX_CFG_REG_VAR("GPSR2", 0xC1081040, 32,
			     GROUP(-3,
				  1, 1, 1, 1, 1, 1, 1,
				  1, 1, 1, 1, 1, 1, 1,
				  1, 1, 1, 1, 1, 1, 1,
				  1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP2_31_29 RESERVED */
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
	{ PINMUX_CFG_REG_VAR("GPSR3", 0xC0800040, 32,
			     GROUP(-15,
				   1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP3_31_17 RESERVED */
		GP_3_16_FN,	GPSR3_16,
		GP_3_15_FN,	GPSR3_15,
		GP_3_14_FN,	GPSR3_14,
		GP_3_13_FN,	GPSR3_13,
		GP_3_12_FN,	GPSR3_12,
		GP_3_11_FN,	GPSR3_11,
		GP_3_10_FN,	GPSR3_10,
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
	{ PINMUX_CFG_REG_VAR("GPSR4", 0xC0800840, 32,
			     GROUP(-16,
				   1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP4_31_16 RESERVED */
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
	{ PINMUX_CFG_REG_VAR("GPSR5", 0xC0400040, 32,
			     GROUP(-9,
				   1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP5_31_23 RESERVED */
		GP_5_22_FN,	GPSR5_22,
		GP_5_21_FN,	GPSR5_21,
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
	{ PINMUX_CFG_REG("GPSR6", 0xC0400840, 32, 1, GROUP(
		0, 		0,
		GP_6_30_FN, 	GPSR6_30,
		GP_6_29_FN,	GPSR6_29,
		GP_6_28_FN,	GPSR6_28,
		GP_6_27_FN,	GPSR6_27,
		GP_6_26_FN,	GPSR6_26,
		GP_6_25_FN,	GPSR6_25,
		GP_6_24_FN,	GPSR6_24,
		GP_6_23_FN,	GPSR6_23,
		GP_6_22_FN,	GPSR6_22,
		GP_6_21_FN,	GPSR6_21,
		GP_6_20_FN,	GPSR6_20,
		GP_6_19_FN,	GPSR6_19,
		GP_6_18_FN,	GPSR6_18,
		GP_6_17_FN,	GPSR6_17,
		GP_6_16_FN,	GPSR6_16,
		GP_6_15_FN,	GPSR6_15,
		GP_6_14_FN,	GPSR6_14,
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
	{ PINMUX_CFG_REG("GPSR7", 0xC0401040, 32, 1, GROUP(
		0, 		0,
		GP_7_30_FN, 	GPSR7_30,
		GP_7_29_FN,	GPSR7_29,
		GP_7_28_FN,	GPSR7_28,
		GP_7_27_FN,	GPSR7_27,
		GP_7_26_FN,	GPSR7_26,
		GP_7_25_FN,	GPSR7_25,
		GP_7_24_FN,	GPSR7_24,
		GP_7_23_FN,	GPSR7_23,
		GP_7_22_FN,	GPSR7_22,
		GP_7_21_FN,	GPSR7_21,
		GP_7_20_FN,	GPSR7_20,
		GP_7_19_FN,	GPSR7_19,
		GP_7_18_FN,	GPSR7_18,
		GP_7_17_FN,	GPSR7_17,
		GP_7_16_FN,	GPSR7_16,
		GP_7_15_FN,	GPSR7_15,
		GP_7_14_FN,	GPSR7_14,
		GP_7_13_FN,	GPSR7_13,
		GP_7_12_FN,	GPSR7_12,
		GP_7_11_FN,	GPSR7_11,
		GP_7_10_FN,	GPSR7_10,
		GP_7_9_FN,	GPSR7_9,
		GP_7_8_FN,	GPSR7_8,
		GP_7_7_FN,	GPSR7_7,
		GP_7_6_FN,	GPSR7_6,
		GP_7_5_FN,	GPSR7_5,
		GP_7_4_FN,	GPSR7_4,
		GP_7_3_FN,	GPSR7_3,
		GP_7_2_FN,	GPSR7_2,
		GP_7_1_FN,	GPSR7_1,
		GP_7_0_FN,	GPSR7_0, ))
	},
	{ PINMUX_CFG_REG_VAR("GPSR8", 0xC0401840, 32,
			     GROUP(1, 1, 1, 1, 1, 1,
				   -10,
				   1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		GP_8_31_FN,	GPSR8_31,
		GP_8_30_FN,	GPSR8_30,
		GP_8_29_FN,	GPSR8_29,
		GP_8_28_FN,	GPSR8_28,
		GP_8_27_FN,	GPSR8_27,
		GP_8_26_FN,	GPSR8_26,
		/* GP8_25_16 RESERVED */
		GP_8_15_FN,	GPSR8_15,
		GP_8_14_FN,	GPSR8_14,
		GP_8_13_FN,	GPSR8_13,
		GP_8_12_FN,	GPSR8_12,
		GP_8_11_FN,	GPSR8_11,
		GP_8_10_FN,	GPSR8_10,
		GP_8_9_FN,	GPSR8_9,
		GP_8_8_FN,	GPSR8_8,
		GP_8_7_FN,	GPSR8_7,
		GP_8_6_FN,	GPSR8_6,
		GP_8_5_FN,	GPSR8_5,
		GP_8_4_FN,	GPSR8_4,
		GP_8_3_FN,	GPSR8_3,
		GP_8_2_FN,	GPSR8_2,
		GP_8_1_FN,	GPSR8_1,
		GP_8_0_FN,	GPSR8_0, ))
	},
	{ PINMUX_CFG_REG_VAR("GPSR9", 0xC9B00040, 32,
			     GROUP(-15,
				   1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP9_31_17 RESERVED */
		GP_9_16_FN,	GPSR9_16,
		GP_9_15_FN,	GPSR9_15,
		GP_9_14_FN,	GPSR9_14,
		GP_9_13_FN,	GPSR9_13,
		GP_9_12_FN,	GPSR9_12,
		GP_9_11_FN,	GPSR9_11,
		GP_9_10_FN,	GPSR9_10,
		GP_9_9_FN,	GPSR9_9,
		GP_9_8_FN,	GPSR9_8,
		GP_9_7_FN,	GPSR9_7,
		GP_9_6_FN,	GPSR9_6,
		GP_9_5_FN,	GPSR9_5,
		GP_9_4_FN,	GPSR9_4,
		GP_9_3_FN,	GPSR9_3,
		GP_9_2_FN,	GPSR9_2,
		GP_9_1_FN,	GPSR9_1,
		GP_9_0_FN,	GPSR9_0, ))
	},
	{ PINMUX_CFG_REG_VAR("GPSR10", 0xC9B00840, 32,
			     GROUP(-18,
				   1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP10_31_14 RESERVED */
		GP_10_13_FN,	GPSR10_13,
		GP_10_12_FN,	GPSR10_12,
		GP_10_11_FN,	GPSR10_11,
		GP_10_10_FN,	GPSR10_10,
		GP_10_9_FN,	GPSR10_9,
		GP_10_8_FN,	GPSR10_8,
		GP_10_7_FN,	GPSR10_7,
		GP_10_6_FN,	GPSR10_6,
		GP_10_5_FN,	GPSR10_5,
		GP_10_4_FN,	GPSR10_4,
		GP_10_3_FN,	GPSR10_3,
		GP_10_2_FN,	GPSR10_2,
		GP_10_1_FN,	GPSR10_1,
		GP_10_0_FN,	GPSR10_0, ))
	},
#undef F_
#undef FM

#define F_(x, y)	x,
#define FM(x)		FN_##x,
	{ PINMUX_CFG_REG("GP0_ALTSEL0", 0xC1080060, 32, 1, GROUP(
		GP_ALTSEL0(0)
		))
	},
	{ PINMUX_CFG_REG("GP0_ALTSEL1", 0xC1080064, 32, 1, GROUP(
		GP_ALTSEL1(0)
		))
	},
	{ PINMUX_CFG_REG("GP0_ALTSEL2", 0xC1080068, 32, 1, GROUP(
		GP_ALTSEL2(0)
		))
	},
	{ PINMUX_CFG_REG("GP0_ALTSEL3", 0xC108006C, 32, 1, GROUP(
		GP_ALTSEL3(0)
		))
	},
	{ PINMUX_CFG_REG("GP1_ALTSEL0", 0xC1080860, 32, 1, GROUP(
		GP_ALTSEL0(1)
		))
	},
	{ PINMUX_CFG_REG("GP1_ALTSEL1", 0xC1080864, 32, 1, GROUP(
		GP_ALTSEL1(1)
		))
	},
	{ PINMUX_CFG_REG("GP1_ALTSEL2", 0xC1080868, 32, 1, GROUP(
		GP_ALTSEL2(1)
		))
	},
	{ PINMUX_CFG_REG("GP1_ALTSEL3", 0xC108086C, 32, 1, GROUP(
		GP_ALTSEL3(1)
		))
	},
	{ PINMUX_CFG_REG("GP2_ALTSEL0", 0xC1081060, 32, 1, GROUP(
		GP_ALTSEL0(2)
		))
	},
	{ PINMUX_CFG_REG("GP2_ALTSEL1", 0xC1081064, 32, 1, GROUP(
		GP_ALTSEL1(2)
		))
	},
	{ PINMUX_CFG_REG("GP2_ALTSEL2", 0xC1081068, 32, 1, GROUP(
		GP_ALTSEL2(2)
		))
	},
	{ PINMUX_CFG_REG("GP2_ALTSEL3", 0xC108106C, 32, 1, GROUP(
		GP_ALTSEL3(2)
		))
	},
	{ PINMUX_CFG_REG("GP3_ALTSEL0", 0xC0800060, 32, 1, GROUP(
		GP_ALTSEL0(3)
		))
	},
	{ PINMUX_CFG_REG("GP3_ALTSEL1", 0xC0800064, 32, 1, GROUP(
		GP_ALTSEL1(3)
		))
	},
	{ PINMUX_CFG_REG("GP3_ALTSEL2", 0xC0800068, 32, 1, GROUP(
		GP_ALTSEL2(3)
		))
	},
	{ PINMUX_CFG_REG("GP3_ALTSEL3", 0xC080006C, 32, 1, GROUP(
		GP_ALTSEL3(3)
		))
	},
	{ PINMUX_CFG_REG("GP4_ALTSEL0", 0xC0800860, 32, 1, GROUP(
		GP_ALTSEL0(4)
		))
	},
	{ PINMUX_CFG_REG("GP4_ALTSEL1", 0xC0800864, 32, 1, GROUP(
		GP_ALTSEL1(4)
		))
	},
	{ PINMUX_CFG_REG("GP4_ALTSEL2", 0xC0800868, 32, 1, GROUP(
		GP_ALTSEL2(4)
		))
	},
	{ PINMUX_CFG_REG("GP4_ALTSEL3", 0xC080086C, 32, 1, GROUP(
		GP_ALTSEL3(4)
		))
	},
	{ PINMUX_CFG_REG("GP5_ALTSEL0", 0xC0400060, 32, 1, GROUP(
		GP_ALTSEL0(5)
		))
	},
	{ PINMUX_CFG_REG("GP5_ALTSEL1", 0xC0400064, 32, 1, GROUP(
		GP_ALTSEL1(5)
		))
	},
	{ PINMUX_CFG_REG("GP5_ALTSEL2", 0xC0400068, 32, 1, GROUP(
		GP_ALTSEL2(5)
		))
	},
	{ PINMUX_CFG_REG("GP5_ALTSEL3", 0xC040006C, 32, 1, GROUP(
		GP_ALTSEL3(5)
		))
	},
	{ PINMUX_CFG_REG("GP6_ALTSEL0", 0xC0400860, 32, 1, GROUP(
		GP_ALTSEL0(6)
		))
	},
	{ PINMUX_CFG_REG("GP6_ALTSEL1", 0xC0400864, 32, 1, GROUP(
		GP_ALTSEL1(6)
		))
	},
	{ PINMUX_CFG_REG("GP6_ALTSEL2", 0xC0400868, 32, 1, GROUP(
		GP_ALTSEL2(6)
		))
	},
	{ PINMUX_CFG_REG("GP6_ALTSEL3", 0xC040086C, 32, 1, GROUP(
		GP_ALTSEL3(6)
		))
	},
	{ PINMUX_CFG_REG("GP7_ALTSEL0", 0xC0401060, 32, 1, GROUP(
		GP_ALTSEL0(7)
		))
	},
	{ PINMUX_CFG_REG("GP7_ALTSEL1", 0xC0401064, 32, 1, GROUP(
		GP_ALTSEL1(7)
		))
	},
	{ PINMUX_CFG_REG("GP7_ALTSEL2", 0xC0401068, 32, 1, GROUP(
		GP_ALTSEL2(7)
		))
	},
	{ PINMUX_CFG_REG("GP7_ALTSEL3", 0xC040106C, 32, 1, GROUP(
		GP_ALTSEL3(7)
		))
	},
	{ PINMUX_CFG_REG("GP8_ALTSEL0", 0xC0401860, 32, 1, GROUP(
		GP_ALTSEL0(8)
		))
	},
	{ PINMUX_CFG_REG("GP8_ALTSEL1", 0xC0401864, 32, 1, GROUP(
		GP_ALTSEL1(8)
		))
	},
	{ PINMUX_CFG_REG("GP8_ALTSEL2", 0xC0401868, 32, 1, GROUP(
		GP_ALTSEL2(8)
		))
	},
	{ PINMUX_CFG_REG("GP8_ALTSEL3", 0xC040186C, 32, 1, GROUP(
		GP_ALTSEL3(8)
		))
	},
	{ PINMUX_CFG_REG("GP9_ALTSEL0", 0xC9B00060, 32, 1, GROUP(
		GP_ALTSEL0(9)
		))
	},
	{ PINMUX_CFG_REG("GP5_ALTSEL1", 0xC9B00064, 32, 1, GROUP(
		GP_ALTSEL1(9)
		))
	},
	{ PINMUX_CFG_REG("GP9_ALTSEL2", 0xC9B00068, 32, 1, GROUP(
		GP_ALTSEL2(9)
		))
	},
	{ PINMUX_CFG_REG("GP9_ALTSEL3", 0xC9B0006C, 32, 1, GROUP(
		GP_ALTSEL3(9)
		))
	},
	{ PINMUX_CFG_REG("GP10_ALTSEL0", 0xC9B00860, 32, 1, GROUP(
		GP_ALTSEL0(10)
		))
	},
	{ PINMUX_CFG_REG("GP10_ALTSEL1", 0xC9B00864, 32, 1, GROUP(
		GP_ALTSEL1(10)
		))
	},
	{ PINMUX_CFG_REG("GP10_ALTSEL2", 0xC9B00868, 32, 1, GROUP(
		GP_ALTSEL2(10)
		))
	},
	{ PINMUX_CFG_REG("GP10_ALTSEL3", 0xC9B0086C, 32, 1, GROUP(
		GP_ALTSEL3(10)
		))
	},
#undef F_
#undef FM

#define F_(x, y)	x,
#define FM(x)		FN_##x,
	{ PINMUX_CFG_REG_VAR("GP1_MODSEL", 0xC1080900, 32,
			     GROUP(-10,
				   1, 1, 1, 1, 1, 1, 1, 1,
				   -14),
			     GROUP(
		/* GP1_MODSEL_31_22 RESERVED */
		GP1_MODSEL_21
		GP1_MODSEL_20
		GP1_MODSEL_19
		GP1_MODSEL_18
		GP1_MODSEL_17
		GP1_MODSEL_16
		GP1_MODSEL_15
		GP1_MODSEL_14
		/* GP1_MODSEL_13_0 RESERVED */
		))
	},
	{ PINMUX_CFG_REG_VAR("GP2_MODSEL", 0xC1081100, 32,
			     GROUP(-11,
				   1, 1,
				   -19),
			     GROUP(
		/* GP2_MODSEL_31_21 RESERVED */
		GP2_MODSEL_20
		GP2_MODSEL_19
		/* GP2_MODSEL_18_0 RESERVED */
		))
	},
	{ PINMUX_CFG_REG_VAR("GP8_MODSEL", 0xC0401900, 32,
			     GROUP(-16,
				   1, 1, 1, 1, 1, 1, 1, 1,
				   1, 1, 1, 1, 1, 1, 1, 1),
			     GROUP(
		/* GP8_MODSEL_31_16 RESERVED */
		GP8_MODSEL_15
		GP8_MODSEL_14
		GP8_MODSEL_13
		GP8_MODSEL_12
		GP8_MODSEL_11
		GP8_MODSEL_10
		GP8_MODSEL_9
		GP8_MODSEL_8
		GP8_MODSEL_7
		GP8_MODSEL_6
		GP8_MODSEL_5
		GP8_MODSEL_4
		GP8_MODSEL_3
		GP8_MODSEL_2
		GP8_MODSEL_1
		GP8_MODSEL_0
		))
	},
	{ },
};

#define RCAR5_PINMUX_DRIVE_REG(name1, r1, name2, r2, name3, r3) \
	.drvctrl0 = r1,	\
	.drvctrl1 = r2,	\
	.drvctrl2 = r3,	\
	.pins =

struct rcar5_pinmux_drive_reg {
	u32 drvctrl0;
	u32 drvctrl1;
	u32 drvctrl2;
	const u16 pins[32];
};

static const struct rcar5_pinmux_drive_reg pinmux_drive_regs[] = {
	{ RCAR5_PINMUX_DRIVE_REG("GP0_DRVCTRL0", 0xC1080080, "GP0_DRVCTRL1", 0xC1080084, "GP0_DRVCTRL2", 0xC1080088) {
		[ 0] = RCAR_GP_PIN(0,  0),	/* GP0_00 */
		[ 1] = RCAR_GP_PIN(0,  1),	/* GP0_01 */
		[ 2] = RCAR_GP_PIN(0,  2),	/* GP0_02 */
		[ 3] = RCAR_GP_PIN(0,  3),	/* STPWT_EXTFXR_A */
		[ 4] = RCAR_GP_PIN(0,  4),	/* FXR_CLKOUT1_A */
		[ 5] = RCAR_GP_PIN(0,  5),	/* FXR_CLKOUT2_A */
		[ 6] = RCAR_GP_PIN(0,  6),	/* CLK_EXTFXR_A */
		[ 7] = RCAR_GP_PIN(0,  7),	/* FXR_TXDA_A */
		[ 8] = RCAR_GP_PIN(0,  8),	/* FXR_TXENA_N_A */
		[ 9] = RCAR_GP_PIN(0,  9),	/* RXDA_EXTFXR_A */
		[10] = RCAR_GP_PIN(0, 10),	/* FXR_TXDB_A */
		[11] = RCAR_GP_PIN(0, 11),	/* FXR_TXENB_N_A */
		[12] = RCAR_GP_PIN(0, 12),	/* RXDB_EXTFXR_A */
		[13] = RCAR_GP_PIN(0, 13),	/* MSIOF0_SCK */
		[14] = RCAR_GP_PIN(0, 14),	/* MSIOF0_TXD */
		[15] = RCAR_GP_PIN(0, 15),	/* MSIOF0_RXD */
		[16] = RCAR_GP_PIN(0, 16),	/* MSIOF0_SYNC */
		[17] = RCAR_GP_PIN(0, 17),	/* MSIOF0_SS1 */
		[18] = RCAR_GP_PIN(0, 18),	/* MSIOF0_SS2 */
		[19] = RCAR_GP_PIN(0, 19),	/* MSIOF1_SCK_A */
		[20] = RCAR_GP_PIN(0, 20),	/* MSIOF1_TXD_A */
		[21] = RCAR_GP_PIN(0, 21),	/* MSIOF1_RXD_A */
		[22] = RCAR_GP_PIN(0, 22),	/* MSIOF1_SYNC_A */
		[23] = RCAR_GP_PIN(0, 23),	/* MSIOF1_SS1_A */
		[24] = RCAR_GP_PIN(0, 24),	/* MSIOF1_SS2_A */
		[25] = RCAR_GP_PIN(0, 25),	/* DP0_HOTPLUG */
		[26] = RCAR_GP_PIN(0, 26),	/* DP1_HOTPLUG */
		[27] = RCAR_GP_PIN(0, 27),	/* DP2_HOTPLUG */
		[28] = SH_PFC_PIN_NONE,
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP1_DRVCTRL0", 0xC1080880, "GP1_DRVCTRL1", 0xC1080884, "GP1_DRVCTRL2", 0xC1080888) {
		[ 0] = RCAR_GP_PIN(1,  0),	/* CAN0RX_INTP0 */
		[ 1] = RCAR_GP_PIN(1,  1),	/* CAN0TX */
		[ 2] = RCAR_GP_PIN(1,  2),	/* CAN1RX_INTP1 */
		[ 3] = RCAR_GP_PIN(1,  3),	/* CAN1TX */
		[ 4] = RCAR_GP_PIN(1,  4),	/* CAN2RX_INTP2 */
		[ 5] = RCAR_GP_PIN(1,  5),	/* CAN2TX */
		[ 6] = RCAR_GP_PIN(1,  6),	/* CAN3RX_INTP3 */
		[ 7] = RCAR_GP_PIN(1,  7),	/* CAN3TX */
		[ 8] = RCAR_GP_PIN(1,  8),	/* CAN4RX_INTP4 */
		[ 9] = RCAR_GP_PIN(1,  9),	/* CAN4TX */
		[10] = RCAR_GP_PIN(1, 10),	/* CAN5RX_INTP5 */
		[11] = RCAR_GP_PIN(1, 11),	/* CAN5TX */
		[12] = RCAR_GP_PIN(1, 12),	/* CAN6RX_INTP6 */
		[13] = RCAR_GP_PIN(1, 13),	/* CAN6TX */
		[14] = RCAR_GP_PIN(1, 14),	/* RLIN30RX_INTP16 */
		[15] = RCAR_GP_PIN(1, 15),	/* RLIN30TX */
		[16] = RCAR_GP_PIN(1, 16),	/* RLIN31RX_INTP17 */
		[17] = RCAR_GP_PIN(1, 17),	/* RLIN31TX */
		[18] = RCAR_GP_PIN(1, 18),	/* RLIN32RX_INTP18 */
		[19] = RCAR_GP_PIN(1, 19),	/* RLIN32TX */
		[20] = RCAR_GP_PIN(1, 20),	/* RLIN33RX_INTP19 */
		[21] = RCAR_GP_PIN(1, 21),	/* RLIN33TX */
		[22] = SH_PFC_PIN_NONE,
		[23] = SH_PFC_PIN_NONE,
		[24] = SH_PFC_PIN_NONE,
		[25] = SH_PFC_PIN_NONE,
		[26] = SH_PFC_PIN_NONE,
		[27] = SH_PFC_PIN_NONE,
		[28] = SH_PFC_PIN_NONE,
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP2_DRVCTRL0", 0xC1081080, "GP2_DRVCTRL1", 0xC1081084, "GP2_DRVCTRL2", 0xC1081088) {
		[ 0] = RCAR_GP_PIN(2,  0),	/* RLIN34RX_INTP20_B */
		[ 1] = RCAR_GP_PIN(2,  1),	/* RLIN34TX_B */
		[ 2] = RCAR_GP_PIN(2,  2),	/* RLIN35RX_INTP21_B */
		[ 3] = RCAR_GP_PIN(2,  3),	/* RLIN35TX_B */
		[ 4] = RCAR_GP_PIN(2,  4),	/* RLIN36RX_INTP22_B */
		[ 5] = RCAR_GP_PIN(2,  5),	/* RLIN36TX_B */
		[ 6] = RCAR_GP_PIN(2,  6),	/* RLIN37RX_INTP23_B */
		[ 7] = RCAR_GP_PIN(2,  7),	/* RLIN37TX_B */
		[ 8] = RCAR_GP_PIN(2,  8),	/* CAN12RX_INTP12_B */
		[ 9] = RCAR_GP_PIN(2,  9),	/* CAN12TX_B */
		[10] = RCAR_GP_PIN(2, 10),	/* CAN13RX_INTP13_B */
		[11] = RCAR_GP_PIN(2, 11),	/* CAN13TX_B */
		[12] = RCAR_GP_PIN(2, 12),	/* CAN14RX_INTP14_B */
		[13] = RCAR_GP_PIN(2, 13),	/* CAN14TX_B */
		[14] = RCAR_GP_PIN(2, 14),	/* CAN15RX_INTP15_B */
		[15] = RCAR_GP_PIN(2, 15),	/* CAN15TX_B */
		[16] = RCAR_GP_PIN(2, 16),	/* CAN_CLK */
		[17] = RCAR_GP_PIN(2, 17),	/* INTP32_B */
		[18] = RCAR_GP_PIN(2, 18),	/* INTP33_B */
		[19] = RCAR_GP_PIN(2, 19),	/* SCL0 */
		[20] = RCAR_GP_PIN(2, 20),	/* SDA0 */
		[21] = RCAR_GP_PIN(2, 21),	/* AVS0 */
		[22] = RCAR_GP_PIN(2, 22),	/* AVS1 */
		[23] = RCAR_GP_PIN(2, 23),	/* EXTCLK0O_B */
		[24] = RCAR_GP_PIN(2, 24),	/* TAUD1O0 */
		[25] = RCAR_GP_PIN(2, 25),	/* TAUD1O1 */
		[26] = RCAR_GP_PIN(2, 26),	/* TAUD1O2 */
		[27] = RCAR_GP_PIN(2, 27),	/* TAUD1O3 */
		[28] = RCAR_GP_PIN(2, 28),	/* INTP34_B */
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP3_DRVCTRL0", 0xC0800080, "GP3_DRVCTRL1", 0xC0800084, "GP3_DRVCTRL2", 0xC0800088) {
		[ 0] = RCAR_GP_PIN(3,  0),	/* QSPI0_SPCLK */
		[ 1] = RCAR_GP_PIN(3,  1),	/* QSPI0_MOSI_IO0 */
		[ 2] = RCAR_GP_PIN(3,  2),	/* QSPI0_MISO_IO1 */
		[ 3] = RCAR_GP_PIN(3,  3),	/* QSPI0_IO2 */
		[ 4] = RCAR_GP_PIN(3,  4),	/* QSPI0_IO3 */
		[ 5] = RCAR_GP_PIN(3,  5),	/* QSPI0_SSL */
		[ 6] = RCAR_GP_PIN(3,  6),	/* RPC_RESET_N */
		[ 7] = RCAR_GP_PIN(3,  7),	/* RPC_WP_N */
		[ 8] = RCAR_GP_PIN(3,  8),	/* RPC_INT_N */
		[ 9] = RCAR_GP_PIN(3,  9),	/* QSPI1_SPCLK */
		[10] = RCAR_GP_PIN(3, 10),	/* QSPI1_MOSI_IO0 */
		[11] = RCAR_GP_PIN(3, 11),	/* QSPI1_MISO_IO1 */
		[12] = RCAR_GP_PIN(3, 12),	/* QSPI1_IO2 */
		[13] = RCAR_GP_PIN(3, 13),	/* QSPI1_IO3 */
		[14] = RCAR_GP_PIN(3, 14),	/* QSPI1_SSL */
		[15] = RCAR_GP_PIN(3, 15),	/* ERROROUT_N */
		[16] = RCAR_GP_PIN(3, 16),	/* ERRORIN_N */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP4_DRVCTRL0", 0xC0800880, "GP4_DRVCTRL1", 0xC0800884, "GP4_DRVCTRL2", 0xC0800888) {
		[ 0] = RCAR_GP_PIN(4,  0),	/* MMC0_SD_CLK */
		[ 1] = RCAR_GP_PIN(4,  1),	/* MMC0_SD_CMD */
		[ 2] = RCAR_GP_PIN(4,  2),	/* MMC0_SD_D0 */
		[ 3] = RCAR_GP_PIN(4,  3),	/* MMC0_SD_D1 */
		[ 4] = RCAR_GP_PIN(4,  4),	/* MMC0_SD_D2 */
		[ 5] = RCAR_GP_PIN(4,  5),	/* MMC0_SD_D3 */
		[ 6] = RCAR_GP_PIN(4,  6),	/* MMC0_D4 */
		[ 7] = RCAR_GP_PIN(4,  7),	/* MMC0_D5 */
		[ 8] = RCAR_GP_PIN(4,  8),	/* MMC0_D6 */
		[ 9] = RCAR_GP_PIN(4,  9),	/* MMC0_D7 */
		[10] = RCAR_GP_PIN(4, 10),	/* MMC0_DS */
		[11] = RCAR_GP_PIN(4, 11),	/* SD0_WP */
		[12] = RCAR_GP_PIN(4, 12),	/* SD0_CD */
		[13] = RCAR_GP_PIN(4, 13),	/* ERRORIN_N */
		[14] = RCAR_GP_PIN(4, 14),	/* PCIE60_CLKREQ_N */
		[15] = RCAR_GP_PIN(4, 15),	/* PCIE61_CLKREQ_N */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP5_DRVCTRL0", 0xC0400080, "GP5_DRVCTRL1", 0xC0400084, "GP5_DRVCTRL2", 0xC0400088) {
		[ 0] = RCAR_GP_PIN(5,  0),	/* HTX0 */
		[ 1] = RCAR_GP_PIN(5,  1),	/* HRX0 */
		[ 2] = RCAR_GP_PIN(5,  2),	/* HRTS0_N */
		[ 3] = RCAR_GP_PIN(5,  3),	/* HCTS0_N */
		[ 4] = RCAR_GP_PIN(5,  4),	/* HSCK0 */
		[ 5] = RCAR_GP_PIN(5,  5),	/* SCIF_CLK */
		[ 6] = RCAR_GP_PIN(5,  6),	/* HTX1 */
		[ 7] = RCAR_GP_PIN(5,  7),	/* HRX1 */
		[ 8] = RCAR_GP_PIN(5,  8),	/* HRTS1_N */
		[ 9] = RCAR_GP_PIN(5,  9),	/* HCTS1_N */
		[10] = RCAR_GP_PIN(5, 10),	/* HSCK1 */
		[11] = RCAR_GP_PIN(5, 11),	/* IRQ0_A */
		[12] = RCAR_GP_PIN(5, 12),	/* IRQ1_A */
		[13] = RCAR_GP_PIN(5, 13),	/* IRQ2_A */
		[14] = RCAR_GP_PIN(5, 14),	/* IRQ3_A */
		[15] = RCAR_GP_PIN(5, 15),	/* TCLK1 */
		[16] = RCAR_GP_PIN(5, 16),	/* TCLK2 */
		[17] = RCAR_GP_PIN(5, 17),	/* TCLK3 */
		[18] = RCAR_GP_PIN(5, 18),	/* TCLK4 */
		[19] = RCAR_GP_PIN(5, 19),	/* TPU0TO0 */
		[20] = RCAR_GP_PIN(5, 20),	/* TPU0TO1 */
		[21] = RCAR_GP_PIN(5, 21),	/* TPU0TO2 */
		[22] = RCAR_GP_PIN(5, 22),	/* TPU0TO3 */
		[23] = SH_PFC_PIN_NONE,
		[24] = SH_PFC_PIN_NONE,
		[25] = SH_PFC_PIN_NONE,
		[26] = SH_PFC_PIN_NONE,
		[27] = SH_PFC_PIN_NONE,
		[28] = SH_PFC_PIN_NONE,
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP6_DRVCTRL0", 0xC0400880, "GP6_DRVCTRL1", 0xC0400884, "GP6_DRVCTRL2", 0xC0400888) {
		[ 0] = RCAR_GP_PIN(6,  0),	/* RIF6_D0 */
		[ 1] = RCAR_GP_PIN(6,  1),	/* RIF6_D1 */
		[ 2] = RCAR_GP_PIN(6,  2),	/* RIF6_SYNC */
		[ 3] = RCAR_GP_PIN(6,  3),	/* RIF6_CLK */
		[ 4] = RCAR_GP_PIN(6,  4),	/* MSIOF7_SCK_A */
		[ 5] = RCAR_GP_PIN(6,  5),	/* MSIOF7_TXD_A */
		[ 6] = RCAR_GP_PIN(6,  6),	/* MSIOF7_RXD_A */
		[ 7] = RCAR_GP_PIN(6,  7),	/* MSIOF7_SYNC_A */
		[ 8] = RCAR_GP_PIN(6,  8),	/* MSIOF7_SS1_A */
		[ 9] = RCAR_GP_PIN(6,  9),	/* MSIOF7_SS2_A */
		[10] = RCAR_GP_PIN(6, 10),	/* MSIOF4_SCK_B */
		[11] = RCAR_GP_PIN(6, 11),	/* MSIOF4_TXD_B */
		[12] = RCAR_GP_PIN(6, 12),	/* MSIOF4_RXD_B */
		[13] = RCAR_GP_PIN(6, 13),	/* MSIOF4_SYNC_B */
		[14] = RCAR_GP_PIN(6, 14),	/* MSIOF4_SS1_B */
		[15] = RCAR_GP_PIN(6, 15),	/* MSIOF4_SS2_B */
		[16] = RCAR_GP_PIN(6, 16),	/* SSI0_SCK */
		[17] = RCAR_GP_PIN(6, 17),	/* SSI0_WS */
		[18] = RCAR_GP_PIN(6, 18),	/* SSI0_SD */
		[19] = RCAR_GP_PIN(6, 19),	/* AUDIO0_CLKOUT0 */
		[20] = RCAR_GP_PIN(6, 20),	/* AUDIO0_CLKOUT1 */
		[21] = RCAR_GP_PIN(6, 21),	/* SSI1_SCK */
		[22] = RCAR_GP_PIN(6, 22),	/* SSI1_WS */
		[23] = RCAR_GP_PIN(6, 23),	/* SSI1_SD */
		[24] = RCAR_GP_PIN(6, 24),	/* AUDIO0_CLKOUT2 */
		[25] = RCAR_GP_PIN(6, 25),	/* AUDIO0_CLKOUT3 */
		[26] = RCAR_GP_PIN(6, 26),	/* SSI2_SCK */
		[27] = RCAR_GP_PIN(6, 27),	/* SSI2_WS */
		[28] = RCAR_GP_PIN(6, 28),	/* SSI2_SD */
		[29] = RCAR_GP_PIN(6, 29),	/* AUDIO1_CLKOUT0 */
		[30] = RCAR_GP_PIN(6, 30),	/* AUDIO1_CLKOUT1 */
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP7_DRVCTRL0", 0xC0401080, "GP7_DRVCTRL1", 0xC0401084, "GP7_DRVCTRL2", 0xC0401088) {
		[ 0] = RCAR_GP_PIN(7,  0),	/* SSI3_SCK */
		[ 1] = RCAR_GP_PIN(7,  1),	/* SSI3_WS */
		[ 2] = RCAR_GP_PIN(7,  2),	/* SSI3_SD */
		[ 3] = RCAR_GP_PIN(7,  3),	/* AUDIO1_CLKOUT2 */
		[ 4] = RCAR_GP_PIN(7,  4),	/* AUDIO1_CLKOUT3 */
		[ 5] = RCAR_GP_PIN(7,  5),	/* SSI4_SCK */
		[ 6] = RCAR_GP_PIN(7,  6),	/* SSI4_WS */
		[ 7] = RCAR_GP_PIN(7,  7),	/* SSI4_SD */
		[ 8] = RCAR_GP_PIN(7,  8),	/* AUDIO_CLKA_A */
		[ 9] = RCAR_GP_PIN(7,  9),	/* SSI5_SCK */
		[10] = RCAR_GP_PIN(7, 10),	/* SSI5_WS */
		[11] = RCAR_GP_PIN(7, 11),	/* SSI5_SD */
		[12] = RCAR_GP_PIN(7, 12),	/* AUDIO_CLKB_A */
		[13] = RCAR_GP_PIN(7, 13),	/* SSI6_SCK */
		[14] = RCAR_GP_PIN(7, 14),	/* SSI6_WS */
		[15] = RCAR_GP_PIN(7, 15),	/* SSI6_SD */
		[16] = RCAR_GP_PIN(7, 16),	/* AUDIO_CLKC_A */
		[17] = RCAR_GP_PIN(7, 17),	/* MSIOF5_SCK */
		[18] = RCAR_GP_PIN(7, 18),	/* GP07_18 */
		[19] = RCAR_GP_PIN(7, 19),	/* GP07_19 */
		[20] = RCAR_GP_PIN(7, 20),	/* MSIOF5_TXD */
		[21] = RCAR_GP_PIN(7, 21),	/* MSIOF5_RXD */
		[22] = RCAR_GP_PIN(7, 22),	/* MSIOF5_SYNC */
		[23] = RCAR_GP_PIN(7, 23),	/* MSIOF5_SS1 */
		[24] = RCAR_GP_PIN(7, 24),	/* MSIOF5_SS2 */
		[25] = RCAR_GP_PIN(7, 25),	/* MSIOF6_SCK_B */
		[26] = RCAR_GP_PIN(7, 26),	/* MSIOF6_TXD_B */
		[27] = RCAR_GP_PIN(7, 27),	/* MSIOF6_RXD_B */
		[28] = RCAR_GP_PIN(7, 28),	/* MSIOF6_SYNC_B */
		[29] = RCAR_GP_PIN(7, 29),	/* MSIOF6_SS1_B */
		[30] = RCAR_GP_PIN(7, 30),	/* MSIOF6_SS2_B */
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP8_DRVCTRL0", 0xC0401880, "GP8_DRVCTRL1", 0xC0401884, "GP8_DRVCTRL2", 0xC0401888) {
		[ 0] = RCAR_GP_PIN(8,  0),	/* SCL1 */
		[ 1] = RCAR_GP_PIN(8,  1),	/* SDA1 */
		[ 2] = RCAR_GP_PIN(8,  2),	/* SCL2 */
		[ 3] = RCAR_GP_PIN(8,  3),	/* SDA2 */
		[ 4] = RCAR_GP_PIN(8,  4),	/* SCL3 */
		[ 5] = RCAR_GP_PIN(8,  5),	/* SDA3 */
		[ 6] = RCAR_GP_PIN(8,  6),	/* SCL4 */
		[ 7] = RCAR_GP_PIN(8,  7),	/* SDA4 */
		[ 8] = RCAR_GP_PIN(8,  8),	/* SCL5 */
		[ 9] = RCAR_GP_PIN(8,  9),	/* SDA5 */
		[10] = RCAR_GP_PIN(8, 10),	/* SCL6 */
		[11] = RCAR_GP_PIN(8, 11),	/* SDA6 */
		[12] = RCAR_GP_PIN(8, 12),	/* SCL7 */
		[13] = RCAR_GP_PIN(8, 13),	/* SDA7 */
		[14] = RCAR_GP_PIN(8, 14),	/* SCL8 */
		[15] = RCAR_GP_PIN(8, 15),	/* SDA8 */
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
		[26] = RCAR_GP_PIN(8, 26),	/* S3CL0 */
		[27] = RCAR_GP_PIN(8, 27),	/* S3DA0 */
		[28] = RCAR_GP_PIN(8, 28),	/* S3CL1 */
		[29] = RCAR_GP_PIN(8, 29),	/* S3DA1 */
		[30] = RCAR_GP_PIN(8, 30),	/* S3CL2 */
		[31] = RCAR_GP_PIN(8, 31),	/* S3DA2 */
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP9_DRVCTRL0", 0xC9B00080, "GP9_DRVCTRL1", 0xC9B00084, "GP9_DRVCTRL2", 0xC9B00088) {
		[ 0] = RCAR_GP_PIN(9,  0),	/* ETHES0_PPS */
		[ 1] = RCAR_GP_PIN(9,  1),	/* ETHES0_CAPTURE */
		[ 2] = RCAR_GP_PIN(9,  2),	/* ETHES0_MATCH */
		[ 3] = RCAR_GP_PIN(9,  3),	/* ETHES4_PPS */
		[ 4] = RCAR_GP_PIN(9,  4),	/* ETHES4_CAPTURE */
		[ 5] = RCAR_GP_PIN(9,  5),	/* ETHES4_MATCH */
		[ 6] = RCAR_GP_PIN(9,  6),	/* ETH25G0_MDIO */
		[ 7] = RCAR_GP_PIN(9,  7),	/* ETH25G0_MDC */
		[ 8] = RCAR_GP_PIN(9,  8),	/* ETH25G0_LINK */
		[ 9] = RCAR_GP_PIN(9,  9),	/* ETH25G0_PHYINT */
		[10] = RCAR_GP_PIN(9, 10),	/* ETH10G0_MDIO */
		[11] = RCAR_GP_PIN(9, 11),	/* ETH10G0_MDC */
		[12] = RCAR_GP_PIN(9, 12),	/* ETH10G0_LINK */
		[13] = RCAR_GP_PIN(9, 13),	/* ETH10G0_PHYINT */
		[14] = RCAR_GP_PIN(9, 14),	/* RSW3_PPS */
		[15] = RCAR_GP_PIN(9, 15),	/* RSW3_CAPTURE */
		[16] = RCAR_GP_PIN(9, 16),	/* RSW3_MATCH */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ RCAR5_PINMUX_DRIVE_REG("GP10_DRVCTRL0", 0xC9B00880, "GP10_DRVCTRL1", 0xC9B00884, "GP10_DRVCTRL2", 0xC9B00888) {
		[ 0] = RCAR_GP_PIN(10,  0),	/* USB0_PWEN */
		[ 1] = RCAR_GP_PIN(10,  1),	/* USB0_OVC */
		[ 2] = RCAR_GP_PIN(10,  2),	/* USB0_VBUS_VALID */
		[ 3] = RCAR_GP_PIN(10,  3),	/* USB1_PWEN */
		[ 4] = RCAR_GP_PIN(10,  4),	/* USB1_OVC */
		[ 5] = RCAR_GP_PIN(10,  5),	/* USB1_VBUS_VALID */
		[ 6] = RCAR_GP_PIN(10,  6),	/* USB2_PWEN */
		[ 7] = RCAR_GP_PIN(10,  7),	/* USB2_OVC */
		[ 8] = RCAR_GP_PIN(10,  8),	/* USB2_VBUS_VALID */
		[ 9] = RCAR_GP_PIN(10,  9),	/* USB3_PWEN */
		[10] = RCAR_GP_PIN(10, 10),	/* USB3_OVC */
		[11] = RCAR_GP_PIN(10, 11),	/* USB3_VBUS_VALID */
		[12] = RCAR_GP_PIN(10, 12),	/* PCIE40_CLKREQ_N */
		[13] = RCAR_GP_PIN(10, 13),	/* PCIE41_CLKREQ_N */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ /* sentinel */ },
};

enum ioctrl_regs {
	GP0_TDSEL0,
	GP0_TDSEL1,
	GP1_TDSEL0,
	GP1_TDSEL1,
	GP2_TDSEL0,
	GP2_TDSEL1,
	GP3_TDSEL0,
	GP3_TDSEL1,
	GP4_TDSEL0,
	GP4_TDSEL1,
	GP5_TDSEL0,
	GP5_TDSEL1,
	GP6_TDSEL0,
	GP6_TDSEL1,
	GP7_TDSEL0,
	GP7_TDSEL1,
	GP8_TDSEL0,
	GP8_TDSEL1,
	GP9_TDSEL0,
	GP9_TDSEL1,
	GP10_TDSEL0,
	GP10_TDSEL1,
};

static const struct pinmux_ioctrl_reg pinmux_ioctrl_regs[] = {
	[GP0_TDSEL0] = { 0xC1080094, },
	[GP0_TDSEL1] = { 0xC1080098, },
	[GP1_TDSEL0] = { 0xC1080894, },
	[GP1_TDSEL1] = { 0xC1080898, },
	[GP2_TDSEL0] = { 0xC1081094, },
	[GP2_TDSEL1] = { 0xC1081098, },
	[GP3_TDSEL0] = { 0xC0800094, },
	[GP3_TDSEL1] = { 0xC0800098, },
	[GP4_TDSEL0] = { 0xC0800894, },
	[GP4_TDSEL1] = { 0xC0800898, },
	[GP5_TDSEL0] = { 0xC0400094, },
	[GP5_TDSEL1] = { 0xC0400098, },
	[GP6_TDSEL0] = { 0xC0400894, },
	[GP6_TDSEL1] = { 0xC0400898, },
	[GP7_TDSEL0] = { 0xC0401094, },
	[GP7_TDSEL1] = { 0xC0401098, },
	[GP8_TDSEL0] = { 0xC0401894, },
	[GP8_TDSEL1] = { 0xC0401898, },
	[GP9_TDSEL0] = { 0xC9B00094, },
	[GP9_TDSEL1] = { 0xC9B00098, },
	[GP10_TDSEL0] = { 0xC9B00894, },
	[GP10_TDSEL1] = { 0xC9B00898, },
	{ /* sentinel */ }
};

static const struct pinmux_bias_reg pinmux_bias_regs[] = {
	{ PINMUX_BIAS_REG("GP0_PULLEN", 0xC10800C0, "GP0_PUDSEL", 0xC10800C4) {
		[ 0] = RCAR_GP_PIN(0,  0),	/* GP0_00 */
		[ 1] = RCAR_GP_PIN(0,  1),	/* GP0_01 */
		[ 2] = RCAR_GP_PIN(0,  2),	/* GP0_02 */
		[ 3] = RCAR_GP_PIN(0,  3),	/* STPWT_EXTFXR_A */
		[ 4] = RCAR_GP_PIN(0,  4),	/* FXR_CLKOUT1_A */
		[ 5] = RCAR_GP_PIN(0,  5),	/* FXR_CLKOUT2_A */
		[ 6] = RCAR_GP_PIN(0,  6),	/* CLK_EXTFXR_A */
		[ 7] = RCAR_GP_PIN(0,  7),	/* FXR_TXDA_A */
		[ 8] = RCAR_GP_PIN(0,  8),	/* FXR_TXENA_N_A */
		[ 9] = RCAR_GP_PIN(0,  9),	/* RXDA_EXTFXR_A */
		[10] = RCAR_GP_PIN(0, 10),	/* FXR_TXDB_A */
		[11] = RCAR_GP_PIN(0, 11),	/* FXR_TXENB_N_A */
		[12] = RCAR_GP_PIN(0, 12),	/* RXDB_EXTFXR_A */
		[13] = RCAR_GP_PIN(0, 13),	/* MSIOF0_SCK */
		[14] = RCAR_GP_PIN(0, 14),	/* MSIOF0_TXD */
		[15] = RCAR_GP_PIN(0, 15),	/* MSIOF0_RXD */
		[16] = RCAR_GP_PIN(0, 16),	/* MSIOF0_SYNC */
		[17] = RCAR_GP_PIN(0, 17),	/* MSIOF0_SS1 */
		[18] = RCAR_GP_PIN(0, 18),	/* MSIOF0_SS2 */
		[19] = RCAR_GP_PIN(0, 19),	/* MSIOF1_SCK_A */
		[20] = RCAR_GP_PIN(0, 20),	/* MSIOF1_TXD_A */
		[21] = RCAR_GP_PIN(0, 21),	/* MSIOF1_RXD_A */
		[22] = RCAR_GP_PIN(0, 22),	/* MSIOF1_SYNC_A */
		[23] = RCAR_GP_PIN(0, 23),	/* MSIOF1_SS1_A */
		[24] = RCAR_GP_PIN(0, 24),	/* MSIOF1_SS2_A */
		[25] = RCAR_GP_PIN(0, 25),	/* DP0_HOTPLUG */
		[26] = RCAR_GP_PIN(0, 26),	/* DP1_HOTPLUG */
		[27] = RCAR_GP_PIN(0, 27),	/* DP2_HOTPLUG */
		[28] = SH_PFC_PIN_NONE,
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP1_PULLEN", 0xC10808C0, "GP1_PUDSEL", 0xC10808C4) {
		[ 0] = RCAR_GP_PIN(1,  0),	/* CAN0RX_INTP0 */
		[ 1] = RCAR_GP_PIN(1,  1),	/* CAN0TX */
		[ 2] = RCAR_GP_PIN(1,  2),	/* CAN1RX_INTP1 */
		[ 3] = RCAR_GP_PIN(1,  3),	/* CAN1TX */
		[ 4] = RCAR_GP_PIN(1,  4),	/* CAN2RX_INTP2 */
		[ 5] = RCAR_GP_PIN(1,  5),	/* CAN2TX */
		[ 6] = RCAR_GP_PIN(1,  6),	/* CAN3RX_INTP3 */
		[ 7] = RCAR_GP_PIN(1,  7),	/* CAN3TX */
		[ 8] = RCAR_GP_PIN(1,  8),	/* CAN4RX_INTP4 */
		[ 9] = RCAR_GP_PIN(1,  9),	/* CAN4TX */
		[10] = RCAR_GP_PIN(1, 10),	/* CAN5RX_INTP5 */
		[11] = RCAR_GP_PIN(1, 11),	/* CAN5TX */
		[12] = RCAR_GP_PIN(1, 12),	/* CAN6RX_INTP6 */
		[13] = RCAR_GP_PIN(1, 13),	/* CAN6TX */
		[14] = RCAR_GP_PIN(1, 14),	/* RLIN30RX_INTP16 */
		[15] = RCAR_GP_PIN(1, 15),	/* RLIN30TX */
		[16] = RCAR_GP_PIN(1, 16),	/* RLIN31RX_INTP17 */
		[17] = RCAR_GP_PIN(1, 17),	/* RLIN31TX */
		[18] = RCAR_GP_PIN(1, 18),	/* RLIN32RX_INTP18 */
		[19] = RCAR_GP_PIN(1, 19),	/* RLIN32TX */
		[20] = RCAR_GP_PIN(1, 20),	/* RLIN33RX_INTP19 */
		[21] = RCAR_GP_PIN(1, 21),	/* RLIN33TX */
		[22] = SH_PFC_PIN_NONE,
		[23] = SH_PFC_PIN_NONE,
		[24] = SH_PFC_PIN_NONE,
		[25] = SH_PFC_PIN_NONE,
		[26] = SH_PFC_PIN_NONE,
		[27] = SH_PFC_PIN_NONE,
		[28] = SH_PFC_PIN_NONE,
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP2_PULLEN", 0xC10810C0, "GP2_PUDSEL", 0xC10810C4) {
		[ 0] = RCAR_GP_PIN(2,  0),	/* RLIN34RX_INTP20_B */
		[ 1] = RCAR_GP_PIN(2,  1),	/* RLIN34TX_B */
		[ 2] = RCAR_GP_PIN(2,  2),	/* RLIN35RX_INTP21_B */
		[ 3] = RCAR_GP_PIN(2,  3),	/* RLIN35TX_B */
		[ 4] = RCAR_GP_PIN(2,  4),	/* RLIN36RX_INTP22_B */
		[ 5] = RCAR_GP_PIN(2,  5),	/* RLIN36TX_B */
		[ 6] = RCAR_GP_PIN(2,  6),	/* RLIN37RX_INTP23_B */
		[ 7] = RCAR_GP_PIN(2,  7),	/* RLIN37TX_B */
		[ 8] = RCAR_GP_PIN(2,  8),	/* CAN12RX_INTP12_B */
		[ 9] = RCAR_GP_PIN(2,  9),	/* CAN12TX_B */
		[10] = RCAR_GP_PIN(2, 10),	/* CAN13RX_INTP13_B */
		[11] = RCAR_GP_PIN(2, 11),	/* CAN13TX_B */
		[12] = RCAR_GP_PIN(2, 12),	/* CAN14RX_INTP14_B */
		[13] = RCAR_GP_PIN(2, 13),	/* CAN14TX_B */
		[14] = RCAR_GP_PIN(2, 14),	/* CAN15RX_INTP15_B */
		[15] = RCAR_GP_PIN(2, 15),	/* CAN15TX_B */
		[16] = RCAR_GP_PIN(2, 16),	/* CAN_CLK */
		[17] = RCAR_GP_PIN(2, 17),	/* INTP32_B */
		[18] = RCAR_GP_PIN(2, 18),	/* INTP33_B */
		[19] = RCAR_GP_PIN(2, 19),	/* SCL0 */
		[20] = RCAR_GP_PIN(2, 20),	/* SDA0 */
		[21] = RCAR_GP_PIN(2, 21),	/* AVS0 */
		[22] = RCAR_GP_PIN(2, 22),	/* AVS1 */
		[23] = RCAR_GP_PIN(2, 23),	/* EXTCLK0O_B */
		[24] = RCAR_GP_PIN(2, 24),	/* TAUD1O0 */
		[25] = RCAR_GP_PIN(2, 25),	/* TAUD1O1 */
		[26] = RCAR_GP_PIN(2, 26),	/* TAUD1O2 */
		[27] = RCAR_GP_PIN(2, 27),	/* TAUD1O3 */
		[28] = RCAR_GP_PIN(2, 28),	/* INTP34_B */
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP3_PULLEN", 0xC08000C0, "GP3_PUDSEL", 0xC08000C4) {
		[ 0] = RCAR_GP_PIN(3,  0),	/* QSPI0_SPCLK */
		[ 1] = RCAR_GP_PIN(3,  1),	/* QSPI0_MOSI_IO0 */
		[ 2] = RCAR_GP_PIN(3,  2),	/* QSPI0_MISO_IO1 */
		[ 3] = RCAR_GP_PIN(3,  3),	/* QSPI0_IO2 */
		[ 4] = RCAR_GP_PIN(3,  4),	/* QSPI0_IO3 */
		[ 5] = RCAR_GP_PIN(3,  5),	/* QSPI0_SSL */
		[ 6] = RCAR_GP_PIN(3,  6),	/* RPC_RESET_N */
		[ 7] = RCAR_GP_PIN(3,  7),	/* RPC_WP_N */
		[ 8] = RCAR_GP_PIN(3,  8),	/* RPC_INT_N */
		[ 9] = RCAR_GP_PIN(3,  9),	/* QSPI1_SPCLK */
		[10] = RCAR_GP_PIN(3, 10),	/* QSPI1_MOSI_IO0 */
		[11] = RCAR_GP_PIN(3, 11),	/* QSPI1_MISO_IO1 */
		[12] = RCAR_GP_PIN(3, 12),	/* QSPI1_IO2 */
		[13] = RCAR_GP_PIN(3, 13),	/* QSPI1_IO3 */
		[14] = RCAR_GP_PIN(3, 14),	/* QSPI1_SSL */
		[15] = RCAR_GP_PIN(3, 15),	/* ERROROUT_N */
		[16] = RCAR_GP_PIN(3, 16),	/* ERRORIN_N */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP4_PULLEN", 0xC08008C0, "GP4_PUDSEL", 0xC08008C4) {
		[ 0] = RCAR_GP_PIN(4,  0),	/* MMC0_SD_CLK */
		[ 1] = RCAR_GP_PIN(4,  1),	/* MMC0_SD_CMD */
		[ 2] = RCAR_GP_PIN(4,  2),	/* MMC0_SD_D0 */
		[ 3] = RCAR_GP_PIN(4,  3),	/* MMC0_SD_D1 */
		[ 4] = RCAR_GP_PIN(4,  4),	/* MMC0_SD_D2 */
		[ 5] = RCAR_GP_PIN(4,  5),	/* MMC0_SD_D3 */
		[ 6] = RCAR_GP_PIN(4,  6),	/* MMC0_D4 */
		[ 7] = RCAR_GP_PIN(4,  7),	/* MMC0_D5 */
		[ 8] = RCAR_GP_PIN(4,  8),	/* MMC0_D6 */
		[ 9] = RCAR_GP_PIN(4,  9),	/* MMC0_D7 */
		[10] = RCAR_GP_PIN(4, 10),	/* MMC0_DS */
		[11] = RCAR_GP_PIN(4, 11),	/* SD0_WP */
		[12] = RCAR_GP_PIN(4, 12),	/* SD0_CD */
		[13] = RCAR_GP_PIN(4, 13),	/* ERRORIN_N */
		[14] = RCAR_GP_PIN(4, 14),	/* PCIE60_CLKREQ_N */
		[15] = RCAR_GP_PIN(4, 15),	/* PCIE61_CLKREQ_N */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP5_PULLEN", 0xC04000C0, "GP5_PUDSEL", 0xC04000C4) {
		[ 0] = RCAR_GP_PIN(5,  0),	/* HTX0 */
		[ 1] = RCAR_GP_PIN(5,  1),	/* HRX0 */
		[ 2] = RCAR_GP_PIN(5,  2),	/* HRTS0_N */
		[ 3] = RCAR_GP_PIN(5,  3),	/* HCTS0_N */
		[ 4] = RCAR_GP_PIN(5,  4),	/* HSCK0 */
		[ 5] = RCAR_GP_PIN(5,  5),	/* SCIF_CLK */
		[ 6] = RCAR_GP_PIN(5,  6),	/* HTX1 */
		[ 7] = RCAR_GP_PIN(5,  7),	/* HRX1 */
		[ 8] = RCAR_GP_PIN(5,  8),	/* HRTS1_N */
		[ 9] = RCAR_GP_PIN(5,  9),	/* HCTS1_N */
		[10] = RCAR_GP_PIN(5, 10),	/* HSCK1 */
		[11] = RCAR_GP_PIN(5, 11),	/* IRQ0_A */
		[12] = RCAR_GP_PIN(5, 12),	/* IRQ1_A */
		[13] = RCAR_GP_PIN(5, 13),	/* IRQ2_A */
		[14] = RCAR_GP_PIN(5, 14),	/* IRQ3_A */
		[15] = RCAR_GP_PIN(5, 15),	/* TCLK1 */
		[16] = RCAR_GP_PIN(5, 16),	/* TCLK2 */
		[17] = RCAR_GP_PIN(5, 17),	/* TCLK3 */
		[18] = RCAR_GP_PIN(5, 18),	/* TCLK4 */
		[19] = RCAR_GP_PIN(5, 19),	/* TPU0TO0 */
		[20] = RCAR_GP_PIN(5, 20),	/* TPU0TO1 */
		[21] = RCAR_GP_PIN(5, 21),	/* TPU0TO2 */
		[22] = RCAR_GP_PIN(5, 22),	/* TPU0TO3 */
		[23] = SH_PFC_PIN_NONE,
		[24] = SH_PFC_PIN_NONE,
		[25] = SH_PFC_PIN_NONE,
		[26] = SH_PFC_PIN_NONE,
		[27] = SH_PFC_PIN_NONE,
		[28] = SH_PFC_PIN_NONE,
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP6_PULLEN", 0xC04008C0, "GP6_PUDSEL", 0xC04008C4) {
		[ 0] = RCAR_GP_PIN(6,  0),	/* RIF6_D0 */
		[ 1] = RCAR_GP_PIN(6,  1),	/* RIF6_D1 */
		[ 2] = RCAR_GP_PIN(6,  2),	/* RIF6_SYNC */
		[ 3] = RCAR_GP_PIN(6,  3),	/* RIF6_CLK */
		[ 4] = RCAR_GP_PIN(6,  4),	/* MSIOF7_SCK_A */
		[ 5] = RCAR_GP_PIN(6,  5),	/* MSIOF7_TXD_A */
		[ 6] = RCAR_GP_PIN(6,  6),	/* MSIOF7_RXD_A */
		[ 7] = RCAR_GP_PIN(6,  7),	/* MSIOF7_SYNC_A */
		[ 8] = RCAR_GP_PIN(6,  8),	/* MSIOF7_SS1_A */
		[ 9] = RCAR_GP_PIN(6,  9),	/* MSIOF7_SS2_A */
		[10] = RCAR_GP_PIN(6, 10),	/* MSIOF4_SCK_B */
		[11] = RCAR_GP_PIN(6, 11),	/* MSIOF4_TXD_B */
		[12] = RCAR_GP_PIN(6, 12),	/* MSIOF4_RXD_B */
		[13] = RCAR_GP_PIN(6, 13),	/* MSIOF4_SYNC_B */
		[14] = RCAR_GP_PIN(6, 14),	/* MSIOF4_SS1_B */
		[15] = RCAR_GP_PIN(6, 15),	/* MSIOF4_SS2_B */
		[16] = RCAR_GP_PIN(6, 16),	/* SSI0_SCK */
		[17] = RCAR_GP_PIN(6, 17),	/* SSI0_WS */
		[18] = RCAR_GP_PIN(6, 18),	/* SSI0_SD */
		[19] = RCAR_GP_PIN(6, 19),	/* AUDIO0_CLKOUT0 */
		[20] = RCAR_GP_PIN(6, 20),	/* AUDIO0_CLKOUT1 */
		[21] = RCAR_GP_PIN(6, 21),	/* SSI1_SCK */
		[22] = RCAR_GP_PIN(6, 22),	/* SSI1_WS */
		[23] = RCAR_GP_PIN(6, 23),	/* SSI1_SD */
		[24] = RCAR_GP_PIN(6, 24),	/* AUDIO0_CLKOUT2 */
		[25] = RCAR_GP_PIN(6, 25),	/* AUDIO0_CLKOUT3 */
		[26] = RCAR_GP_PIN(6, 26),	/* SSI2_SCK */
		[27] = RCAR_GP_PIN(6, 27),	/* SSI2_WS */
		[28] = RCAR_GP_PIN(6, 28),	/* SSI2_SD */
		[29] = RCAR_GP_PIN(6, 29),	/* AUDIO1_CLKOUT0 */
		[30] = RCAR_GP_PIN(6, 30),	/* AUDIO1_CLKOUT1 */
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP7_PULLEN", 0xC04010C0, "GP7_PUDSEL", 0xC04010C4) {
		[ 0] = RCAR_GP_PIN(7,  0),	/* SSI3_SCK */
		[ 1] = RCAR_GP_PIN(7,  1),	/* SSI3_WS */
		[ 2] = RCAR_GP_PIN(7,  2),	/* SSI3_SD */
		[ 3] = RCAR_GP_PIN(7,  3),	/* AUDIO1_CLKOUT2 */
		[ 4] = RCAR_GP_PIN(7,  4),	/* AUDIO1_CLKOUT3 */
		[ 5] = RCAR_GP_PIN(7,  5),	/* SSI4_SCK */
		[ 6] = RCAR_GP_PIN(7,  6),	/* SSI4_WS */
		[ 7] = RCAR_GP_PIN(7,  7),	/* SSI4_SD */
		[ 8] = RCAR_GP_PIN(7,  8),	/* AUDIO_CLKA_A */
		[ 9] = RCAR_GP_PIN(7,  9),	/* SSI5_SCK */
		[10] = RCAR_GP_PIN(7, 10),	/* SSI5_WS */
		[11] = RCAR_GP_PIN(7, 11),	/* SSI5_SD */
		[12] = RCAR_GP_PIN(7, 12),	/* AUDIO_CLKB_A */
		[13] = RCAR_GP_PIN(7, 13),	/* SSI6_SCK */
		[14] = RCAR_GP_PIN(7, 14),	/* SSI6_WS */
		[15] = RCAR_GP_PIN(7, 15),	/* SSI6_SD */
		[16] = RCAR_GP_PIN(7, 16),	/* AUDIO_CLKC_A */
		[17] = RCAR_GP_PIN(7, 17),	/* MSIOF5_SCK */
		[18] = RCAR_GP_PIN(7, 18),	/* GP07_18 */
		[19] = RCAR_GP_PIN(7, 19),	/* GP07_19 */
		[20] = RCAR_GP_PIN(7, 20),	/* MSIOF5_TXD */
		[21] = RCAR_GP_PIN(7, 21),	/* MSIOF5_RXD */
		[22] = RCAR_GP_PIN(7, 22),	/* MSIOF5_SYNC */
		[23] = RCAR_GP_PIN(7, 23),	/* MSIOF5_SS1 */
		[24] = RCAR_GP_PIN(7, 24),	/* MSIOF5_SS2 */
		[25] = RCAR_GP_PIN(7, 25),	/* MSIOF6_SCK_B */
		[26] = RCAR_GP_PIN(7, 26),	/* MSIOF6_TXD_B */
		[27] = RCAR_GP_PIN(7, 27),	/* MSIOF6_RXD_B */
		[28] = RCAR_GP_PIN(7, 28),	/* MSIOF6_SYNC_B */
		[29] = RCAR_GP_PIN(7, 29),	/* MSIOF6_SS1_B */
		[30] = RCAR_GP_PIN(7, 30),	/* MSIOF6_SS2_B */
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP8_PULLEN", 0xC04018C0, "GP8_PUDSEL", 0xC04018C4) {
		[ 0] = RCAR_GP_PIN(8,  0),	/* SCL1 */
		[ 1] = RCAR_GP_PIN(8,  1),	/* SDA1 */
		[ 2] = RCAR_GP_PIN(8,  2),	/* SCL2 */
		[ 3] = RCAR_GP_PIN(8,  3),	/* SDA2 */
		[ 4] = RCAR_GP_PIN(8,  4),	/* SCL3 */
		[ 5] = RCAR_GP_PIN(8,  5),	/* SDA3 */
		[ 6] = RCAR_GP_PIN(8,  6),	/* SCL4 */
		[ 7] = RCAR_GP_PIN(8,  7),	/* SDA4 */
		[ 8] = RCAR_GP_PIN(8,  8),	/* SCL5 */
		[ 9] = RCAR_GP_PIN(8,  9),	/* SDA5 */
		[10] = RCAR_GP_PIN(8, 10),	/* SCL6 */
		[11] = RCAR_GP_PIN(8, 11),	/* SDA6 */
		[12] = RCAR_GP_PIN(8, 12),	/* SCL7 */
		[13] = RCAR_GP_PIN(8, 13),	/* SDA7 */
		[14] = RCAR_GP_PIN(8, 14),	/* SCL8 */
		[15] = RCAR_GP_PIN(8, 15),	/* SDA8 */
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
		[26] = RCAR_GP_PIN(8, 26),	/* S3CL0 */
		[27] = RCAR_GP_PIN(8, 27),	/* S3DA0 */
		[28] = RCAR_GP_PIN(8, 28),	/* S3CL1 */
		[29] = RCAR_GP_PIN(8, 29),	/* S3DA1 */
		[30] = RCAR_GP_PIN(8, 30),	/* S3CL2 */
		[31] = RCAR_GP_PIN(8, 31),	/* S3DA2 */
	} },
	{ PINMUX_BIAS_REG("GP9_PULLEN", 0xC9B000C0, "GP9_PUDSEL", 0xC9B000C4) {
		[ 0] = RCAR_GP_PIN(9,  0),	/* ETHES0_PPS */
		[ 1] = RCAR_GP_PIN(9,  1),	/* ETHES0_CAPTURE */
		[ 2] = RCAR_GP_PIN(9,  2),	/* ETHES0_MATCH */
		[ 3] = RCAR_GP_PIN(9,  3),	/* ETHES4_PPS */
		[ 4] = RCAR_GP_PIN(9,  4),	/* ETHES4_CAPTURE */
		[ 5] = RCAR_GP_PIN(9,  5),	/* ETHES4_MATCH */
		[ 6] = RCAR_GP_PIN(9,  6),	/* ETH25G0_MDIO */
		[ 7] = RCAR_GP_PIN(9,  7),	/* ETH25G0_MDC */
		[ 8] = RCAR_GP_PIN(9,  8),	/* ETH25G0_LINK */
		[ 9] = RCAR_GP_PIN(9,  9),	/* ETH25G0_PHYINT */
		[10] = RCAR_GP_PIN(9, 10),	/* ETH10G0_MDIO */
		[11] = RCAR_GP_PIN(9, 11),	/* ETH10G0_MDC */
		[12] = RCAR_GP_PIN(9, 12),	/* ETH10G0_LINK */
		[13] = RCAR_GP_PIN(9, 13),	/* ETH10G0_PHYINT */
		[14] = RCAR_GP_PIN(9, 14),	/* RSW3_PPS */
		[15] = RCAR_GP_PIN(9, 15),	/* RSW3_CAPTURE */
		[16] = RCAR_GP_PIN(9, 16),	/* RSW3_MATCH */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ PINMUX_BIAS_REG("GP10_PULLEN", 0xC9B008C0, "GP10_PUDSEL", 0xC9B008C4) {
		[ 0] = RCAR_GP_PIN(10,  0),	/* USB0_PWEN */
		[ 1] = RCAR_GP_PIN(10,  1),	/* USB0_OVC */
		[ 2] = RCAR_GP_PIN(10,  2),	/* USB0_VBUS_VALID */
		[ 3] = RCAR_GP_PIN(10,  3),	/* USB1_PWEN */
		[ 4] = RCAR_GP_PIN(10,  4),	/* USB1_OVC */
		[ 5] = RCAR_GP_PIN(10,  5),	/* USB1_VBUS_VALID */
		[ 6] = RCAR_GP_PIN(10,  6),	/* USB2_PWEN */
		[ 7] = RCAR_GP_PIN(10,  7),	/* USB2_OVC */
		[ 8] = RCAR_GP_PIN(10,  8),	/* USB2_VBUS_VALID */
		[ 9] = RCAR_GP_PIN(10,  9),	/* USB3_PWEN */
		[10] = RCAR_GP_PIN(10, 10),	/* USB3_OVC */
		[11] = RCAR_GP_PIN(10, 11),	/* USB3_VBUS_VALID */
		[12] = RCAR_GP_PIN(10, 12),	/* PCIE40_CLKREQ_N */
		[13] = RCAR_GP_PIN(10, 13),	/* PCIE41_CLKREQ_N */
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
		[29] = SH_PFC_PIN_NONE,
		[30] = SH_PFC_PIN_NONE,
		[31] = SH_PFC_PIN_NONE,
	} },
	{ /* sentinel */ },
};

static int rcar5_pinconf_write_bit(struct sh_pfc *pfc, unsigned int bit,
				   u16 value, u32 reg)
{
	u32 val = sh_pfc_read(pfc, reg);

	val &= ~BIT(bit);
	val |= value << bit;

	sh_pfc_write(pfc, reg, val);

	return 0;
}

static int rcar5_pinconf_set_drive_strength(struct sh_pfc *pfc,
					    unsigned int pin, u16 strength)
{
	unsigned int bank = pin / 32;
	unsigned int bit = pin % 32;
	const struct rcar5_pinmux_drive_reg *reg = &pinmux_drive_regs[bank];

	if (reg->pins[bit] != pin)
		return -EINVAL;

	if (strength < 3 || strength > 24)
		return -EINVAL;

	/* Convert the value from mA based on a full drive strength value of
	 * 24mA. We can make the full value configurable later if needed.
	 */
	strength = strength / 3 - 1;

	rcar5_pinconf_write_bit(pfc, bit, bitfield_extract(strength, 0, 1), reg->drvctrl0);
	rcar5_pinconf_write_bit(pfc, bit, bitfield_extract(strength, 1, 1), reg->drvctrl1);
	rcar5_pinconf_write_bit(pfc, bit, bitfield_extract(strength, 2, 1), reg->drvctrl2);

	return 0;
}

static const struct sh_pfc_soc_operations r8a78000_pin_ops = {
	.get_bias = rcar_pinmux_get_bias,
	.set_bias = rcar_pinmux_set_bias,
	.set_drive_strength = rcar5_pinconf_set_drive_strength,
};

const struct sh_pfc_soc_info r8a78000_pinmux_info = {
	.name = "r8a78000_pfc",
	.ops = &r8a78000_pin_ops,
	.unlock_reg = 0x1ff,	/* PMMRn mask */

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
