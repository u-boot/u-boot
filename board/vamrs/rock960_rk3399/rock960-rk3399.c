// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <asm/arch/periph.h>
#include <power/regulator.h>
#include <spl.h>

int board_init(void)
{
	int ret;

	ret = regulators_enable_boot_on(false);
	if (ret)
		debug("%s: Cannot enable boot on regulator\n", __func__);

	return 0;
}

void spl_board_init(void)
{
	struct udevice *pinctrl;
	int ret;

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto err;
	}

	/* Enable debug UART */
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_UART_DBG);
	if (ret) {
		debug("%s: Failed to set up console UART\n", __func__);
		goto err;
	}

	preloader_console_init();
	return;
err:
	printf("%s: Error %d\n", __func__, ret);

	/* No way to report error here */
	hang();
}
