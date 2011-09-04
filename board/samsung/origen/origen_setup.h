/*
 * Machine Specific Values for ORIGEN board based on S5PV310
 *
 * Copyright (C) 2011 Samsung Electronics
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

#ifndef _ORIGEN_SETUP_H
#define _ORIGEN_SETUP_H

#include <config.h>
#include <version.h>
#include <asm/arch/cpu.h>

/* Offsets of clock registers (sources and dividers) */
#define CLK_SRC_CPU_OFFSET	0x14200
#define CLK_DIV_CPU0_OFFSET	0x14500
#define CLK_DIV_CPU1_OFFSET	0x14504

#define CLK_SRC_DMC_OFFSET	0x10200
#define CLK_DIV_DMC0_OFFSET	0x10500
#define CLK_DIV_DMC1_OFFSET	0x10504

#define CLK_SRC_TOP0_OFFSET	0xC210
#define CLK_SRC_TOP1_OFFSET	0xC214
#define CLK_DIV_TOP_OFFSET	0xC510

#define CLK_SRC_LEFTBUS_OFFSET	0x4200
#define CLK_DIV_LEFTBUS_OFFSET	0x4500

#define CLK_SRC_RIGHTBUS_OFFSET	0x8200
#define CLK_DIV_RIGHTBUS_OFFSET	0x8500

#define CLK_SRC_FSYS_OFFSET	0xC240
#define CLK_DIV_FSYS1_OFFSET	0xC544
#define CLK_DIV_FSYS2_OFFSET	0xC548
#define CLK_DIV_FSYS3_OFFSET	0xC54C

#define CLK_SRC_PERIL0_OFFSET	0xC250
#define CLK_DIV_PERIL0_OFFSET	0xC550

#define APLL_LOCK_OFFSET	0x14000
#define MPLL_LOCK_OFFSET	0x14008
#define APLL_CON0_OFFSET	0x14100
#define APLL_CON1_OFFSET	0x14104
#define MPLL_CON0_OFFSET	0x14108
#define MPLL_CON1_OFFSET	0x1410C

#define EPLL_LOCK_OFFSET	0xC010
#define VPLL_LOCK_OFFSET	0xC020
#define EPLL_CON0_OFFSET	0xC110
#define EPLL_CON1_OFFSET	0xC114
#define VPLL_CON0_OFFSET	0xC120
#define VPLL_CON1_OFFSET	0xC124

/* DMC: DRAM Controllor Register offsets */
#define DMC_CONCONTROL		0x00
#define DMC_MEMCONTROL		0x04
#define DMC_MEMCONFIG0		0x08
#define DMC_MEMCONFIG1		0x0C
#define DMC_DIRECTCMD		0x10
#define DMC_PRECHCONFIG		0x14
#define DMC_PHYCONTROL0		0x18
#define DMC_PHYCONTROL1		0x1C
#define DMC_PHYCONTROL2		0x20
#define DMC_TIMINGAREF		0x30
#define DMC_TIMINGROW		0x34
#define DMC_TIMINGDATA		0x38
#define DMC_TIMINGPOWER		0x3C
#define DMC_PHYZQCONTROL	0x44

/* Bus Configuration Register Address */
#define ASYNC_CONFIG		0x10010350

/* MIU Config Register Offsets*/
#define APB_SFR_INTERLEAVE_CONF_OFFSET	0x400
#define APB_SFR_ARBRITATION_CONF_OFFSET		0xC00

/* Offset for inform registers */
#define INFORM0_OFFSET			0x800
#define INFORM1_OFFSET			0x804

/* GPIO Offsets for UART: GPIO Contol Register */
#define S5PC210_GPIO_A0_CON_OFFSET	0x00
#define S5PC210_GPIO_A1_CON_OFFSET	0x20

/* UART Register offsets */
#define ULCON_OFFSET		0x00
#define UCON_OFFSET		0x04
#define UFCON_OFFSET		0x08
#define UBRDIV_OFFSET		0x28
#define UFRACVAL_OFFSET		0x2C

/* TZPC : Register Offsets */
#define TZPC0_BASE		0x10110000
#define TZPC1_BASE		0x10120000
#define TZPC2_BASE		0x10130000
#define TZPC3_BASE		0x10140000
#define TZPC4_BASE		0x10150000
#define TZPC5_BASE		0x10160000

