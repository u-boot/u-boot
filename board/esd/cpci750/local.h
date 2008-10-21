/*
 * (C) Copyright 2003
 * Ingo Assmus <ingo.assmus@keymile.com>
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

/*
 * include/local.h - local configuration options, board specific
 */

#ifndef __LOCAL_H
#define __LOCAL_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

/* This tells PPCBoot that the config options are compiled in */
/* #undef ENV_IS_EMBEDDED */
/* Don't touch this! PPCBOOT figures this out  based on other
 * magic. */

/* Uncomment and define any of the below options */

/* #define CONFIG_750CX */ /* The 750CX doesn't support as many things in L2CR */
#define CONFIG_750FX       /* The 750FX doesn't support as many things in L2CR like 750CX*/

/* These want string arguments */
/* #define CONFIG_BOOTARGS */
/* #define CONFIG_BOOTCOMMAND */
/* #define CONFIG_RAMBOOTCOMMAND */
/* #define CONFIG_NFSBOOTCOMMAND */
/* #define CONFIG_SYS_AUTOLOAD */
/* #define CONFIG_PREBOOT */

/* These don't */

/* #define CONFIG_BOOTDELAY  */
/* #define CONFIG_BAUDRATE */
/* #define CONFIG_LOADS_ECHO */
/* #define CONFIG_ETHADDR */
/* #define CONFIG_ETH2ADDR */
/* #define CONFIG_ETH3ADDR */
/* #define CONFIG_IPADDR */
/* #define CONFIG_SERVERIP */
/* #define CONFIG_ROOTPATH */
/* #define CONFIG_GATEWAYIP */
/* #define CONFIG_NETMASK */
/* #define CONFIG_HOSTNAME */
/* #define CONFIG_BOOTFILE */
/* #define CONFIG_LOADADDR */

/* these hardware addresses are pretty bogus, please change them to
   suit your needs */

/* first ethernet */
/* #define CONFIG_ETHADDR          86:06:2d:7e:c6:53 */
#define CONFIG_ETHADDR          64:36:00:00:00:01

/* next two ethernet hwaddrs */
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		86:06:2d:7e:c6:54
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR		86:06:2d:7e:c6:55

#define CONFIG_ENV_OVERWRITE
#endif	/* __CONFIG_H */
