/*
 * (C) Copyright 2001
 * Hans-Jörg Frieden, Hyperion Entertainment
 * Hans-JoergF@hyperion-entertainment.com
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
#ifndef _CMD_MENU_H
#define _CMD_MENU_H

#include <common.h>
#include <command.h>

#if defined(CONFIG_AMIGAONEG3SE) && (CONFIG_COMMANDS & CFG_CMD_BSP)
#define CMD_TBL_MENU     MK_CMD_TBL_ENTRY(                      \
        "menu", 3,   1,      1,      do_menu,                   \
        "menu    - display BIOS setup menu\n",                     \
        ""                                                      \
),

int do_menu( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] );
#else
#define CMD_TBL_MENU
#endif

#endif /* _CMD_MENU_H */
