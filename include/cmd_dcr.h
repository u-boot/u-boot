/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * IBM 4XX DCR Functions
 */
#ifndef _CMD_DCR_H
#define _CMD_DCR_H

#if defined(CONFIG_4xx) && defined(CFG_CMD_SETGETDCR)
#define CMD_TBL_GETDCR	MK_CMD_TBL_ENTRY(				\
	"getdcr",	6,	2,	1,	do_getdcr,		\
	"getdcr  - Get an IBM PPC 4xx DCR's value\n",			\
	"dcrn - return a DCR's value.\n"				\
),
#define CMD_TBL_SETDCR	MK_CMD_TBL_ENTRY(				\
	"setdcr",	6,	2,	1,	do_setdcr,		\
	"setdcr  - Set an IBM PPC 4xx DCR's value\n",			\
	"dcrn - set a DCR's value.\n"					\
),
extern int do_getdcr (cmd_tbl_t *, int, int, char *[]);
extern int do_setdcr (cmd_tbl_t *, int, int, char *[]);

/* Supporting routines */
extern unsigned long get_dcr(unsigned short dcrn);
extern unsigned long set_dcr(unsigned short dcrn, unsigned long value);

#else

#define CMD_TBL_GETDCR
#define CMD_TBL_SETDCR

#endif /* CONFIG_4xx & CFG_CMD_SETGETDCR */

#endif	/* _CMD_DCR_H */
