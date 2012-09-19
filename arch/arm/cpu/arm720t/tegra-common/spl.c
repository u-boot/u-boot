/*
 * (C) Copyright 2012
 * NVIDIA Inc, <www.nvidia.com>
 *
 * Allen Martin <amartin@nvidia.com>
 *
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
#include <common.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <nand.h>
#include <mmc.h>
#include <fat.h>
#include <version.h>
#include <i2c.h>
#include <image.h>
#include <malloc.h>
#include <linux/compiler.h>
#include "board.h"
#include "cpu.h"

#include <asm/io.h>
#include <asm/arch/tegra20.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/pmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/scu.h>
#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

/* Define global data structure pointer to it*/
static gd_t gdata __attribute__ ((section(".data")));
static bd_t bdata __attribute__ ((section(".data")));

inline void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}

void board_init_f(ulong dummy)
{
	board_init_uart_f();

	/* Initialize periph GPIOs */
#ifdef CONFIG_SPI_UART_SWITCH
	gpio_early_init_uart();
#else
	gpio_config_uart();
#endif

	/*
	 * We call relocate_code() with relocation target same as the
	 * CONFIG_SYS_SPL_TEXT_BASE. This will result in relocation getting
	 * skipped. Instead, only .bss initialization will happen. That's
	 * all we need
	 */
	debug(">>board_init_f()\n");
	relocate_code(CONFIG_SPL_STACK, &gdata, CONFIG_SPL_TEXT_BASE);
}

/* This requires UART clocks to be enabled */
static void preloader_console_init(void)
{
	const char *u_boot_rev = U_BOOT_VERSION;

	gd = &gdata;
	gd->bd = &bdata;
	gd->flags |= GD_FLG_RELOC;
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->have_console = 1;

	/* Avoid a second "U-Boot" coming from this string */
	u_boot_rev = &u_boot_rev[7];

	printf("\nU-Boot SPL %s (%s - %s)\n", u_boot_rev, U_BOOT_DATE,
		U_BOOT_TIME);
}

void board_init_r(gd_t *id, ulong dummy)
{
	struct pmux_tri_ctlr *pmt = (struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;

	/* enable JTAG */
	writel(0xC0, &pmt->pmt_cfg_ctl);

	debug(">>spl:board_init_r()\n");

	mem_malloc_init(CONFIG_SYS_SPL_MALLOC_START,
			CONFIG_SYS_SPL_MALLOC_SIZE);

#ifdef CONFIG_SPL_BOARD_INIT
	spl_board_init();
#endif

	clock_early_init();
	serial_init();
	preloader_console_init();

	start_cpu((u32)CONFIG_SYS_TEXT_BASE);
	halt_avp();
	/* not reached */
}

int board_usb_init(const void *blob)
{
	return 0;
}
