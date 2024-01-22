// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra30 high-level function multiplexing */

#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>

int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != FUNCMUX_DEFAULT;

	switch (id) {
	case PERIPH_ID_UART1:
		switch (config) {
		case FUNCMUX_UART1_ULPI:
			pinmux_set_func(PMUX_PINGRP_ULPI_DATA0_PO1,
					PMUX_FUNC_UARTA);
			pinmux_set_func(PMUX_PINGRP_ULPI_DATA1_PO2,
					PMUX_FUNC_UARTA);
			pinmux_set_func(PMUX_PINGRP_ULPI_DATA2_PO3,
					PMUX_FUNC_UARTA);
			pinmux_set_func(PMUX_PINGRP_ULPI_DATA3_PO4,
					PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PMUX_PINGRP_ULPI_DATA0_PO1);
			pinmux_tristate_disable(PMUX_PINGRP_ULPI_DATA1_PO2);
			pinmux_tristate_disable(PMUX_PINGRP_ULPI_DATA2_PO3);
			pinmux_tristate_disable(PMUX_PINGRP_ULPI_DATA3_PO4);
			break;
		}
		break;
	case PERIPH_ID_UART5:
		switch (config) {
		case FUNCMUX_UART5_SDMMC1:
			pinmux_set_func(PMUX_PINGRP_SDMMC1_DAT3_PY4,
					PMUX_FUNC_UARTE);
			pinmux_set_func(PMUX_PINGRP_SDMMC1_DAT2_PY5,
					PMUX_FUNC_UARTE);

			pinmux_set_io(PMUX_PINGRP_SDMMC1_DAT3_PY4, PMUX_PIN_OUTPUT);
			pinmux_set_io(PMUX_PINGRP_SDMMC1_DAT2_PY5, PMUX_PIN_INPUT);

			pinmux_tristate_disable(PMUX_PINGRP_SDMMC1_DAT3_PY4);
			pinmux_tristate_disable(PMUX_PINGRP_SDMMC1_DAT2_PY5);
			break;
		}
		break;

	/* Add other periph IDs here as needed */

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
