/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
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
 *
 */
#ifndef	_CMD_SCSI_H
#define	_CMD_SCSI_H

#include <common.h>
#include <command.h>


#if (CONFIG_COMMANDS & CFG_CMD_SCSI)

#define	CMD_TBL_SCSI	MK_CMD_TBL_ENTRY(					\
	"scsi",	4,	5,	1,	do_scsi,				\
	"scsi    - SCSI sub-system\n",						\
	"reset - reset SCSI controller\n"					\
	"scsi info  - show available SCSI devices\n"				\
	"scsi scan  - (re-)scan SCSI bus\n"					\
	"scsi device [dev] - show or set current device\n"			\
	"scsi part [dev] - print partition table of one or all SCSI devices\n"	\
	"scsi read addr blk# cnt - read `cnt' blocks starting at block `blk#'\n"\
	"     to memory address `addr'\n"					\
),


#define	CMD_TBL_SCSIBOOT	MK_CMD_TBL_ENTRY(					\
	"scsiboot",	5,	3,	1,	do_scsiboot,					\
	"scsiboot- boot from SCSI device\n",						\
	"loadAddr dev:part\n"			\
),

int do_scsi (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_scsiboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);


#else
#define CMD_TBL_SCSI
#define CMD_TBL_SCSIBOOT
#endif

#endif	/* _CMD_SCSI_H */

