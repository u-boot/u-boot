/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * MDIO bus access interface
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */
#ifndef _XILINX_LL_TEMAC_MDIO_
#define _XILINX_LL_TEMAC_MDIO_

#include <net.h>
#include <miiphy.h>

#include <asm/types.h>
#include <asm/byteorder.h>

#include "xilinx_ll_temac.h"

int ll_temac_local_mdio_read(struct temac_reg *regs, int addr, int devad,
				int regnum);
void ll_temac_local_mdio_write(struct temac_reg *regs, int addr, int devad,
				int regnum, u16 value);

int ll_temac_phy_read(struct mii_dev *bus, int addr, int devad, int regnum);
int ll_temac_phy_write(struct mii_dev *bus, int addr, int devad, int regnum,
			u16 value);

int ll_temac_phy_addr(struct mii_dev *bus);

struct ll_temac_mdio_info {
	struct temac_reg *regs;
	char *name;
};

int xilinx_ll_temac_mdio_initialize(bd_t *bis, struct ll_temac_mdio_info *info);

#endif /* _XILINX_LL_TEMAC_MDIO_ */
