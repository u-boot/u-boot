/*
 * Power and Sleep Controller (PSC) functions.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 * Copyright (C) 2004 Texas Instruments.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <asm/arch/hardware.h>

/*
 * The PSC manages three inputs to a "module" which may be a peripheral or
 * CPU.  Those inputs are the module's:  clock; reset signal; and sometimes
 * its power domain.  For our purposes, we only care whether clock and power
 * are active, and the module is out of reset.
 *
 * DaVinci chips may include two separate power domains: "Always On" and "DSP".
 * Chips without a DSP generally have only one domain.
 *
 * The "Always On" power domain is always on when the chip is on, and is
 * powered by the VDD pins (on DM644X). The majority of DaVinci modules
 * lie within the "Always On" power domain.
 *
 * A separate domain called the "DSP" domain houses the C64x+ and other video
 * hardware such as VICP. In some chips, the "DSP" domain is not always on.
 * The "DSP" power domain is powered by the CVDDDSP pins (on DM644X).
 */

/* Works on Always On power domain only (no PD argument) */
void lpsc_on(unsigned int id)
{
	dv_reg_p mdstat, mdctl;

	if (id >= DAVINCI_LPSC_GEM)
		return;			/* Don't work on DSP Power Domain */

	mdstat = REG_P(PSC_MDSTAT_BASE + (id * 4));
	mdctl = REG_P(PSC_MDCTL_BASE + (id * 4));

	while (REG(PSC_PTSTAT) & 0x01)
		continue;

	if ((*mdstat & 0x1f) == 0x03)
		return;			/* Already on and enabled */

	*mdctl |= 0x03;

	switch (id) {
#ifdef CONFIG_SOC_DM644X
	/* Special treatment for some modules as for sprue14 p.7.4.2 */
	case DAVINCI_LPSC_VPSSSLV:
	case DAVINCI_LPSC_EMAC:
	case DAVINCI_LPSC_EMAC_WRAPPER:
	case DAVINCI_LPSC_MDIO:
	case DAVINCI_LPSC_USB:
	case DAVINCI_LPSC_ATA:
	case DAVINCI_LPSC_VLYNQ:
	case DAVINCI_LPSC_UHPI:
	case DAVINCI_LPSC_DDR_EMIF:
	case DAVINCI_LPSC_AEMIF:
	case DAVINCI_LPSC_MMC_SD:
	case DAVINCI_LPSC_MEMSTICK:
	case DAVINCI_LPSC_McBSP:
	case DAVINCI_LPSC_GPIO:
		*mdctl |= 0x200;
		break;
#endif
	}

	REG(PSC_PTCMD) = 0x01;

	while (REG(PSC_PTSTAT) & 0x03)
		continue;
	while ((*mdstat & 0x1f) != 0x03)	/* Probably an overkill... */
		continue;
}

/* Not all DaVinci chips have a DSP power domain. */
#ifdef CONFIG_SOC_DM644X

/* If DSPLINK is used, we don't want U-Boot to power on the DSP. */
#if !defined(CONFIG_SYS_USE_DSPLINK)
void dsp_on(void)
{
	int i;

	if (REG(PSC_PDSTAT1) & 0x1f)
		return;			/* Already on */

	REG(PSC_GBLCTL) |= 0x01;
	REG(PSC_PDCTL1) |= 0x01;
	REG(PSC_PDCTL1) &= ~0x100;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) &= 0xfffffeff;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) &= 0xfffffeff;
	REG(PSC_PTCMD) = 0x02;

	for (i = 0; i < 100; i++) {
		if (REG(PSC_EPCPR) & 0x02)
			break;
	}

	REG(PSC_CHP_SHRTSW) = 0x01;
	REG(PSC_PDCTL1) |= 0x100;
	REG(PSC_EPCCR) = 0x02;

	for (i = 0; i < 100; i++) {
		if (!(REG(PSC_PTSTAT) & 0x02))
			break;
	}

	REG(PSC_GBLCTL) &= ~0x1f;
}
#endif /* CONFIG_SYS_USE_DSPLINK */

#endif /* have a DSP */
