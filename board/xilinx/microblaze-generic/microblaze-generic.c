// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 */

/*
 * This is a board specific file.  It's OK to include board specific
 * header files
 */

#include <common.h>
#include <config.h>
#include <fdtdec.h>
#include <asm/processor.h>
#include <asm/microblaze_intc.h>
#include <asm/asm.h>
#include <asm/gpio.h>
#include <dm/uclass.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_XILINX_GPIO
static int reset_pin = -1;
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT)
static struct udevice *watchdog_dev;
#endif /* !CONFIG_SPL_BUILD && CONFIG_WDT */

ulong ram_base;

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = ram_base;
	gd->bd->bi_dram[0].size = get_effective_memsize();

	return 0;
}

int dram_init(void)
{
	int node;
	fdt_addr_t addr;
	fdt_size_t size;
	const void *blob = gd->fdt_blob;

	node = fdt_node_offset_by_prop_value(blob, -1, "device_type",
					     "memory", 7);
	if (node == -FDT_ERR_NOTFOUND) {
		debug("DRAM: Can't get memory node\n");
		return 1;
	}
	addr = fdtdec_get_addr_size(blob, node, "reg", &size);
	if (addr == FDT_ADDR_T_NONE || size == 0) {
		debug("DRAM: Can't get base address or size\n");
		return 1;
	}
	ram_base = addr;

	gd->ram_top = addr; /* In setup_dest_addr() is done +ram_size */
	gd->ram_size = size;

	return 0;
};

#if !defined(CONFIG_SYSRESET) || defined(CONFIG_SPL_BUILD)
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_XILINX_GPIO
	if (reset_pin != -1)
		gpio_direction_output(reset_pin, 1);
#endif
#endif
	puts("Resetting board\n");
	__asm__ __volatile__ ("	mts rmsr, r0;" \
				"bra r0");

	return 0;
}
#endif

static int gpio_init(void)
{
#ifdef CONFIG_XILINX_GPIO
	reset_pin = gpio_alloc(CONFIG_SYS_GPIO_0_ADDR, "reset", 1);
	if (reset_pin != -1)
		gpio_request(reset_pin, "reset_pin");
#endif
	return 0;
}

#ifdef CONFIG_WDT
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
#if !defined(CONFIG_SPL_BUILD)
	ulong now;
	static ulong next_reset;

	if (!watchdog_dev)
		return;

	now = timer_get_us();

	/* Do not reset the watchdog too often */
	if (now > next_reset) {
		wdt_reset(watchdog_dev);
		next_reset = now + 1000;
	}
#endif /* !CONFIG_SPL_BUILD */
}
#endif /* CONFIG_WDT */

int board_late_init(void)
{
	gpio_init();

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT)
	watchdog_dev = NULL;

	if (uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev)) {
		debug("Watchdog: Not found by seq!\n");
		if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
			puts("Watchdog: Not found!\n");
			return 0;
		}
	}

	wdt_start(watchdog_dev, 0, 0);
	puts("Watchdog: Started\n");
#endif /* !CONFIG_SPL_BUILD && CONFIG_WDT */

	return 0;
}
