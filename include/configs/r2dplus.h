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
#define CONFIG_CMD_SH_ZIMAGEBOOT

/* SCIF */
#define CONFIG_SCIF_CONSOLE	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_CONS_SCIF1	1
#define BOARD_LATE_INIT		1

#define CONFIG_BOOTDELAY	-1
#define CONFIG_BOOTARGS		"console=ttySC0,115200"
#define CONFIG_ENV_OVERWRITE	1

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		(0x8C000000)
#define CONFIG_SYS_SDRAM_SIZE		(0x04000000)

#define CONFIG_SYS_TEXT_BASE	0x0FFC0000
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		512
/* List of legal baudrate settings for this board */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 57600, 38400, 19200, 9600 }

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_TEXT_BASE - 0x100000)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 32 * 1024 * 1024)
/* Address of u-boot image in Flash */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

/*
 * NOR Flash ( Spantion S29GL256P )
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE		(0xA0000000)
#define CONFIG_SYS_MAX_FLASH_BANKS (1)
#define CONFIG_SYS_MAX_FLASH_SECT  256
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x40000
#define CONFIG_ENV_SIZE        (CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR        (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

/*
 * SuperH Clock setting
 */
#define CONFIG_SYS_CLK_FREQ	60000000
#define CONFIG_SYS_TMU_CLK_DIV		4
#define CONFIG_SYS_HZ		1000
#define	CONFIG_SYS_PLL_SETTLING_TIME	100/* in us */

/*
 * IDE support
 */
#define CONFIG_IDE_RESET	1
#define CONFIG_SYS_PIO_MODE		1
#define CONFIG_SYS_IDE_MAXBUS		1 /* IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE	1
#define CONFIG_SYS_ATA_BASE_ADDR	0xb4000000
#define CONFIG_SYS_ATA_STRIDE		2 /* 1bit shift */
#define CONFIG_SYS_ATA_DATA_OFFSET	0x1000	/* data reg offset */
#define CONFIG_SYS_ATA_REG_OFFSET	0x1000	/* reg offset */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x800	/* alternate register offset */
#define CONFIG_IDE_SWAP_IO

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
#define CONFIG_PCI_SYS_BUS	(CONFIG_SYS_SDRAM_BASE & 0x1fffffff)
#define CONFIG_PCI_SYS_PHYS	(CONFIG_SYS_SDRAM_BASE & 0x1fffffff)
#define CONFIG_PCI_SYS_SIZE	CONFIG_SYS_SDRAM_SIZE

/*
 * Network device (RTL8139) support
 */
#define CONFIG_NET_MULTI
#define CONFIG_RTL8139

#endif /* __CONFIG_H */
