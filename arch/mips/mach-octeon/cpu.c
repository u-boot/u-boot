// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <asm/global_data.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <mach/clock.h>
#include <mach/cavm-reg.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * TRUE for devices having registers with little-endian byte
 * order, FALSE for registers with native-endian byte order.
 * PCI mandates little-endian, USB and SATA are configurable,
 * but we chose little-endian for these.
 *
 * This table will be referened in the Octeon platform specific
 * mangle-port.h header.
 */
const bool octeon_should_swizzle_table[256] = {
	[0x00] = true,	/* bootbus/CF */
	[0x1b] = true,	/* PCI mmio window */
	[0x1c] = true,	/* PCI mmio window */
	[0x1d] = true,	/* PCI mmio window */
	[0x1e] = true,	/* PCI mmio window */
	[0x68] = true,	/* OCTEON III USB */
	[0x69] = true,	/* OCTEON III USB */
	[0x6c] = true,	/* OCTEON III SATA */
	[0x6f] = true,	/* OCTEON II USB */
};

static int get_clocks(void)
{
	const u64 ref_clock = PLL_REF_CLK;
	void __iomem *rst_boot;
	u64 val;

	rst_boot = ioremap(CAVM_RST_BOOT, 0);
	val = ioread64(rst_boot);
	gd->cpu_clk = ref_clock * FIELD_GET(RST_BOOT_C_MUL, val);
	gd->bus_clk = ref_clock * FIELD_GET(RST_BOOT_PNR_MUL, val);

	debug("%s: cpu: %lu, bus: %lu\n", __func__, gd->cpu_clk, gd->bus_clk);

	return 0;
}

/* Early mach init code run from flash */
int mach_cpu_init(void)
{
	void __iomem *mio_boot_reg_cfg0;

	/* Remap boot-bus 0x1fc0.0000 -> 0x1f40.0000 */
	/* ToDo: Move this to an early running bus (bootbus) DM driver */
	mio_boot_reg_cfg0 = ioremap(CAVM_MIO_BOOT_REG_CFG0, 0);
	clrsetbits_be64(mio_boot_reg_cfg0, 0xffff, 0x1f40);

	/* Get clocks and store them in GD */
	get_clocks();

	return 0;
}

/**
 * Returns number of cores
 *
 * @return	number of CPU cores for the specified node
 */
static int cavm_octeon_num_cores(void)
{
	void __iomem *ciu_fuse;

	ciu_fuse = ioremap(CAVM_CIU_FUSE, 0);
	return fls64(ioread64(ciu_fuse) & 0xffffffffffff);
}

int print_cpuinfo(void)
{
	printf("SoC:   Octeon CN73xx (%d cores)\n", cavm_octeon_num_cores());

	return 0;
}
