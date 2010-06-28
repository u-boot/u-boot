/*
 * (C) Copyright 2008
 * Niklaus Giger, niklaus.giger@member.fsf.org
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

#ifndef _VXWORKS_H_
#define _VXWORKS_H_

int do_bootvx(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/*
 * Use bootaddr to find the location in memory that VxWorks
 * will look for the bootline string. The default value for
 * PowerPC is LOCAL_MEM_LOCAL_ADRS + BOOT_LINE_OFFSET which
 * defaults to 0x4200
 */
#ifndef CONFIG_SYS_VXWORKS_BOOT_ADDR
#define CONFIG_SYS_VXWORKS_BOOT_ADDR 0x4200
#endif

#ifndef CONFIG_SYS_VXWORKS_BOOT_DEVICE
#if defined(CONFIG_4xx)
#define		CONFIG_SYS_VXWORKS_BOOT_DEVICE "emac(0,0)"
#elif defined(CONFIG_IOP480)
#define		CONFIG_SYS_VXWORKS_BOOT_DEVICE "dc(0,0)"
#else
#define		CONFIG_SYS_VXWORKS_BOOT_DEVICE "eth(0,0)"
#endif
#endif

#ifndef CONFIG_SYS_VXWORKS_SERVERNAME
#define CONFIG_SYS_VXWORKS_SERVERNAME	"srv"
#endif

#endif
