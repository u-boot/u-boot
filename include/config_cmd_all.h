/*
 * Copyright 2007 Freescale Semiconductor, Inc.
 *
 * This file is licensed under the terms of the GNU General Public
 * License Version 2. This file is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _CONFIG_CMD_ALL_H
#define _CONFIG_CMD_ALL_H

/*
 * Alphabetical list of all possible commands.
 */

#define CONFIG_CMD_MFSL		/* FSL support for Microblaze	*/
#define CONFIG_CMD_MTDPARTS	/* mtd parts support		*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_ONENAND	/* OneNAND support		*/
#define CONFIG_CMD_PCI		/* pciinfo			*/
#define CONFIG_CMD_PCMCIA	/* PCMCIA support		*/
#define CONFIG_CMD_PORTIO	/* Port I/O			*/
#define CONFIG_CMD_REGINFO	/* Register dump		*/
#define CONFIG_CMD_REISER	/* Reiserfs support		*/
#define CONFIG_CMD_READ		/* Read data from partition	*/
#define CONFIG_CMD_SANDBOX	/* sb command to access sandbox features */
#define CONFIG_CMD_SAVES	/* save S record dump		*/
#define CONFIG_SCSI		/* SCSI Support			*/
#define CONFIG_CMD_SDRAM	/* SDRAM DIMM SPD info printout */
#define CONFIG_CMD_TERMINAL	/* built-in Serial Terminal	*/
#define CONFIG_CMD_UBIFS	/* UBIFS Support		*/
#define CONFIG_CMD_UNIVERSE	/* Tundra Universe Support	*/
#define CONFIG_CMD_ZFS		/* ZFS Support			*/

#endif	/* _CONFIG_CMD_ALL_H */
