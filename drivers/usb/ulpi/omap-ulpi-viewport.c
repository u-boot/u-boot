/*
 * OMAP ulpi viewport support
 * Based on drivers/usb/ulpi/ulpi-viewport.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com
 * Author: Govindraj R <govindraj.raja@ti.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <asm/io.h>
#include <usb/ulpi.h>

#define OMAP_ULPI_WR_OPSEL	(3 << 21)
#define OMAP_ULPI_ACCESS	(1 << 31)

/*
 * Wait for the ULPI Access to complete
 */
static int ulpi_wait(struct ulpi_viewport *ulpi_vp, u32 mask)
{
	int timeout = CONFIG_USB_ULPI_TIMEOUT;

	while (--timeout) {
		if ((readl(ulpi_vp->viewport_addr) & mask))
			return 0;

		udelay(1);
	}

	return ULPI_ERROR;
}

/*
 * Wake the ULPI PHY up for communication
 *
 * returns 0 on success.
 */
static int ulpi_wakeup(struct ulpi_viewport *ulpi_vp)
{
	int err;

	if (readl(ulpi_vp->viewport_addr) & OMAP_ULPI_ACCESS)
		return 0; /* already awake */

	writel(OMAP_ULPI_ACCESS, ulpi_vp->viewport_addr);

	err = ulpi_wait(ulpi_vp, OMAP_ULPI_ACCESS);
	if (err)
		debug("ULPI wakeup timed out\n");

	return err;
}

/*
 * Issue a ULPI read/write request
 */
static int ulpi_request(struct ulpi_viewport *ulpi_vp, u32 value)
{
	int err;

	err = ulpi_wakeup(ulpi_vp);
	if (err)
		return err;

	writel(value, ulpi_vp->viewport_addr);

	err = ulpi_wait(ulpi_vp, OMAP_ULPI_ACCESS);
	if (err)
		debug("ULPI request timed out\n");

	return err;
}

int ulpi_write(struct ulpi_viewport *ulpi_vp, u8 *reg, u32 value)
{
	u32 val = ((ulpi_vp->port_num & 0xf) << 24) |
			OMAP_ULPI_WR_OPSEL | ((u32)reg << 16) | (value & 0xff);

	return ulpi_request(ulpi_vp, val);
}

u32 ulpi_read(struct ulpi_viewport *ulpi_vp, u8 *reg)
{
	int err;
	u32 val = ((ulpi_vp->port_num & 0xf) << 24) |
			 OMAP_ULPI_WR_OPSEL | ((u32)reg << 16);

	err = ulpi_request(ulpi_vp, val);
	if (err)
		return err;

	return readl(ulpi_vp->viewport_addr) & 0xff;
}
