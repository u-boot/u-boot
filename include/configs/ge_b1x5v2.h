/*
 * GE B1x5v2
 *
 * Copyright 2018-2020 GE Inc.
 * Copyright 2018-2020 Collabora Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __GE_B1X5V2_CONFIG_H
#define __GE_B1X5V2_CONFIG_H

#include "mx6_common.h"

#include "imx6_spl.h"
#define CONFIG_SPL_TARGET		"u-boot-with-spl.imx"

/* PWM */
#define CONFIG_IMX6_PWM_PER_CLK		66000000

/* UART */
#define CONFIG_MXC_UART_BASE		UART3_BASE

#if CONFIG_MXC_UART_BASE == UART2_BASE
/* UART2 requires CONFIG_DEBUG_UART_BASE=0x21e8000 */
#define CONSOLE_DEVICE "ttymxc1" /* System on Module debug connector */
#else
/* UART3 requires CONFIG_DEBUG_UART_BASE=0x21ec000 */
#define CONSOLE_DEVICE "ttymxc2" /* Base board debug connector */
#endif

/* USB */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2 /* Enabled USB controller number */
#define CONFIG_USBD_HS

/* Video */
#define CONFIG_HIDE_LOGO_VERSION
#define CONFIG_IMX_VIDEO_SKIP

/* Memory */
#define PHYS_SDRAM		       MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE	       PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Command definition */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"image=/boot/fitImage\0" \
	"fdt_addr_r=0x18000000\0" \
	"splash_addr_r=0x20000000\0" \
	"mmcdev=2\0" \
	"mmcpart=1\0" \
	"console=console="CONSOLE_DEVICE",115200\0" \
	"quiet=quiet loglevel=0\0" \
	"rootdev=/dev/mmcblk1p\0" \
	"setargs=setenv bootargs ${console} ${quiet} ${fsckforcerepair} " \
		"bootcause=${bootcause} vt.global_cursor_default=0 vt.cur_default=1 " \
		"root=${rootdev}${mmcpart} video=HDMI-A-1:${resolution} rootwait ro\0" \
	"loadimage=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"showsplashscreen=load mmc ${mmcdev}:${mmcpart} ${splash_addr_r} /boot/splashscreen-${resolution}.bmp; " \
		"bmp display ${splash_addr_r};\0" \
	"setconfidx=" \
		"if test \"${devicetype}\" = \"B105v2\"; then " \
			"setenv confidx 1; " \
		"elif test \"${devicetype}\" = \"B125v2\"; then " \
			"setenv confidx 2; " \
		"elif test \"${devicetype}\" = \"B155v2\"; then " \
			"setenv confidx 3; " \
		"elif test \"${devicetype}\" = \"B105Pv2\"; then " \
			"setenv confidx 4; " \
		"elif test \"${devicetype}\" = \"B125Pv2\"; then " \
			"setenv confidx 5; " \
		"fi;\0" \
	"set_default_type=setenv devicetype B155v2; setenv resolution 1366x768;" \
		"setenv fdtfile imx6dl-b155v2.dtb; run setconfidx;\0" \
	"checkconfidx=env exists confidx || run set_default_type;\0" \
	"checkfsckforcerepair=" \
		"if test \"${bootcount}\" > \"3\" ; then " \
			"setenv fsckforcerepair fsck.repair=1; " \
		"fi;\0" \
	"helix=run setconfidx; run checkconfidx; run checkfsckforcerepair; run setargs; " \
		"regulator dev LED_VCC; regulator enable; " \
		"regulator dev 5V0_AUDIO; regulator enable; " \
		"bootm ${loadaddr}#conf@${confidx};\0" \
	"failbootcmd=" \
		"echo reached failbootcmd;" \
		"cls; setcurs 5 4; " \
		"lcdputs \"Monitor failed to start. Try again, or contact GE Service for support.\"; " \
		"bootcount reset; \0" \
	"hasfirstboot=" \
		"load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"/boot/bootcause/firstboot;\0" \
	"swappartitions=" \
		"setexpr mmcpart 3 - ${mmcpart};\0" \
	"doboot=" \
		"echo Booting from mmc:${mmcdev}:${mmcpart} ...; " \
		"run helix;\0" \
	"altbootcmd=" \
		"setenv mmcpart 1; run hasfirstboot || setenv mmcpart 2; " \
		"run hasfirstboot || setenv mmcpart 0; " \
		"if test ${mmcpart} != 0; then " \
			"setenv bootcause REVERT; " \
			"run swappartitions loadimage doboot; " \
		"fi; " \
		"run failbootcmd\0" \
	"tryboot=" \
		"setenv mmcpart 1; run hasfirstboot || setenv mmcpart 2; " \
		"run loadimage || run swappartitions && run loadimage || " \
		"setenv mmcpart 0 && echo MISSING IMAGE;" \
		"run showsplashscreen; sleep 1; " \
		"run doboot; run failbootcmd;\0" \

#define CONFIG_BOOTCOMMAND "run tryboot;"

#endif /* __GE_B1X5V2_CONFIG_H */
