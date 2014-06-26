/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Sricharan R	<r.sricharan@ti.com>
 * Nishant Kamat <nskamat@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _MUX_DATA_DRA7XX_H_
#define _MUX_DATA_DRA7XX_H_

#include <asm/arch/mux_dra7xx.h>

const struct pad_conf_entry core_padconf_array_essential[] = {
	{MMC1_CLK, (IEN | PTU | PDIS | M0)},	/* MMC1_CLK */
	{MMC1_CMD, (IEN | PTU | PDIS | M0)},	/* MMC1_CMD */
	{MMC1_DAT0, (IEN | PTU | PDIS | M0)},	/* MMC1_DAT0 */
	{MMC1_DAT1, (IEN | PTU | PDIS | M0)},	/* MMC1_DAT1 */
	{MMC1_DAT2, (IEN | PTU | PDIS | M0)},	/* MMC1_DAT2 */
	{MMC1_DAT3, (IEN | PTU | PDIS | M0)},	/* MMC1_DAT3 */
	{MMC1_SDCD, (FSC | IEN | PTU | PDIS | M0)}, /* MMC1_SDCD */
	{MMC1_SDWP, (FSC | IEN | PTD | PEN | M14)}, /* MMC1_SDWP */
	{GPMC_A19, (IEN | PTU | PDIS | M1)},	/* mmc2_dat4 */
	{GPMC_A20, (IEN | PTU | PDIS | M1)},	/* mmc2_dat5 */
	{GPMC_A21, (IEN | PTU | PDIS | M1)},	/* mmc2_dat6 */
	{GPMC_A22, (IEN | PTU | PDIS | M1)},	/* mmc2_dat7 */
	{GPMC_A23, (IEN | PTU | PDIS | M1)},	/* mmc2_clk */
	{GPMC_A24, (IEN | PTU | PDIS | M1)},	/* mmc2_dat0 */
	{GPMC_A25, (IEN | PTU | PDIS | M1)},	/* mmc2_dat1 */
	{GPMC_A26, (IEN | PTU | PDIS | M1)},	/* mmc2_dat2 */
	{GPMC_A27, (IEN | PTU | PDIS | M1)},	/* mmc2_dat3 */
	{GPMC_CS1, (IEN | PTU | PDIS | M1)},	/* mmm2_cmd */
#if (CONFIG_CONS_INDEX == 1)
	{UART1_RXD, (FSC | IEN | PTU | PDIS | M0)}, /* UART1_RXD */
	{UART1_TXD, (FSC | IEN | PTU | PDIS | M0)}, /* UART1_TXD */
	{UART1_CTSN, (IEN | PTU | PDIS | M3)},	/* UART1_CTSN */
	{UART1_RTSN, (IEN | PTU | PDIS | M3)},	/* UART1_RTSN */
#elif (CONFIG_CONS_INDEX == 3)
	{UART3_RXD, (FSC | IEN | PTU | PDIS | M0)}, /* UART3_RXD */
	{UART3_TXD, (FSC | IEN | PTU | PDIS | M0)}, /* UART3_TXD */
#endif
	{I2C1_SDA, (IEN | PTU | PDIS | M0)},	/* I2C1_SDA */
	{I2C1_SCL, (IEN | PTU | PDIS | M0)},	/* I2C1_SCL */
	{MDIO_MCLK, (PTU | PEN | M0)},		/* MDIO_MCLK  */
	{MDIO_D, (IEN | PTU | PEN | M0)},	/* MDIO_D  */
	{RGMII0_TXC, (M0) },
	{RGMII0_TXCTL, (M0) },
	{RGMII0_TXD3, (M0) },
	{RGMII0_TXD2, (M0) },
	{RGMII0_TXD1, (M0) },
	{RGMII0_TXD0, (M0) },
	{RGMII0_RXC, (IEN | M0) },
	{RGMII0_RXCTL, (IEN | M0) },
	{RGMII0_RXD3, (IEN | M0) },
	{RGMII0_RXD2, (IEN | M0) },
	{RGMII0_RXD1, (IEN | M0) },
	{RGMII0_RXD0, (IEN | M0) },
	{GPMC_A13, (IEN | PDIS | M1)},  /* QSPI1_RTCLK */
	{GPMC_A14, (IEN | PDIS | M1)},  /* QSPI1_D[3] */
	{GPMC_A15, (IEN | PDIS | M1)},  /* QSPI1_D[2] */
	{GPMC_A16, (IEN | PDIS | M1)},  /* QSPI1_D[1] */
	{GPMC_A17, (IEN | PDIS | M1)},  /* QSPI1_D[0] */
	{GPMC_A18, (M1)},  /* QSPI1_SCLK */
	{GPMC_A3, (IEN | PDIS | M1)},   /* QSPI1_CS2 */
	{GPMC_A4, (IEN | PDIS | M1)},   /* QSPI1_CS3 */
	{GPMC_CS2, (IEN | PTU | PDIS | M1)},    /* QSPI1_CS0 */
	{GPMC_CS3, (IEN | PTU | PDIS | M1)},    /* QSPI1_CS1*/
	{USB2_DRVVBUS, (M0 | IEN | FSC) },
};
#endif /* _MUX_DATA_DRA7XX_H_ */
