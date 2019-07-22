// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 */
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_rk3288.h>
#include <asm/arch-rockchip/pmu_rk3288.h>
#include <asm/arch-rockchip/sdram_common.h>

DECLARE_GLOBAL_DATA_PTR;

#define GRF_BASE	0xff770000

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "dwmmc@ff0f0000",
	[BROM_BOOTSOURCE_SD] = "dwmmc@ff0c0000",
};

#ifdef CONFIG_SPL_BUILD
static void configure_l2ctlr(void)
{
	u32 l2ctlr;

	l2ctlr = read_l2ctlr();
	l2ctlr &= 0xfffc0000; /* clear bit0~bit17 */

	/*
	 * Data RAM write latency: 2 cycles
	 * Data RAM read latency: 2 cycles
	 * Data RAM setup latency: 1 cycle
	 * Tag RAM write latency: 1 cycle
	 * Tag RAM read latency: 1 cycle
	 * Tag RAM setup latency: 1 cycle
	 */
	l2ctlr |= (1 << 3 | 1 << 0);
	write_l2ctlr(l2ctlr);
}
#endif

int arch_cpu_init(void)
{
#ifdef CONFIG_SPL_BUILD
	configure_l2ctlr();
#else
	/* We do some SoC one time setting here. */
	struct rk3288_grf * const grf = (void *)GRF_BASE;

	/* Use rkpwm by default */
	rk_setreg(&grf->soc_con2, 1 << 0);
#endif

	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	/* Enable early UART on the RK3288 */
	struct rk3288_grf * const grf = (void *)GRF_BASE;

	rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C7_MASK << GPIO7C7_SHIFT |
		     GPIO7C6_MASK << GPIO7C6_SHIFT,
		     GPIO7C7_UART2DBG_SOUT << GPIO7C7_SHIFT |
		     GPIO7C6_UART2DBG_SIN << GPIO7C6_SHIFT);
}
#endif

#ifdef CONFIG_SPL_OS_BOOT

#define PMU_BASE		0xff730000
int dram_init_banksize(void)
{
	struct rk3288_pmu *const pmu = (void *)PMU_BASE;
	size_t size = rockchip_sdram_size((phys_addr_t)&pmu->sys_reg[2]);

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = size;

	return 0;
}
#endif
