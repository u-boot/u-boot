/*
 * (C) Copyright 2016 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _ASM_ARCH_WDT_H
#define _ASM_ARCH_WDT_H

#define WDT_BASE			0x1e785000

/*
 * Special value that needs to be written to counter_restart register to
 * (re)start the timer
 */
#define WDT_COUNTER_RESTART_VAL		0x4755

/* Control register */
#define WDT_CTRL_RESET_MODE_SHIFT	5
#define WDT_CTRL_RESET_MODE_MASK	3

#define WDT_CTRL_EN			(1 << 0)
#define WDT_CTRL_RESET			(1 << 1)
#define WDT_CTRL_CLK1MHZ		(1 << 4)
#define WDT_CTRL_2ND_BOOT		(1 << 7)

/* Values for Reset Mode */
#define WDT_CTRL_RESET_SOC		0
#define WDT_CTRL_RESET_CHIP		1
#define WDT_CTRL_RESET_CPU		2
#define WDT_CTRL_RESET_MASK		3

/* Reset Mask register */
#define WDT_RESET_ARM			(1 << 0)
#define WDT_RESET_COPROC		(1 << 1)
#define WDT_RESET_SDRAM			(1 << 2)
#define WDT_RESET_AHB			(1 << 3)
#define WDT_RESET_I2C			(1 << 4)
#define WDT_RESET_MAC1			(1 << 5)
#define WDT_RESET_MAC2			(1 << 6)
#define WDT_RESET_GCRT			(1 << 7)
#define WDT_RESET_USB20			(1 << 8)
#define WDT_RESET_USB11_HOST		(1 << 9)
#define WDT_RESET_USB11_EHCI2		(1 << 10)
#define WDT_RESET_VIDEO			(1 << 11)
#define WDT_RESET_HAC			(1 << 12)
#define WDT_RESET_LPC			(1 << 13)
#define WDT_RESET_SDSDIO		(1 << 14)
#define WDT_RESET_MIC			(1 << 15)
#define WDT_RESET_CRT2C			(1 << 16)
#define WDT_RESET_PWM			(1 << 17)
#define WDT_RESET_PECI			(1 << 18)
#define WDT_RESET_JTAG			(1 << 19)
#define WDT_RESET_ADC			(1 << 20)
#define WDT_RESET_GPIO			(1 << 21)
#define WDT_RESET_MCTP			(1 << 22)
#define WDT_RESET_XDMA			(1 << 23)
#define WDT_RESET_SPI			(1 << 24)
#define WDT_RESET_MISC			(1 << 25)

#ifndef __ASSEMBLY__
struct ast_wdt {
	u32 counter_status;
	u32 counter_reload_val;
	u32 counter_restart;
	u32 ctrl;
	u32 timeout_status;
	u32 clr_timeout_status;
	u32 reset_width;
#ifdef CONFIG_ASPEED_AST2500
	u32 reset_mask;
#else
	u32 reserved0;
#endif
};

void wdt_stop(struct ast_wdt *wdt);
void wdt_start(struct ast_wdt *wdt, u32 timeout);

/**
 * Reset peripherals specified by mask
 *
 * Note, that this is only supported by ast2500 SoC
 *
 * @wdt: watchdog to use for this reset
 * @mask: reset mask.
 */
int ast_wdt_reset_masked(struct ast_wdt *wdt, u32 mask);

/**
 * ast_get_wdt() - get a pointer to watchdog registers
 *
 * @wdt_number: 0-based WDT peripheral number
 * @return pointer to registers or -ve error on error
 */
struct ast_wdt *ast_get_wdt(u8 wdt_number);
#endif  /* __ASSEMBLY__ */

#endif /* _ASM_ARCH_WDT_H */
