#ifndef __CONFIG_H
#define __CONFIG_H

#undef DEBUG

#define CONFIG_SH		1
#define CONFIG_SH4		1
#define CONFIG_CPU_SH7751	1
#define CONFIG_CPU_SH_TYPE_R	1
#define CONFIG_R2DPLUS		1
#define __LITTLE_ENDIAN__	1

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DFL
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_PCI
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_IDE
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION

/* SCIF */
#define CFG_SCIF_CONSOLE	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_CONS_SCIF1	1
#define BOARD_LATE_INIT		1

#define CONFIG_BOOTDELAY	-1
#define CONFIG_BOOTARGS		"console=ttySC0,115200"
#define CONFIG_ENV_OVERWRITE	1

/* Network setting */
#define CONFIG_NETMASK		255.0.0.0
#define CONFIG_IPADDR		10.0.192.51
#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_GATEWAYIP	10.0.0.1

/* SDRAM */
#define CFG_SDRAM_BASE		(0x8C000000)
#define CFG_SDRAM_SIZE		(0x04000000)

#define CFG_LONGHELP
#define CFG_PROMPT		"=> "
#define CFG_CBSIZE		256
#define CFG_PBSIZE		256
#define CFG_MAXARGS		16
#define CFG_BARGSIZE		512
/* List of legal baudrate settings for this board */
#define CFG_BAUDRATE_TABLE	{ 115200, 57600, 38400, 19200, 9600 }

#define CFG_MEMTEST_START	(CFG_SDRAM_BASE)
#define CFG_MEMTEST_END		(TEXT_BASE - 0x100000)

#define CFG_LOAD_ADDR		(CFG_SDRAM_BASE + 32 * 1024 * 1024)
/* Address of u-boot image in Flash */
#define CFG_MONITOR_BASE	(CFG_FLASH_BASE)
#define CFG_MONITOR_LEN		(128 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CFG_MALLOC_LEN		(256 * 1024)
/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_SIZE	(256)
#define CFG_BOOTMAPSZ		(8 * 1024 * 1024)

/*
 * NOR Flash
 */
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER

#if defined(CONFIG_R2DPLUS_OLD)
#define CFG_FLASH_BASE		(0xA0000000)
#define CFG_MAX_FLASH_BANKS (1)	/* Max number of
				 * Flash memory banks
				 */
#define CFG_MAX_FLASH_SECT  142
#define CFG_FLASH_BANKS_LIST    { CFG_FLASH_BASE }

#else /* CONFIG_R2DPLUS_OLD */

#define CFG_FLASH_BASE		(0xA0000000)
#define CFG_FLASH_CFI_WIDTH	0x04	/* 32bit */
#define CFG_MAX_FLASH_BANKS	(2)
#define CFG_MAX_FLASH_SECT	270
#define CFG_FLASH_BANKS_LIST    { CFG_FLASH_BASE,\
			CFG_FLASH_BASE + 0x100000,\
			CFG_FLASH_BASE + 0x400000,\
			CFG_FLASH_BASE + 0x700000, }
#endif /* CONFIG_R2DPLUS_OLD */

#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_SIZE		(CFG_ENV_SECT_SIZE)
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_FLASH_ERASE_TOUT	120000
#define CFG_FLASH_WRITE_TOUT	500

/*
 * SuperH Clock setting
 */
#define CONFIG_SYS_CLK_FREQ	60000000
#define TMU_CLK_DIVIDER		4
#define CFG_HZ			(CONFIG_SYS_CLK_FREQ / TMU_CLK_DIVIDER)
#define	CFG_PLL_SETTLING_TIME	100/* in us */

/*
 * IDE support
 */
#define CONFIG_IDE_RESET	1
#define CFG_PIO_MODE		1
#define CFG_IDE_MAXBUS		1 /* IDE bus */
#define CFG_IDE_MAXDEVICE	1
#define CFG_ATA_BASE_ADDR	0xb4000000
#define CFG_ATA_STRIDE		2 /* 1bit shift */
#define CFG_ATA_DATA_OFFSET	0x1000	/* data reg offset */
#define CFG_ATA_REG_OFFSET	0x1000	/* reg offset */
#define CFG_ATA_ALT_OFFSET	0x800	/* alternate register offset */

/*
 * SuperH PCI Bridge Configration
 */
#define CONFIG_PCI
#define CONFIG_SH4_PCI
#define CONFIG_SH7751_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW	1
#define __io
#define __mem_pci

#define CONFIG_PCI_MEM_BUS	0xFD000000	/* Memory space base addr */
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x01000000	/* Size of Memory window */
#define CONFIG_PCI_IO_BUS	0xFE240000	/* IO space base address */
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x00040000	/* Size of IO window */

/*
 * Network device (RTL8139) support
 */
#define CONFIG_NET_MULTI
#define CONFIG_RTL8139
#define _IO_BASE		0x00000000
#define KSEG1ADDR(x)		(x)

#endif /* __CONFIG_H */
