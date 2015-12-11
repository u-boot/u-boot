/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* This is a board specific file.  It's OK to include board specific
 * header files */

#include <common.h>
#include <config.h>
#include <fdtdec.h>
#include <asm/processor.h>
#include <asm/microblaze_intc.h>
#include <asm/asm.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_XILINX_GPIO
static int reset_pin = -1;
#endif

ulong ram_base;

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = ram_base;
	gd->bd->bi_dram[0].size = get_effective_memsize();
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

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_XILINX_GPIO
	if (reset_pin != -1)
		gpio_direction_output(reset_pin, 1);
#endif

#ifdef CONFIG_XILINX_TB_WATCHDOG
	hw_watchdog_disable();
#endif
#endif
	puts ("Reseting board\n");
	__asm__ __volatile__ ("	mts rmsr, r0;" \
				"bra r0");

	return 0;
}

static int gpio_init(void)
{
#ifdef CONFIG_XILINX_GPIO
	reset_pin = gpio_alloc(CONFIG_SYS_GPIO_0_ADDR, "reset", 1);
	if (reset_pin != -1)
		gpio_request(reset_pin, "reset_pin");
#endif
	return 0;
}

int board_late_init(void)
{
	gpio_init();

	return 0;
}