#define TZPC_DECPROT0SET_OFFSET	0x804
#define TZPC_DECPROT1SET_OFFSET	0x810
#define TZPC_DECPROT2SET_OFFSET	0x81C
#define TZPC_DECPROT3SET_OFFSET	0x828

/* CLK_SRC_CPU */
#define MUX_HPM_SEL_MOUTAPLL		0x0
#define MUX_HPM_SEL_SCLKMPLL		0x1
#define MUX_CORE_SEL_MOUTAPLL		0x0
#define MUX_CORE_SEL_SCLKMPLL		0x1
#define MUX_MPLL_SEL_FILPLL		0x0
#define MUX_MPLL_SEL_MOUTMPLLFOUT	0x1
#define MUX_APLL_SEL_FILPLL		0x0
#define MUX_APLL_SEL_MOUTMPLLFOUT	0x1
#define CLK_SRC_CPU_VAL			((MUX_HPM_SEL_MOUTAPLL << 20) \
					| (MUX_CORE_SEL_MOUTAPLL << 16) \
					| (MUX_MPLL_SEL_MOUTMPLLFOUT << 8)\
					| (MUX_APLL_SEL_MOUTMPLLFOUT << 0))

/* CLK_DIV_CPU0 */
#define APLL_RATIO		0x0
#define PCLK_DBG_RATIO		0x1
#define ATB_RATIO		0x3
#define PERIPH_RATIO		0x3
#define COREM1_RATIO		0x7
#define COREM0_RATIO		0x3
#define CORE_RATIO		0x0
#define CLK_DIV_CPU0_VAL	((APLL_RATIO << 24) \
				| (PCLK_DBG_RATIO << 20) \
				| (ATB_RATIO << 16) \
				| (PERIPH_RATIO << 12) \
				| (COREM1_RATIO << 8) \
				| (COREM0_RATIO << 4) \
				| (CORE_RATIO << 0))

/* CLK_DIV_CPU1 */
#define HPM_RATIO		0x0
#define COPY_RATIO		0x3
#define CLK_DIV_CPU1_VAL	((HPM_RATIO << 4) | (COPY_RATIO))

/* CLK_SRC_DMC */
#define MUX_PWI_SEL_XXTI		0x0
#define MUX_PWI_SEL_XUSBXTI		0x1
#define MUX_PWI_SEL_SCLK_HDMI24M	0x2
#define MUX_PWI_SEL_SCLK_USBPHY0	0x3
#define MUX_PWI_SEL_SCLK_USBPHY1	0x4
#define MUX_PWI_SEL_SCLK_HDMIPHY	0x5
#define MUX_PWI_SEL_SCLKMPLL		0x6
#define MUX_PWI_SEL_SCLKEPLL		0x7
#define MUX_PWI_SEL_SCLKVPLL		0x8
#define MUX_DPHY_SEL_SCLKMPLL		0x0
#define MUX_DPHY_SEL_SCLKAPLL		0x1
#define MUX_DMC_BUS_SEL_SCLKMPLL	0x0
#define MUX_DMC_BUS_SEL_SCLKAPLL	0x1
#define CLK_SRC_DMC_VAL			((MUX_PWI_SEL_XUSBXTI << 16) \
					| (MUX_DPHY_SEL_SCLKMPLL << 8) \
					| (MUX_DMC_BUS_SEL_SCLKMPLL << 4))

/* CLK_DIV_DMC0 */
#define CORE_TIMERS_RATIO	0x1
#define COPY2_RATIO		0x3
#define DMCP_RATIO		0x1
#define DMCD_RATIO		0x1
#define DMC_RATIO		0x1
#define DPHY_RATIO		0x1
#define ACP_PCLK_RATIO		0x1
#define ACP_RATIO		0x3
#define CLK_DIV_DMC0_VAL	((CORE_TIMERS_RATIO << 28) \
				| (COPY2_RATIO << 24) \
				| (DMCP_RATIO << 20) \
				| (DMCD_RATIO << 16) \
				| (DMC_RATIO << 12) \
				| (DPHY_RATIO << 8) \
				| (ACP_PCLK_RATIO << 4)	\
				| (ACP_RATIO << 0))

