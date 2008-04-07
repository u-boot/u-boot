/*
 * U-boot - Configuration file for BF537 STAMP board
 */

#ifndef __CONFIG_BF537_H__
#define __CONFIG_BF537_H__

#include <asm/blackfin-config-pre.h>

#define CFG_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_BAUDRATE		57600
/* Set default serial console for bf537 */
#define CONFIG_UART_CONSOLE	0
#define CONFIG_BOOTDELAY	5
/* define CONFIG_BF537_STAMP_LEDCMD to enable LED command*/
/*#define CONFIG_BF537_STAMP_LEDCMD	1*/

#define CONFIG_PANIC_HANG 1

#define CONFIG_BFIN_CPU	bf537-0.2
#define CONFIG_BFIN_BOOT_MODE BFIN_BOOT_BYPASS

#define CONFIG_BFIN_MAC

/* This sets the default state of the cache on U-Boot's boot */
#define CONFIG_ICACHE_ON
#define CONFIG_DCACHE_ON

/* Define if want to do post memory test */
#undef CONFIG_POST_TEST

#define CONFIG_RTC_BFIN		1
#define CONFIG_BOOT_RETRY_TIME	-1	/* Enable this if bootretry required, currently its disabled */

/* CONFIG_CLKIN_HZ is any value in Hz				*/
#define CONFIG_CLKIN_HZ		25000000
/* CONFIG_CLKIN_HALF controls what is passed to PLL 0=CLKIN	*/
/*						    1=CLKIN/2	*/
#define CONFIG_CLKIN_HALF	0
/* CONFIG_PLL_BYPASS controls if the PLL is used 0=don't bypass */
/*						    1=bypass PLL*/
#define CONFIG_PLL_BYPASS	0
/* CONFIG_VCO_MULT controls what the multiplier of the PLL is.	*/
/* Values can range from 1-64					*/
#define CONFIG_VCO_MULT			20
/* CONFIG_CCLK_DIV controls what the core clock divider is	*/
/* Values can be 1, 2, 4, or 8 ONLY				*/
#define CONFIG_CCLK_DIV			1
/* CONFIG_SCLK_DIV controls what the peripheral clock divider is*/
/* Values can range from 1-15					*/
#define CONFIG_SCLK_DIV			5
/* CONFIG_SPI_BAUD controls the SPI peripheral clock divider	*/
/* Values can range from 2-65535				*/
/* SCK Frequency = SCLK / (2 * CONFIG_SPI_BAUD)			*/
#define CONFIG_SPI_BAUD			2
#define CONFIG_SPI_BAUD_INITBLOCK	4

#if ( CONFIG_CLKIN_HALF == 0 )
#define CONFIG_VCO_HZ ( CONFIG_CLKIN_HZ * CONFIG_VCO_MULT )
#else
#define CONFIG_VCO_HZ (( CONFIG_CLKIN_HZ * CONFIG_VCO_MULT ) / 2 )
#endif

#if (CONFIG_PLL_BYPASS == 0)
#define CONFIG_CCLK_HZ ( CONFIG_VCO_HZ / CONFIG_CCLK_DIV )
#define CONFIG_SCLK_HZ ( CONFIG_VCO_HZ / CONFIG_SCLK_DIV )
#else
#define CONFIG_CCLK_HZ CONFIG_CLKIN_HZ
#define CONFIG_SCLK_HZ CONFIG_CLKIN_HZ
#endif

#define CONFIG_MEM_SIZE			64	/* 128, 64, 32, 16 */
#define CONFIG_MEM_ADD_WDTH		10	/* 8, 9, 10, 11 */
#define CONFIG_MEM_MT48LC32M8A2_75	1

#define CONFIG_LOADS_ECHO		1

/*
 * rarpb, bootp or dhcp commands will perform only a
 * configuration lookup from the BOOTP/DHCP server
 * but not try to load any image using TFTP
 */
#define CFG_AUTOLOAD			"no"

/*
 * Network Settings
 */
/* network support */
#ifdef CONFIG_BFIN_MAC
#define CONFIG_IPADDR		192.168.0.15
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_HOSTNAME		BF537
#endif

#define CONFIG_ROOTPATH		/romfs
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	02:80:ad:20:31:e8 */
/* This is the routine that copies the MAC in Flash to the 'ethaddr' setting */

#define CFG_LONGHELP		1
#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOT_RETRY_TIME	-1	/* Enable this if bootretry required, currently its disabled */
#define CONFIG_BOOTCOMMAND 	"run ramboot"

