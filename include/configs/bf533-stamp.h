/*
 * U-boot - Configuration file for BF533 STAMP board
 */

#ifndef __CONFIG_STAMP_H__
#define __CONFIG_STAMP_H__

#define CONFIG_STAMP			1
#define CONFIG_RTC_BFIN			1
#define CONFIG_BF533			1
/*
 * Boot Mode Set
 * Blackfin can support several boot modes
 */
#define BF533_BYPASS_BOOT	0x0001	/* Bootmode 0: Execute from 16-bit externeal memory ( bypass BOOT ROM) */
#define BF533_PARA_BOOT		0x0002	/* Bootmode 1: Boot from 8-bit or 16-bit flash */
#define BF533_SPI_BOOT		0x0004	/* Bootmode 3: Boot from SPI flash */
/* Define the boot mode */
#define BFIN_BOOT_MODE		BF533_BYPASS_BOOT
/* #define BFIN_BOOT_MODE	BF533_SPI_BOOT */

#define CONFIG_PANIC_HANG 1

#define ADSP_BF531		0x31
#define ADSP_BF532		0x32
#define ADSP_BF533		0x33
#define BFIN_CPU		ADSP_BF533

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
#define CONFIG_SMC91111_BASE	0x20300300

/* FLASH/ETHERNET uses the same address range */
#define SHARED_RESOURCES 	1

/* Is I2C bit-banged? */
#define CONFIG_SOFT_I2C		1

/*
 * Software (bit-bang) I2C driver configuration
 */
#define PF_SCL			PF3
#define PF_SDA			PF2

/*
 * Video splash screen support
 */
#define  CONFIG_VIDEO		0

#define CONFIG_VDSP		1

/*
 * Clock settings
 */

/* CONFIG_CLKIN_HZ is any value in Hz				*/
#define CONFIG_CLKIN_HZ		11059200
/* CONFIG_CLKIN_HALF controls what is passed to PLL 0=CLKIN	*/
/*						    1=CLKIN/2	*/
#define CONFIG_CLKIN_HALF	0
/* CONFIG_PLL_BYPASS controls if the PLL is used 0=don't bypass	*/
/*						 1=bypass PLL	*/
#define CONFIG_PLL_BYPASS	0
/* CONFIG_VCO_MULT controls what the multiplier of the PLL is.	*/
/* Values can range from 1-64					*/
#define CONFIG_VCO_MULT		36
/* CONFIG_CCLK_DIV controls what the core clock divider is	*/
/* Values can be 1, 2, 4, or 8 ONLY				*/
#define CONFIG_CCLK_DIV		1
/* CONFIG_SCLK_DIV controls what the peripheral clock divider is*/
/* Values can range from 1-15					*/
#define CONFIG_SCLK_DIV		5
/* CONFIG_SPI_BAUD controls the SPI peripheral clock divider	*/
/* Values can range from 2-65535				*/
/* SCK Frequency = SCLK / (2 * CONFIG_SPI_BAUD)			*/
#define CONFIG_SPI_BAUD		2

#if (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#define CONFIG_SPI_BAUD_INITBLOCK	4
#endif

/*
 * Network settings
 */

#if (CONFIG_DRIVER_SMC91111)
#if 0
#define	CONFIG_MII
#endif

/* network support */
#define CONFIG_IPADDR		192.168.0.15
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_HOSTNAME		STAMP
#define CONFIG_ROOTPATH		/checkout/uClinux-dist/romfs

/* To remove hardcoding and enable MAC storage in EEPROM  */
/* #define CONFIG_ETHADDR		02:80:ad:20:31:b8 */
#endif /* CONFIG_DRIVER_SMC91111 */

/*
 * Flash settings
 */

#define CFG_FLASH_CFI		/* The flash is CFI compatible  */
#define CFG_FLASH_CFI_DRIVER	/* Use common CFI driver	*/
#define	CFG_FLASH_CFI_AMD_RESET

#define CFG_FLASH_BASE		0x20000000
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	67	/* max number of sectors on one chip */

#if (BFIN_BOOT_MODE == BF533_BYPASS_BOOT)
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0x20004000
#define	CFG_ENV_OFFSET		(CFG_ENV_ADDR - CFG_FLASH_BASE)
#elif (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#define CFG_ENV_IS_IN_EEPROM	1
#define CFG_ENV_OFFSET		0x4000
#define CFG_ENV_HEADER		(CFG_ENV_OFFSET + 0x12A)	/* 0x12A is the length of LDR file header */
#endif

