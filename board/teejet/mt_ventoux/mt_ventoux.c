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
#include <malloc.h>
#include <fpga.h>
#include <video_fb.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/dss.h>
#include <asm/arch/clock.h>
#include <i2c.h>
#include <spartan3.h>
#include <asm/gpio.h>
#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/ehci-omap.h>
#endif
#include "mt_ventoux.h"

DECLARE_GLOBAL_DATA_PTR;

#define BUZZER		140
#define SPEAKER		141
#define USB1_PWR	127
#define USB2_PWR	149

#ifndef CONFIG_FPGA
#error "The Teejet mt_ventoux must have CONFIG_FPGA enabled"
#endif

#define FPGA_RESET	62
#define FPGA_PROG	116
#define FPGA_CCLK	117
#define FPGA_DIN	118
#define FPGA_INIT	119
#define FPGA_DONE	154

#define LCD_PWR		138
#define LCD_PON_PIN	139

#if defined(CONFIG_VIDEO) && !defined(CONFIG_SPL_BUILD)
static struct {
	u32 xres;
	u32 yres;
} panel_resolution[] = {
	{ 480, 272 },
	{ 800, 480 }
};

static struct panel_config lcd_cfg[] = {
	{
	.timing_h       = PANEL_TIMING_H(40, 5, 2),
	.timing_v       = PANEL_TIMING_V(8, 8, 2),
	.pol_freq       = 0x00003000, /* Pol Freq */
	.divisor        = 0x00010033, /* 9 Mhz Pixel Clock */
	.panel_type     = 0x01, /* TFT */
	.data_lines     = 0x03, /* 24 Bit RGB */
	.load_mode      = 0x02, /* Frame Mode */
	.panel_color	= 0,
	.gfx_format	= GFXFORMAT_RGB24_UNPACKED,
	},
	{
	.timing_h       = PANEL_TIMING_H(20, 192, 4),
	.timing_v       = PANEL_TIMING_V(2, 20, 10),
	.pol_freq       = 0x00004000, /* Pol Freq */
	.divisor        = 0x0001000E, /* 36Mhz Pixel Clock */
	.panel_type     = 0x01, /* TFT */
	.data_lines     = 0x03, /* 24 Bit RGB */
	.load_mode      = 0x02, /* Frame Mode */
	.panel_color	= 0,
	.gfx_format	= GFXFORMAT_RGB24_UNPACKED,
	}
};
#endif

/* Timing definitions for FPGA */
static const u32 gpmc_fpga[] = {
	FPGA_GPMC_CONFIG1,
	FPGA_GPMC_CONFIG2,
	FPGA_GPMC_CONFIG3,
	FPGA_GPMC_CONFIG4,
	FPGA_GPMC_CONFIG5,
	FPGA_GPMC_CONFIG6,
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


static inline void fpga_reset(int nassert)
{
	gpio_set_value(FPGA_RESET, !nassert);
}

int fpga_pgm_fn(int nassert, int nflush, int cookie)
{
	debug("%s:%d: FPGA PROGRAM ", __func__, __LINE__);

	gpio_set_value(FPGA_PROG, !nassert);

	return nassert;
}

int fpga_init_fn(int cookie)
{
	return !gpio_get_value(FPGA_INIT);
}

int fpga_done_fn(int cookie)
{
	return gpio_get_value(FPGA_DONE);
}

int fpga_pre_config_fn(int cookie)
{
	debug("%s:%d: FPGA pre-configuration\n", __func__, __LINE__);

	/* Setting GPIOs for programming Mode */
	gpio_request(FPGA_RESET, "FPGA_RESET");
	gpio_direction_output(FPGA_RESET, 1);
	gpio_request(FPGA_PROG, "FPGA_PROG");
	gpio_direction_output(FPGA_PROG, 1);
	gpio_request(FPGA_CCLK, "FPGA_CCLK");
	gpio_direction_output(FPGA_CCLK, 1);
	gpio_request(FPGA_DIN, "FPGA_DIN");
	gpio_direction_output(FPGA_DIN, 0);
	gpio_request(FPGA_INIT, "FPGA_INIT");
	gpio_direction_input(FPGA_INIT);
	gpio_request(FPGA_DONE, "FPGA_DONE");
	gpio_direction_input(FPGA_DONE);

	/* Be sure that signal are deasserted */
	gpio_set_value(FPGA_RESET, 1);
	gpio_set_value(FPGA_PROG, 1);

	return 0;
}

int fpga_post_config_fn(int cookie)
{
	debug("%s:%d: FPGA post-configuration\n", __func__, __LINE__);

	fpga_reset(true);
	udelay(100);
	fpga_reset(false);

	return 0;
}

/* Write program to the FPGA */
int fpga_wr_fn(int nassert_write, int flush, int cookie)
{
	gpio_set_value(FPGA_DIN, nassert_write);

	return nassert_write;
}

int fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	gpio_set_value(FPGA_CCLK, assert_clk);

	return assert_clk;
}

