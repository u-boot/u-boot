/*
 * (C) Copyright 2001
 * Thomas Frieden, Hyperion Entertainment
 * ThomasF@hyperion-entertainment.com
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
#ifndef _CMD_BOOTA_H
#define _CMD_BOOTA_H

#include <common.h>
#include <command.h>

#if defined(CONFIG_AMIGAONEG3SE) && (CONFIG_COMMANDS & CFG_CMD_BSP)
#define CMD_TBL_BOOTA     MK_CMD_TBL_ENTRY(                      \
        "boota", 5,   3,      1,      do_boota,                   \
        "boota   - boot an Amiga kernel\n",                     \
        "address disk"                                        \
),

int do_boota (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] );
#else
#define CMD_TBL_BOOTA
#endif

#endif /* _CMD_BOOTA_H */
