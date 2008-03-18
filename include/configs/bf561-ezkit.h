/*
 * U-boot - Configuration file for BF561 EZKIT board
 */

#ifndef __CONFIG_EZKIT561_H__
#define __CONFIG_EZKIT561_H__

#include <asm/blackfin-config-pre.h>

#define CONFIG_VDSP		1
#define CONFIG_BF561		1

#define CFG_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_BAUDRATE		57600
/* Set default serial console for bf537 */
#define CONFIG_UART_CONSOLE	0
#define CONFIG_EZKIT561		1
#define CONFIG_BOOTDELAY	5

#define CONFIG_PANIC_HANG 1

#define CONFIG_BFIN_CPU	bf561-0.3

/*
* Boot Mode Set
* Blackfin can support several boot modes
*/
#define BF561_BYPASS_BOOT	0x21
#define BF561_PARA_BOOT		0x22
#define BF561_SPI_BOOT		0x24
/* Define the boot mode */
#define BFIN_BOOT_MODE	BF561_BYPASS_BOOT

/* This sets the default state of the cache on U-Boot's boot */
#define CONFIG_ICACHE_ON
#define CONFIG_DCACHE_ON

/* Define where the uboot will be loaded by on-chip boot rom */
#define APP_ENTRY 0x00001000

/*
 * Stringize definitions - needed for environmental settings
 */
#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)

/*
 * Board settings
 */
#define CONFIG_DRIVER_SMC91111	1
#define CONFIG_SMC91111_BASE	0x2C010300
#define CONFIG_ASYNC_EBIU_BASE	CONFIG_SMC91111_BASE & ~(4*1024*1024)
#define CONFIG_SMC_USE_32_BIT	1
#define CONFIG_MISC_INIT_R	1

/*
 * Clock settings
 */

/* CONFIG_CLKIN_HZ is any value in Hz				*/
#define CONFIG_CLKIN_HZ		30000000
/* CONFIG_CLKIN_HALF controls what is passed to PLL 0=CLKIN	*/
/*						    1=CLKIN/2	*/
#define CONFIG_CLKIN_HALF	0
/* CONFIG_PLL_BYPASS controls if the PLL is used 0=don't bypass	*/
/*						 1=bypass PLL	*/
#define CONFIG_PLL_BYPASS	0
/* CONFIG_VCO_MULT controls what the multiplier of the PLL is	*/
/* Values can range from 1-64					*/
#define CONFIG_VCO_MULT		20
/* CONFIG_CCLK_DIV controls what the core clock divider is	*/
/* Values can be 1, 2, 4, or 8 ONLY				*/
#define CONFIG_CCLK_DIV		1
/* CONFIG_SCLK_DIV controls what the peripheral clock divider is */
/* Values can range from 1-15					*/
#define CONFIG_SCLK_DIV		5
/* CONFIG_SPI_BAUD controls the SPI peripheral clock divider	*/
/* Values can range from 2-65535				*/
/* SCK Frequency = SCLK / (2 * CONFIG_SPI_BAUD)			*/
#define CONFIG_SPI_BAUD		2
#define CONFIG_SPI_BAUD_INITBLOCK	4

/*
 * Network settings
 */
#if (CONFIG_DRIVER_SMC91111)
#define CONFIG_IPADDR		192.168.0.15
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_HOSTNAME		ezkit561
#define CONFIG_ROOTPATH		/arm-cross-build/BF561/uClinux-dist/romfs
#endif				/* CONFIG_DRIVER_SMC91111 */

/*
 * Flash settings
 */

#define CFG_FLASH_CFI		/* The flash is CFI compatible */
#define CFG_FLASH_CFI_DRIVER	/* Use common CFI driver */
#define CFG_FLASH_CFI_AMD_RESET
#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_FLASH_BASE		0x20000000
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	135	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		0x20020000
#define	CFG_ENV_SECT_SIZE	0x10000	/* Total Size of Environment Sector */
/* JFFS Partition offset set  */
#define CFG_JFFS2_FIRST_BANK	0
#define CFG_JFFS2_NUM_BANKS	1
/* 512k reserved for u-boot */
#define CFG_JFFS2_FIRST_SECTOR	8

/*
 * SDRAM settings & memory map
 */

#define CONFIG_MEM_SIZE			64	/* 128, 64, 32, 16 */
#define CONFIG_MEM_ADD_WDTH		9	/* 8, 9, 10, 11    */
#define CONFIG_MEM_MT48LC16M16A2TG_75	1

#define	CFG_SDRAM_BASE		0x00000000
#define CFG_MAX_RAM_SIZE	(CONFIG_MEM_SIZE * 1024 * 1024)

