/*
 * (C) Copyright 2008-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2009
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PPC4xx_CONFIG_H
#define __PPC4xx_CONFIG_H

#include <common.h>

struct ppc4xx_config {
	char label[16];
	char description[64];
	u8 val[CONFIG_4xx_CONFIG_BLOCKSIZE];
};

extern struct ppc4xx_config ppc4xx_config_val[];
extern int ppc4xx_config_count;

#endif /* __PPC4xx_CONFIG_H */
