/*
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * Based on da850evm.c, original Copyrights follow:
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/davinci_nand.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <asm/gpio.h>
#include "../../../drivers/video/da8xx-fb.h"

DECLARE_GLOBAL_DATA_PTR;

static const struct da8xx_panel lcd_panel = {
	/* Casio COM57H531x */
	.name = "Casio_COM57H531x",
	.width = 640,
	.height = 480,
	.hfp = 12,
	.hbp = 144,
	.hsw = 30,
	.vfp = 10,
	.vbp = 35,
	.vsw = 3,
	.pxl_clk = 25000000,
	.invert_pxl_clk = 0,
};

static const struct display_panel disp_panel = {
	QVGA,
	16,
	16,
	COLOR_ACTIVE,
};

static const struct lcd_ctrl_config lcd_cfg = {
	&disp_panel,
	.ac_bias		= 255,
	.ac_bias_intrpt		= 0,
	.dma_burst_sz		= 16,
	.bpp			= 16,
	.fdd			= 255,
	.tft_alt_mode		= 0,
	.stn_565_mode		= 0,
	.mono_8bit_mode		= 0,
	.invert_line_clock	= 1,
	.invert_frm_clock	= 1,
	.sync_edge		= 0,
	.sync_ctrl		= 1,
	.raster_order		= 0,
};

/* SPI0 pin muxer settings */
static const struct pinmux_config spi1_pins[] = {
	{ pinmux(5), 1, 1 },
	{ pinmux(5), 1, 2 },
	{ pinmux(5), 1, 4 },
	{ pinmux(5), 1, 5 }
};

/* I2C pin muxer settings */
static const struct pinmux_config i2c_pins[] = {
	{ pinmux(4), 2, 2 },
	{ pinmux(4), 2, 3 }
};

/* UART0 pin muxer settings */
static const struct pinmux_config uart_pins[] = {
	{ pinmux(3), 2, 7 },
	{ pinmux(3), 2, 6 },
	{ pinmux(3), 2, 4 },
	{ pinmux(3), 2, 5 }
};

#ifdef CONFIG_DRIVER_TI_EMAC
#define HAS_RMII 1
static const struct pinmux_config emac_pins[] = {
	{ pinmux(14), 8, 2 },
	{ pinmux(14), 8, 3 },
	{ pinmux(14), 8, 4 },
	{ pinmux(14), 8, 5 },
	{ pinmux(14), 8, 6 },
	{ pinmux(14), 8, 7 },
	{ pinmux(15), 8, 1 },
	{ pinmux(4), 8, 0 },
	{ pinmux(4), 8, 1 }
};
#endif

#ifdef CONFIG_NAND_DAVINCI
const struct pinmux_config nand_pins[] = {
	{ pinmux(7), 1, 0},	/* CS2 */
	{ pinmux(7), 0, 1},	/* CS3  in three state*/
	{ pinmux(7), 1, 4 },	/* EMA_WE */
	{ pinmux(7), 1, 5 },	/* EMA_OE */
	{ pinmux(9), 1, 0 },	/* EMA_D[7] */
	{ pinmux(9), 1, 1 },	/* EMA_D[6] */
	{ pinmux(9), 1, 2 },	/* EMA_D[5] */
	{ pinmux(9), 1, 3 },	/* EMA_D[4] */
	{ pinmux(9), 1, 4 },	/* EMA_D[3] */
	{ pinmux(9), 1, 5 },	/* EMA_D[2] */
	{ pinmux(9), 1, 6 },	/* EMA_D[1] */
	{ pinmux(9), 1, 7 },	/* EMA_D[0] */
	{ pinmux(12), 1, 5 },	/* EMA_A[2] */
	{ pinmux(12), 1, 6 },	/* EMA_A[1] */
	{ pinmux(6), 1, 0 }	/* EMA_CLK */
};
#endif

const struct pinmux_config gpio_pins[] = {
	{ pinmux(13), 8, 0 }, /* GPIO6[15] RESETOUTn on SOM*/
	{ pinmux(13), 8, 5 }, /* GPIO6[10] U0_SW0 on EA20-00101_2*/
	{ pinmux(13), 8, 3 }, /* GPIO6[12] U0_SW1 on EA20-00101_2*/
	{ pinmux(19), 8, 5 }, /* GPIO6[1]  DISP_ON */
	{ pinmux(14), 8, 1 }  /* GPIO6[6]  LCD_B_PWR*/
};

