/*
 * U-Boot - main board file
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <netdev.h>
#include <asm/blackfin.h>
#include <asm/gpio.h>
#include <asm/portmux.h>
#include <asm/sdh.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF548 EZ-Kit board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

int board_early_init_f(void)
{
	/* Set async addr lines as peripheral */
	const unsigned short pins[] = {
		P_A4, P_A5, P_A6, P_A7, P_A8, P_A9, P_A10, P_A11, P_A12,
		P_A13, P_A14, P_A15, P_A16, P_A17, P_A18, P_A19, P_A20,
		P_A21, P_A22, P_A23, P_A24, 0
	};
	return peripheral_request_list(pins, "async");
}

#ifdef CONFIG_SMC911X
int board_eth_init(bd_t *bis)
{
	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
}
#endif

#ifdef CONFIG_BFIN_SDH
int board_mmc_init(bd_t *bis)
{
	return bfin_mmc_init(bis);
}
#endif

#ifdef CONFIG_USB_BLACKFIN
void board_musb_init(void)
{
	/*
	 * Rev 1.0 BF549 EZ-KITs require PE7 to be high for both device
	 * and OTG host modes, while rev 1.1 and greater require PE7 to
	 * be low for device mode and high for host mode.  We set it high
	 * here because we are in host mode.
	 */
	gpio_request(GPIO_PE7, "musb-vbus");
	gpio_direction_output(GPIO_PE7, 1);
}
#endif
