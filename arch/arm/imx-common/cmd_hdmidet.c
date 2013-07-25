/*
 * Copyright (C) 2012 Boundary Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
