/*
 * Pinmux configurations for the DA830 SoCs
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/davinci_misc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/pinmux_defs.h>

/* SPI0 pin muxer settings */
const struct pinmux_config spi0_pins_base[] = {
	{ pinmux(7), 1, 3 },  /* SPI0_SOMI */
	{ pinmux(7), 1, 4 },  /* SPI0_SIMO */
	{ pinmux(7), 1, 6 }   /* SPI0_CLK */
};

const struct pinmux_config spi0_pins_scs0[] = {
	{ pinmux(7), 1, 7 }   /* SPI0_SCS[0] */
};

const struct pinmux_config spi0_pins_ena[] = {
	{ pinmux(7), 1, 5 }   /* SPI0_ENA */
};

/* NAND pin muxer settings */
const struct pinmux_config emifa_pins_cs0[] = {
	{ pinmux(18), 1, 2 }   /* EMA_CS[0] */
};

const struct pinmux_config emifa_pins_cs2[] = {
	{ pinmux(18), 1, 3 }   /* EMA_CS[2] */
};

const struct pinmux_config emifa_pins_cs3[] = {
	{ pinmux(18), 1, 4 }   /* EMA_CS[3] */
};

#ifdef CONFIG_USE_NAND
const struct pinmux_config emifa_pins[] = {
	{ pinmux(13), 1, 6 },  /* EMA_D[0] */
	{ pinmux(13), 1, 7 },  /* EMA_D[1] */
	{ pinmux(14), 1, 0 },  /* EMA_D[2] */
	{ pinmux(14), 1, 1 },  /* EMA_D[3] */
	{ pinmux(14), 1, 2 },  /* EMA_D[4] */
	{ pinmux(14), 1, 3 },  /* EMA_D[5] */
	{ pinmux(14), 1, 4 },  /* EMA_D[6] */
	{ pinmux(14), 1, 5 },  /* EMA_D[7] */
	{ pinmux(14), 1, 6 },  /* EMA_D[8] */
	{ pinmux(14), 1, 7 },  /* EMA_D[9] */
	{ pinmux(15), 1, 0 },  /* EMA_D[10] */
	{ pinmux(15), 1, 1 },  /* EMA_D[11] */
	{ pinmux(15), 1, 2 },  /* EMA_D[12] */
	{ pinmux(15), 1, 3 },  /* EMA_D[13] */
	{ pinmux(15), 1, 4 },  /* EMA_D[14] */
	{ pinmux(15), 1, 5 },  /* EMA_D[15] */
	{ pinmux(15), 1, 6 },  /* EMA_A[0] */
	{ pinmux(15), 1, 7 },  /* EMA_A[1] */
	{ pinmux(16), 1, 0 },  /* EMA_A[2] */
	{ pinmux(16), 1, 1 },  /* EMA_A[3] */
	{ pinmux(16), 1, 2 },  /* EMA_A[4] */
	{ pinmux(16), 1, 3 },  /* EMA_A[5] */
	{ pinmux(16), 1, 4 },  /* EMA_A[6] */
	{ pinmux(16), 1, 5 },  /* EMA_A[7] */
	{ pinmux(16), 1, 6 },  /* EMA_A[8] */
	{ pinmux(16), 1, 7 },  /* EMA_A[9] */
	{ pinmux(17), 1, 0 },  /* EMA_A[10] */
	{ pinmux(17), 1, 1 },  /* EMA_A[11] */
	{ pinmux(17), 1, 2 },  /* EMA_A[12] */
	{ pinmux(17), 1, 3 },  /* EMA_BA[1] */
	{ pinmux(17), 1, 4 },  /* EMA_BA[0] */
	{ pinmux(17), 1, 5 },  /* EMA_CLK */
	{ pinmux(17), 1, 6 },  /* EMA_SDCKE */
	{ pinmux(17), 1, 7 },  /* EMA_CAS */
	{ pinmux(18), 1, 0 },  /* EMA_CAS */
	{ pinmux(18), 1, 1 },  /* EMA_WE */
	{ pinmux(18), 1, 5 },  /* EMA_OE */
	{ pinmux(18), 1, 6 },  /* EMA_WE_DQM[1] */
	{ pinmux(18), 1, 7 },  /* EMA_WE_DQM[0] */
	{ pinmux(10), 1, 0 }   /* Tristate */
};
#endif

/* EMAC PHY interface pins */
const struct pinmux_config emac_pins_rmii[] = {
	{ pinmux(10), 2, 1 },  /* RMII_TXD[0] */
	{ pinmux(10), 2, 2 },  /* RMII_TXD[1] */
	{ pinmux(10), 2, 3 },  /* RMII_TXEN */
	{ pinmux(10), 2, 4 },  /* RMII_CRS_DV */
	{ pinmux(10), 2, 5 },  /* RMII_RXD[0] */
	{ pinmux(10), 2, 6 },  /* RMII_RXD[1] */
	{ pinmux(10), 2, 7 }   /* RMII_RXER */
};

const struct pinmux_config emac_pins_mdio[] = {
	{ pinmux(11), 2, 0 },  /* MDIO_CLK */
	{ pinmux(11), 2, 1 }   /* MDIO_D */
};

const struct pinmux_config emac_pins_rmii_clk_source[] = {
	{ pinmux(9), 0, 5 }    /* ref.clk from external source */
};

/* UART2 pin muxer settings */
const struct pinmux_config uart2_pins_txrx[] = {
	{ pinmux(8), 2, 7 },   /* UART2_RXD */
	{ pinmux(9), 2, 0 }    /* UART2_TXD */
};

/* I2C0 pin muxer settings */
const struct pinmux_config i2c0_pins[] = {
	{ pinmux(8), 2, 3 },   /* I2C0_SDA */
	{ pinmux(8), 2, 4 }    /* I2C0_SCL */
};

/* USB0_DRVVBUS pin muxer settings */
const struct pinmux_config usb_pins[] = {
	{ pinmux(9), 1, 1 }    /* USB0_DRVVBUS */
};

#ifdef CONFIG_DAVINCI_MMC
/* MMC0 pin muxer settings */
const struct pinmux_config mmc0_pins_8bit[] = {
	{ pinmux(15), 2, 7 },  /* MMCSD0_CLK */
	{ pinmux(16), 2, 0 },  /* MMCSD0_CMD */
	{ pinmux(13), 2, 6 },  /* MMCSD0_DAT_0 */
	{ pinmux(13), 2, 7 },  /* MMCSD0_DAT_1 */
	{ pinmux(14), 2, 0 },  /* MMCSD0_DAT_2 */
	{ pinmux(14), 2, 1 },  /* MMCSD0_DAT_3 */
	{ pinmux(14), 2, 2 },  /* MMCSD0_DAT_4 */
	{ pinmux(14), 2, 3 },  /* MMCSD0_DAT_5 */
	{ pinmux(14), 2, 4 },  /* MMCSD0_DAT_6 */
	{ pinmux(14), 2, 5 }   /* MMCSD0_DAT_7 */
	/* DA830 supports 8-bit mode */
};
#endif
