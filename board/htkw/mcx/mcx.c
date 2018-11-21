// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Ilya Yanok, Emcraft Systems
 *
 * Based on ti/evm/evm.c
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/omap_gpio.h>
#include <asm/arch/dss.h>
#include <asm/arch/clock.h>
#include <errno.h>
#include <i2c.h>
#ifdef CONFIG_USB_EHCI_HCD
#include <usb.h>
#include <asm/ehci-omap.h>
#endif
#include "mcx.h"

DECLARE_GLOBAL_DATA_PTR;

#define HOT_WATER_BUTTON	42
#define LCD_OUTPUT		55

/* Address of the framebuffer in RAM. */
#define FB_START_ADDRESS 0x88000000

#ifdef CONFIG_USB_EHCI_HCD
static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_USBHS_PORT_MODE_UNUSED,
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

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	gpio_direction_output(LCD_OUTPUT, 0);

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	if (gpio_request(HOT_WATER_BUTTON, "hot-water-button") < 0) {
		puts("Failed to get hot-water-button pin\n");
		return -ENODEV;
	}
	gpio_direction_input(HOT_WATER_BUTTON);

	/*
	 * if hot-water-button is pressed
	 * change bootcmd
	 */
	if (gpio_get_value(HOT_WATER_BUTTON))
		return 0;

	env_set("bootcmd", "run swupdate");

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
	MUX_MCX();
}

#if defined(CONFIG_MMC_OMAP_HS)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#if defined(CONFIG_VIDEO) && !defined(CONFIG_SPL_BUILD)

static struct panel_config lcd_cfg = {
	.timing_h       = PANEL_TIMING_H(40, 40, 48),
	.timing_v       = PANEL_TIMING_V(29, 13, 3),
	.pol_freq       = 0x00003000, /* Pol Freq */
	.divisor        = 0x0001000E,
	.panel_type     = 0x01, /* TFT */
	.data_lines     = 0x03, /* 24 Bit RGB */
	.load_mode      = 0x02, /* Frame Mode */
	.panel_color	= 0,
	.lcd_size	= PANEL_LCD_SIZE(800, 480),
	.gfx_format	= GFXFORMAT_RGB24_UNPACKED,
};

int board_video_init(void)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	void *fb;

	fb = (void *)FB_START_ADDRESS;

	lcd_cfg.frame_buffer = fb;

	setbits_le32(&prcm_base->fclken_dss, FCK_DSS_ON);
	setbits_le32(&prcm_base->iclken_dss, ICK_DSS_ON);

	omap3_dss_panel_config(&lcd_cfg);
	omap3_dss_enable();

	return 0;
}
#endif
