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
#define CFG_CMD_BDI		0x00000001	/* bdinfo			*/
#define CFG_CMD_LOADS		0x00000002	/* loads			*/
#define CFG_CMD_LOADB		0x00000004	/* loadb			*/
#define CFG_CMD_IMI		0x00000008	/* iminfo			*/
#define CFG_CMD_CACHE		0x00000010	/* icache, dcache		*/
#define CFG_CMD_FLASH		0x00000020	/* flinfo, erase, protect	*/
#define CFG_CMD_MEMORY		0x00000040	/* md, mm, nm, mw, cp, cmp,	*/
						/* crc, base, loop, mtest	*/
#define CFG_CMD_NET		0x00000080	/* bootp, tftpboot, rarpboot	*/
#define CFG_CMD_ENV		0x00000100	/* saveenv			*/
#define CFG_CMD_KGDB		0x00000200	/* kgdb				*/
#define CFG_CMD_PCMCIA		0x00000400	/* PCMCIA support		*/
#define CFG_CMD_IDE		0x00000800	/* IDE harddisk support		*/
#define CFG_CMD_PCI		0x00001000	/* pciinfo			*/
#define CFG_CMD_IRQ		0x00002000	/* irqinfo			*/
#define CFG_CMD_BOOTD		0x00004000	/* bootd			*/
#define CFG_CMD_CONSOLE		0x00008000	/* coninfo			*/
#define CFG_CMD_EEPROM		0x00010000	/* EEPROM read/write support	*/
#define CFG_CMD_ASKENV		0x00020000	/* ask for env variable		*/
#define CFG_CMD_RUN		0x00040000	/* run command in env variable	*/
#define CFG_CMD_ECHO		0x00080000	/* echo arguments		*/
#define CFG_CMD_I2C		0x00100000	/* I2C serial bus support	*/
#define CFG_CMD_REGINFO		0x00200000	/* Register dump		*/
#define CFG_CMD_IMMAP		0x00400000	/* IMMR dump support		*/
#define CFG_CMD_DATE		0x00800000	/* support for RTC, date/time...*/
#define CFG_CMD_DHCP		0x01000000	/* DHCP Support			*/
#define CFG_CMD_BEDBUG		0x02000000	/* Include BedBug Debugger	*/
#define CFG_CMD_FDC		0x04000000	/* Floppy Disk Support		*/
#define CFG_CMD_SCSI		0x08000000	/* SCSI Support			*/
#define CFG_CMD_AUTOSCRIPT	0x10000000	/* Autoscript Support		*/
#define CFG_CMD_MII		0x20000000	/* MII support			*/
#define CFG_CMD_SETGETDCR	0x40000000	/* DCR support on 4xx		*/
#define CFG_CMD_BSP		0x80000000	/* Board Specific functions	*/

#define CFG_CMD_ELF	0x0000000100000000	/* ELF (VxWorks) load/boot cmd	*/
#define CFG_CMD_MISC	0x0000000200000000	/* Misc functions like sleep etc*/
#define CFG_CMD_USB	0x0000000400000000	/* USB Support			*/
#define CFG_CMD_DOC	0x0000000800000000	/* Disk-On-Chip Support		*/
#define CFG_CMD_JFFS2	0x0000001000000000	/* JFFS2 Support		*/
#define CFG_CMD_DTT	0x0000002000000000	/* Digital Therm and Thermostat */
#define CFG_CMD_SDRAM	0x0000004000000000	/* SDRAM DIMM SPD info printout */
#define CFG_CMD_DIAG	0x0000008000000000	/* Diagnostics			*/
#define CFG_CMD_FPGA	0x0000010000000000	/* FPGA configuration Support	*/
#define CFG_CMD_HWFLOW	0x0000020000000000	/* RTS/CTS hw flow control	*/
#define CFG_CMD_SAVES	0x0000040000000000	/* save S record dump		*/
#define CFG_CMD_SPI	0x0000100000000000	/* SPI utility			*/
#define CFG_CMD_FDOS	0x0000200000000000	/* Floppy DOS support		*/
#define CFG_CMD_VFD	0x0000400000000000	/* VFD support (TRAB)		*/

#define CFG_CMD_ALL	0xFFFFFFFFFFFFFFFF	/* ALL commands			*/

/* Commands that are considered "non-standard" for some reason
 * (memory hogs, requires special hardware, not fully tested, etc.)
 */
#define CFG_CMD_NONSTD (CFG_CMD_ASKENV	| \
			CFG_CMD_BEDBUG	| \
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
			CFG_CMD_FDOS	| \
			CFG_CMD_HWFLOW	| \
			CFG_CMD_I2C	| \
			CFG_CMD_IDE	| \
			CFG_CMD_IMMAP	| \
			CFG_CMD_IRQ	| \
			CFG_CMD_JFFS2	| \
			CFG_CMD_KGDB	| \
			CFG_CMD_MII	| \
			CFG_CMD_PCI	| \
			CFG_CMD_PCMCIA	| \
			CFG_CMD_REGINFO | \
			CFG_CMD_SAVES	| \
			CFG_CMD_SCSI	| \
			CFG_CMD_SDRAM	| \
			CFG_CMD_SPI	| \
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