#define CFG_MEMTEST_START	0x0	/* memtest works on */
#define CFG_MEMTEST_END		( (CONFIG_MEM_SIZE - 1) * 1024*1024)	/* 1 ... 63 MB in DRAM */

#define	CONFIG_LOADADDR		0x01000000	/* default load address */
#define CFG_LOAD_ADDR		CONFIG_LOADADDR
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor   */
#define CFG_MONITOR_BASE	(CFG_MAX_RAM_SIZE - CFG_MONITOR_LEN)

#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()  */
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)

#define CFG_GBL_DATA_SIZE	0x4000
#define CFG_GBL_DATA_ADDR	(CFG_MALLOC_BASE - CFG_GBL_DATA_SIZE)
#define CONFIG_STACKBASE	(CFG_GBL_DATA_ADDR  - 4)
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

#if ( CONFIG_CLKIN_HALF == 0 )
#define CONFIG_VCO_HZ		( CONFIG_CLKIN_HZ * CONFIG_VCO_MULT )
#else
#define CONFIG_VCO_HZ		(( CONFIG_CLKIN_HZ * CONFIG_VCO_MULT ) / 2 )
#endif

#if (CONFIG_PLL_BYPASS == 0)
#define CONFIG_CCLK_HZ		( CONFIG_VCO_HZ / CONFIG_CCLK_DIV )
#define CONFIG_SCLK_HZ		( CONFIG_VCO_HZ / CONFIG_SCLK_DIV )
#else
#define CONFIG_CCLK_HZ		CONFIG_CLKIN_HZ
#define CONFIG_SCLK_HZ		CONFIG_CLKIN_HZ
#endif

/*
 * Command settings
 */

#define CFG_AUTOLOAD	"no"	/* rarpb, bootp, dhcp commands will	*/
				/* only perform a configuration		*/
				/* lookup from the BOOTP/DHCP server	*/
				/* but not try to load any image	*/
				/* using TFTP				*/
#define CONFIG_BOOT_RETRY_TIME	-1	/* Enable this if bootretry required, */
					/* currently its disabled */
#define CONFIG_BOOTCOMMAND	"run ramboot"
#define CONFIG_BOOTARGS		"root=/dev/mtdblock0 rw console=ttyBF0,57600"

#if (CONFIG_DRIVER_SMC91111)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"ramargs=setenv bootargs root=/dev/mtdblock0 rw console=ttyBF0,57600\0" 		\
	"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):"	\
		"$(rootpath) console=ttyBF0,57600\0"						\
	"addip=setenv bootargs $(bootargs) ip=$(ipaddr):$(serverip):"	\
		"$(gatewayip):$(netmask):$(hostname):eth0:off\0"	\
	"ramboot=tftpboot $(loadaddr) linux; "		\
		"run ramargs; run addip; bootelf\0"			\
	"nfsboot=tftpboot $(loadaddr) linux; "		\
		"run nfsargs; run addip; bootelf\0"			\
	"update=tftpboot $(loadaddr) u-boot.bin; "	\
		"protect off 0x20000000 0x2003FFFF; "			\
		"erase 0x20000000 0x2003FFFF; "				\
		"cp.b $(loadaddr) 0x20000000 $(filesize)\0" \
	""
#else
#define CONFIG_EXTRA_ENV_SETTINGS \
	"ramargs=setenv bootargs root=/dev/mtdblock0 rw console=ttyBF0,57600\0"		\
	"flashboot=bootm 0x20100000\0"					\
	""
#endif


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_JFFS2

#if defined(CONFIG_DRIVER_SMC91111)
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#endif


/*
 * Console settings
 */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define	CFG_PROMPT		"bfin> "	/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE		1024		/* Console I/O Buffer Size */
#else
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size */
#endif
#define	CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_LOADS_ECHO	1

/*
 * Miscellaneous configurable options
 */
#define	CFG_HZ			1000		/* decrementer freq: 10 ms ticks */
#define CFG_BOOTM_LEN		0x4000000	/* Large Image Length, set to 64 Meg */

/*
 * FLASH organization and environment definitions
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define AMGCTLVAL		0x3F
#define AMBCTL0VAL		0x7BB07BB0
#define AMBCTL1VAL		0xFFC27BB0

#ifdef CONFIG_VDSP
#define ET_EXEC_VDSP		0x8
#define SHT_STRTAB_VDSP		0x1
#define ELFSHDRSIZE_VDSP	0x2C
#define VDSP_ENTRY_ADDR		0xFFA00000
#endif

#endif				/* __CONFIG_EZKIT561_H__ */
