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

#define CONFIG_CMD_AMBAPP	/* AMBA Plug & Play Bus print utility */
#define CONFIG_CMD_ASKENV	/* ask for env variable		*/
#define CONFIG_CMD_BEDBUG	/* Include BedBug Debugger	*/
#define CONFIG_CMD_BMP		/* BMP support			*/
#define CONFIG_CMD_BOOTZ	/* boot zImage			*/
#define CONFIG_CMD_BSP		/* Board Specific functions	*/
#define CONFIG_CMD_CACHE	/* icache, dcache		*/
#define CONFIG_CMD_CDP		/* Cisco Discovery Protocol	*/
#define CONFIG_CMD_CLK		/* Clock support		*/
#define CONFIG_CMD_DATE		/* support for RTC, date/time...*/
#define CONFIG_CMD_DHCP		/* DHCP Support			*/
#define CONFIG_CMD_DIAG		/* Diagnostics			*/
#define CONFIG_CMD_DISPLAY	/* Display support		*/
#define CONFIG_CMD_DOC		/* Disk-On-Chip Support		*/
#define CONFIG_CMD_DTT		/* Digital Therm and Thermostat */
#define CONFIG_CMD_EEPROM	/* EEPROM read/write support	*/
#define CONFIG_CMD_ELF		/* ELF (VxWorks) load/boot cmd	*/
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_FDC		/* Floppy Disk Support		*/
#define CONFIG_CMD_FUSE		/* Device fuse support		*/
#define CONFIG_CMD_GETTIME	/* Get time since boot         */
#define CONFIG_CMD_HASH		/* calculate hash / digest	*/
#define CONFIG_CMD_HWFLOW	/* RTS/CTS hw flow control	*/
#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_IDE		/* IDE harddisk support		*/
#define CONFIG_CMD_IMMAP	/* IMMR dump support		*/
#define CONFIG_CMD_IO		/* Access to X86 IO space	*/
#define CONFIG_CMD_IRQ		/* irqinfo			*/
#define CONFIG_CMD_JFFS2	/* JFFS2 Support		*/
#define CONFIG_CMD_KGDB		/* kgdb				*/
#define CONFIG_CMD_LICENSE	/* console license display	*/
#define CONFIG_CMD_MEMINFO	/* meminfo			*/
#define CONFIG_CMD_MEMTEST	/* mtest			*/
#define CONFIG_CMD_MFSL		/* FSL support for Microblaze	*/
#define CONFIG_CMD_MII		/* MII support			*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_MTDPARTS	/* mtd parts support		*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_ONENAND	/* OneNAND support		*/
#define CONFIG_CMD_PCI		/* pciinfo			*/
#define CONFIG_CMD_PCMCIA	/* PCMCIA support		*/
#define CONFIG_CMD_PING		/* ping support			*/
#define CONFIG_CMD_PORTIO	/* Port I/O			*/
#define CONFIG_CMD_REGINFO	/* Register dump		*/
#define CONFIG_CMD_REISER	/* Reiserfs support		*/
#define CONFIG_CMD_RARP		/* rarpboot support		*/
#define CONFIG_CMD_READ		/* Read data from partition	*/
#define CONFIG_CMD_SANDBOX	/* sb command to access sandbox features */
#define CONFIG_CMD_SAVES	/* save S record dump		*/
#define CONFIG_CMD_SCSI		/* SCSI Support			*/
#define CONFIG_CMD_SDRAM	/* SDRAM DIMM SPD info printout */
#define CONFIG_CMD_SNTP		/* SNTP support			*/
#define CONFIG_CMD_SPI		/* SPI utility			*/
#define CONFIG_CMD_TERMINAL	/* built-in Serial Terminal	*/
#define CONFIG_CMD_UBI		/* UBI Support			*/
#define CONFIG_CMD_UBIFS	/* UBIFS Support		*/
#define CONFIG_CMD_UNIVERSE	/* Tundra Universe Support	*/
#define CONFIG_CMD_UNZIP	/* unzip from memory to memory	*/
#define CONFIG_CMD_USB		/* USB Support			*/
#define CONFIG_CMD_ZFS		/* ZFS Support			*/
#define CONFIG_CMD_ZYNQ_AES	/* AES support for Zynq		*/
#define CONFIG_CMD_ZYNQ_RSA	/* RSA-AES support for Zynq	*/

#endif	/* _CONFIG_CMD_ALL_H */
