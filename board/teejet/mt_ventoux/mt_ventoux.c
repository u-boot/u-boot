/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <common.h>
#include <netdev.h>
#include <fpga.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <i2c.h>
#include <spartan3.h>
#include <asm/gpio.h>
#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/ehci-omap.h>
#endif
#include "mt_ventoux.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_FPGA
#error "The Teejet mt_ventoux must have CONFIG_FPGA enabled"
#endif

#define FPGA_RESET	62
#define FPGA_PROG	116
#define FPGA_CCLK	117
#define FPGA_DIN	118
#define FPGA_INIT	119
#define FPGA_DONE	154

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

int ehci_hcd_init(void)
{
	return omap_ehci_hcd_init(&usbhs_bdata);
}

int ehci_hcd_stop(void)
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

	fpga_reset(TRUE);
	udelay(100);
	fpga_reset(FALSE);

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

Xilinx_Spartan3_Slave_Serial_fns mt_ventoux_fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_clk_fn,
	fpga_init_fn,
	fpga_done_fn,
	fpga_wr_fn,
	fpga_post_config_fn,
};

Xilinx_desc fpga = XILINX_XC6SLX4_DESC(slave_serial,
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

	return 0;
}

int misc_init_r(void)
{
	dieid_num_r();

	return 0;
}

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

#if defined(CONFIG_OMAP_HSMMC) && \
	!defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0);
}
#endif
