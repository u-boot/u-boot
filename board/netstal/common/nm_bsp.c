/*
 *(C) Copyright 2005-2007 Netstal Maschinen AG
 *    Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_BSP)
/*
 * Command nm_bsp: Netstal Maschinen BSP specific command
 */
int nm_bsp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("%s: flag %d,  argc %d,  argv[0] %s\n",  __FUNCTION__,
	       flag,  argc,  argv[0]);
	printf("Netstal Maschinen BSP specific command. None at the moment.\n");
	return 0;
}

U_BOOT_CMD(
	  nm_bsp, 1,      1,      nm_bsp,
	  "nm_bsp  - Netstal Maschinen BSP specific command. \n",
	  "Help for Netstal Maschinen BSP specific command.\n"
	  );
#endif
