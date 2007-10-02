/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * DAVE Srl <www.dave-tech.it>
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

#ifndef _SDRAM_H_
#define _SDRAM_H_

#include <config.h>

#define ONE_BILLION	1000000000

struct sdram_conf_s {
	unsigned long size;
	int rows;
	unsigned long reg;
};

typedef struct sdram_conf_s sdram_conf_t;

/* Bitfields offsets */
#define SDRAM0_TR_CASL		(31 - 8)
#define SDRAM0_TR_PTA		(31 - 13)
#define SDRAM0_TR_CTP		(31 - 15)
#define SDRAM0_TR_LDF		(31 - 17)
#define SDRAM0_TR_RFTA		(31 - 29)
#define SDRAM0_TR_RCD		(31 - 31)

#ifdef CFG_SDRAM_CL
/* SDRAM timings [ns] according to AMCC/IBM names (see SDRAM_faq.doc) */
#define CFG_SDRAM_CASL		CFG_SDRAM_CL
#define CFG_SDRAM_PTA		CFG_SDRAM_tRP
#define CFG_SDRAM_CTP		(CFG_SDRAM_tRC - CFG_SDRAM_tRCD - CFG_SDRAM_tRP)
#define CFG_SDRAM_LDF		0
#ifdef CFG_SDRAM_tRFC
#define CFG_SDRAM_RFTA		CFG_SDRAM_tRFC
#else
#define CFG_SDRAM_RFTA		CFG_SDRAM_tRC
#endif
#define CFG_SDRAM_RCD		CFG_SDRAM_tRCD
#endif /* #ifdef CFG_SDRAM_CL */

/*
 * Some defines for the 440 DDR controller
 */
#define SDRAM_CFG0_DC_EN	0x80000000	/* SDRAM Controller Enable	*/
#define SDRAM_CFG0_MEMCHK	0x30000000	/* Memory data error checking mask*/
#define SDRAM_CFG0_MEMCHK_NON	0x00000000	/* No ECC generation		*/
#define SDRAM_CFG0_MEMCHK_GEN	0x20000000	/* ECC generation		*/
#define SDRAM_CFG0_MEMCHK_CHK	0x30000000	/* ECC generation and checking	*/
#define SDRAM_CFG0_DRAMWDTH	0x02000000	/* DRAM width mask		*/
#define SDRAM_CFG0_DRAMWDTH_32	0x00000000	/* 32 bits			*/
#define SDRAM_CFG0_DRAMWDTH_64	0x02000000	/* 64 bits			*/

#endif
