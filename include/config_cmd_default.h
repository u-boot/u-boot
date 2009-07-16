/*
 * Copyright 2007 Freescale Semiconductor, Inc.
 *
 * This file is licensed under the terms of the GNU General Public
 * License Version 2. This file is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _CONFIG_CMD_DEFAULT_H
#define _CONFIG_CMD_DEFAULT_H

/*
 * Alphabetical list of all commands that are configured by default.
 * This is essentially all commands minus those that are considered
 * "non-standard" for some reason (memory hogs, requires special
 * hardware, not fully tested, etc.).
 */

#define CONFIG_CMD_BDI		/* bdinfo			*/
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#define CONFIG_CMD_IMI		/* iminfo			*/
#define CONFIG_CMD_ITEST	/* Integer (and string) test	*/
#ifndef CONFIG_SYS_NO_FLASH
#define CONFIG_CMD_FLASH	/* flinfo, erase, protect	*/
#define CONFIG_CMD_IMLS		/* List all found images	*/
#endif
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#define CONFIG_CMD_NFS		/* NFS support			*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_XIMG		/* Load part of Multi Image	*/

#endif	/* _CONFIG_CMD_DEFAULT_H */
