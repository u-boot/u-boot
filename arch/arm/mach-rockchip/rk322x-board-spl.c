/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <ram.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/bootrom.h>
#include <asm/arch/cru_rk322x.h>
#include <asm/arch/grf_rk322x.h>
#include <asm/arch/hardware.h>
#include <asm/arch/timer.h>
#include <asm/arch/uart.h>

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}
DECLARE_GLOBAL_DATA_PTR;

#define GRF_BASE	0x11000000
#define SGRF_BASE	0x10140000

#define DEBUG_UART_BASE	0x11030000

void board_debug_uart_init(void)
{
	static struct rk322x_grf * const grf = (void *)GRF_BASE;
	enum {
		GPIO1B2_SHIFT		= 4,
		GPIO1B2_MASK		= 3 << GPIO1B2_SHIFT,
		GPIO1B2_GPIO            = 0,
		GPIO1B2_UART1_SIN,
		GPIO1B2_UART21_SIN,

		GPIO1B1_SHIFT		= 2,
		GPIO1B1_MASK		= 3 << GPIO1B1_SHIFT,
		GPIO1B1_GPIO            = 0,
		GPIO1B1_UART1_SOUT,
		GPIO1B1_UART21_SOUT,
	};
	enum {
		CON_IOMUX_UART2SEL_SHIFT= 8,
		CON_IOMUX_UART2SEL_MASK	= 1 << CON_IOMUX_UART2SEL_SHIFT,
		CON_IOMUX_UART2SEL_2	= 0,
		CON_IOMUX_UART2SEL_21,
	};

	/* Enable early UART2 channel 1 on the RK322x */
	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK | GPIO1B2_MASK,
		     GPIO1B2_UART21_SIN << GPIO1B2_SHIFT |
		     GPIO1B1_UART21_SOUT << GPIO1B1_SHIFT);
	/* Set channel C as UART2 input */
	rk_clrsetreg(&grf->con_iomux,
		     CON_IOMUX_UART2SEL_MASK,
		     CON_IOMUX_UART2SEL_21 << CON_IOMUX_UART2SEL_SHIFT);
}

#define SGRF_DDR_CON0 0x10150000
void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();
	printascii("SPL Init");

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	rockchip_timer_init();
	printf("timer init done\n");
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		printf("DRAM init failed: %d\n", ret);
		return;
	}

	/* Disable the ddr secure region setting to make it non-secure */
	rk_clrreg(SGRF_DDR_CON0, 0x4000);
#if defined(CONFIG_ROCKCHIP_SPL_BACK_TO_BROM) && !defined(CONFIG_SPL_BOARD_INIT)
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
#endif
}
