/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk.h>

/*
 * zynq_slcr_mio_get_status - Get the status of MIO peripheral.
 *
 * @peri_name: Name of the peripheral for checking MIO status
 * @get_pins: Pointer to array of get pin for this peripheral
 * @num_pins: Number of pins for this peripheral
 * @mask: Mask value
 * @check_val: Required check value to get the status of  periph
 */
struct zynq_slcr_mio_get_status {
	const char *peri_name;
	const int *get_pins;
	int num_pins;
	u32 mask;
	u32 check_val;
};

static const struct zynq_slcr_mio_get_status mio_periphs[] = {
};

/*
 * zynq_slcr_get_mio_pin_status - Get the MIO pin status of peripheral.
 *
 * @periph: Name of the peripheral
 *
 * Returns count to indicate the number of pins configured for the
 * given @periph.
 */
int zynq_slcr_get_mio_pin_status(const char *periph)
{
	const struct zynq_slcr_mio_get_status *mio_ptr;
	int val, i, j;
	int mio = 0;

	for (i = 0; i < ARRAY_SIZE(mio_periphs); i++) {
		if (strcmp(periph, mio_periphs[i].peri_name) == 0) {
			mio_ptr = &mio_periphs[i];
			for (j = 0; j < mio_ptr->num_pins; j++) {
				val = readl(&slcr_base->mio_pin
						[mio_ptr->get_pins[j]]);
				if ((val & mio_ptr->mask) == mio_ptr->check_val)
					mio++;
			}
			break;
		}
	}

	return mio;
}
