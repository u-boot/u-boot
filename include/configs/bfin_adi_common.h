/*
 * U-Boot - Common settings for Analog Devices boards
 */

#ifndef __CONFIG_BFIN_ADI_COMMON_H__
#define __CONFIG_BFIN_ADI_COMMON_H__

/*
 * Command Settings
 */
#ifndef _CONFIG_CMD_DEFAULT_H
# include <config_cmd_default.h>
# if ADI_CMDS_NETWORK
#  define CONFIG_CMD_DHCP
#  define CONFIG_CMD_PING
#  ifdef CONFIG_BFIN_MAC
#   define CONFIG_CMD_MII
#  endif
# else
#  undef CONFIG_CMD_BOOTD
#  undef CONFIG_CMD_NET
#  undef CONFIG_CMD_NFS
# endif
# ifdef CONFIG_LIBATA
#  define CONFIG_CMD_FAT
#  define CONFIG_CMD_SATA
#  define CONFIG_DOS_PARTITION
# endif
# ifdef CONFIG_MMC
#  define CONFIG_CMD_FAT
#  define CONFIG_CMD_MMC
#  define CONFIG_DOS_PARTITION
# endif
# ifdef CONFIG_USB
#  define CONFIG_CMD_EXT2
#  define CONFIG_CMD_FAT
#  define CONFIG_CMD_USB
#  define CONFIG_CMD_USB_STORAGE
#  define CONFIG_DOS_PARTITION
# endif
# ifdef CONFIG_NAND_PLAT
#  define CONFIG_CMD_NAND
# endif
# ifdef CONFIG_POST
#  define CONFIG_CMD_DIAG
# endif
# ifdef CONFIG_RTC_BFIN
#  define CONFIG_CMD_DATE
# endif
# ifdef CONFIG_SPI
#  define CONFIG_CMD_EEPROM
# endif
# if defined(CONFIG_BFIN_SPI) || defined(CONFIG_SOFT_SPI)
#  define CONFIG_CMD_SPI
# endif
# ifdef CONFIG_SPI_FLASH
#  define CONFIG_CMD_SF
# endif
# if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
#  define CONFIG_CMD_I2C
# endif
# ifdef CONFIG_SYS_NO_FLASH
#  undef CONFIG_CMD_FLASH
#  undef CONFIG_CMD_IMLS
# else
#  define CONFIG_CMD_JFFS2
# endif
# define CONFIG_CMD_BOOTLDR
# define CONFIG_CMD_CACHE
# define CONFIG_CMD_CPLBINFO
# define CONFIG_CMD_ELF
# define CONFIG_ELF_SIMPLE_LOAD
# define CONFIG_CMD_REGINFO
# define CONFIG_CMD_STRINGS
# if defined(__ADSPBF51x__) || defined(__ADSPBF52x__) || defined(__ADSPBF54x__)
#  define CONFIG_CMD_OTP
#  define CONFIG_CMD_SPIBOOTLDR
# endif
#endif

/*
 * Console Settings
 */
#define CONFIG_SYS_LONGHELP	1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_LOADS_ECHO	1
#define CONFIG_JTAG_CONSOLE
#ifndef CONFIG_BAUDRATE
# define CONFIG_BAUDRATE	57600
#endif

/*
 * Debug Settings
 */
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_DEBUG_DUMP	1
#define CONFIG_KALLSYMS		1
#define CONFIG_PANIC_HANG	1

/*
 * Env Settings
 */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART)
# define CONFIG_BOOTDELAY	-1
#else
# define CONFIG_BOOTDELAY	5
#endif
#ifndef CONFIG_BOOTCOMMAND
# define CONFIG_BOOTCOMMAND	"run ramboot"
#endif
#ifdef CONFIG_VIDEO
# define CONFIG_BOOTARGS_VIDEO "console=tty0 "
#else
# define CONFIG_BOOTARGS_VIDEO ""
#endif
#ifndef CONFIG_BOOTARGS_ROOT
# define CONFIG_BOOTARGS_ROOT "/dev/mtdblock0 rw"
#endif
#define CONFIG_BOOTARGS	\
	"root=" CONFIG_BOOTARGS_ROOT " " \
	"clkin_hz=" MK_STR(CONFIG_CLKIN_HZ) " " \
	"earlyprintk=" \
		"serial," \
		"uart" MK_STR(CONFIG_UART_CONSOLE) "," \
		MK_STR(CONFIG_BAUDRATE) " " \
	CONFIG_BOOTARGS_VIDEO \
	"console=ttyBF0," MK_STR(CONFIG_BAUDRATE)
#if defined(CONFIG_CMD_NAND)
# define NAND_ENV_SETTINGS \
	"nandargs=set bootargs " CONFIG_BOOTARGS "\0" \
	"nandboot=" \
		"nand read $(loadaddr) 0x20000 0x100000;" \
		"run nandargs;" \
		"bootm" \
		"\0"
#else
# define NAND_ENV_SETTINGS
#endif
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
		"sf probe " MK_STR(BFIN_BOOT_SPI_SSEL) ";" \
		"sf erase 0 0x40000;" \
		"sf write $(loadaddr) 0 $(filesize)"
#  endif
# elif (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_NAND)
#  define UBOOT_ENV_UPDATE \
		"nand unlock 0 0x40000;" \
		"nand erase 0 0x40000;" \
		"nand write $(loadaddr) 0 0x40000"
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
	\
	"ramfile=uImage\0" \
	"ramargs=set bootargs " CONFIG_BOOTARGS "\0" \
	"ramboot=" \
		"tftp $(loadaddr) $(ramfile);" \
		"run ramargs;" \
		"run addip;" \
		"bootm" \
		"\0" \
	\
	"nfsfile=vmImage\0" \
	"nfsargs=set bootargs " \
		"root=/dev/nfs rw " \
		"nfsroot=$(serverip):$(rootpath),tcp,nfsvers=3" \
		"\0" \
	"nfsboot=" \
		"tftp $(loadaddr) $(nfsfile);" \
		"run nfsargs;" \
		"run addip;" \
		"bootm" \
		"\0"
#else
# define NETWORK_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
	NAND_ENV_SETTINGS \
	NETWORK_ENV_SETTINGS \
	"flashboot=bootm 0x20100000\0"

/*
 * Network Settings
 */
#ifdef CONFIG_CMD_NET
# define CONFIG_IPADDR		192.168.0.15
# define CONFIG_NETMASK		255.255.255.0
# define CONFIG_GATEWAYIP	192.168.0.1
# define CONFIG_SERVERIP	192.168.0.2
# define CONFIG_ROOTPATH	/romfs
# ifdef CONFIG_CMD_DHCP
#  ifndef CONFIG_SYS_AUTOLOAD
#   define CONFIG_SYS_AUTOLOAD "no"
#  endif
# endif
# define CONFIG_NET_RETRY_COUNT 20
#endif

/*
 * Misc Settings
 */
#define CONFIG_LZMA

#endif
