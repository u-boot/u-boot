/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * MDIO bus access
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * CREDITS: tsec driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */

#include <config.h>
#include <common.h>
#include <miiphy.h>
#include <phy.h>
#include <malloc.h>
#include <asm/io.h>

#include "xilinx_ll_temac.h"
#include "xilinx_ll_temac_mdio.h"

#if !defined(CONFIG_MII)
# error "LL_TEMAC requires MII -- missing CONFIG_MII"
#endif

#if !defined(CONFIG_PHYLIB)
# error "LL_TEMAC requires PHYLIB -- missing CONFIG_PHYLIB"
#endif

/*
 * Prior to PHY access, the MDIO clock must be setup. This driver will set a
 * safe default that should work with PLB bus speeds of up to 150 MHz and keep
 * the MDIO clock below 2.5 MHz. If the user wishes faster access to the PHY
 * then the clock divisor can be set to a different value by setting the
 * correct bus speed value with CONFIG_XILINX_LL_TEMAC_CLK.
 */
#if !defined(CONFIG_XILINX_LL_TEMAC_CLK)
#define MDIO_CLOCK_DIV		MC_CLKDIV_10(150000000)
#else
#define MDIO_CLOCK_DIV		MC_CLKDIV_25(CONFIG_XILINX_LL_TEMAC_CLK)
#endif

static int ll_temac_mdio_setup(struct mii_dev *bus)
{
	struct temac_reg *regs = (struct temac_reg *)bus->priv;

	/* setup MDIO clock */
	ll_temac_indirect_set(regs, TEMAC_MC,
			MC_MDIOEN | (MDIO_CLOCK_DIV & MC_CLKDIV_MASK));

	return 0;
}

/*
 * Indirect MII PHY read via ll_temac.
 *
 * http://www.xilinx.com/support/documentation/ip_documentation/xps_ll_temac.pdf
 * page 67, Using the MII Management to Access PHY Registers
 */
int ll_temac_local_mdio_read(struct temac_reg *regs, int addr, int devad,
				int regnum)
{
	out_be32(&regs->lsw,
		((addr << LSW_PHYAD_POS) & LSW_PHYAD_MASK) |
		(regnum & LSW_REGAD_MASK));
	out_be32(&regs->ctl, TEMAC_MIIMAI);

	ll_temac_check_status(regs, RSE_MIIM_RR);

	return in_be32(&regs->lsw) & LSW_REGDAT_MASK;
}

/*
 * Indirect MII PHY write via ll_temac.
 *
 * http://www.xilinx.com/support/documentation/ip_documentation/xps_ll_temac.pdf
 * page 67, Using the MII Management to Access PHY Registers
 */
void ll_temac_local_mdio_write(struct temac_reg *regs, int addr, int devad,
				int regnum, u16 value)
{
	out_be32(&regs->lsw, (value & LSW_REGDAT_MASK));
	out_be32(&regs->ctl, CTL_WEN | TEMAC_MIIMWD);

	out_be32(&regs->lsw,
		((addr << LSW_PHYAD_POS) & LSW_PHYAD_MASK) |
		(regnum & LSW_REGAD_MASK));
	out_be32(&regs->ctl, CTL_WEN | TEMAC_MIIMAI);

	ll_temac_check_status(regs, RSE_MIIM_WR);
}

int ll_temac_phy_read(struct mii_dev *bus, int addr, int devad, int regnum)
{
	struct temac_reg *regs = (struct temac_reg *)bus->priv;

	return ll_temac_local_mdio_read(regs, addr, devad, regnum);
}

int ll_temac_phy_write(struct mii_dev *bus, int addr, int devad, int regnum,
			u16 value)
{
	struct temac_reg *regs = (struct temac_reg *)bus->priv;

	ll_temac_local_mdio_write(regs, addr, devad, regnum, value);

	return 0;
}

/*
 * Use MII register 1 (MII status register) to detect PHY
 *
 * A Mask used to verify certain PHY features (register content)
 * in the PHY detection register:
 *  Auto-negotiation support, 10Mbps half/full duplex support
 */
#define PHY_DETECT_REG		MII_BMSR
#define PHY_DETECT_MASK		(BMSR_10FULL | BMSR_10HALF | BMSR_ANEGCAPABLE)

/* Looking for a valid PHY address */
int ll_temac_phy_addr(struct mii_dev *bus)
{
	struct temac_reg *regs = (struct temac_reg *)bus->priv;
	unsigned short val;
	unsigned int phy;

	for (phy = PHY_MAX_ADDR; phy >= 0; phy--) {
		val = ll_temac_local_mdio_read(regs, phy, 0, PHY_DETECT_REG);
		if ((val != 0xFFFF) &&
		((val & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			return phy;
		}
	}

	return -1;
}

int xilinx_ll_temac_mdio_initialize(bd_t *bis, struct ll_temac_mdio_info *info)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate LL_TEMAC MDIO bus: %s\n",
				info->name);
		return -1;
	}

	bus->read = ll_temac_phy_read;
	bus->write = ll_temac_phy_write;
	bus->reset = NULL;

	/* use given name or generate its own unique name */
	if (info->name) {
		strncpy(bus->name, info->name, MDIO_NAME_LEN);
	} else {
		snprintf(bus->name, MDIO_NAME_LEN, "lltemii.%p", info->regs);
		info->name = bus->name;
	}

	bus->priv = info->regs;

	ll_temac_mdio_setup(bus);
	return mdio_register(bus);
}
