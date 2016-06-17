/*
 * U-Boot - Configuration file for BlackStamp board
 * Configuration by Ben Matthews for UR LLE using bf533-stamp.h
 * as a template
 * See http://blackfin.uclinux.org/gf/project/blackstamp/
 */

#ifndef __CONFIG_BLACKSTAMP_H__
#define __CONFIG_BLACKSTAMP_H__

#include <asm/config-pre.h>

/*
 * Debugging: Set these options if you're having problems
 */
/*
 * #define CONFIG_DEBUG_EARLY_SERIAL
 * #define DEBUG
 * #define CONFIG_DEBUG_DUMP
 * #define CONFIG_DEBUG_DUMP_SYMS
*/
#define CONFIG_PANIC_HANG 0

/* CPU Options
 * Be sure to set the Silicon Revision Correctly
 */
#define CONFIG_BFIN_CPU		bf532-0.5
#define CONFIG_BFIN_BOOT_MODE	BFIN_BOOT_SPI_MASTER

/*
 * Board settings
 */
#define CONFIG_SMC91111	1
#define CONFIG_SMC91111_BASE	0x20300300

/* FLASH/ETHERNET uses the same address range
 * Depending on what you have the CPLD doing
 * this probably isn't needed
 */
#define SHARED_RESOURCES	1

/* Is I2C bit-banged? */

/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ			25000000
/* CLKIN_HALF controls the DF bit in PLL_CTL      0 = CLKIN		*/
/*                                                1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF		0
/* PLL_BYPASS controls the BYPASS bit in PLL_CTL  0 = do not bypass	*/
/*                                                1 = bypass PLL	*/
#define CONFIG_PLL_BYPASS		0
/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-63 (where 0 means 64)			*/
#define CONFIG_VCO_MULT			16
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			3

/*
 * Network settings
 */

#ifdef CONFIG_SMC91111
#define CONFIG_IPADDR		192.168.0.15
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_HOSTNAME		blackstamp
#define CONFIG_ROOTPATH		"/checkout/uClinux-dist/romfs"
#define CONFIG_SYS_AUTOLOAD		"no"
#endif

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x40000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x40000

/*
 * SDRAM settings & memory map
 */

#define CONFIG_MEM_SIZE		64	/* 128, 64, 32, 16 */
#define CONFIG_MEM_ADD_WDTH	10	/* 8, 9, 10, 11    */

#define CONFIG_SYS_MONITOR_LEN	(256 << 10)
#define CONFIG_SYS_MALLOC_LEN	(384 << 10)

/*
 * Command settings
 */

#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_ENV_OVERWRITE	1

#ifdef CONFIG_SMC91111
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#else
#endif

#ifdef CONFIG_SYS_I2C_SOFT
# define CONFIG_CMD_I2C
#endif

#define CONFIG_CMD_BOOTLDR
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CPLBINFO
#define CONFIG_CMD_DATE
#define CONFIG_CMD_SF

#define CONFIG_BOOTDELAY     5
#define CONFIG_BOOTCOMMAND   "run ramboot"
#define CONFIG_BOOTARGS \
	"root=/dev/mtdblock0 rw " \
	"clkin_hz=" __stringify(CONFIG_CLKIN_HZ) " " \
	"earlyprintk=" \
		"serial," \
		"uart" __stringify(CONFIG_UART_CONSOLE) "," \
		__stringify(CONFIG_BAUDRATE) " " \
	"console=ttyBF0," __stringify(CONFIG_BAUDRATE)

#if defined(CONFIG_CMD_NET)
# if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS)
#  define UBOOT_ENV_FILE "u-boot.bin"
# else
#  define UBOOT_ENV_FILE "u-boot.ldr"
# endif
# if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#  ifdef CONFIG_SPI
#   define UBOOT_ENV_UPDATE \
		"eeprom write $(loadaddr) 0x0 $(filesize)"
#  else
#   define UBOOT_ENV_UPDATE \
		"sf probe " __stringify(BFIN_BOOT_SPI_SSEL) ";" \
		"sf erase 0 0x40000;" \
		"sf write $(loadaddr) 0 $(filesize)"
#  endif
# else
#  define UBOOT_ENV_UPDATE \
		"protect off 0x20000000 0x2003FFFF;" \
		"erase 0x20000000 0x2003FFFF;" \
		"cp.b $(loadaddr) 0x20000000 $(filesize)"
# endif
# define NETWORK_ENV_SETTINGS \
	"ubootfile=" UBOOT_ENV_FILE "\0" \
	"update=" \
		"tftp $(loadaddr) $(ubootfile);" \
		UBOOT_ENV_UPDATE \
		"\0" \
	"addip=set bootargs $(bootargs) " \
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask):" \
		   "$(hostname):eth0:off" \
		"\0" \
	"ramargs=set bootargs " CONFIG_BOOTARGS "\0" \
	"ramboot=" \
		"tftp $(loadaddr) uImage;" \
		"run ramargs;" \
		"run addip;" \
		"bootm" \
		"\0" \
	"nfsargs=set bootargs " \
		"root=/dev/nfs rw " \
		"nfsroot=$(serverip):$(rootpath),tcp,nfsvers=3" \
		"\0" \
	"nfsboot=" \
		"tftp $(loadaddr) vmImage;" \
		"run nfsargs;" \
		"run addip;" \
		"bootm" \
		"\0"
#else
# define NETWORK_ENV_SETTINGS
#endif

/*
 * Console settings
 */
#define CONFIG_BAUDRATE		57600
#define CONFIG_LOADS_ECHO	1
#define CONFIG_UART_CONSOLE	0
#define CONFIG_BFIN_SERIAL

/*
 * I2C settings
 * By default PF2 is used as SDA and PF3 as SCL on the Stamp board
 * Located on the expansion connector on pins 86/85
 * Note these pins are arbitrarily chosen because we aren't using
 * them yet. You can (and probably should) change these values!
 */
#ifdef CONFIG_SYS_I2C_SOFT
#define CONFIG_SOFT_I2C_GPIO_SCL GPIO_PF9
#define CONFIG_SOFT_I2C_GPIO_SDA GPIO_PF8
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0xFE
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_RTC_BFIN		1

/*
 * Serial Flash Infomation
 */
#define CONFIG_BFIN_SPI
/* For the M25P64 SCK Should be Kept < 15Mhz */
#define CONFIG_ENV_SPI_MAX_HZ	15000000
#define CONFIG_SF_DEFAULT_SPEED	15000000

/*
 * FLASH organization and environment definitions
 */

#define CONFIG_EBIU_AMGCTL_VAL		0xFF
#define CONFIG_EBIU_AMBCTL0_VAL		0xBBC3BBC3
#define CONFIG_EBIU_AMBCTL1_VAL		0x99B39983
#define CONFIG_EBIU_SDRRC_VAL		0x268
#define CONFIG_EBIU_SDGCTL_VAL		0x911109

/* Even though Rev C boards have Parallel Flash
 * We aren't supporting it. Newer versions of the
 * hardware don't support Parallel Flash at all.
 */
#define CONFIG_SYS_NO_FLASH
#undef CONFIG_CMD_JFFS2

#endif
