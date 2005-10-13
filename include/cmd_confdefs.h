/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Definitions for Configuring the monitor commands
 */
#ifndef _CMD_CONFIG_H
#define _CMD_CONFIG_H

/*
 * Configurable monitor commands
 */
#define CFG_CMD_BDI		0x00000001ULL	/* bdinfo			*/
#define CFG_CMD_LOADS		0x00000002ULL	/* loads			*/
#define CFG_CMD_LOADB		0x00000004ULL	/* loadb			*/
#define CFG_CMD_IMI		0x00000008ULL	/* iminfo			*/
#define CFG_CMD_CACHE		0x00000010ULL	/* icache, dcache		*/
#define CFG_CMD_FLASH		0x00000020ULL	/* flinfo, erase, protect	*/
#define CFG_CMD_MEMORY		0x00000040ULL	/* md, mm, nm, mw, cp, cmp,	*/
						/* crc, base, loop, mtest	*/
#define CFG_CMD_NET		0x00000080ULL	/* bootp, tftpboot, rarpboot	*/
#define CFG_CMD_ENV		0x00000100ULL	/* saveenv			*/
#define CFG_CMD_KGDB		0x0000000000000200ULL	/* kgdb				*/
#define CFG_CMD_PCMCIA		0x00000400ULL	/* PCMCIA support		*/
#define CFG_CMD_IDE		0x00000800ULL	/* IDE harddisk support		*/
#define CFG_CMD_PCI		0x00001000ULL	/* pciinfo			*/
#define CFG_CMD_IRQ		0x00002000ULL	/* irqinfo			*/
#define CFG_CMD_BOOTD		0x00004000ULL	/* bootd			*/
#define CFG_CMD_CONSOLE		0x00008000ULL	/* coninfo			*/
#define CFG_CMD_EEPROM		0x00010000ULL	/* EEPROM read/write support	*/
#define CFG_CMD_ASKENV		0x00020000ULL	/* ask for env variable		*/
#define CFG_CMD_RUN		0x00040000ULL	/* run command in env variable	*/
#define CFG_CMD_ECHO		0x00080000ULL	/* echo arguments		*/
#define CFG_CMD_I2C		0x00100000ULL	/* I2C serial bus support	*/
#define CFG_CMD_REGINFO		0x00200000ULL	/* Register dump		*/
#define CFG_CMD_IMMAP		0x00400000ULL	/* IMMR dump support		*/
#define CFG_CMD_DATE		0x00800000ULL	/* support for RTC, date/time...*/
#define CFG_CMD_DHCP		0x01000000ULL	/* DHCP Support			*/
#define CFG_CMD_BEDBUG		0x02000000ULL	/* Include BedBug Debugger	*/
#define CFG_CMD_FDC		0x04000000ULL	/* Floppy Disk Support		*/
#define CFG_CMD_SCSI		0x08000000ULL	/* SCSI Support			*/
#define CFG_CMD_AUTOSCRIPT	0x10000000ULL	/* Autoscript Support		*/
#define CFG_CMD_MII		0x20000000ULL	/* MII support			*/
#define CFG_CMD_SETGETDCR	0x40000000ULL	/* DCR support on 4xx		*/
#define CFG_CMD_BSP		0x80000000ULL	/* Board Specific functions	*/

#define CFG_CMD_ELF	0x0000000100000000ULL	/* ELF (VxWorks) load/boot cmd	*/
#define CFG_CMD_MISC	0x0000000200000000ULL	/* Misc functions like sleep etc*/
#define CFG_CMD_USB	0x0000000400000000ULL	/* USB Support			*/
#define CFG_CMD_DOC	0x0000000800000000ULL	/* Disk-On-Chip Support		*/
#define CFG_CMD_JFFS2	0x0000001000000000ULL	/* JFFS2 Support		*/
#define CFG_CMD_DTT	0x0000002000000000ULL	/* Digital Therm and Thermostat */
#define CFG_CMD_SDRAM	0x0000004000000000ULL	/* SDRAM DIMM SPD info printout */
#define CFG_CMD_DIAG	0x0000008000000000ULL	/* Diagnostics			*/
#define CFG_CMD_FPGA	0x0000010000000000ULL	/* FPGA configuration Support	*/
#define CFG_CMD_HWFLOW	0x0000020000000000ULL	/* RTS/CTS hw flow control	*/
#define CFG_CMD_SAVES	0x0000040000000000ULL	/* save S record dump		*/
#define CFG_CMD_SPI	0x0000100000000000ULL	/* SPI utility			*/
#define CFG_CMD_FDOS	0x0000200000000000ULL	/* Floppy DOS support		*/
#define CFG_CMD_VFD	0x0000400000000000ULL	/* VFD support (TRAB)		*/
#define CFG_CMD_NAND	0x0000800000000000ULL	/* NAND support			*/
#define CFG_CMD_BMP	0x0001000000000000ULL	/* BMP support			*/
#define CFG_CMD_PORTIO	0x0002000000000000ULL	/* Port I/O			*/
#define CFG_CMD_PING	0x0004000000000000ULL	/* ping support			*/
#define CFG_CMD_MMC	0x0008000000000000ULL	/* MMC support			*/
#define CFG_CMD_FAT	0x0010000000000000ULL	/* FAT support			*/
#define CFG_CMD_IMLS	0x0020000000000000ULL	/* List all found images	*/
#define CFG_CMD_ITEST	0x0040000000000000ULL	/* Integer (and string) test	*/
#define CFG_CMD_NFS	0x0080000000000000ULL	/* NFS support			*/
#define CFG_CMD_REISER	0x0100000000000000ULL	/* Reiserfs support		*/
#define CFG_CMD_CDP	0x0200000000000000ULL	/* Cisco Discovery Protocol 	*/
#define CFG_CMD_XIMG	0x0400000000000000ULL	/* Load part of Multi Image	*/
#define CFG_CMD_UNIVERSE 0x0800000000000000ULL	/* Tundra Universe Support      */
#define CFG_CMD_EXT2	0x1000000000000000ULL	/* EXT2 Support			*/
#define CFG_CMD_SNTP	0x2000000000000000ULL	/* SNTP support			*/
#define CFG_CMD_DISPLAY	0x4000000000000000ULL	/* Display support		*/

