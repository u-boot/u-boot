/* U-boot for BlackVME. (C) Wojtek Skulski 2010.
 * The board includes ADSP-BF561 rev. 0.5,
 * 32-bit SDRAM (2 * MT48LC16M16A2TG or MT48LC32M16A2TG),
 * Gigabit Ether AX88180 (ASIX) + 88E1111 rev. B2 (Marvell),
 * SPI  boot flash on PF2 (M25P64 8MB, or M25P128 16 MB),
 * FPGA boot flash on PF3 (M25P64 8MB, or M25P128 16 MB),
 * Spartan6-LX150 (memory-mapped; both PPIs also connected).
 * See http://www.skutek.com
 */

#ifndef __CONFIG_BLACKVME_H__
#define __CONFIG_BLACKVME_H__

#include <asm/config-pre.h>

/* Debugging: Set these options if you're having problems
 * #define CONFIG_DEBUG_EARLY_SERIAL
 * #define DEBUG
 * #define CONFIG_DEBUG_DUMP
 * #define CONFIG_DEBUG_DUMP_SYMS
 * CONFIG_PANIC_HANG means that the board will not auto-reboot
 */
#define CONFIG_PANIC_HANG 0

/* CPU Options */
#define CONFIG_BFIN_CPU        bf561-0.5
#define CONFIG_BFIN_BOOT_MODE  BFIN_BOOT_SPI_MASTER

/*
 *		CLOCK SETTINGS CAVEAT
 * You CANNOT just change the clock settings, esp. the SCLK.
 * The SDRAM timing, SPI baud, and the serial UART baud
 * use SCLK frequency to set their own frequencies. Therefore,
 * if you change the SCLK_DIV, you may also have to adjust
 * SDRAM refresh and other timings.
 * --------------------------------------------------------------
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *		25 *  8 / 1 = 200 MHz
 *		25 * 16 / 1 = 400 MHz
 *		25 * 24 / 1 = 600 MHz
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 *		25 *  8 / 2 = 100 MHz
 *		25 * 24 / 6 = 100 MHz
 *		25 * 24 / 5 = 120 MHz
 *		25 * 16 / 3 = 133 MHz
 * 25 MHz because the oscillator also feeds the ether chip.
 * CONFIG_CLKIN_HZ is 25 MHz written in Hz
 * CLKIN_HALF controls the DF bit in PLL_CTL
 *	0 = CLKIN	1 = CLKIN / 2
 * PLL_BYPASS controls the BYPASS bit in PLL_CTL
 *	0 = do not bypass	1 = bypass PLL
 * VCO_MULT = MSEL (multiplier) in PLL_CTL
 * Values can range from 0-63 (where 0 means 64)
 * CCLK_DIV = core clock divider (1, 2, 4, or 8 ONLY)
 * SCLK_DIV = system clock divider, 1 to 15
 */
#define CONFIG_CLKIN_HZ		25000000
#define CONFIG_CLKIN_HALF	0
#define CONFIG_PLL_BYPASS	0
#define CONFIG_VCO_MULT		8
#define CONFIG_CCLK_DIV		1
#define CONFIG_SCLK_DIV		2

/*
 * Ether chip in async memory space AMS3, same as BF561-EZ-KIT.
 * Used in 32-bit mode. 16-bit mode not supported.
 * http://docs.blackfin.uclinux.org/doku.php?id=hw:cards:ax88180
 */
/*
 * Network settings using a dedicated 2nd ether card in PC
 * Windows will automatically acquire IP of that card
 * Then use the dedicated card IP + 1 for the board
 * http://docs.blackfin.uclinux.org/doku.php?id=setting_up_the_network
 */
#define CONFIG_DRIVER_AX88180	1
#define AX88180_BASE		0x2c000000
#define CONFIG_CMD_MII		/* enable probing PHY */

#define CONFIG_HOSTNAME	blackvme	/* Bfin board  */
#define CONFIG_IPADDR		169.254.144.145	/* Bfin board  */
#define CONFIG_GATEWAYIP	169.254.144.144	/* dedic card  */
#define CONFIG_SERVERIP	169.254.144.144	/* tftp server */
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_ROOTPATH		"/export/uClinux-dist/romfs"	/*NFS*/
#define CFG_AUTOLOAD		"no"
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_ENV_OVERWRITE	1	/* enable changing MAC at runtime */
/* Comment out hardcoded MAC to enable MAC storage in EEPROM */
/* # define CONFIG_ETHADDR	ff:ee:dd:cc:bb:aa */

/*
 * SDRAM settings & memory map
 */

#define CONFIG_MEM_SIZE		64	/* 128, 64, 32, 16 */
#define CONFIG_MEM_ADD_WDTH	 9	/* 8, 9, 10, 11    */
/*
 * SDRAM reference page
 * http://docs.blackfin.uclinux.org/doku.php?id=bfin:sdram
 * NOTE: BlackVME populates only SDRAM bank 0
 */
/* CONFIG_EBIU_SDBCTL_VAL bank ctrl may be needed in future */
#define CONFIG_EBIU_SDGCTL_VAL  0x91114d  /* global control */
#define CONFIG_EBIU_SDRRC_VAL   0x306     /* refresh rate */