#define	CFG_ENV_SIZE		0x2000
#define CFG_ENV_SECT_SIZE 	0x2000	/* Total Size of Environment Sector */
#define	ENV_IS_EMBEDDED

#define CFG_FLASH_ERASE_TOUT	30000	/* Timeout for Chip Erase (in ms) */
#define CFG_FLASH_ERASEBLOCK_TOUT	5000	/* Timeout for Block Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	1	/* Timeout for Flash Write (in ms) */

/* JFFS Partition offset set  */
#define CFG_JFFS2_FIRST_BANK 0
#define CFG_JFFS2_NUM_BANKS  1
/* 512k reserved for u-boot */
#define CFG_JFFS2_FIRST_SECTOR 	11

/*
 * following timeouts shall be used once the
 * Flash real protection is enabled
 */
#define CFG_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CFG_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */

/*
 * SDRAM settings & memory map
 */

#define CONFIG_MEM_SIZE		128	/* 128, 64, 32, 16 */
#define CONFIG_MEM_ADD_WDTH     11	/* 8, 9, 10, 11    */
#define CONFIG_MEM_MT48LC64M4A2FB_7E	1

#if (BFIN_BOOT_MODE == BF533_BYPASS_BOOT)
#define CFG_MEMTEST_START	0x00000000	/* memtest works on */
#elif (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#endif

#define	CFG_SDRAM_BASE		0x00000000

#define CFG_MAX_RAM_SIZE	(CONFIG_MEM_SIZE * 1024 *1024)
#define CFG_MEMTEST_END		(CFG_MAX_RAM_SIZE - 0x80000 - 1)
#define CONFIG_LOADADDR		0x01000000

#define CFG_LOAD_ADDR 		CONFIG_LOADADDR
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)     /* Reserve 128 kB for malloc()	*/
#define CFG_GBL_DATA_SIZE	0x4000		/* Reserve 16k for Global Data  */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

#define CFG_MONITOR_BASE		(CFG_MAX_RAM_SIZE - 0x40000)
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)
#define CFG_GBL_DATA_ADDR	(CFG_MALLOC_BASE - CFG_GBL_DATA_SIZE)
#define CONFIG_STACKBASE	(CFG_GBL_DATA_ADDR  - 4)

/* Check to make sure everything fits in SDRAM */
#if ((CFG_MONITOR_BASE + CFG_MONITOR_LEN) > CFG_MAX_RAM_SIZE)
	#error Memory Map does not fit into configuration
#endif

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

#if (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#if (CONFIG_SCLK_HZ / (2*CONFIG_SPI_BAUD) > 20000000)
#define CONFIG_SPI_FLASH_FAST_READ 1 /* Needed if SPI_CLK > 20 MHz */
#else
#undef CONFIG_SPI_FLASH_FAST_READ
#endif
#endif

/*
 * Command settings
 */

#define CFG_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1

#if (BFIN_BOOT_MODE == BF533_BYPASS_BOOT)
#define CFG_AUTOLOAD		"no"	/*rarpb, bootp or dhcp commands will perform only a */
#endif

/* configuration lookup from the BOOTP/DHCP server, */
/* but not try to load any image using TFTP	    */

#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOT_RETRY_TIME	-1	/* Enable this if bootretry required, currently its disabled */
#if (BFIN_BOOT_MODE == BF533_BYPASS_BOOT)
#define CONFIG_BOOTCOMMAND	"run ramboot"
#elif (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#define CONFIG_BOOTCOMMAND 	"eeprom read 0x1000000 0x100000 0x180000;icache on;dcache on;bootm 0x1000000"
#endif

#define CONFIG_BOOTARGS		"root=/dev/mtdblock0 rw console=ttyBF0,57600"

#if (CONFIG_DRIVER_SMC91111)
#define CONFIG_COMMANDS1	(CONFIG_CMD_DFL | \
				 CFG_CMD_PING   | \
				 CFG_CMD_ELF    | \
				 CFG_CMD_CACHE  | \
				 CFG_CMD_JFFS2  | \
				 CFG_CMD_EEPROM | \
				 CFG_CMD_DATE)

