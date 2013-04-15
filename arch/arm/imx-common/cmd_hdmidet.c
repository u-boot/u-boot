/*
 * Copyright (C) 2012 Boundary Devices Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/io.h>

static int do_hdmidet(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct hdmi_regs *hdmi	= (struct hdmi_regs *)HDMI_ARB_BASE_ADDR;
	u8 reg = readb(&hdmi->phy_stat0) & HDMI_PHY_HPD;
	return (reg&HDMI_PHY_HPD) ? 0 : 1;
}

U_BOOT_CMD(hdmidet, 1, 1, do_hdmidet,
	"detect HDMI monitor",
	""
);
