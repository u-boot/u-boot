/*
 * Freescale i.MX23/i.MX28 Registers
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __IMX_REGS_H__
#define __IMX_REGS_H__

#include <asm/imx-common/regs-apbh.h>
#include <asm/arch/regs-base.h>
#include <asm/imx-common/regs-bch.h>
#include <asm/arch/regs-digctl.h>
#include <asm/imx-common/regs-gpmi.h>
#include <asm/arch/regs-i2c.h>
#include <asm/arch/regs-lcdif.h>
#include <asm/arch/regs-lradc.h>
#include <asm/arch/regs-ocotp.h>
#include <asm/arch/regs-pinctrl.h>
#include <asm/arch/regs-rtc.h>
#include <asm/arch/regs-ssp.h>
#include <asm/arch/regs-timrot.h>
#include <asm/arch/regs-usb.h>
#include <asm/arch/regs-usbphy.h>

#ifdef CONFIG_MX23
#include <asm/arch/regs-clkctrl-mx23.h>
#include <asm/arch/regs-power-mx23.h>
#endif

#ifdef CONFIG_MX28
#include <asm/arch/regs-clkctrl-mx28.h>
#include <asm/arch/regs-power-mx28.h>
#endif

#endif	/* __IMX_REGS_H__ */