#else
#define CONFIG_COMMANDS1	(CONFIG_CMD_DFL | \
				 CFG_CMD_ELF    | \
				 CFG_CMD_CACHE  | \
				 CFG_CMD_JFFS2  | \
				 CFG_CMD_EEPROM | \
				 CFG_CMD_DATE)
#endif

#if (BFIN_BOOT_MODE == BF533_BYPASS_BOOT)
#if (CONFIG_DRIVER_SMC91111)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"ramargs=setenv bootargs root=/dev/mtdblock0 rw console=ttyBF0,57600\0" \
	"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):" \
		"$(rootpath) console=ttyBF0,57600\0" \
	"addip=setenv bootargs $(bootargs) ip=$(ipaddr):$(serverip):" \
		"$(gatewayip):$(netmask):$(hostname):eth0:off\0" \
	"ramboot=tftpboot $(loadaddr) linux; " \
		"run ramargs;run addip;bootelf\0" \
	"nfsboot=tftpboot $(loadaddr) linux; " \
		"run nfsargs;run addip;bootelf\0" \
	"flashboot=bootm 0x20100000\0" \
	"update=tftpboot $(loadaddr) u-boot.bin; " \
		"protect off 0x20000000 0x2003FFFF; erase 0x20000000 0x2003FFFF;" \
		"cp.b $(loadaddr) 0x20000000 $(filesize)\0" \
	""
#else
#define CONFIG_EXTRA_ENV_SETTINGS \
	"ramargs=setenv bootargs root=/dev/mtdblock0 rw console=ttyBF0,57600\0" \
	"flashboot=bootm 0x20100000\0" \
	"
#endif

#elif (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"ramargs=setenv bootargs root=/dev/mtdblock0 rw console=ttyBF0,57600\0" \
	"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):" \
		"$(rootpath) console=ttyBF0,57600\0"	\
	"addip=setenv bootargs $(bootargs) ip=$(ipaddr):$(serverip):" \
		"$(gatewayip):$(netmask):$(hostname):eth0:off\0" \
	"ramboot=tftpboot $(loadaddr) linux; " \
		"run ramargs;run addip;bootelf\0" \
	"nfsboot=tftpboot $(loadaddr) linux; "	\
		"run nfsargs;run addip;bootelf\0" \
	"flashboot=bootm 0x20100000\0" \
	"update=tftpboot $(loadaddr) u-boot.ldr;"	\
		"eeprom write $(loadaddr) 0x0 $(filesize);\0"\
	""
#endif

#ifdef CONFIG_SOFT_I2C
#if (!CONFIG_SOFT_I2C)
#undef CONFIG_SOFT_I2C
#endif
#endif

#if (CONFIG_SOFT_I2C)
#define CONFIG_COMMANDS2   CFG_CMD_I2C
#else
#define CONFIG_COMMANDS2 0
#endif /* CONFIG_SOFT_I2C */

#if (BFIN_BOOT_MODE == BF533_BYPASS_BOOT)
#define CONFIG_COMMANDS  ( CONFIG_COMMANDS1 | CONFIG_COMMANDS2 | CFG_CMD_DHCP)
#elif (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#define CONFIG_COMMANDS  ( CONFIG_COMMANDS1 | CONFIG_COMMANDS2)
#endif

/* This must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Console settings
 */

#define CONFIG_BAUDRATE		57600
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#if (BFIN_BOOT_MODE == BF533_SPI_BOOT)
#if (BFIN_CPU == ADSP_BF531)
#define	CFG_PROMPT	"serial_bf531> "	/* Monitor Command Prompt */
#elif (BFIN_CPU == ADSP_BF532)
#define	CFG_PROMPT	"serial_bf532> "	/* Monitor Command Prompt */
#else
#define	CFG_PROMPT	"serial_bf533> "	/* Monitor Command Prompt */
#endif
#else
#if (BFIN_CPU == ADSP_BF531)
#define	CFG_PROMPT	"bf531> "	/* Monitor Command Prompt */
#elif (BFIN_CPU == ADSP_BF532)
#define	CFG_PROMPT	"bf532> "	/* Monitor Command Prompt */
#else
#define	CFG_PROMPT	"bf533> "	/* Monitor Command Prompt */
#endif
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE	(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_LOADS_ECHO	1

/*
 * I2C settings
 * By default PF2 is used as SDA and PF3 as SCL on the Stamp board
 */
#if (CONFIG_SOFT_I2C)

#define I2C_INIT		(*pFIO_DIR |=  PF_SCL); asm("ssync;")
#define I2C_ACTIVE		(*pFIO_DIR |=  PF_SDA); *pFIO_INEN &= ~PF_SDA; asm("ssync;")
#define I2C_TRISTATE		(*pFIO_DIR &= ~PF_SDA); *pFIO_INEN |= PF_SDA; asm("ssync;")
#define I2C_READ		((volatile)(*pFIO_FLAG_D & PF_SDA) != 0); asm("ssync;")
#define I2C_SDA(bit)	if(bit) { \
				*pFIO_FLAG_S = PF_SDA; \
				asm("ssync;"); \
				} \
			else	{ \
				*pFIO_FLAG_C = PF_SDA; \
				asm("ssync;"); \
				}
