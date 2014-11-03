/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <status_led.h>
#include <usb.h>
#include <mmc.h>
#include <linux/compiler.h>
#include <linux/usb/musb.h>

#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/am35x_def.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/musb.h>
#include <asm/omap_musb.h>
#include <asm/ehci-omap.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"CM-T3517 board",
	"NAND 128/512M",
};

#ifdef CONFIG_USB_MUSB_AM35X
static struct musb_hdrc_config cm_t3517_musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

static struct omap_musb_board_data cm_t3517_musb_board_data = {
	.set_phy_power		= am35x_musb_phy_power,
	.clear_irq		= am35x_musb_clear_irq,
	.reset			= am35x_musb_reset,
};

static struct musb_hdrc_platform_data cm_t3517_musb_pdata = {
#if defined(CONFIG_MUSB_HOST)
	.mode           = MUSB_HOST,
#elif defined(CONFIG_MUSB_GADGET)
	.mode		= MUSB_PERIPHERAL,
#else
#error "Please define either CONFIG_MUSB_HOST or CONFIG_MUSB_GADGET"
#endif
	.config         = &cm_t3517_musb_config,
	.power          = 250,
	.platform_ops	= &am35x_ops,
	.board_data	= &cm_t3517_musb_board_data,
};

static void cm_t3517_musb_init(void)
{
	/*
	 * Set up USB clock/mode in the DEVCONF2 register.
	 * USB2.0 PHY reference clock is 13 MHz
	 */
	clrsetbits_le32(&am35x_scm_general_regs->devconf2,
			CONF2_REFFREQ | CONF2_OTGMODE | CONF2_PHY_GPIOMODE,
			CONF2_REFFREQ_13MHZ | CONF2_SESENDEN |
			CONF2_VBDTCTEN | CONF2_DATPOL);

	if (musb_register(&cm_t3517_musb_pdata, &cm_t3517_musb_board_data,
			  (void *)AM35XX_IPSS_USBOTGSS_BASE))
		printf("Failed initializing AM35x MUSB!\n");
}
#else
static inline void am3517_evm_musb_init(void) {}
#endif

int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set(STATUS_LED_BOOT, STATUS_LED_ON);
#endif

	cm_t3517_musb_init();

	return 0;
}

int misc_init_r(void)
{
	cl_print_pcb_info();
	dieid_num_r();

	return 0;
}

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
#define SB_T35_CD_GPIO 144
#define SB_T35_WP_GPIO 59

int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, SB_T35_CD_GPIO, SB_T35_WP_GPIO);
}
#endif

#ifdef CONFIG_USB_EHCI_OMAP
static struct omap_usbhs_board_data cm_t3517_usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

#define CM_T3517_USB_HUB_RESET_GPIO	152
#define SB_T35_USB_HUB_RESET_GPIO	98

int ehci_hcd_init(int index, enum usb_init_type init,
			struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	cl_usb_hub_init(CM_T3517_USB_HUB_RESET_GPIO, "cm-t3517 hub rst");
	cl_usb_hub_init(SB_T35_USB_HUB_RESET_GPIO, "sb-t35 hub rst");

	return omap_ehci_hcd_init(index, &cm_t3517_usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(void)
{
	cl_usb_hub_deinit(CM_T3517_USB_HUB_RESET_GPIO);
	cl_usb_hub_deinit(SB_T35_USB_HUB_RESET_GPIO);

	return omap_ehci_hcd_stop();
}
#endif /* CONFIG_USB_EHCI_OMAP */