/* Async memory global settings. (ASRAM, not SDRAM)
 * HRM page 16-10. Global ASRAM control = 0x3F. Six lower bits = 1
 * CLKOUT enabled, all async banks enabled, core has priority
 * bank 0&1 16 bit (FPGA)
 * bank 2&3 32 bit (ether and USB chips)
 */
#define CONFIG_EBIU_AMGCTL_VAL  0x3F   /* ASRAM setup */

/* Async mem timing: BF561 HRM page 16-12 and 16-15.
 * Default values 0xFFC2 FFC2 are the slowest supported.
 * Example settings of CONFIG_EBIU_AMBCTL1_VAL
 * 1. EZ-KIT settings: 0xFFC2 7BB0
 * 2. Bank 3 good timing for AX88180 @ 125MHz = 0x8850 xxxx
 *    See the following page:
 *    http://docs.blackfin.uclinux.org/doku.php?id=hw:cards:ax88180
 * 3. Bank 3 timing for AX88180 @ SCLK = 100 MHz:
 * AX88180  WEN = 5 clocks  REN 6 clocks @ SCLK = 100 MHz
 * One extra clock needed because AX88180 is asynchronous to CPU.
 */
			   /* bank 1   0 */
#define CONFIG_EBIU_AMBCTL0_VAL 0xFFC2FFC2
			   /* bank 3   2 */
#define CONFIG_EBIU_AMBCTL1_VAL 0xFFC2FFC2

/* memory layout */

#define CONFIG_SYS_MONITOR_LEN	(256 << 10)
#define CONFIG_SYS_MALLOC_LEN	(384 << 10)

/*
 * Serial SPI Flash
 * For the M25P64 SCK should be kept < 15 MHz
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x40000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x40000

#define CONFIG_ENV_SPI_MAX_HZ	15000000
#define CONFIG_SF_DEFAULT_SPEED	15000000
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO

/*
 * Interactive command settings
 */

#define CONFIG_SYS_LONGHELP	1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_AUTO_COMPLETE	1

#include <config_cmd_default.h>

#define CONFIG_CMD_BOOTLDR
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CPLBINFO
#define CONFIG_CMD_SF
#define CONFIG_CMD_ELF

/*
 * Default: boot from SPI flash.
 * "sfboot" is a composite command defined in extra settings
 */
#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOTCOMMAND	"run sfboot"

/*
 * Console settings
 */
#define CONFIG_BAUDRATE		57600
#define CONFIG_LOADS_ECHO	1
#define CONFIG_UART_CONSOLE	0

/*
 * U-Boot environment variables. Use "printenv" to examine.
 * http://docs.blackfin.uclinux.org/doku.php?id=bootloaders:u-boot:env
 */
#define CONFIG_BOOTARGS \
	"root=/dev/mtdblock0 rw " \
	"clkin_hz=" __stringify(CONFIG_CLKIN_HZ) " " \
	"earlyprintk=serial,uart0," \
	__stringify(CONFIG_BAUDRATE) " " \
	"console=ttyBF0," __stringify(CONFIG_BAUDRATE) " "

/* Convenience env variables & commands.
 * Reserve kernstart = 0x20000  = 128 kB for U-Boot.
 * Reserve kernarea  = 0x500000 = 5 MB   for kernel (reasonable size).
 * U-Boot image is saved at flash offset=0.
 * Kernel image is saved at flash offset=$kernstart.
 * Instructions. Ksave takes about a minute to complete.
 *	1. Update U-Boot: run uget; run usave
 *	2. Update kernel: run kget; run ksave
 * After updating U-Boot also update the kernel per above instructions
 * to make the saved environment consistent with the flash.
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernstart=0x20000\0" \
	"kernarea=0x500000\0" \
	"uget=tftp u-boot.ldr\0" \
	"kget=tftp uImage\0" \
	"usave=sf probe 2; " \
		"sf erase 0 $(kernstart); " \
		"sf write $(fileaddr) 0 $(filesize)\0" \
	"ksave=sf probe 2; " \
		"saveenv; " \
		"echo Now patiently wait for the prompt...; " \
		"sf erase $(kernstart) $(kernarea); " \
		"sf write $(fileaddr) $(kernstart) $(filesize)\0" \
	"sfboot=sf probe 2; " \
		"sf read $(loadaddr) $(kernstart) $(filesize); " \
		"run addip; bootm\0" \
	"addip=setenv bootargs $(bootargs) " \
	"ip=$(ipaddr):$(serverip):$(gatewayip):" \
		"$(netmask):$(hostname):eth0:off\0"

/*
 * Soft I2C settings (BF561 does not have hard I2C)
 * PF12,13 on SPI connector 0.
 */
#ifdef CONFIG_SYS_I2C_SOFT
# define CONFIG_CMD_I2C
# define CONFIG_SOFT_I2C_GPIO_SCL	GPIO_PF12
# define CONFIG_SOFT_I2C_GPIO_SDA	GPIO_PF13
# define CONFIG_SYS_I2C_SPEED		50000
# define CONFIG_SYS_I2C_SLAVE		0xFE
#endif

/*
 * No Parallel Flash on this board
 */
#define CONFIG_SYS_NO_FLASH
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_JFFS2
#undef CONFIG_CMD_FLASH

#endif