#define I2C_SCL(bit)	if(bit) { \
				*pFIO_FLAG_S = PF_SCL; \
				asm("ssync;"); \
				} \
			else	{ \
				*pFIO_FLAG_C = PF_SCL; \
				asm("ssync;"); \
				}
#define I2C_DELAY		udelay(5)	/* 1/4 I2C clock duration */

#define CFG_I2C_SPEED		50000
#define CFG_I2C_SLAVE		0xFE
#endif /* CONFIG_SOFT_I2C */

/*
 * Compact Flash settings
 */

/* Enabled below option for CF support */
/* #define CONFIG_STAMP_CF	1 */

#if defined(CONFIG_STAMP_CF) && (CONFIG_COMMANDS & CFG_CMD_IDE)

#define CONFIG_MISC_INIT_R	1
#define CONFIG_DOS_PARTITION	1
/*
 * IDE/ATA stuff
 */
#undef  CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef  CONFIG_IDE_LED			/* no led for ide supported */
#undef  CONFIG_IDE_RESET		/* no reset for ide supported */

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE busses */
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*1) /* max. 1 drives per IDE bus */

#define CFG_ATA_BASE_ADDR	0x20200000
#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_DATA_OFFSET	0x0020	/* Offset for data I/O */
#define CFG_ATA_REG_OFFSET	0x0020	/* Offset for normal register accesses */
#define CFG_ATA_ALT_OFFSET	0x0007	/* Offset for alternate registers */

#define CFG_ATA_STRIDE		2
#endif

/*
 * Miscellaneous configurable options
 */

#define	CFG_HZ			1000	/* 1ms time tick */

#define CFG_BOOTM_LEN		0x4000000/* Large Image Length, set to 64 Meg */

#define CONFIG_SHOW_BOOT_PROGRESS 1	/* Show boot progress on LEDs */

#define CONFIG_SPI

#ifdef  CONFIG_VIDEO
#if (CONFIG_VIDEO)
#define CONFIG_SPLASH_SCREEN	1
#define CONFIG_SILENT_CONSOLE	1
#else
#undef CONFIG_VIDEO
#endif
#endif

/*
 * FLASH organization and environment definitions
 */
#define	CFG_BOOTMAPSZ		(8 << 20)/* Initial Memory map for Linux */

/* 0xFF, 0xBBC3BBc3, 0x99B39983 */
/*#define AMGCTLVAL		(AMBEN_P0 | AMBEN_P1 | AMBEN_P2 | AMCKEN)
#define AMBCTL0VAL		(B1WAT_11 | B1RAT_11 | B1HT_3 | B1ST_4 | B1TT_4 | B1RDYPOL | \
				B1RDYEN | B0WAT_11 | B0RAT_11 | B0HT_3 | B0ST_4 | B0TT_4 | B0RDYPOL | B0RDYEN)
#define AMBCTL1VAL   		(B3WAT_9 | B3RAT_9 | B3HT_2 | B3ST_3 | B3TT_4 | B3RDYPOL | \
				B3RDYEN | B2WAT_9 | B2RAT_9 | B2HT_2 | B2ST_4 | B2TT_4 | B2RDYPOL | B2RDYEN)
*/
#define AMGCTLVAL		0xFF
#define AMBCTL0VAL		0xBBC3BBC3
#define AMBCTL1VAL		0x99B39983
#define CF_AMBCTL1VAL		0x99B3ffc2

#ifdef CONFIG_VDSP
#define ET_EXEC_VDSP		0x8
#define SHT_STRTAB_VDSP		0x1
#define ELFSHDRSIZE_VDSP	0x2C
#define VDSP_ENTRY_ADDR		0xFFA00000
#endif

#endif
