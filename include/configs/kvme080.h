/*
 * (C) Copyright 2005
 * Sangmoon Kim, dogoil@etinsys.com.
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

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MPC824X		1
#define CONFIG_MPC8245		1
#define CONFIG_KVME080		1

#define CONFIG_CONS_INDEX	1

#define CONFIG_BAUDRATE		115200

#define CONFIG_BOOTDELAY	5

#define CONFIG_IPADDR		192.168.0.2
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_SERVERIP		192.168.0.1

#define CONFIG_BOOTARGS \
	"console=ttyS0,115200 " \
	"root=/dev/nfs rw nfsroot=192.168.0.1:/opt/eldk/ppc_82xx " \
	"ip=192.168.0.2:192.168.0.1:192.168.0.1:255.255.255.0:" \
	"kvme080:eth0:none " \
	"mtdparts=phys_mapped_flash:12m(root),-(kernel)"

#define CONFIG_BOOTCOMMAND \
	"tftp 800000 kvme080/uImage; " \
	"bootm 800000"

#define CONFIG_LOADADDR		800000

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_MISC_INIT_R

#define CONFIG_LOADS_ECHO	1
#undef	CFG_LOADS_BAUD_CHANGE

#undef	CONFIG_WATCHDOG

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define CONFIG_RTC_DS164x

#define CONFIG_COMMANDS		( CONFIG_CMD_DFL	| \
				  CFG_CMD_ASKENV	| \
				  CFG_CMD_CACHE		| \
				  CFG_CMD_DATE		| \
				  CFG_CMD_DHCP		| \
				  CFG_CMD_DIAG		| \
				  CFG_CMD_EEPROM	| \
				  CFG_CMD_ELF		| \
				  CFG_CMD_I2C		| \
				  CFG_CMD_JFFS2		| \
				  CFG_CMD_NFS		| \
				  CFG_CMD_PCI		| \
				  CFG_CMD_PING		| \
				  CFG_CMD_SDRAM		| \
				  CFG_CMD_SNTP)

#define CONFIG_NETCONSOLE

#include <cmd_confdefs.h>

#define CFG_LONGHELP
#define CFG_PROMPT		"=> "
#define CFG_CBSIZE		256
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS		16
#define CFG_BARGSIZE		CFG_CBSIZE

#define CFG_MEMTEST_START	0x00400000
#define CFG_MEMTEST_END		0x07C00000

#define CFG_LOAD_ADDR		0x00100000
#define CFG_HZ			1000

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_INIT_RAM_ADDR	0x40000000
#define CFG_INIT_RAM_END	0x1000
#define CFG_GBL_DATA_SIZE	128
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x7C000000
#define CFG_EUMB_ADDR		0xFC000000
#define CFG_NVRAM_BASE_ADDR	0xFF000000
#define CFG_NS16550_COM1	0xFF080000
#define CFG_NS16550_COM2	0xFF080010
#define CFG_NS16550_COM3	0xFF080020
#define CFG_NS16550_COM4	0xFF080030
#define CFG_RESET_ADDRESS	0xFFF00100

#define CFG_MAX_RAM_SIZE	0x20000000
#define CFG_FLASH_SIZE		(16 * 1024 * 1024)
#define CFG_NVRAM_SIZE		0x7FFF8

#define CONFIG_VERY_BIG_RAM

#define CFG_MONITOR_LEN		0x00040000
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MALLOC_LEN		(512 << 10)

#define CFG_BOOTMAPSZ		(8 << 20)

#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_USE_BUFFER_WRITE
#define CFG_FLASH_PROTECTION
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_PROTECT_CLEAR

#define CFG_MAX_FLASH_BANKS	1
#define CFG_MAX_FLASH_SECT	256

#define CFG_FLASH_ERASE_TOUT	120000
#define CFG_FLASH_WRITE_TOUT	500

#define CFG_JFFS2_FIRST_BANK	0
#define CFG_JFFS2_NUM_BANKS	1

#define CFG_ENV_IS_IN_NVRAM	1
#define CONFIG_ENV_OVERWRITE	1
#define CFG_NVRAM_ACCESS_ROUTINE
#define CFG_ENV_ADDR		CFG_NVRAM_BASE_ADDR
#define CFG_ENV_SIZE		0x400
#define CFG_ENV_OFFSET		0

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		14745600

#define CONFIG_PCI
#define CONFIG_PCI_PNP

#define CONFIG_NET_MULTI
#define CONFIG_EEPRO100
#define CONFIG_EEPRO100_SROM_WRITE

#define CFG_RX_ETH_BUFFER	8

#define CONFIG_HARD_I2C		1
#define CFG_I2C_SPEED		400000
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR		0x57
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10

#define CONFIG_SYS_CLK_FREQ	33333333

#define CFG_CACHELINE_SIZE	32
#if CONFIG_COMMANDS & CFG_CMD_KGDB
#  define CFG_CACHELINE_SHIFT	5
#endif

#define CFG_DLL_EXTEND		0x00
#define CFG_PCI_HOLD_DEL	0x20

#define CFG_ROMNAL		15
#define CFG_ROMFAL		31

#define CFG_REFINT		430

#define CFG_DBUS_SIZE2		1

#define CFG_BSTOPRE		121
#define CFG_REFREC		8
#define CFG_RDLAT		4
#define CFG_PRETOACT		3
#define CFG_ACTTOPRE		5
#define CFG_ACTORW		3
#define CFG_SDMODE_CAS_LAT	3
#define CFG_SDMODE_WRAP		0

#define CFG_REGISTERD_TYPE_BUFFER	1
#define CFG_EXTROM			1
#define CFG_REGDIMM			0

#define CFG_BANK0_START		0x00000000
#define CFG_BANK0_END		(0x4000000 - 1)
#define CFG_BANK0_ENABLE	1
#define CFG_BANK1_START		0x04000000
#define CFG_BANK1_END		(0x8000000 - 1)
#define CFG_BANK1_ENABLE	1
#define CFG_BANK2_START		0x3ff00000
#define CFG_BANK2_END		0x3fffffff
#define CFG_BANK2_ENABLE	0
#define CFG_BANK3_START		0x3ff00000
#define CFG_BANK3_END		0x3fffffff
#define CFG_BANK3_ENABLE	0
#define CFG_BANK4_START		0x00000000
#define CFG_BANK4_END		0x00000000
#define CFG_BANK4_ENABLE	0
#define CFG_BANK5_START		0x00000000
#define CFG_BANK5_END		0x00000000
#define CFG_BANK5_ENABLE	0
#define CFG_BANK6_START		0x00000000
#define CFG_BANK6_END		0x00000000
#define CFG_BANK6_ENABLE	0
#define CFG_BANK7_START		0x00000000
#define CFG_BANK7_END		0x00000000
#define CFG_BANK7_ENABLE	0

#define CFG_BANK_ENABLE		0x03

#define CFG_ODCR		0x75
#define CFG_PGMAX		0x32

#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT1L	(CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

#define CFG_IBAT2L	(0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT2U	(0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT3L	(0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT3U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U
#define CFG_DBAT3L	CFG_IBAT3L
#define CFG_DBAT3U	CFG_IBAT3U

#define BOOTFLAG_COLD	0x01
#define BOOTFLAG_WARM	0x02

#endif	/* __CONFIG_H */
