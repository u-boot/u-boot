/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <i2c.h>
#include <asm/gpio.h>
#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/ehci-omap.h>
#endif
#include "twister.h"

DECLARE_GLOBAL_DATA_PTR;

/* Timing definitions for Ethernet Controller */
static const u32 gpmc_smc911[] = {
	NET_GPMC_CONFIG1,
	NET_GPMC_CONFIG2,
	NET_GPMC_CONFIG3,
	NET_GPMC_CONFIG4,
	NET_GPMC_CONFIG5,
	NET_GPMC_CONFIG6,
};

static const u32 gpmc_XR16L2751[] = {
	XR16L2751_GPMC_CONFIG1,
	XR16L2751_GPMC_CONFIG2,
	XR16L2751_GPMC_CONFIG3,
	XR16L2751_GPMC_CONFIG4,
	XR16L2751_GPMC_CONFIG5,
	XR16L2751_GPMC_CONFIG6,
};

#ifdef CONFIG_USB_EHCI
static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	return omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(int index)
{
	return omap_ehci_hcd_stop();
}
#endif

int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	/* Chip select 1  and 3 are used for XR16L2751 UART controller */
	enable_gpmc_cs_config(gpmc_XR16L2751, &gpmc_cfg->cs[1],
		XR16L2751_UART1_BASE, GPMC_SIZE_16M);

	enable_gpmc_cs_config(gpmc_XR16L2751, &gpmc_cfg->cs[3],
		XR16L2751_UART2_BASE, GPMC_SIZE_16M);

	gpio_request(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO, "USB_PHY1_RESET");
	gpio_direction_output(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO, 1);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
int misc_init_r(void)
{
	char *eth_addr;
	struct tam3517_module_info info;
	int ret;

	dieid_num_r();

	eth_addr = getenv("ethaddr");
	if (eth_addr)
		return 0;

	TAM3517_READ_EEPROM(&info, ret);
	if (!ret)
		TAM3517_READ_MAC_FROM_EEPROM(&info);

	return 0;
}
#endif

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_TWISTER();
}

int board_eth_init(bd_t *bis)
{
	davinci_emac_initialize();

	/* init cs for extern lan */
	enable_gpmc_cs_config(gpmc_smc911, &gpmc_cfg->cs[5],
		CONFIG_SMC911X_BASE, GPMC_SIZE_16M);
	if (smc911x_initialize(0, CONFIG_SMC911X_BASE) <= 0)
		printf("\nError initializing SMC911x controlleri\n");

	return 0;
}

#if defined(CONFIG_OMAP_HSMMC) && \
	!defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
/*
 * Do board specific preperation before SPL
 * Linux boot
 */
void spl_board_prepare_for_linux(void)
{
	/* init cs for extern lan */
	enable_gpmc_cs_config(gpmc_smc911, &gpmc_cfg->cs[5],
		CONFIG_SMC911X_BASE, GPMC_SIZE_16M);
}
int spl_start_uboot(void)
{
	int val = 0;
	if (!gpio_request(SPL_OS_BOOT_KEY, "U-Boot key")) {
		gpio_direction_input(SPL_OS_BOOT_KEY);
		val = gpio_get_value(SPL_OS_BOOT_KEY);
		gpio_free(SPL_OS_BOOT_KEY);
	}
	return val;
}
#endif
