/*
 * U-boot - Configuration file for CSP Minotaur board
 *
 * Thu Oct 25 15:30:44 CEST 2007 <hackfin@section5.ch>
 *    Minotaur config, brushed up for official uClinux dist.
 *    Parallel flash support disabled, SPI flash boot command
 *    added ('run flashboot').
 *
 * Flash image map:
 *
 * 0x00000000      u-boot bootstrap
 * 0x00010000      environment
 * 0x00020000      u-boot code
 * 0x00030000      uImage.initramfs
 *
 */

#ifndef __CONFIG_BF537_MINOTAUR_H__
#define __CONFIG_BF537_MINOTAUR_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf537-0.2
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_SPI_MASTER


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
#define CONFIG_VCO_MULT			20
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			5


/*
 * Memory Settings
 */
#define CONFIG_MEM_SIZE			32
#define CONFIG_MEM_ADD_WDTH		9

#define CONFIG_EBIU_SDRRC_VAL		0x306
#define CONFIG_EBIU_SDGCTL_VAL		0x91114d

#define CONFIG_EBIU_AMGCTL_VAL		0xFF
#define CONFIG_EBIU_AMBCTL0_VAL		0x7BB07BB0
#define CONFIG_EBIU_AMBCTL1_VAL		0xFFC27BB0

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)


/*
 * Network Settings
 */
#ifndef __ADSPBF534__
#define CONFIG_BFIN_MAC
#define CONFIG_NETCONSOLE	1
#endif
#ifdef CONFIG_BFIN_MAC
#define CONFIG_IPADDR		192.168.0.15
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_HOSTNAME		bf537-minotaur
#endif

#define CONFIG_SYS_AUTOLOAD	"no"
#define CONFIG_ROOTPATH		"/romfs"
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	02:80:ad:20:31:42 */

#define CONFIG_LIB_RAND

/*
 * Flash Settings
 */
/* We don't have a parallel flash chip there */
#define CONFIG_SYS_NO_FLASH


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO


/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x10000
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR


/*
 * I2C settings
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_ADI
#define CONFIG_SYS_I2C_SPEED		50000
#define CONFIG_SYS_I2C_SLAVE		0


/*
 * Misc Settings
 */
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_MISC_INIT_R

#define CONFIG_BAUDRATE		57600
#define CONFIG_UART_CONSOLE	0
#define CONFIG_BFIN_SERIAL

#define CONFIG_PANIC_HANG	1
#define CONFIG_RTC_BFIN		1
#define CONFIG_BOOT_RETRY_TIME	-1
#define CONFIG_LOADS_ECHO		1

#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART)
# define CONFIG_BOOTDELAY	-1
#else
# define CONFIG_BOOTDELAY	5
#endif

#include <config_cmd_default.h>

#ifdef CONFIG_BFIN_MAC
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#else
# undef CONFIG_CMD_NET
# undef CONFIG_CMD_NFS
#endif

#define CONFIG_CMD_BOOTLDR
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ELF
#undef CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_SF

#define CONFIG_BOOTCOMMAND	"run ramboot"
#define CONFIG_BOOTARGS	"root=/dev/mtdblock0 rw"
#define CONFIG_SYS_PROMPT	"minotaur> "

#define BOOT_ENV_SETTINGS \
	"update=tftpboot $(loadaddr) u-boot.ldr;" \
		"sf probe " __stringify(BFIN_BOOT_SPI_SSEL) ";" \
		"sf erase 0 0x30000;" \
		"sf write $(loadaddr) 0 $(filesize)" \
	"flashboot=sf read 0x1000000 0x30000 0x320000;" \
		"bootm 0x1000000\0"
#ifdef CONFIG_BFIN_MAC
# define NETWORK_ENV_SETTINGS \
	"nfsargs=setenv bootargs root=/dev/nfs rw " \
		"nfsroot=$(serverip):$(rootpath)\0" \
	"addip=setenv bootargs $(bootargs) " \
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)" \
		":$(hostname):eth0:off\0" \
	"ramboot=tftpboot $(loadaddr) linux;" \
		"run ramargs;run addip;bootelf\0" \
	"nfsboot=tftpboot $(loadaddr) linux;" \
		"run nfsargs;run addip;bootelf\0"
#else
# define NETWORK_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
	NETWORK_ENV_SETTINGS \
	"ramargs=setenv bootargs " CONFIG_BOOTARGS "\0" \
	BOOT_ENV_SETTINGS

#endif
