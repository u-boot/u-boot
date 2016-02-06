/*
 * U-Boot - Configuration file for SSV DNP5370 board
 */

#ifndef __CONFIG_DNP5370_H__
#define __CONFIG_DNP5370_H__

/* this must come first */
#include <asm/config-pre.h>

/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU       bf537-0.3
#define CONFIG_BFIN_BOOT_MODE BFIN_BOOT_BYPASS

/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
#define CONFIG_CLKIN_HZ                 25000000
#define CONFIG_CLKIN_HALF               0
#define CONFIG_PLL_BYPASS               0
#define CONFIG_VCO_MULT                 24
#define CONFIG_CCLK_DIV                 1
#define CONFIG_SCLK_DIV                 5

/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH     9
#define CONFIG_MEM_SIZE         32

#define CONFIG_EBIU_SDRRC_VAL   0x03a0
#define CONFIG_EBIU_SDBCTL_VAL  0x0013
#define CONFIG_EBIU_SDGCTL_VAL  0x8091998d

#define CONFIG_EBIU_AMGCTL_VAL  0xF7
#define CONFIG_EBIU_AMBCTL0_VAL 0x7BB07BB0
#define CONFIG_EBIU_AMBCTL1_VAL 0xFFC27BB0

#define CONFIG_SYS_MONITOR_LEN  (256 * 1024)
#define CONFIG_SYS_MALLOC_LEN   (128 * 1024)

/*
 * Network Settings
 */
#ifndef __ADSPBF534__
#define CONFIG_ROOTPATH        "/romfs"

#define CONFIG_BFIN_MAC         1
#define CONFIG_PHY_ADDR         0
#define CONFIG_RMII             1

#define CONFIG_CMD_MII
#define CONFIG_CMD_PING

#endif

/*
 * Flash Settings
 *
 * Only 3 MB of the 4 MB NOR flash are addressable.
 * But limiting the flash size does not seem to work.
 * It seems the CFI detection has precedence.
 */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE       0x20000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS  1
#define CONFIG_SYS_MAX_FLASH_SECT   71 /* (M29W320EB) */

/* 512k reserved for u-boot */
#define CONFIG_SYS_JFFS2_FIRST_SECTOR 15

/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_FLASH     1
#define CONFIG_ENV_ADDR       0x20004000
#define CONFIG_ENV_SIZE       0x00002000
#define CONFIG_ENV_SECT_SIZE  0x00002000 /* Total Size of Environment Sector */
#define CONFIG_ENV_OFFSET     0x00004000 /* (CONFIG_ENV_ADDR - CONFIG_FLASH_BASE) */

#define ENV_IS_EMBEDDED
#define LDS_BOARD_TEXT \
	arch/blackfin/lib/built-in.o (.text*); \
	arch/blackfin/cpu/built-in.o (.text*); \
	. = DEFINED(env_offset) ? env_offset : .; \
	common/env_embedded.o (.text*);

/*
 * Misc Settings
 */
#define CONFIG_CMD_STRINGS
#define CONFIG_MISC_INIT_R
#define CONFIG_RTC_BFIN
#define CONFIG_SYS_LONGHELP

/* This disables the hardware watchdog (not inside the bfin) */
#define CONFIG_DNP5370_EXT_WD_DISABLE 1

#define CONFIG_UART_CONSOLE 0
#define CONFIG_BFIN_SERIAL
#define CONFIG_BAUDRATE     115200
#define CONFIG_BOOTCOMMAND  "bootm 0x20030000"
#define CONFIG_BOOTARGS     "console=ttyBF0,115200 root=/dev/mtdblock3 rootfstype=ext2"

/* Convenience commands to update Linux in NOR flash */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fetchme=tftpboot 0x01000000 uImage;" \
		"iminfo\0" \
	"flashme=protect off 0x20030000 0x2003ffff;" \
		"erase 0x20030000 0x202effff;" \
		"cp.b 0x01000000 0x20030000 0x2c0000\0" \
	"runme=bootm 0x01000000\0"

#endif