#if defined(CONFIG_POST_TEST)
/* POST support */
#define CONFIG_POST 		( CFG_POST_MEMORY | \
				  CFG_POST_UART	  | \
				  CFG_POST_FLASH  | \
				  CFG_POST_ETHER  | \
				  CFG_POST_LED	  | \
				  CFG_POST_BUTTON)
#else
#undef CONFIG_POST
#endif

#ifdef CONFIG_POST
#define FLASH_START_POST_BLOCK	11	/* Should > = 11 */
#define FLASH_END_POST_BLOCK	71	/* Should < = 71 */
#endif

/* CF-CARD IDE-HDD Support */

/* #define CONFIG_BFIN_TRUE_IDE */	/* Add CF flash card support */
/* #define CONFIG_BFIN_CF_IDE */	/* Add CF flash card support */
/* #define CONFIG_BFIN_HDD_IDE */	/* Add IDE Disk Drive (HDD) support */

#if defined(CONFIG_BFIN_CF_IDE) || defined(CONFIG_BFIN_HDD_IDE) || defined(CONFIG_BFIN_TRUE_IDE)
# define CONFIG_BFIN_IDE	1
#endif

/*#define CONFIG_BF537_NAND */		/* Add nand flash support */

#define CONFIG_NETCONSOLE	1
#define CONFIG_NET_MULTI	1

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
#define CONFIG_CMD_I2C
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_DATE

#ifndef CONFIG_BFIN_MAC
#undef CONFIG_CMD_NET
#else
#define CONFIG_CMD_PING
#endif

#if defined(CONFIG_BFIN_CF_IDE) \
	|| defined(CONFIG_BFIN_HDD_IDE) \
	|| defined(CONFIG_BFIN_TRUE_IDE)
#define CONFIG_CMD_IDE
#endif

#define CONFIG_CMD_DHCP

#if defined(CONFIG_POST)
#define CONFIG_CMD_DIAG
#endif

#ifdef CONFIG_BF537_NAND
#define CONFIG_CMD_NAND
#endif


#define CONFIG_BOOTARGS "root=/dev/mtdblock0 rw console=ttyBF0,57600"
#define CONFIG_LOADADDR	0x1000000

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"ramargs=setenv bootargs root=/dev/mtdblock0 rw console=ttyBF0,57600\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "		\
	"nfsroot=$(serverip):$(rootpath) console=ttyBF0,57600\0"\
	"addip=setenv bootargs $(bootargs) "			\
	"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"	\
	":$(hostname):eth0:off\0"				\
	"ramboot=tftpboot $(loadaddr) linux;"			\
	"run ramargs;run addip;bootelf\0"			\
	"nfsboot=tftpboot $(loadaddr) linux;"			\
	"run nfsargs;run addip;bootelf\0"			\
	"flashboot=bootm 0x20100000\0"				\
	"update=tftpboot $(loadaddr) u-boot.bin;"		\
	"protect off 0x20000000 0x2007FFFF;"			\
	"erase 0x20000000 0x2007FFFF;cp.b 0x1000000 0x20000000 $(filesize)\0"	\
	""

#define	CFG_PROMPT		"bfin> "	/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define	CFG_CBSIZE		256	/* Console I/O Buffer Size */
#endif
#define CFG_MAX_RAM_SIZE       	(CONFIG_MEM_SIZE * 1024*1024)
#define	CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define	CFG_MAXARGS		16	/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_MEMTEST_START	0x0	/* memtest works on */
#define CFG_MEMTEST_END		( (CONFIG_MEM_SIZE - 1) * 1024*1024)	/* 1 ... 63 MB in DRAM */
#define	CFG_LOAD_ADDR		CONFIG_LOADADDR	/* default load address */
#define	CFG_HZ			1000	/* decrementer freq: 10 ms ticks */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define	CFG_SDRAM_BASE		0x00000000

#define CFG_FLASH_BASE		0x20000000
#define CFG_FLASH_CFI		/* The flash is CFI compatible */
#define CFG_FLASH_CFI_DRIVER	/* Use common CFI driver */
#define CFG_FLASH_PROTECTION
#define CFG_MAX_FLASH_BANKS	1
#define CFG_MAX_FLASH_SECT	71	/* some have 67 sectors (M29W320DB), but newer have 71 (M29W320EB) */

#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MONITOR_BASE	(CFG_MAX_RAM_SIZE - CFG_MONITOR_LEN)
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)
#define CFG_GBL_DATA_SIZE	0x4000
#define CFG_GBL_DATA_ADDR	(CFG_MALLOC_BASE - CFG_GBL_DATA_SIZE)
#define CONFIG_STACKBASE	(CFG_GBL_DATA_ADDR  - 4)

