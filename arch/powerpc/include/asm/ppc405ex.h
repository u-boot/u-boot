/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef _PPC405EX_H_
#define _PPC405EX_H_

#define CONFIG_SDRAM_PPC4xx_IBM_DDR2	/* IBM DDR(2) controller */

#define CONFIG_NAND_NDFC

/* Memory mapped register */
#define CONFIG_SYS_PERIPHERAL_BASE	0xef600000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0200)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0800)

/* SDR */
#define SDR0_SDCS0		0x0060
#define SDR0_UART0		0x0120	/* UART0 Config */
#define SDR0_UART1		0x0121	/* UART1 Config */
#define SDR0_SRST		0x0200
#define SDR0_CUST0		0x4000
#define SDR0_PFC0		0x4100
#define SDR0_PFC1		0x4101
#define SDR0_MFR		0x4300	/* SDR0_MFR reg */

#define SDR0_ECID0		0x0080
#define SDR0_ECID1		0x0081
#define SDR0_ECID2		0x0082
#define SDR0_ECID3		0x0083

#define SDR0_SDCS_SDD		(0x80000000 >> 31)

#define SDR0_SRST_DMC		(0x80000000 >> 10)

#define SDR0_CUST0_MUX_E_N_G_MASK	0xC0000000 /* Mux_Emac_NDFC_GPIO */
#define SDR0_CUST0_MUX_EMAC_SEL		0x40000000 /* Emac Selection */
#define SDR0_CUST0_MUX_NDFC_SEL		0x80000000 /* NDFC Selection */
#define SDR0_CUST0_MUX_GPIO_SEL		0xC0000000 /* GPIO Selection */

#define SDR0_CUST0_NDFC_EN_MASK		0x20000000 /* NDFC Enable Mask */
#define SDR0_CUST0_NDFC_ENABLE		0x20000000 /* NDFC Enable */
#define SDR0_CUST0_NDFC_DISABLE		0x00000000 /* NDFC Disable */

#define SDR0_CUST0_NDFC_BW_MASK	  	0x10000000 /* NDFC Boot Width */
#define SDR0_CUST0_NDFC_BW_16_BIT 	0x10000000 /* NDFC Boot Width= 16 Bit */
#define SDR0_CUST0_NDFC_BW_8_BIT  	0x00000000 /* NDFC Boot Width=  8 Bit */

#define SDR0_CUST0_NDFC_BP_MASK		0x0F000000 /* NDFC Boot Page */
#define SDR0_CUST0_NDFC_BP_ENCODE(n)	((((u32)(n)) & 0xF) << 24)
#define SDR0_CUST0_NDFC_BP_DECODE(n)	((((u32)(n)) >> 24) & 0xF)

#define SDR0_CUST0_NDFC_BAC_MASK	0x00C00000 /* NDFC Boot Address Cycle */
#define SDR0_CUST0_NDFC_BAC_ENCODE(n)	((((u32)(n)) & 0x3) << 22)
#define SDR0_CUST0_NDFC_BAC_DECODE(n)	((((u32)(n)) >> 22) & 0x3)

#define SDR0_CUST0_NDFC_ARE_MASK	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_ENABLE	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_DISABLE	0x00000000 /* NDFC Auto Read Disable */

#define SDR0_CUST0_NRB_MASK		0x00100000 /* NDFC Ready / Busy */
#define SDR0_CUST0_NRB_BUSY		0x00100000 /* Busy */
#define SDR0_CUST0_NRB_READY		0x00000000 /* Ready */

#define SDR0_PFC1_U1ME			0x02000000
#define SDR0_PFC1_U0ME			0x00080000
#define SDR0_PFC1_U0IM			0x00040000
#define SDR0_PFC1_SIS			0x00020000
#define SDR0_PFC1_DMAAEN		0x00010000
#define SDR0_PFC1_DMADEN		0x00008000
#define SDR0_PFC1_USBEN			0x00004000
#define SDR0_PFC1_AHBSWAP		0x00000020
#define SDR0_PFC1_USBBIGEN		0x00000010
#define SDR0_PFC1_GPT_FREQ		0x0000000f

#endif /* _PPC405EX_H_ */