/* CLK_DIV_DMC1 */
#define DPM_RATIO		0x1
#define DVSEM_RATIO		0x1
#define PWI_RATIO		0x1
#define CLK_DIV_DMC1_VAL	((DPM_RATIO << 24) \
				| (DVSEM_RATIO << 16) \
				| (PWI_RATIO << 8))

/* CLK_SRC_TOP0 */
#define MUX_ONENAND_SEL_ACLK_133	0x0
#define MUX_ONENAND_SEL_ACLK_160	0x1
#define MUX_ACLK_133_SEL_SCLKMPLL	0x0
#define MUX_ACLK_133_SEL_SCLKAPLL	0x1
#define MUX_ACLK_160_SEL_SCLKMPLL	0x0
#define MUX_ACLK_160_SEL_SCLKAPLL	0x1
#define MUX_ACLK_100_SEL_SCLKMPLL	0x0
#define MUX_ACLK_100_SEL_SCLKAPLL	0x1
#define MUX_ACLK_200_SEL_SCLKMPLL	0x0
#define MUX_ACLK_200_SEL_SCLKAPLL	0x1
#define MUX_VPLL_SEL_FINPLL		0x0
#define MUX_VPLL_SEL_FOUTVPLL		0x1
#define MUX_EPLL_SEL_FINPLL		0x0
#define MUX_EPLL_SEL_FOUTEPLL		0x1
#define MUX_ONENAND_1_SEL_MOUTONENAND	0x0
#define MUX_ONENAND_1_SEL_SCLKVPLL	0x1
#define CLK_SRC_TOP0_VAL		((MUX_ONENAND_SEL_ACLK_133 << 28) \
					| (MUX_ACLK_133_SEL_SCLKMPLL << 24) \
					| (MUX_ACLK_160_SEL_SCLKMPLL << 20) \
					| (MUX_ACLK_100_SEL_SCLKMPLL << 16) \
					| (MUX_ACLK_200_SEL_SCLKMPLL << 12) \
					| (MUX_VPLL_SEL_FINPLL << 8) \
					| (MUX_EPLL_SEL_FINPLL << 4)\
					| (MUX_ONENAND_1_SEL_MOUTONENAND << 0))

/* CLK_SRC_TOP1 */
#define VPLLSRC_SEL_FINPLL	0x0
#define VPLLSRC_SEL_SCLKHDMI24M	0x1
#define CLK_SRC_TOP1_VAL	(VPLLSRC_SEL_FINPLL)

/* CLK_DIV_TOP */
#define ONENAND_RATIO		0x0
#define ACLK_133_RATIO		0x5
#define ACLK_160_RATIO		0x4
#define ACLK_100_RATIO		0x7
#define ACLK_200_RATIO		0x3
#define CLK_DIV_TOP_VAL		((ONENAND_RATIO << 16)	\
				| (ACLK_133_RATIO << 12)\
				| (ACLK_160_RATIO << 8)	\
				| (ACLK_100_RATIO << 4)	\
				| (ACLK_200_RATIO << 0))

/* CLK_SRC_LEFTBUS */
#define MUX_GDL_SEL_SCLKMPLL	0x0
#define MUX_GDL_SEL_SCLKAPLL	0x1
#define CLK_SRC_LEFTBUS_VAL	(MUX_GDL_SEL_SCLKMPLL)

/* CLK_DIV_LEFTBUS */
#define GPL_RATIO		0x1
#define GDL_RATIO		0x3
#define CLK_DIV_LEFTBUS_VAL	((GPL_RATIO << 4) | (GDL_RATIO))

/* CLK_SRC_RIGHTBUS */
#define MUX_GDR_SEL_SCLKMPLL	0x0
#define MUX_GDR_SEL_SCLKAPLL	0x1
#define CLK_SRC_RIGHTBUS_VAL	(MUX_GDR_SEL_SCLKMPLL)

/* CLK_DIV_RIGHTBUS */
#define GPR_RATIO		0x1
#define GDR_RATIO		0x3
#define CLK_DIV_RIGHTBUS_VAL	((GPR_RATIO << 4) | (GDR_RATIO))

/* CLK_SRS_FSYS: 6 = SCLKMPLL */
#define SATA_SEL_SCLKMPLL	0
#define SATA_SEL_SCLKAPLL	1

