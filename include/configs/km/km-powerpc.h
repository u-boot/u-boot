/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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

#ifndef __CONFIG_KEYMILE_POWERPC_H
#define __CONFIG_KEYMILE_POWERPC_H

#define CONFIG_BOOTCOUNT_LIMIT

#define CONFIG_CMD_DTT
#define CONFIG_JFFS2_CMDLINE

/* EEprom support 24C08, 24C16, 24C64 */
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3  /* 8 Byte write page */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

#define CONFIG_ENV_SIZE		0x04000		/* Size of Environment */
#define CONFIG_FLASH_CFI_MTD

#define CONFIG_SYS_MEMTEST_START 0x00100000	/* memtest works on */

#define CONFIG_SYS_MEMTEST_END	0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */

/******************************************************************************
 * (PRAM usage)
 * ... -------------------------------------------------------
 * ... |ROOTFSSIZE | PNVRAM |PHRAM |RESERVED_PRAM | END_OF_RAM
 * ... |<------------------- pram -------------------------->|
 * ... -------------------------------------------------------
 * @END_OF_RAM:
 * @CONFIG_KM_RESERVED_PRAM: reserved pram for special purpose
 * @CONFIG_KM_PHRAM: address for /var
 * @CONFIG_KM_PNVRAM: address for PNVRAM (for the application)
 * @CONFIG_KM_ROOTFSSIZE: address for rootfilesystem in RAM
 */

/* size of rootfs in RAM */
#define CONFIG_KM_ROOTFSSIZE	0x0
/* pseudo-non volatile RAM [hex] */
#define CONFIG_KM_PNVRAM	0x80000
/* physical RAM MTD size [hex] */
#define CONFIG_KM_PHRAM		0x100000
/* resereved pram area at the end of memroy [hex] */
#define CONFIG_KM_RESERVED_PRAM	0x0
/* enable protected RAM */
#define CONFIG_PRAM		0

#define CONFIG_KM_CRAMFS_ADDR	0x800000
#define CONFIG_KM_KERNEL_ADDR	0x400000	/* 3968Kbytes */
#define CONFIG_KM_FDT_ADDR	0x7E0000	/* 128Kbytes */

/* architecture specific default bootargs */
#define CONFIG_KM_DEF_BOOT_ARGS_CPU		""

#define CONFIG_KM_DEF_ENV_CPU						\
	"boot=bootm ${load_addr_r} - ${fdt_addr_r}\0"			\
	"cramfsloadfdt="						\
		"cramfsload ${fdt_addr_r} "				\
		"fdt_0x${IVM_BoardId}_0x${IVM_HWKey}.dtb\0"		\
	"fdt_addr_r=" xstr(CONFIG_KM_FDT_ADDR) "\0"			\
	"u-boot="xstr(CONFIG_HOSTNAME) "/u-boot.bin\0"			\
	"update="							\
		"protect off " xstr(BOOTFLASH_START) " +${filesize} && "\
		"erase " xstr(BOOTFLASH_START) "  +${filesize} && "	\
		"cp.b ${load_addr_r} " xstr(BOOTFLASH_START)		\
		"  ${filesize} && "					\
		"protect on " xstr(BOOTFLASH_START) "  +${filesize}\0"  \
	""

#endif /* __CONFIG_KEYMILE_POWERPC_H */
