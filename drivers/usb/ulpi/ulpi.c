/*
 * Copyright (C) 2011 Jana Rapava <fermata7@gmail.com>
 * Copyright (C) 2011 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Jana Rapava <fermata7@gmail.com>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * Based on:
 * linux/drivers/usb/otg/ulpi.c
 * Generic ULPI USB transceiver support
 *
 * Original Copyright follow:
 * Copyright (C) 2009 Daniel Mack <daniel@caiaq.de>
 *
 * Based on sources from
 *
 *   Sascha Hauer <s.hauer@pengutronix.de>
 *   Freescale Semiconductors
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
 */

#include <common.h>
#include <exports.h>
#include <usb/ulpi.h>

#define ULPI_ID_REGS_COUNT	4
#define ULPI_TEST_VALUE		0x55	/* 0x55 == 0b01010101 */

static struct ulpi_regs *ulpi = (struct ulpi_regs *)0;

static int ulpi_integrity_check(struct ulpi_viewport *ulpi_vp)
{
	u32 val, tval = ULPI_TEST_VALUE;
	int err, i;

	/* Use the 'special' test value to check all bits */
	for (i = 0; i < 2; i++, tval <<= 1) {
		err = ulpi_write(ulpi_vp, &ulpi->scratch, tval);
		if (err)
			return err;

		val = ulpi_read(ulpi_vp, &ulpi->scratch);
		if (val != tval) {
			printf("ULPI integrity check failed\n");
			return val;
		}
	}

	return 0;
}

int ulpi_init(struct ulpi_viewport *ulpi_vp)
{
	u32 val, id = 0;
	u8 *reg = &ulpi->product_id_high;
	int i;

	/* Assemble ID from four ULPI ID registers (8 bits each). */
	for (i = 0; i < ULPI_ID_REGS_COUNT; i++) {
		val = ulpi_read(ulpi_vp, reg - i);
		if (val == ULPI_ERROR)
			return val;

		id = (id << 8) | val;
	}

	/* Split ID into vendor and product ID. */
	debug("ULPI transceiver ID 0x%04x:0x%04x\n", id >> 16, id & 0xffff);

	return ulpi_integrity_check(ulpi_vp);
}

int ulpi_select_transceiver(struct ulpi_viewport *ulpi_vp, unsigned speed)
{
	u32 tspeed = ULPI_FC_FULL_SPEED;
	u32 val;

	switch (speed) {
	case ULPI_FC_HIGH_SPEED:
	case ULPI_FC_FULL_SPEED:
	case ULPI_FC_LOW_SPEED:
	case ULPI_FC_FS4LS:
		tspeed = speed;
		break;
	default:
		printf("ULPI: %s: wrong transceiver speed specified: %u, "
			"falling back to full speed\n", __func__, speed);
	}

	val = ulpi_read(ulpi_vp, &ulpi->function_ctrl);
	if (val == ULPI_ERROR)
		return val;

	/* clear the previous speed setting */
	val = (val & ~ULPI_FC_XCVRSEL_MASK) | tspeed;

	return ulpi_write(ulpi_vp, &ulpi->function_ctrl, val);
}

int ulpi_set_vbus(struct ulpi_viewport *ulpi_vp, int on, int ext_power,
			int ext_ind)
{
	u32 flags = ULPI_OTG_DRVVBUS;
	u8 *reg = on ? &ulpi->otg_ctrl_set : &ulpi->otg_ctrl_clear;

	if (ext_power)
		flags |= ULPI_OTG_DRVVBUS_EXT;
	if (ext_ind)
		flags |= ULPI_OTG_EXTVBUSIND;

	return ulpi_write(ulpi_vp, reg, flags);
}

int ulpi_set_pd(struct ulpi_viewport *ulpi_vp, int enable)
{
	u32 val = ULPI_OTG_DP_PULLDOWN | ULPI_OTG_DM_PULLDOWN;
	u8 *reg = enable ? &ulpi->otg_ctrl_set : &ulpi->otg_ctrl_clear;

	return ulpi_write(ulpi_vp, reg, val);
}

int ulpi_opmode_sel(struct ulpi_viewport *ulpi_vp, unsigned opmode)
{
	u32 topmode = ULPI_FC_OPMODE_NORMAL;
	u32 val;

	switch (opmode) {
	case ULPI_FC_OPMODE_NORMAL:
	case ULPI_FC_OPMODE_NONDRIVING:
	case ULPI_FC_OPMODE_DISABLE_NRZI:
	case ULPI_FC_OPMODE_NOSYNC_NOEOP:
		topmode = opmode;
		break;
	default:
		printf("ULPI: %s: wrong OpMode specified: %u, "
			"falling back to OpMode Normal\n", __func__, opmode);
	}

	val = ulpi_read(ulpi_vp, &ulpi->function_ctrl);
	if (val == ULPI_ERROR)
		return val;

	/* clear the previous opmode setting */
	val = (val & ~ULPI_FC_OPMODE_MASK) | topmode;

	return ulpi_write(ulpi_vp, &ulpi->function_ctrl, val);
}

int ulpi_serial_mode_enable(struct ulpi_viewport *ulpi_vp, unsigned smode)
{
	switch (smode) {
	case ULPI_IFACE_6_PIN_SERIAL_MODE:
	case ULPI_IFACE_3_PIN_SERIAL_MODE:
		break;
	default:
		printf("ULPI: %s: unrecognized Serial Mode specified: %u\n",
			__func__, smode);
		return ULPI_ERROR;
	}

	return ulpi_write(ulpi_vp, &ulpi->iface_ctrl_set, smode);
}

int ulpi_suspend(struct ulpi_viewport *ulpi_vp)
{
	int err;

	err = ulpi_write(ulpi_vp, &ulpi->function_ctrl_clear,
			ULPI_FC_SUSPENDM);
	if (err)
		printf("ULPI: %s: failed writing the suspend bit\n", __func__);

	return err;
}

/*
 * Wait for ULPI PHY reset to complete.
 * Actual wait for reset must be done in a view port specific way,
 * because it involves checking the DIR line.
 */
static int __ulpi_reset_wait(struct ulpi_viewport *ulpi_vp)
{
	u32 val;
	int timeout = CONFIG_USB_ULPI_TIMEOUT;

	/* Wait for the RESET bit to become zero */
	while (--timeout) {
		/*
		 * This function is generic and suppose to work
		 * with any viewport, so we cheat here and don't check
		 * for the error of ulpi_read(), if there is one, then
		 * there will be a timeout.
		 */
		val = ulpi_read(ulpi_vp, &ulpi->function_ctrl);
		if (!(val & ULPI_FC_RESET))
			return 0;

		udelay(1);
	}

	printf("ULPI: %s: reset timed out\n", __func__);

	return ULPI_ERROR;
}
int ulpi_reset_wait(struct ulpi_viewport *ulpi_vp)
	__attribute__((weak, alias("__ulpi_reset_wait")));

int ulpi_reset(struct ulpi_viewport *ulpi_vp)
{
	int err;

	err = ulpi_write(ulpi_vp,
			&ulpi->function_ctrl_set, ULPI_FC_RESET);
	if (err) {
		printf("ULPI: %s: failed writing reset bit\n", __func__);
		return err;
	}

	return ulpi_reset_wait(ulpi_vp);
}
