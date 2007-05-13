#ifndef __CONFIG_H
#define __CONFIG_H

#undef DEBUG

#define CONFIG_SH		1
#define CONFIG_SH4		1
#define CONFIG_CPU_SH7750	1
#define CONFIG_MS7750SE		1
#define __LITTLE_ENDIAN__	1

//#define CONFIG_COMMANDS         (CONFIG_CMD_DFL | CFG_CMD_NET |CFG_CMD_PING)
#define CONFIG_COMMANDS        	CONFIG_CMD_DFL & ~CFG_CMD_NET 

#define CFG_SCIF_CONSOLE	1
#define CONFIG_BAUDRATE		38400
#define CONFIG_CONS_SCIF1	1
#define BOARD_LATE_INIT		1

#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	-1
#define CONFIG_BOOTARGS    	"console=ttySC0,115200"
#define CONFIG_ENV_OVERWRITE	1

#define CFG_SDRAM_BASE		(0x8C000000)
#define CFG_SDRAM_SIZE		(64 * 1024 * 1024)

#define CFG_LONGHELP		
#define CFG_PROMPT		"=> "		
#define CFG_CBSIZE		256	
#define CFG_PBSIZE		256	
#define CFG_MAXARGS		16
#define CFG_BARGSIZE		512	
#define CFG_BAUDRATE_TABLE	{ 115200, 57600, 38400, 19200, 9600 }		/* List of legal baudrate settings for this board */

#define CFG_MEMTEST_START	(CFG_SDRAM_BASE)
#define CFG_MEMTEST_END		(TEXT_BASE - 0x100000)


#define CFG_LOAD_ADDR		(CFG_SDRAM_BASE + 4 * 1024 * 1024)
#define CFG_MONITOR_BASE	(CFG_FLASH_BASE)	/* Address of u-boot image in Flash */
#define CFG_MONITOR_LEN		(128 * 1024)	
#define CFG_MALLOC_LEN		(256 * 1024)		/* Size of DRAM reserved for malloc() use */

#define CFG_GBL_DATA_SIZE	(256)			/* size in bytes reserved for initial data */
#define CFG_BOOTMAPSZ		(8 * 1024 * 1024)	
#define CFG_RX_ETH_BUFFER	(8)

#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#undef CFG_FLASH_CFI_BROKEN_TABLE
#undef  CFG_FLASH_QUIET_TEST
#define CFG_FLASH_EMPTY_INFO				/* print 'E' for empty sector on flinfo */

#define CFG_FLASH_BASE		(0xA1000000)
#define CFG_MAX_FLASH_BANKS	(1)			/* Max number of 
						 	 * Flash memory banks
						 	 */
#define CFG_MAX_FLASH_SECT	142
#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_SIZE		(CFG_ENV_SECT_SIZE)
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_FLASH_ERASE_TOUT  	120000
#define CFG_FLASH_WRITE_TOUT	500

#define CONFIG_SYS_CLK_FREQ	33333333
#define TMU_CLK_DIVIDER		4
#define CFG_HZ			(CONFIG_SYS_CLK_FREQ / TMU_CLK_DIVIDER)
#define	CFG_PLL_SETTLING_TIME	100		/* in us */

#endif /* __CONFIG_H */
