/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
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

#ifndef _CMD_LOG_H_
#define _CMD_LOG_H_

#include <common.h>
#include <command.h>

#if defined(CONFIG_LOGBUFFER)

#define LOG_BUF_LEN 16843
#define LOG_BU_MASK ~(LOG_BUF_LEN-1)

#define	CMD_TBL_LOG	MK_CMD_TBL_ENTRY(			\
	"log",	3,     255,	1,	do_log,			\
	"log     - manipulate logbuffer\n",                     \
	"log info   - show pointer details\n"			\
	"log reset  - clear contents\n"				\
	"log show   - show contents\n"				\
	"log append <msg> - append <msg> to the logbuffer\n"	\
),
int do_log (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_LOG
#endif	/* CONFIG_LOGBUFFER */
/* ----------------------------------------------------------------------------*/
#endif	/* _CMD_LOG_H_ */
