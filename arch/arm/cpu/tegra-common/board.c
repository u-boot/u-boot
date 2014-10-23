/*
 *  (C) Copyright 2010-2014
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/mc.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/sys_proto.h>
#include <asm/arch-tegra/warmboot.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* UARTs which we can enable */
	UARTA	= 1 << 0,
	UARTB	= 1 << 1,
	UARTC	= 1 << 2,
	UARTD	= 1 << 3,
	UARTE	= 1 << 4,
	UART_COUNT = 5,
};

/* Read the RAM size directly from the memory controller */
unsigned int query_sdram_size(void)
{
	struct mc_ctlr *const mc = (struct mc_ctlr *)NV_PA_MC_BASE;
	u32 size_mb;

	size_mb = readl(&mc->mc_emem_cfg);
#if defined(CONFIG_TEGRA20)
	debug("mc->mc_emem_cfg (MEM_SIZE_KB) = 0x%08x\n", size_mb);
	size_mb = get_ram_size((void *)PHYS_SDRAM_1, size_mb * 1024);
#else
	debug("mc->mc_emem_cfg (MEM_SIZE_MB) = 0x%08x\n", size_mb);
	size_mb = get_ram_size((void *)PHYS_SDRAM_1, size_mb * 1024 * 1024);
#endif

#if defined(CONFIG_TEGRA30) || defined(CONFIG_TEGRA114)
	/* External memory limited to 2047 MB due to IROM/HI-VEC */
	if (size_mb == SZ_2G) size_mb -= SZ_1M;
#endif

	return size_mb;
}

int dram_init(void)
{
	/* We do not initialise DRAM here. We just query the size */
	gd->ram_size = query_sdram_size();
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board: %s\n", sysinfo.board_string);
	return 0;
}
#endif	/* CONFIG_DISPLAY_BOARDINFO */

static int uart_configs[] = {
#if defined(CONFIG_TEGRA20)
 #if defined(CONFIG_TEGRA_UARTA_UAA_UAB)
	FUNCMUX_UART1_UAA_UAB,
 #elif defined(CONFIG_TEGRA_UARTA_GPU)
	FUNCMUX_UART1_GPU,
 #elif defined(CONFIG_TEGRA_UARTA_SDIO1)
	FUNCMUX_UART1_SDIO1,
 #else
	FUNCMUX_UART1_IRRX_IRTX,
#endif
	FUNCMUX_UART2_UAD,
	-1,
	FUNCMUX_UART4_GMC,
	-1,
#elif defined(CONFIG_TEGRA30)
	FUNCMUX_UART1_ULPI,	/* UARTA */
	-1,
	-1,
	-1,
	-1,
#elif defined(CONFIG_TEGRA114)
	-1,
	-1,
	-1,
	FUNCMUX_UART4_GMI,	/* UARTD */
	-1,
#else	/* Tegra124 */
	FUNCMUX_UART1_KBC,	/* UARTA */
	-1,
	-1,
	FUNCMUX_UART4_GPIO,	/* UARTD */
	-1,
#endif
};

/**
 * Set up the specified uarts
 *
 * @param uarts_ids	Mask containing UARTs to init (UARTx)
 */
static void setup_uarts(int uart_ids)
{
	static enum periph_id id_for_uart[] = {
		PERIPH_ID_UART1,
		PERIPH_ID_UART2,
		PERIPH_ID_UART3,
		PERIPH_ID_UART4,
		PERIPH_ID_UART5,
	};
	size_t i;

	for (i = 0; i < UART_COUNT; i++) {
		if (uart_ids & (1 << i)) {
			enum periph_id id = id_for_uart[i];

			funcmux_select(id, uart_configs[i]);
			clock_ll_start_uart(id);
		}
	}
}

void board_init_uart_f(void)
{
	int uart_ids = 0;	/* bit mask of which UART ids to enable */

#ifdef CONFIG_TEGRA_ENABLE_UARTA
	uart_ids |= UARTA;
#endif
#ifdef CONFIG_TEGRA_ENABLE_UARTB
	uart_ids |= UARTB;
#endif
#ifdef CONFIG_TEGRA_ENABLE_UARTC
	uart_ids |= UARTC;
#endif
#ifdef CONFIG_TEGRA_ENABLE_UARTD
	uart_ids |= UARTD;
#endif
#ifdef CONFIG_TEGRA_ENABLE_UARTE
	uart_ids |= UARTE;
#endif
	setup_uarts(uart_ids);
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