#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#define CFG_ENV_IS_IN_EEPROM	1
#define CFG_ENV_OFFSET		0x4000
#define CFG_ENV_HEADER		(CFG_ENV_OFFSET + 0x16e) /* 0x12A is the length of LDR file header */
#else
#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0x20004000
#define CFG_ENV_OFFSET		(CFG_ENV_ADDR - CFG_FLASH_BASE)
#endif
#define CFG_ENV_SIZE		0x2000
#define	CFG_ENV_SECT_SIZE	0x2000	/* Total Size of Environment Sector */
#define ENV_IS_EMBEDDED

/* JFFS Partition offset set  */
#define CFG_JFFS2_FIRST_BANK	0
#define CFG_JFFS2_NUM_BANKS	1
/* 512k reserved for u-boot */
#define CFG_JFFS2_FIRST_SECTOR	15

#define CONFIG_SPI

/*
 * Stack sizes
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

#define POLL_MODE		1
#define FLASH_TOT_SECT		71
#define FLASH_SIZE		0x400000
#define CFG_FLASH_SIZE		0x400000

/*
 * Board NAND Infomation
 */

#define CFG_NAND_ADDR		0x20212000
#define CFG_NAND_BASE		CFG_NAND_ADDR
#define CFG_MAX_NAND_DEVICE	1
#define SECTORSIZE		512
#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3
#define NAND_ChipID_UNKNOWN	0x00
#define NAND_MAX_FLOORS		1
#define NAND_MAX_CHIPS		1
#define BFIN_NAND_READY		PF3

#define NAND_WAIT_READY(nand)  			\
	do { 					\
		int timeout = 0; 		\
		while(!(*pPORTFIO & PF3)) 	\
			if (timeout++ > 100000)	\
				break;		\
	} while (0)

#define BFIN_NAND_CLE		(1<<2)	/* A2 -> Command Enable */
#define BFIN_NAND_ALE		(1<<1)	/* A1 -> Address Enable */

#define WRITE_NAND_COMMAND(d, adr) do{ *(volatile __u8 *)((unsigned long)adr | BFIN_NAND_CLE) = (__u8)(d); } while(0)
#define WRITE_NAND_ADDRESS(d, adr) do{ *(volatile __u8 *)((unsigned long)adr | BFIN_NAND_ALE) = (__u8)(d); } while(0)
#define WRITE_NAND(d, adr) do{ *(volatile __u8 *)((unsigned long)adr) = (__u8)d; } while(0)
#define READ_NAND(adr) ((volatile unsigned char)(*(volatile __u8 *)(unsigned long)adr))

/*
 * Initialize PSD4256 registers for using I2C
 */
#define CONFIG_MISC_INIT_R

#define CFG_BOOTM_LEN		0x4000000	/* Large Image Length, set to 64 Meg */

/*
 * I2C settings
 * By default PF1 is used as SDA and PF0 as SCL on the Stamp board
 */
/* #define CONFIG_SOFT_I2C	1*/	/* I2C bit-banged */
#define CONFIG_HARD_I2C		1	/* I2C TWI */
#if defined CONFIG_HARD_I2C
#define CONFIG_TWICLK_KHZ	50
#endif

#define CONFIG_EBIU_SDRRC_VAL  0x306
#define CONFIG_EBIU_SDGCTL_VAL 0x91114d
#define CONFIG_EBIU_SDBCTL_VAL 0x25

#define CONFIG_EBIU_AMGCTL_VAL		0xFF
#define CONFIG_EBIU_AMBCTL0_VAL		0x7BB07BB0
#define CONFIG_EBIU_AMBCTL1_VAL		0xFFC27BB0

#if defined CONFIG_SOFT_I2C
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PF_SCL			PF0
#define PF_SDA			PF1

#define I2C_INIT		(*pFIO_DIR |=  PF_SCL); asm("ssync;")
#define I2C_ACTIVE		(*pFIO_DIR |=  PF_SDA); *pFIO_INEN &= ~PF_SDA; asm("ssync;")
#define I2C_TRISTATE		(*pFIO_DIR &= ~PF_SDA); *pFIO_INEN |= PF_SDA; asm("ssync;")
#define I2C_READ		((volatile)(*pFIO_FLAG_D & PF_SDA) != 0); asm("ssync;")
#define I2C_SDA(bit)		if(bit) { \
					*pFIO_FLAG_S = PF_SDA; \
					asm("ssync;"); \
					} \
				else    { \
					*pFIO_FLAG_C = PF_SDA; \
					asm("ssync;"); \
					}
