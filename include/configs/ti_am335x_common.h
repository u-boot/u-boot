/*
 * ti_am335x_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * For more details, please see the technical documents listed at
 * http://www.ti.com/product/am3359#technicaldocuments
 */

#ifndef __CONFIG_TI_AM335X_COMMON_H__
#define __CONFIG_TI_AM335X_COMMON_H__

#define CONFIG_AM33XX
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SYS_CACHELINE_SIZE       64
#define CONFIG_MAX_RAM_BANK_SIZE	(1024 << 20)	/* 1GB */
#define CONFIG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */
#define CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC

#include <asm/arch/omap.h>

/* NS16550 Configuration */
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#endif
#define CONFIG_SYS_NS16550_CLK		48000000

/* Network defines. */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_BOOTP_DNS		/* Configurable parts of CMD_DHCP */
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_NET_RETRY_COUNT         10
#define CONFIG_CMD_PING
#define CONFIG_DRIVER_TI_CPSW		/* Driver for IP block */
#define CONFIG_MII			/* Required in net/eth.c */

/*
 * RTC related defines. To use bootcount you must set bootlimit in the
 * environment to a non-zero value and enable CONFIG_BOOTCOUNT_LIMIT
 * in the board config.
 */
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x44E3E000

/* Enable the HW watchdog, since we can use this with bootcount */
#define CONFIG_HW_WATCHDOG
#define CONFIG_OMAP_WATCHDOG

/*
 * SPL related defines.  The Public RAM memory map the ROM defines the
 * area between 0x402F0400 and 0x4030B800 as a download area and
 * 0x4030B800 to 0x4030CE00 as a public stack area.  The ROM also
 * supports X-MODEM loading via UART, and we leverage this and then use
 * Y-MODEM to load u-boot.img, when booted over UART.
 */
#define CONFIG_SPL_TEXT_BASE		0x402F0400
#define CONFIG_SPL_MAX_SIZE		(0x4030B800 - CONFIG_SPL_TEXT_BASE)
#define CONFIG_SYS_SPL_ARGS_ADDR	(CONFIG_SYS_SDRAM_BASE + \
					 (128 << 20))

/* Enable the watchdog inside of SPL */
#define CONFIG_SPL_WATCHDOG_SUPPORT

/*
 * Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#if !defined(CONFIG_SPL_BUILD) && !defined(CONFIG_NOR_BOOT)
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/*
 * When building U-Boot such that there is no previous loader
 * we need to call board_early_init_f.  This is taken care of in
 * s_init when we have SPL used.
 */
#if !defined(CONFIG_SKIP_LOWLEVEL_INIT) && !defined(CONFIG_SPL)
#define CONFIG_BOARD_EARLY_INIT_F
#endif

#ifdef CONFIG_NAND
#define CONFIG_SPL_NAND_AM33XX_BCH	/* ELM support */
#endif

/* Now bring in the rest of the common code. */
#include <configs/ti_armv7_omap.h>

#endif	/* __CONFIG_TI_AM335X_COMMON_H__ */
