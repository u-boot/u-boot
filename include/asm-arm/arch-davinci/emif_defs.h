/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
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
#ifndef _EMIF_DEFS_H_
#define _EMIF_DEFS_H_

#include <asm/arch/hardware.h>

typedef struct {
	dv_reg		ERCSR;
	dv_reg		AWCCR;
	dv_reg		SDBCR;
	dv_reg		SDRCR;
	dv_reg		AB1CR;
	dv_reg		AB2CR;
	dv_reg		AB3CR;
	dv_reg		AB4CR;
	dv_reg		SDTIMR;
	dv_reg		DDRSR;
	dv_reg		DDRPHYCR;
	dv_reg		DDRPHYSR;
	dv_reg		TOTAR;
	dv_reg		TOTACTR;
	dv_reg		DDRPHYID_REV;
	dv_reg		SDSRETR;
	dv_reg		EIRR;
	dv_reg		EIMR;
	dv_reg		EIMSR;
	dv_reg		EIMCR;
	dv_reg		IOCTRLR;
	dv_reg		IOSTATR;
	u_int8_t	RSVD0[8];
	dv_reg		NANDFCR;
	dv_reg		NANDFSR;
	u_int8_t	RSVD1[8];
	dv_reg		NANDF1ECC;
	dv_reg		NANDF2ECC;
	dv_reg		NANDF3ECC;
	dv_reg		NANDF4ECC;
} emif_registers;

typedef emif_registers	*emifregs;
#endif