xilinx_spartan3_slave_serial_fns mt_ventoux_fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_clk_fn,
	fpga_init_fn,
	fpga_done_fn,
	fpga_wr_fn,
	fpga_post_config_fn,
};

xilinx_desc fpga = XILINX_XC6SLX4_DESC(slave_serial,
			(void *)&mt_ventoux_fpga_fns, 0);

/* Initialize the FPGA */
static void mt_ventoux_init_fpga(void)
{
	fpga_pre_config_fn(0);

	/* Setting CS1 for FPGA access */
	enable_gpmc_cs_config(gpmc_fpga, &gpmc_cfg->cs[1],
		FPGA_BASE_ADDR, GPMC_SIZE_128M);

	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	mt_ventoux_init_fpga();

	/* GPIO_140: speaker #mute */
	MUX_VAL(CP(MCBSP3_DX),		(IEN | PTU | EN | M4))
	/* GPIO_141: Buzz Hi */
	MUX_VAL(CP(MCBSP3_DR),		(IEN  | PTU | EN | M4))

	/* Turning off the buzzer */
	gpio_request(BUZZER, "BUZZER_MUTE");
	gpio_request(SPEAKER, "SPEAKER");
	gpio_direction_output(BUZZER, 0);
	gpio_direction_output(SPEAKER, 0);

	/* Activate USB power */
	gpio_request(USB1_PWR, "USB1_PWR");
	gpio_request(USB2_PWR, "USB2_PWR");
	gpio_direction_output(USB1_PWR, 1);
	gpio_direction_output(USB2_PWR, 1);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
int misc_init_r(void)
{
	char *eth_addr;
	struct tam3517_module_info info;
	int ret;

	TAM3517_READ_EEPROM(&info, ret);
	omap_die_id_display();

	if (ret)
		return 0;
	eth_addr = getenv("ethaddr");
	if (!eth_addr)
		TAM3517_READ_MAC_FROM_EEPROM(&info);

	TAM3517_PRINT_SOM_INFO(&info);
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
	MUX_MT_VENTOUX();
}

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int board_eth_init(bd_t *bis)
{
	davinci_emac_initialize();
	return 0;
}

#if defined(CONFIG_MMC_OMAP_HS) && \
	!defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#if defined(CONFIG_VIDEO) && !defined(CONFIG_SPL_BUILD)
int board_video_init(void)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct panel_config *panel = &lcd_cfg[0];
	char *s;
	u32 index = 0;

	void *fb;

	fb = (void *)0x88000000;

	s = getenv("panel");
	if (s) {
		index = simple_strtoul(s, NULL, 10);
		if (index < ARRAY_SIZE(lcd_cfg))
			panel = &lcd_cfg[index];
		else
			return 0;
	}

	panel->frame_buffer = fb;
	printf("Panel: %dx%d\n", panel_resolution[index].xres,
		panel_resolution[index].yres);
	panel->lcd_size = (panel_resolution[index].yres - 1) << 16 |
		(panel_resolution[index].xres - 1);

	gpio_request(LCD_PWR, "LCD Power");
	gpio_request(LCD_PON_PIN, "LCD Pon");
	gpio_direction_output(LCD_PWR, 0);
	gpio_direction_output(LCD_PON_PIN, 1);


	setbits_le32(&prcm_base->fclken_dss, FCK_DSS_ON);
	setbits_le32(&prcm_base->iclken_dss, ICK_DSS_ON);

	omap3_dss_panel_config(panel);
	omap3_dss_enable();

	return 0;
}
#endif
