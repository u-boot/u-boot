/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

int pmic_init(unsigned char bus)
{
	static const char name[] = "MAX77686_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

#ifdef CONFIG_OF_CONTROL
	const void *blob = gd->fdt_blob;
	int node, parent;

	node = fdtdec_next_compatible(blob, 0, COMPAT_MAXIM_MAX77686_PMIC);
	if (node < 0) {
		debug("PMIC: No node for PMIC Chip in device tree\n");
		debug("node = %d\n", node);
		return -1;
	}

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	p->bus = i2c_get_bus_num_fdt(parent);
	if (p->bus < 0) {
		debug("%s: Cannot find I2C bus\n", __func__);
		return -1;
	}
	p->hw.i2c.addr = fdtdec_get_int(blob, node, "reg", 9);
#else
	p->bus = bus;
	p->hw.i2c.addr = MAX77686_I2C_ADDR;
#endif

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->hw.i2c.tx_num = 1;

	puts("Board PMIC init\n");

	return 0;
}