#define MMC_SEL_XXTI		0
#define MMC_SEL_XUSBXTI		1
#define MMC_SEL_SCLK_HDMI24M	2
#define MMC_SEL_SCLK_USBPHY0	3
#define MMC_SEL_SCLK_USBPHY1	4
#define MMC_SEL_SCLK_HDMIPHY	5
#define MMC_SEL_SCLKMPLL	6
#define MMC_SEL_SCLKEPLL	7
#define MMC_SEL_SCLKVPLL	8

#define MMCC0_SEL		MMC_SEL_SCLKMPLL
#define MMCC1_SEL		MMC_SEL_SCLKMPLL
#define MMCC2_SEL		MMC_SEL_SCLKMPLL
#define MMCC3_SEL		MMC_SEL_SCLKMPLL
#define MMCC4_SEL		MMC_SEL_SCLKMPLL
#define CLK_SRC_FSYS_VAL	((SATA_SEL_SCLKMPLL << 24) \
				| (MMCC4_SEL << 16) \
				| (MMCC3_SEL << 12) \
				| (MMCC2_SEL << 8) \
				| (MMCC1_SEL << 4) \
				| (MMCC0_SEL << 0))

/* SCLK_MMC[0-4] = MOUTMMC[0-4]/(MMC[0-4]_RATIO + 1)/(MMC[0-4]_PRE_RATIO +1) */
/* CLK_DIV_FSYS1 */
#define MMC0_RATIO		0xF
#define MMC0_PRE_RATIO		0x0
#define MMC1_RATIO		0xF
#define MMC1_PRE_RATIO		0x0
#define CLK_DIV_FSYS1_VAL	((MMC1_PRE_RATIO << 24) \
				| (MMC1_RATIO << 16) \
				| (MMC0_PRE_RATIO << 8) \
				| (MMC0_RATIO << 0))

/* CLK_DIV_FSYS2 */
#define MMC2_RATIO		0xF
#define MMC2_PRE_RATIO		0x0
#define MMC3_RATIO		0xF
#define MMC3_PRE_RATIO		0x0
#define CLK_DIV_FSYS2_VAL	((MMC3_PRE_RATIO << 24) \
				| (MMC3_RATIO << 16) \
				| (MMC2_PRE_RATIO << 8) \
				| (MMC2_RATIO << 0))

/* CLK_DIV_FSYS3 */
#define MMC4_RATIO		0xF
#define MMC4_PRE_RATIO		0x0
#define CLK_DIV_FSYS3_VAL	((MMC4_PRE_RATIO << 8) \
				| (MMC4_RATIO << 0))

/* CLK_SRC_PERIL0 */
#define UART_SEL_XXTI		0
#define UART_SEL_XUSBXTI	1
#define UART_SEL_SCLK_HDMI24M	2
#define UART_SEL_SCLK_USBPHY0	3
#define UART_SEL_SCLK_USBPHY1	4
#define UART_SEL_SCLK_HDMIPHY	5
#define UART_SEL_SCLKMPLL	6
#define UART_SEL_SCLKEPLL	7
#define UART_SEL_SCLKVPLL	8

#define UART0_SEL		UART_SEL_SCLKMPLL
#define UART1_SEL		UART_SEL_SCLKMPLL
#define UART2_SEL		UART_SEL_SCLKMPLL
#define UART3_SEL		UART_SEL_SCLKMPLL
#define UART4_SEL		UART_SEL_SCLKMPLL
#define CLK_SRC_PERIL0_VAL	((UART4_SEL << 16) \
				| (UART3_SEL << 12) \
				| (UART2_SEL << 8) \
				| (UART1_SEL << 4) \
				| (UART0_SEL << 0))

/* SCLK_UART[0-4] = MOUTUART[0-4]/(UART[0-4]_RATIO + 1) */
/* CLK_DIV_PERIL0 */
#define UART0_RATIO		7
#define UART1_RATIO		7
#define UART2_RATIO		7
#define UART3_RATIO		7
#define UART4_RATIO		7
#define CLK_DIV_PERIL0_VAL	((UART4_RATIO << 16) \
				| (UART3_RATIO << 12) \
				| (UART2_RATIO << 8) \
				| (UART1_RATIO << 4) \
				| (UART0_RATIO << 0))

/* Required period to generate a stable clock output */
/* PLL_LOCK_TIME */
#define PLL_LOCKTIME		0x1C20

