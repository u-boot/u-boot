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
#if defined(CONFIG_NOR)
	/* NOR only pin-mux */
	{GPMC_A0  , M0 | IDIS | PDIS}, /* nor.GPMC_A[0 ] */
	{GPMC_A1  , M0 | IDIS | PDIS}, /* nor.GPMC_A[1 ] */
	{GPMC_A2  , M0 | IDIS | PDIS}, /* nor.GPMC_A[2 ] */
	{GPMC_A3  , M0 | IDIS | PDIS}, /* nor.GPMC_A[3 ] */
	{GPMC_A4  , M0 | IDIS | PDIS}, /* nor.GPMC_A[4 ] */
	{GPMC_A5  , M0 | IDIS | PDIS}, /* nor.GPMC_A[5 ] */
	{GPMC_A6  , M0 | IDIS | PDIS}, /* nor.GPMC_A[6 ] */
	{GPMC_A7  , M0 | IDIS | PDIS}, /* nor.GPMC_A[7 ] */
	{GPMC_A8  , M0 | IDIS | PDIS}, /* nor.GPMC_A[8 ] */
	{GPMC_A9  , M0 | IDIS | PDIS}, /* nor.GPMC_A[9 ] */
	{GPMC_A10 , M0 | IDIS | PDIS}, /* nor.GPMC_A[10] */
	{GPMC_A11 , M0 | IDIS | PDIS}, /* nor.GPMC_A[11] */
	{GPMC_A12 , M0 | IDIS | PDIS}, /* nor.GPMC_A[12] */
	{GPMC_A13 , M0 | IDIS | PDIS}, /* nor.GPMC_A[13] */
	{GPMC_A14 , M0 | IDIS | PDIS}, /* nor.GPMC_A[14] */
	{GPMC_A15 , M0 | IDIS | PDIS}, /* nor.GPMC_A[15] */
	{GPMC_A16 , M0 | IDIS | PDIS}, /* nor.GPMC_A[16] */
	{GPMC_A17 , M0 | IDIS | PDIS}, /* nor.GPMC_A[17] */
	{GPMC_A18 , M0 | IDIS | PDIS}, /* nor.GPMC_A[18] */
	{GPMC_A19 , M0 | IDIS | PDIS}, /* nor.GPMC_A[19] */
	{GPMC_A20 , M0 | IDIS | PDIS}, /* nor.GPMC_A[20] */
	{GPMC_A21 , M0 | IDIS | PDIS}, /* nor.GPMC_A[21] */
	{GPMC_A22 , M0 | IDIS | PDIS}, /* nor.GPMC_A[22] */
	{GPMC_A23 , M0 | IDIS | PDIS}, /* nor.GPMC_A[23] */
	{GPMC_A24 , M0 | IDIS | PDIS}, /* nor.GPMC_A[24] */
	{GPMC_A25 , M0 | IDIS | PDIS}, /* nor.GPMC_A[25] */
	{GPMC_A26 , M0 | IDIS | PDIS}, /* nor.GPMC_A[26] */
#else
	/* eMMC pinmux */
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
#endif
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
	{VIN2A_D12, (M3) },
	{VIN2A_D13, (M3) },
	{VIN2A_D14, (M3) },
	{VIN2A_D15, (M3) },
	{VIN2A_D16, (M3) },
	{VIN2A_D17, (M3) },
	{VIN2A_D18, (IEN | M3)},
	{VIN2A_D19, (IEN | M3)},
	{VIN2A_D20, (IEN | M3)},
	{VIN2A_D21, (IEN | M3)},
	{VIN2A_D22, (IEN | M3)},
	{VIN2A_D23, (IEN | M3)},
#if defined(CONFIG_NAND) || defined(CONFIG_NOR)
	/* NAND / NOR pin-mux */
	{GPMC_AD0 , M0 | IEN | PDIS}, /* GPMC_AD0  */
	{GPMC_AD1 , M0 | IEN | PDIS}, /* GPMC_AD1  */
	{GPMC_AD2 , M0 | IEN | PDIS}, /* GPMC_AD2  */
	{GPMC_AD3 , M0 | IEN | PDIS}, /* GPMC_AD3  */
	{GPMC_AD4 , M0 | IEN | PDIS}, /* GPMC_AD4  */
	{GPMC_AD5 , M0 | IEN | PDIS}, /* GPMC_AD5  */
	{GPMC_AD6 , M0 | IEN | PDIS}, /* GPMC_AD6  */
	{GPMC_AD7 , M0 | IEN | PDIS}, /* GPMC_AD7  */
	{GPMC_AD8 , M0 | IEN | PDIS}, /* GPMC_AD8  */
	{GPMC_AD9 , M0 | IEN | PDIS}, /* GPMC_AD9  */
	{GPMC_AD10, M0 | IEN | PDIS}, /* GPMC_AD10 */
	{GPMC_AD11, M0 | IEN | PDIS}, /* GPMC_AD11 */
	{GPMC_AD12, M0 | IEN | PDIS}, /* GPMC_AD12 */
	{GPMC_AD13, M0 | IEN | PDIS}, /* GPMC_AD13 */
	{GPMC_AD14, M0 | IEN | PDIS}, /* GPMC_AD14 */
	{GPMC_AD15, M0 | IEN | PDIS}, /* GPMC_AD15 */
	{GPMC_CS0,	M0 | IDIS | PEN | PTU}, /* GPMC chip-select */
	{GPMC_ADVN_ALE,	M0 | IDIS | PEN | PTD}, /* GPMC Addr latch */
	{GPMC_OEN_REN,	M0 | IDIS | PEN | PTU}, /* GPMC Read enable */
	{GPMC_WEN,	M0 | IDIS | PEN | PTU}, /* GPMC Write enable_n */
	{GPMC_BEN0,	M0 | IDIS | PEN | PTD}, /* GPMC Byte/Column En */
	{GPMC_WAIT0,	M0 | IEN  | PEN | PTU}, /* GPMC Wait/Ready */
	/* GPMC_WPN (Write Protect) is controlled by DIP Switch SW10(12) */
#else
	/* QSPI pin-mux */
	{GPMC_A13, (IEN | PDIS | M1)},  /* QSPI1_RTCLK */
	{GPMC_A14, (IEN | PDIS | M1)},  /* QSPI1_D[3] */
	{GPMC_A15, (IEN | PDIS | M1)},  /* QSPI1_D[2] */
	{GPMC_A16, (IEN | PDIS | M1)},  /* QSPI1_D[0] */
	{GPMC_A17, (IEN | PDIS | M1)},  /* QSPI1_D[1] */
	{GPMC_A18, (M1)},  /* QSPI1_SCLK */
	{GPMC_A3, (IEN | PDIS | M1)},   /* QSPI1_CS2 */
	{GPMC_A4, (IEN | PDIS | M1)},   /* QSPI1_CS3 */
	{GPMC_CS2, (IEN | PTU | PDIS | M1)},    /* QSPI1_CS0 */
	{GPMC_CS3, (IEN | PTU | PDIS | M1)},    /* QSPI1_CS1*/
#endif /* CONFIG_NAND || CONFIG_NOR */
	{USB2_DRVVBUS, (M0 | IEN | FSC) },
	{SPI1_CS1, (PEN | IDIS | M14) },
};
#endif /* _MUX_DATA_DRA7XX_H_ */