#define I2C_SCL(bit)		if(bit) { \
					*pFIO_FLAG_S = PF_SCL; \
					asm("ssync;"); \
					} \
				else    { \
					*pFIO_FLAG_C = PF_SCL; \
					asm("ssync;"); \
					}
#define I2C_DELAY		udelay(5)	/* 1/4 I2C clock duration */
#endif

#define CFG_I2C_SPEED		50000
#define CFG_I2C_SLAVE		0xFE

/* 0xFF, 0x7BB07BB0, 0x22547BB0 */
/* #define AMGCTLVAL		(AMBEN_P0 | AMBEN_P1 | AMBEN_P2 | AMCKEN)
#define AMBCTL0VAL		(B1WAT_7 | B1RAT_11 | B1HT_2 | B1ST_3 | B1TT_4 | ~B1RDYPOL | \
				~B1RDYEN | B0WAT_7 | B0RAT_11 | B0HT_2 | B0ST_3 | B0TT_4 | ~B0RDYPOL | ~B0RDYEN)
#define AMBCTL1VAL		(B3WAT_2 | B3RAT_2 | B3HT_1 | B3ST_1 | B3TT_4 | B3RDYPOL | ~B3RDYEN | \
				B2WAT_7 | B2RAT_11 | B2HT_2 | B2ST_3 | B2TT_4 | ~B2RDYPOL | ~B2RDYEN)
*/

#define AMGCTLVAL		0xFF
#define AMBCTL0VAL		0x7BB07BB0
#define AMBCTL1VAL		0xFFC27BB0

#if defined(CONFIG_BFIN_IDE)

#define CONFIG_DOS_PARTITION	1
/*
 * IDE/ATA stuff
 */
#undef  CONFIG_IDE_8xx_DIRECT	/* no pcmcia interface required */
#undef  CONFIG_IDE_LED		/* no led for ide supported */
#undef  CONFIG_IDE_RESET	/* no reset for ide supported */

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE busses */
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*1)	/* max. 1 drives per IDE bus */

#undef  AMBCTL1VAL
#define AMBCTL1VAL		0xFFC3FFC3

#define CONFIG_CF_ATASEL_DIS	0x20311800
#define CONFIG_CF_ATASEL_ENA	0x20311802

#if defined(CONFIG_BFIN_TRUE_IDE)
/*
 * Note that these settings aren't for the most part used in include/ata.h
 * when all of the ATA registers are setup
 */
#define CFG_ATA_BASE_ADDR	0x2031C000
#define CFG_ATA_IDE0_OFFSET	0x0000
#define CFG_ATA_DATA_OFFSET	0x0020	/* Offset for data I/O */
#define CFG_ATA_REG_OFFSET	0x0020	/* Offset for normal register accesses */
#define CFG_ATA_ALT_OFFSET	0x001C	/* Offset for alternate registers */
#define CFG_ATA_STRIDE		2	/* CF.A0 --> Blackfin.Ax */
#endif				/* CONFIG_BFIN_TRUE_IDE */

#if defined(CONFIG_BFIN_CF_IDE)	/* USE CompactFlash Storage Card in the common memory space */
#define CFG_ATA_BASE_ADDR	0x20211800
#define CFG_ATA_IDE0_OFFSET	0x0000
#define CFG_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O */
#define CFG_ATA_REG_OFFSET	0x0000	/* Offset for normal register accesses */
#define CFG_ATA_ALT_OFFSET	0x000E	/* Offset for alternate registers */
#define CFG_ATA_STRIDE		1	/* CF.A0 --> Blackfin.Ax */
#endif				/* CONFIG_BFIN_CF_IDE */

#if defined(CONFIG_BFIN_HDD_IDE)	/* USE TRUE IDE */
#define CFG_ATA_BASE_ADDR	0x20314000
#define CFG_ATA_IDE0_OFFSET	0x0000
#define CFG_ATA_DATA_OFFSET	0x0020	/* Offset for data I/O */
#define CFG_ATA_REG_OFFSET	0x0020	/* Offset for normal register accesses */
#define CFG_ATA_ALT_OFFSET	0x001C	/* Offset for alternate registers */
#define CFG_ATA_STRIDE		2	/* CF.A0 --> Blackfin.A1 */

#undef  CONFIG_SCLK_DIV
#define CONFIG_SCLK_DIV		8
#endif				/* CONFIG_BFIN_HDD_IDE */

#endif				/*CONFIG_BFIN_IDE */

#include <asm/blackfin-config-post.h>

#endif