/* PLL Values */
#define DISABLE			0
#define ENABLE			1
#define SET_PLL(mdiv, pdiv, sdiv)	((ENABLE << 31)\
					| (mdiv << 16) \
					| (pdiv << 8) \
					| (sdiv << 0))

/* APLL_CON0 */
#define APLL_MDIV		0xFA
#define APLL_PDIV		0x6
#define APLL_SDIV		0x1
#define APLL_CON0_VAL		SET_PLL(APLL_MDIV, APLL_PDIV, APLL_SDIV)

/* APLL_CON1 */
#define APLL_AFC_ENB		0x1
#define APLL_AFC		0xC
#define APLL_CON1_VAL		((APLL_AFC_ENB << 31) | (APLL_AFC << 0))

/* MPLL_CON0 */
#define MPLL_MDIV		0xC8
#define MPLL_PDIV		0x6
#define MPLL_SDIV		0x1
#define MPLL_CON0_VAL		SET_PLL(MPLL_MDIV, MPLL_PDIV, MPLL_SDIV)

/* MPLL_CON1 */
#define MPLL_AFC_ENB		0x0
#define MPLL_AFC		0x1C
#define MPLL_CON1_VAL		((MPLL_AFC_ENB << 31) | (MPLL_AFC << 0))

/* EPLL_CON0 */
#define EPLL_MDIV		0x30
#define EPLL_PDIV		0x3
#define EPLL_SDIV		0x2
#define EPLL_CON0_VAL		SET_PLL(EPLL_MDIV, EPLL_PDIV, EPLL_SDIV)

/* EPLL_CON1 */
#define EPLL_K			0x0
#define EPLL_CON1_VAL		(EPLL_K >> 0)

/* VPLL_CON0 */
#define VPLL_MDIV		0x35
#define VPLL_PDIV		0x3
#define VPLL_SDIV		0x2
#define VPLL_CON0_VAL		SET_PLL(VPLL_MDIV, VPLL_PDIV, VPLL_SDIV)

/* VPLL_CON1 */
#define VPLL_SSCG_EN		DISABLE
#define VPLL_SEL_PF_DN_SPREAD	0x0
#define VPLL_MRR		0x11
#define VPLL_MFR		0x0
#define VPLL_K			0x400
#define VPLL_CON1_VAL		((VPLL_SSCG_EN << 31)\
				| (VPLL_SEL_PF_DN_SPREAD << 29) \
				| (VPLL_MRR << 24) \
				| (VPLL_MFR << 16) \
				| (VPLL_K << 0))
/*
 * UART GPIO_A0/GPIO_A1 Control Register Value
 * 0x2: UART Function
 */
#define S5PC210_GPIO_A0_CON_VAL	0x22222222
#define S5PC210_GPIO_A1_CON_VAL	0x222222

/* ULCON: UART Line Control Value 8N1 */
#define WORD_LEN_5_BIT		0x00
#define WORD_LEN_6_BIT		0x01
#define WORD_LEN_7_BIT		0x02
#define WORD_LEN_8_BIT		0x03

#define STOP_BIT_1		0x00
#define STOP_BIT_2		0x01

#define NO_PARITY			0x00
#define ODD_PARITY			0x4
#define EVEN_PARITY			0x5
#define FORCED_PARITY_CHECK_AS_1	0x6
#define FORCED_PARITY_CHECK_AS_0	0x7

#define INFRAMODE_NORMAL		0x00
#define INFRAMODE_INFRARED		0x01

#define ULCON_VAL		((INFRAMODE_NORMAL << 6) \
				| (NO_PARITY << 3) \
				| (STOP_BIT_1 << 2) \
				| (WORD_LEN_8_BIT << 0))

/*
 * UCON: UART Control Value
 * Tx_interrupt Type: Level
 * Rx_interrupt Type: Level
 * Rx Timeout Enabled: Yes
 * Rx-Error Atatus_Int Enable: Yes
 * Loop_Back: No
 * Break Signal: No
 * Transmit mode : Interrupt request/polling
 * Receive mode : Interrupt request/polling
 */
#define TX_PULSE_INTERRUPT	0
#define TX_LEVEL_INTERRUPT	1
#define RX_PULSE_INTERRUPT	0
#define RX_LEVEL_INTERRUPT	1

