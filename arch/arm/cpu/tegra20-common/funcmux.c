/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* Tegra20 high-level function multiplexing */
#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>

int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != FUNCMUX_DEFAULT;

	switch (id) {
	case PERIPH_ID_UART1:
		switch (config) {
		case FUNCMUX_UART1_IRRX_IRTX:
			pinmux_set_func(PINGRP_IRRX, PMUX_FUNC_UARTA);
			pinmux_set_func(PINGRP_IRTX, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PINGRP_IRRX);
			pinmux_tristate_disable(PINGRP_IRTX);
			break;
		case FUNCMUX_UART1_UAA_UAB:
			pinmux_set_func(PINGRP_UAA, PMUX_FUNC_UARTA);
			pinmux_set_func(PINGRP_UAB, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PINGRP_UAA);
			pinmux_tristate_disable(PINGRP_UAB);
			bad_config = 0;
			break;
		case FUNCMUX_UART1_GPU:
			pinmux_set_func(PINGRP_GPU, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PINGRP_GPU);
			bad_config = 0;
			break;
		case FUNCMUX_UART1_SDIO1:
			pinmux_set_func(PINGRP_SDIO1, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PINGRP_SDIO1);
			bad_config = 0;
			break;
		}
		if (!bad_config) {
			/*
			 * Tegra appears to boot with function UARTA pre-
			 * selected on mux group SDB. If two mux groups are
			 * both set to the same function, it's unclear which
			 * group's pins drive the RX signals into the HW.
			 * For UARTA, SDB certainly overrides group IRTX in
			 * practice. To solve this, configure some alternative
			 * function on SDB to avoid the conflict. Also, tri-
			 * state the group to avoid driving any signal onto it
			 * until we know what's connected.
			 */
			pinmux_tristate_enable(PINGRP_SDB);
			pinmux_set_func(PINGRP_SDB,  PMUX_FUNC_SDIO3);
		}
		break;

	case PERIPH_ID_UART2:
		if (config == FUNCMUX_UART2_IRDA) {
			pinmux_set_func(PINGRP_UAD, PMUX_FUNC_IRDA);
			pinmux_tristate_disable(PINGRP_UAD);
		}
		break;

	case PERIPH_ID_UART4:
		if (config == FUNCMUX_UART4_GMC) {
			pinmux_set_func(PINGRP_GMC, PMUX_FUNC_UARTD);
			pinmux_tristate_disable(PINGRP_GMC);
		}
		break;

	case PERIPH_ID_DVC_I2C:
		/* there is only one selection, pinmux_config is ignored */
		if (config == FUNCMUX_DVC_I2CP) {
			pinmux_set_func(PINGRP_I2CP, PMUX_FUNC_I2C);
			pinmux_tristate_disable(PINGRP_I2CP);
		}
		break;

	case PERIPH_ID_I2C1:
		/* support pinmux_config of 0 for now, */
		if (config == FUNCMUX_I2C1_RM) {
			pinmux_set_func(PINGRP_RM, PMUX_FUNC_I2C);
			pinmux_tristate_disable(PINGRP_RM);
		}
		break;
	case PERIPH_ID_I2C2: /* I2C2 */
		switch (config) {
		case FUNCMUX_I2C2_DDC:	/* DDC pin group, select I2C2 */
			pinmux_set_func(PINGRP_DDC, PMUX_FUNC_I2C2);
			/* PTA to HDMI */
			pinmux_set_func(PINGRP_PTA, PMUX_FUNC_HDMI);
			pinmux_tristate_disable(PINGRP_DDC);
			break;
		case FUNCMUX_I2C2_PTA:	/* PTA pin group, select I2C2 */
			pinmux_set_func(PINGRP_PTA, PMUX_FUNC_I2C2);
			/* set DDC_SEL to RSVDx (RSVD2 works for now) */
			pinmux_set_func(PINGRP_DDC, PMUX_FUNC_RSVD2);
			pinmux_tristate_disable(PINGRP_PTA);
			bad_config = 0;
			break;
		}
		break;
	case PERIPH_ID_I2C3: /* I2C3 */
		/* support pinmux_config of 0 for now */
		if (config == FUNCMUX_I2C3_DTF) {
			pinmux_set_func(PINGRP_DTF, PMUX_FUNC_I2C3);
			pinmux_tristate_disable(PINGRP_DTF);
		}
		break;

	case PERIPH_ID_SDMMC1:
		if (config == FUNCMUX_SDMMC1_SDIO1_4BIT) {
			pinmux_set_func(PINGRP_SDIO1, PMUX_FUNC_SDIO1);
			pinmux_tristate_disable(PINGRP_SDIO1);
		}
		break;

	case PERIPH_ID_SDMMC2:
		if (config == FUNCMUX_SDMMC2_DTA_DTD_8BIT) {
			pinmux_set_func(PINGRP_DTA, PMUX_FUNC_SDIO2);
			pinmux_set_func(PINGRP_DTD, PMUX_FUNC_SDIO2);

			pinmux_tristate_disable(PINGRP_DTA);
			pinmux_tristate_disable(PINGRP_DTD);
		}
		break;

	case PERIPH_ID_SDMMC3:
		switch (config) {
		case FUNCMUX_SDMMC3_SDB_SLXA_8BIT:
			pinmux_set_func(PINGRP_SLXA, PMUX_FUNC_SDIO3);
			pinmux_set_func(PINGRP_SLXC, PMUX_FUNC_SDIO3);
			pinmux_set_func(PINGRP_SLXD, PMUX_FUNC_SDIO3);
			pinmux_set_func(PINGRP_SLXK, PMUX_FUNC_SDIO3);

			pinmux_tristate_disable(PINGRP_SLXA);
			pinmux_tristate_disable(PINGRP_SLXC);
			pinmux_tristate_disable(PINGRP_SLXD);
			pinmux_tristate_disable(PINGRP_SLXK);
			/* fall through */

		case FUNCMUX_SDMMC3_SDB_4BIT:
			pinmux_set_func(PINGRP_SDB, PMUX_FUNC_SDIO3);
			pinmux_set_func(PINGRP_SDC, PMUX_FUNC_SDIO3);
			pinmux_set_func(PINGRP_SDD, PMUX_FUNC_SDIO3);

			pinmux_tristate_disable(PINGRP_SDB);
			pinmux_tristate_disable(PINGRP_SDC);
			pinmux_tristate_disable(PINGRP_SDD);
			bad_config = 0;
			break;
		}
		break;

	case PERIPH_ID_SDMMC4:
		switch (config) {
		case FUNCMUX_SDMMC4_ATC_ATD_8BIT:
			pinmux_set_func(PINGRP_ATC, PMUX_FUNC_SDIO4);
			pinmux_set_func(PINGRP_ATD, PMUX_FUNC_SDIO4);

			pinmux_tristate_disable(PINGRP_ATC);
			pinmux_tristate_disable(PINGRP_ATD);
			break;

		case FUNCMUX_SDMMC4_ATB_GMA_GME_8_BIT:
			pinmux_set_func(PINGRP_GME, PMUX_FUNC_SDIO4);
			pinmux_tristate_disable(PINGRP_GME);
			/* fall through */

		case FUNCMUX_SDMMC4_ATB_GMA_4_BIT:
			pinmux_set_func(PINGRP_ATB, PMUX_FUNC_SDIO4);
			pinmux_set_func(PINGRP_GMA, PMUX_FUNC_SDIO4);

			pinmux_tristate_disable(PINGRP_ATB);
			pinmux_tristate_disable(PINGRP_GMA);
			bad_config = 0;
			break;
		}
		break;

	case PERIPH_ID_KBC:
		if (config == FUNCMUX_DEFAULT) {
			enum pmux_pingrp grp[] = {PINGRP_KBCA, PINGRP_KBCB,
				PINGRP_KBCC, PINGRP_KBCD, PINGRP_KBCE,
				PINGRP_KBCF};
			int i;

			for (i = 0; i < ARRAY_SIZE(grp); i++) {
				pinmux_tristate_disable(grp[i]);
				pinmux_set_func(grp[i], PMUX_FUNC_KBC);
				pinmux_set_pullupdown(grp[i], PMUX_PULL_UP);
			}
		}
		break;

	case PERIPH_ID_USB2:
		if (config == FUNCMUX_USB2_ULPI) {
			pinmux_set_func(PINGRP_UAA, PMUX_FUNC_ULPI);
			pinmux_set_func(PINGRP_UAB, PMUX_FUNC_ULPI);
			pinmux_set_func(PINGRP_UDA, PMUX_FUNC_ULPI);

			pinmux_tristate_disable(PINGRP_UAA);
			pinmux_tristate_disable(PINGRP_UAB);
			pinmux_tristate_disable(PINGRP_UDA);
		}
		break;

	case PERIPH_ID_SPI1:
		if (config == FUNCMUX_SPI1_GMC_GMD) {
			pinmux_set_func(PINGRP_GMC, PMUX_FUNC_SFLASH);
			pinmux_set_func(PINGRP_GMD, PMUX_FUNC_SFLASH);

			pinmux_tristate_disable(PINGRP_GMC);
			pinmux_tristate_disable(PINGRP_GMD);
		}
		break;

	case PERIPH_ID_NDFLASH:
		switch (config) {
		case FUNCMUX_NDFLASH_ATC:
			pinmux_set_func(PINGRP_ATC, PMUX_FUNC_NAND);
			pinmux_tristate_disable(PINGRP_ATC);
			break;
		case FUNCMUX_NDFLASH_KBC_8_BIT:
			pinmux_set_func(PINGRP_KBCA, PMUX_FUNC_NAND);
			pinmux_set_func(PINGRP_KBCC, PMUX_FUNC_NAND);
			pinmux_set_func(PINGRP_KBCD, PMUX_FUNC_NAND);
			pinmux_set_func(PINGRP_KBCE, PMUX_FUNC_NAND);
			pinmux_set_func(PINGRP_KBCF, PMUX_FUNC_NAND);

			pinmux_tristate_disable(PINGRP_KBCA);
			pinmux_tristate_disable(PINGRP_KBCC);
			pinmux_tristate_disable(PINGRP_KBCD);
			pinmux_tristate_disable(PINGRP_KBCE);
			pinmux_tristate_disable(PINGRP_KBCF);

			bad_config = 0;
			break;
		}
		break;

	default:
		debug("%s: invalid periph_id %d", __func__, id);
		return -1;
	}

	if (bad_config) {
		debug("%s: invalid config %d for periph_id %d", __func__,
		      config, id);
		return -1;
	}

	return 0;
}
