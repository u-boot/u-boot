/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 *	Jun-jie Zhang <b18070@freescale.com>
 *	Mingkai Hu <Mingkai.hu@freescale.com>
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
#ifndef __FSL_PHY_H__
#define __FSL_PHY_H__

#include <net.h>
#include <miiphy.h>
#include <asm/fsl_enet.h>

/* PHY register offsets */
#define PHY_EXT_PAGE_ACCESS	0x1f

/* MII Management Configuration Register */
#define MIIMCFG_RESET_MGMT          0x80000000
#define MIIMCFG_MGMT_CLOCK_SELECT   0x00000007
#define MIIMCFG_INIT_VALUE	    0x00000003

/* MII Management Command Register */
#define MIIMCOM_READ_CYCLE	0x00000001
#define MIIMCOM_SCAN_CYCLE	0x00000002

/* MII Management Address Register */
#define MIIMADD_PHY_ADDR_SHIFT	8

/* MII Management Indicator Register */
#define MIIMIND_BUSY		0x00000001
#define MIIMIND_NOTVALID	0x00000004

void tsec_local_mdio_write(struct tsec_mii_mng *phyregs, int port_addr,
		int dev_addr, int reg, int value);
int tsec_local_mdio_read(struct tsec_mii_mng *phyregs, int port_addr,
		int dev_addr, int regnum);
int tsec_phy_read(struct mii_dev *bus, int addr, int dev_addr, int regnum);
int tsec_phy_write(struct mii_dev *bus, int addr, int dev_addr, int regnum,
		u16 value);
int memac_mdio_write(struct mii_dev *bus, int port_addr, int dev_addr,
		int regnum, u16 value);
int memac_mdio_read(struct mii_dev *bus, int port_addr, int dev_addr,
		int regnum);

struct fsl_pq_mdio_info {
	struct tsec_mii_mng *regs;
	char *name;
};
int fsl_pq_mdio_init(bd_t *bis, struct fsl_pq_mdio_info *info);

#endif /* __FSL_PHY_H__ */
