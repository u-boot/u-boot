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
 *  Definitions for Configuring the monitor commands
 */
#ifndef _CMD_CONFIG_H
#define _CMD_CONFIG_H

/*
 * Configurable monitor commands
 */
#define CFG_CMD_BDI		0x00000001U	/* bdinfo			*/
#define CFG_CMD_LOADS		0x00000002U	/* loads			*/
#define CFG_CMD_LOADB		0x00000004U	/* loadb			*/
#define CFG_CMD_IMI		0x00000008U	/* iminfo			*/
#define CFG_CMD_CACHE		0x00000010U	/* icache, dcache		*/
#define CFG_CMD_FLASH		0x00000020U	/* flinfo, erase, protect	*/
#define CFG_CMD_MEMORY		0x00000040U	/* md, mm, nm, mw, cp, cmp,	*/
						/* crc, base, loop, mtest	*/
#define CFG_CMD_NET		0x00000080U	/* bootp, tftpboot, rarpboot	*/
#define CFG_CMD_ENV		0x00000100U	/* saveenv			*/
#define CFG_CMD_KGDB		0x00000200U	/* kgdb				*/
#define CFG_CMD_PCMCIA		0x00000400U	/* PCMCIA support		*/
#define CFG_CMD_IDE		0x00000800U	/* IDE harddisk support		*/
#define CFG_CMD_PCI		0x00001000U	/* pciinfo			*/
#define CFG_CMD_IRQ		0x00002000U	/* irqinfo			*/
#define CFG_CMD_BOOTD		0x00004000U	/* bootd			*/
#define CFG_CMD_CONSOLE		0x00008000U	/* coninfo			*/
#define CFG_CMD_EEPROM		0x00010000U	/* EEPROM read/write support	*/
#define CFG_CMD_ASKENV		0x00020000U	/* ask for env variable		*/
#define CFG_CMD_RUN		0x00040000U	/* run command in env variable	*/
#define CFG_CMD_ECHO		0x00080000U	/* echo arguments		*/
#define CFG_CMD_I2C		0x00100000U	/* I2C serial bus support	*/
#define CFG_CMD_REGINFO		0x00200000U	/* Register dump		*/
#define CFG_CMD_IMMAP		0x00400000U	/* IMMR dump support		*/
#define CFG_CMD_DATE		0x00800000U	/* support for RTC, date/time...*/
#define CFG_CMD_DHCP		0x01000000U	/* DHCP Support			*/
#define CFG_CMD_BEDBUG		0x02000000U	/* Include BedBug Debugger	*/
#define CFG_CMD_FDC		0x04000000U	/* Floppy Disk Support		*/
#define CFG_CMD_SCSI		0x08000000U	/* SCSI Support			*/
#define CFG_CMD_AUTOSCRIPT	0x10000000U	/* Autoscript Support		*/
#define CFG_CMD_MII		0x20000000U	/* MII support			*/
#define CFG_CMD_SETGETDCR	0x40000000U	/* DCR support on 4xx		*/
#define CFG_CMD_BSP		0x80000000U	/* Board Specific functions	*/

#define CFG_CMD_ELF	0x0000000100000000U	/* ELF (VxWorks) load/boot cmd	*/
#define CFG_CMD_MISC	0x0000000200000000U	/* Misc functions like sleep etc*/
#define CFG_CMD_USB	0x0000000400000000U	/* USB Support			*/
#define CFG_CMD_DOC	0x0000000800000000U	/* Disk-On-Chip Support		*/
#define CFG_CMD_JFFS2	0x0000001000000000U	/* JFFS2 Support		*/
#define CFG_CMD_DTT	0x0000002000000000U	/* Digital Therm and Thermostat */
#define CFG_CMD_SDRAM	0x0000004000000000U	/* SDRAM DIMM SPD info printout */
#define CFG_CMD_DIAG	0x0000008000000000U	/* Diagnostics			*/
#define CFG_CMD_FPGA	0x0000010000000000U	/* FPGA configuration Support	*/
#define CFG_CMD_HWFLOW	0x0000020000000000U	/* RTS/CTS hw flow control	*/
#define CFG_CMD_SAVES	0x0000040000000000U	/* save S record dump		*/
#define CFG_CMD_SPI	0x0000100000000000U	/* SPI utility			*/
#define CFG_CMD_FDOS	0x0000200000000000U	/* Floppy DOS support		*/
#define CFG_CMD_VFD	0x0000400000000000U	/* VFD support (TRAB)		*/
#define CFG_CMD_NAND	0x0000800000000000U	/* NAND support        		*/
#define CFG_CMD_BMP	0x0001000000000000U	/* BMP support			*/
#define CFG_CMD_PORTIO	0x0002000000000000U	/* Port I/O		        */
#define CFG_CMD_PING	0x0004000000000000U	/* ping support			*/
#define CFG_CMD_MMC	0x0008000000000000U	/* MMC support			*/
#define CFG_CMD_FAT	0x0010000000000000U	/* FAT support			*/
#define CFG_CMD_IMLS	0x0020000000000000U	/* List all found images        */
#define CFG_CMD_ITEST	0x0040000000000000U	/* Integer (and string) test	*/
#define CFG_CMD_NFS	0x0080000000000000U	/* NFS support			*/
#define CFG_CMD_REISER  0x0100000000000000U     /* Reiserfs support		*/
#define CFG_CMD_CDP	0x0200000000000000U	/* Cisco Discovery Protocol 	*/

#define CFG_CMD_ALL	0xFFFFFFFFFFFFFFFFU	/* ALL commands			*/

/* Commands that are considered "non-standard" for some reason
 * (memory hogs, requires special hardware, not fully tested, etc.)
 */
#define CFG_CMD_NONSTD (CFG_CMD_ASKENV	| \
			CFG_CMD_BEDBUG	| \
			CFG_CMD_BMP	| \
			CFG_CMD_BSP	| \
			CFG_CMD_CACHE	| \
			CFG_CMD_DATE	| \
			CFG_CMD_DHCP	| \
			CFG_CMD_DIAG	| \
			CFG_CMD_DOC	| \
			CFG_CMD_DTT	| \
			CFG_CMD_ECHO	| \
			CFG_CMD_EEPROM	| \
			CFG_CMD_ELF	| \
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
			CFG_CMD_SPI	| \
			CFG_CMD_USB	| \
			CFG_CMD_VFD	| \
			CFG_CMD_CDP 	)

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
#define CONFIG_BOOTP_SEND_HOSTNAME      0x00000100

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