#define RX_TIME_OUT		ENABLE
#define RX_ERROR_STATE_INT_ENB	ENABLE
#define LOOP_BACK		DISABLE
#define BREAK_SIGNAL		DISABLE

#define TX_MODE_DISABLED	0X00
#define TX_MODE_IRQ_OR_POLL	0X01
#define TX_MODE_DMA		0X02

#define RX_MODE_DISABLED	0X00
#define RX_MODE_IRQ_OR_POLL	0X01
#define RX_MODE_DMA		0X02

#define UCON_VAL		((TX_LEVEL_INTERRUPT << 9) \
				| (RX_LEVEL_INTERRUPT << 8) \
				| (RX_TIME_OUT << 7) \
				| (RX_ERROR_STATE_INT_ENB << 6) \
				| (LOOP_BACK << 5) \
				| (BREAK_SIGNAL << 4) \
				| (TX_MODE_IRQ_OR_POLL << 2) \
				| (RX_MODE_IRQ_OR_POLL << 0))

/*
 * UFCON: UART FIFO Control Value
 * Tx FIFO Trigger LEVEL: 2 Bytes (001)
 * Rx FIFO Trigger LEVEL: 2 Bytes (001)
 * Tx Fifo Reset: No
 * Rx Fifo Reset: No
 * FIFO Enable: Yes
 */
#define TX_FIFO_TRIGGER_LEVEL_0_BYTES	0x00
#define TX_FIFO_TRIGGER_LEVEL_2_BYTES	0x1
#define TX_FIFO_TRIGGER_LEVEL_4_BYTES	0x2
#define TX_FIFO_TRIGGER_LEVEL_6_BYTES	0x3
#define TX_FIFO_TRIGGER_LEVEL_8_BYTES	0x4
#define TX_FIFO_TRIGGER_LEVEL_10_BYTES	0x5
#define TX_FIFO_TRIGGER_LEVEL_12_BYTES	0x6
#define TX_FIFO_TRIGGER_LEVEL_14_BYTES	0x7

#define RX_FIFO_TRIGGER_LEVEL_2_BYTES	0x0
#define RX_FIFO_TRIGGER_LEVEL_4_BYTES	0x1
#define RX_FIFO_TRIGGER_LEVEL_6_BYTES	0x2
#define RX_FIFO_TRIGGER_LEVEL_8_BYTES	0x3
#define RX_FIFO_TRIGGER_LEVEL_10_BYTES	0x4
#define RX_FIFO_TRIGGER_LEVEL_12_BYTES	0x5
#define RX_FIFO_TRIGGER_LEVEL_14_BYTES	0x6
#define RX_FIFO_TRIGGER_LEVEL_16_BYTES	0x7

#define TX_FIFO_TRIGGER_LEVEL		TX_FIFO_TRIGGER_LEVEL_2_BYTES
#define RX_FIFO_TRIGGER_LEVEL		RX_FIFO_TRIGGER_LEVEL_4_BYTES
#define TX_FIFO_RESET			DISABLE
#define RX_FIFO_RESET			DISABLE
#define FIFO_ENABLE			ENABLE
#define UFCON_VAL			((TX_FIFO_TRIGGER_LEVEL << 8) \
					| (RX_FIFO_TRIGGER_LEVEL << 4) \
					| (TX_FIFO_RESET << 2) \
					| (RX_FIFO_RESET << 1) \
					| (FIFO_ENABLE << 0))
/*
 * Baud Rate Division Value
 * 115200 BAUD:
 * UBRDIV_VAL = SCLK_UART/((115200 * 16) - 1)
 * UBRDIV_VAL = (800 MHz)/((115200 * 16) - 1)
 */
#define UBRDIV_VAL		0x35

/*
 * Fractional Part of Baud Rate Divisor:
 * 115200 BAUD:
 * UBRFRACVAL = ((((SCLK_UART*10/(115200*16) -10))%10)*16/10)
 * UBRFRACVAL = ((((800MHz*10/(115200*16) -10))%10)*16/10)
 */
#define UFRACVAL_VAL		0x4

/*
 * TZPC Register Value :
 * R0SIZE: 0x0 : Size of secured ram
 */
#define R0SIZE			0x0

/*
 * TZPC Decode Protection Register Value :
 * DECPROTXSET: 0xFF : Set Decode region to non-secure
 */
#define DECPROTXSET		0xFF
#endif