#define CFG_CMD_ALL	0xFFFFFFFFFFFFFFFFULL	/* ALL commands			*/

/* Commands that are considered "non-standard" for some reason
 * (memory hogs, requires special hardware, not fully tested, etc.)
 */
#define CFG_CMD_NONSTD (CFG_CMD_ASKENV	| \
			CFG_CMD_BEDBUG	| \
			CFG_CMD_BMP	| \
			CFG_CMD_BSP	| \
			CFG_CMD_CACHE	| \
			CFG_CMD_CDP	| \
			CFG_CMD_DATE	| \
			CFG_CMD_DHCP	| \
			CFG_CMD_DIAG	| \
			CFG_CMD_DISPLAY	| \
			CFG_CMD_DOC	| \
			CFG_CMD_DTT	| \
			CFG_CMD_ECHO	| \
			CFG_CMD_EEPROM	| \
			CFG_CMD_ELF	| \
			CFG_CMD_EXT2	| \
			CFG_CMD_FDC	| \
			CFG_CMD_FAT	| \
			CFG_CMD_FDOS	| \
			CFG_CMD_HWFLOW	| \
			CFG_CMD_I2C	| \
			CFG_CMD_IDE	| \
			CFG_CMD_IMMAP	| \
			CFG_CMD_IRQ	| \
			CFG_CMD_JFFS2	| \
			CFG_CMD_KGDB	| \
			CFG_CMD_MII	| \
			CFG_CMD_MMC	| \
			CFG_CMD_NAND	| \
			CFG_CMD_PCI	| \
			CFG_CMD_PCMCIA	| \
			CFG_CMD_PING	| \
			CFG_CMD_PORTIO	| \
			CFG_CMD_REGINFO | \
			CFG_CMD_REISER	| \
			CFG_CMD_SAVES	| \
			CFG_CMD_SCSI	| \
			CFG_CMD_SDRAM	| \
			CFG_CMD_SNTP	| \
			CFG_CMD_SPI	| \
			CFG_CMD_UNIVERSE | \
			CFG_CMD_USB	| \
			CFG_CMD_VFD	)

/* Default configuration
 */
#define CONFIG_CMD_DFL	(CFG_CMD_ALL & ~CFG_CMD_NONSTD)

#ifndef CONFIG_COMMANDS
#define CONFIG_COMMANDS CONFIG_CMD_DFL
#endif


/*
 * optional BOOTP fields
 */

#define CONFIG_BOOTP_SUBNETMASK		0x00000001
#define CONFIG_BOOTP_GATEWAY		0x00000002
#define CONFIG_BOOTP_HOSTNAME		0x00000004
#define CONFIG_BOOTP_NISDOMAIN		0x00000008
#define CONFIG_BOOTP_BOOTPATH		0x00000010
#define CONFIG_BOOTP_BOOTFILESIZE	0x00000020
#define CONFIG_BOOTP_DNS		0x00000040
#define CONFIG_BOOTP_DNS2		0x00000080
#define CONFIG_BOOTP_SEND_HOSTNAME	0x00000100
#define CONFIG_BOOTP_NTPSERVER		0x00000200
#define CONFIG_BOOTP_TIMEOFFSET		0x00000400

#define CONFIG_BOOTP_VENDOREX		0x80000000

#define CONFIG_BOOTP_ALL		(~CONFIG_BOOTP_VENDOREX)


#define CONFIG_BOOTP_DEFAULT		(CONFIG_BOOTP_SUBNETMASK | \
					CONFIG_BOOTP_GATEWAY	 | \
					CONFIG_BOOTP_HOSTNAME	 | \
					CONFIG_BOOTP_BOOTPATH)

#ifndef CONFIG_BOOTP_MASK
#define CONFIG_BOOTP_MASK		CONFIG_BOOTP_DEFAULT
#endif

#endif	/* _CMD_CONFIG_H */