const struct pinmux_config lcd_pins[] = {
	{ pinmux(17), 2, 1 }, /* LCD_D_0 */
	{ pinmux(17), 2, 0 }, /* LCD_D_1 */
	{ pinmux(16), 2, 7 }, /* LCD_D_2 */
	{ pinmux(16), 2, 6 }, /* LCD_D_3 */
	{ pinmux(16), 2, 5 }, /* LCD_D_4 */
	{ pinmux(16), 2, 4 }, /* LCD_D_5 */
	{ pinmux(16), 2, 3 }, /* LCD_D_6 */
	{ pinmux(16), 2, 2 }, /* LCD_D_7 */
	{ pinmux(18), 2, 1 }, /* LCD_D_8 */
	{ pinmux(18), 2, 0 }, /* LCD_D_9 */
	{ pinmux(17), 2, 7 }, /* LCD_D_10 */
	{ pinmux(17), 2, 6 }, /* LCD_D_11 */
	{ pinmux(17), 2, 5 }, /* LCD_D_12 */
	{ pinmux(17), 2, 4 }, /* LCD_D_13 */
	{ pinmux(17), 2, 3 }, /* LCD_D_14 */
	{ pinmux(17), 2, 2 }, /* LCD_D_15 */
	{ pinmux(18), 2, 6 }, /* LCD_PCLK */
	{ pinmux(19), 2, 0 }, /* LCD_HSYNC */
	{ pinmux(19), 2, 1 }, /* LCD_VSYNC */
	{ pinmux(19), 2, 6 }, /* DA850_NLCD_AC_ENB_CS */
};

const struct pinmux_config halten_pin[] = {
	{ pinmux(3),  4, 2 } /* GPIO8[6] HALTEN */
};

static const struct pinmux_resource pinmuxes[] = {
#ifdef CONFIG_SPI_FLASH
	PINMUX_ITEM(spi1_pins),
#endif
	PINMUX_ITEM(uart_pins),
	PINMUX_ITEM(i2c_pins),
#ifdef CONFIG_NAND_DAVINCI
	PINMUX_ITEM(nand_pins),
#endif
#ifdef CONFIG_VIDEO
	PINMUX_ITEM(lcd_pins),
#endif
};

static const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI1 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART0 },	/* console */
	{ DAVINCI_LPSC_GPIO },
	{ DAVINCI_LPSC_LCDC }, /* LCD */
};

int board_early_init_f(void)
{
	/* PinMux for GPIO */
	if (davinci_configure_pin_mux(gpio_pins, ARRAY_SIZE(gpio_pins)) != 0)
		return 1;

	/* Set DISP_ON high to enable LCD output*/
	gpio_direction_output(97, 1);

	/* Set the RESETOUTn low */
	gpio_direction_output(111, 0);

	/* Set U0_SW0 low for UART0 as console*/
	gpio_direction_output(106, 0);

	/* Set U0_SW1 low for UART0 as console*/
	gpio_direction_output(108, 0);

	/* Set LCD_B_PWR low to power down LCD Backlight*/
	gpio_direction_output(102, 0);

#ifndef CONFIG_USE_IRQ
	irq_init();
#endif

	/*
	 * NAND CS setup - cycle counts based on da850evm NAND timings in the
	 * Linux kernel @ 25MHz EMIFA
	 */
#ifdef CONFIG_NAND_DAVINCI
	writel((DAVINCI_ABCR_WSETUP(0) |
		DAVINCI_ABCR_WSTROBE(1) |
		DAVINCI_ABCR_WHOLD(0) |
		DAVINCI_ABCR_RSETUP(0) |
		DAVINCI_ABCR_RSTROBE(1) |
		DAVINCI_ABCR_RHOLD(0) |
		DAVINCI_ABCR_TA(0) |
		DAVINCI_ABCR_ASIZE_8BIT),
	       &davinci_emif_regs->ab1cr); /* CS2 */
#endif

	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	if (da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc)))
		return 1;

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART0),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

#ifdef CONFIG_DRIVER_TI_EMAC
	if (davinci_configure_pin_mux(emac_pins, ARRAY_SIZE(emac_pins)) != 0)
		return 1;

	davinci_emac_mii_mode_sel(HAS_RMII);
#endif /* CONFIG_DRIVER_TI_EMAC */

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart0_ctrl_regs->pwremu_mgmt);

	/*
	 * Reconfigure the LCDC priority to the highest to ensure that
	 * the throughput/latency requirements for the LCDC are met.
	 */
	writel(readl(&davinci_syscfg_regs->mstpri[2]) & 0x0fffffff,
	       &davinci_syscfg_regs->mstpri[2]);


	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_init(void)
{
	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_EA20;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	da8xx_video_init(&lcd_panel, &lcd_cfg, 16);

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT

int board_late_init(void)
{
	unsigned char buf[2];
	int ret;

	/* PinMux for HALTEN */
	if (davinci_configure_pin_mux(halten_pin, ARRAY_SIZE(halten_pin)) != 0)
		return 1;

	/* Set HALTEN to high */
	gpio_direction_output(134, 1);

	/* Set fixed contrast settings for LCD via I2C potentiometer */
	buf[0] = 0x00;
	buf[1] = 0xd7;
	ret = i2c_write(0x2e, 6, 1, buf, 2);
	if (ret)
		puts("\nContrast Settings FAILED\n");

	/* Set LCD_B_PWR high to power up LCD Backlight*/
	gpio_set_value(102, 1);
	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

#ifdef CONFIG_DRIVER_TI_EMAC

/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	/*
	 * This board has a RMII PHY. However, the MDC line on the SOM
	 * must not be disabled (there is no MII PHY on the
	 * baseboard) via the GPIO2[6], because this pin
	 * disables at the same time the SPI flash.
	 */

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */
