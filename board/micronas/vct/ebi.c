/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
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
#include <asm/io.h>
#include "vct.h"

int ebi_initialize(void)
{
#if defined(CONFIG_VCT_NOR)
	if (ebi_init_nor_flash())
		return -1;
#endif

#if defined(CONFIG_VCT_ONENAND)
	if (ebi_init_onenand())
		return -1;
#endif

#if defined(CONFIG_DRIVER_SMC911X)
	if (ebi_init_smc911x())
		return -1;
#endif

	reg_write(EBI_CTRL_SIG_ACTLV(EBI_BASE), 0x00004100);

	ebi_wait();

	return 0;
}
