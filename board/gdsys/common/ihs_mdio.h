/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _IHS_MDIO_H_
#define _IHS_MDIO_H_

struct ihs_mdio_info {
	u32 fpga;
	char *name;
};

int ihs_mdio_init(struct ihs_mdio_info *info);

#endif
