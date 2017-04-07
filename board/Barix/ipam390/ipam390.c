/*
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 * Based on:
 * U-Boot:board/davinci/da8xxevm/da850evm.c
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/davinci_nand.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <hwconfig.h>
#include <bootstage.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DRIVER_TI_EMAC
#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
#define HAS_RMII 1
#else
#define HAS_RMII 0
#endif
#endif /* CONFIG_DRIVER_TI_EMAC */

void dsp_lpsc_on(unsigned domain, unsigned int id)
{
	dv_reg_p mdstat, mdctl, ptstat, ptcmd;
	struct davinci_psc_regs *psc_regs;

	psc_regs = davinci_psc0_regs;
	mdstat = &psc_regs->psc0.mdstat[id];
	mdctl = &psc_regs->psc0.mdctl[id];
	ptstat = &psc_regs->ptstat;
	ptcmd = &psc_regs->ptcmd;

	while (*ptstat & (0x1 << domain))
		;

	if ((*mdstat & 0x1f) == 0x03)
		return;                 /* Already on and enabled */

	*mdctl |= 0x03;

	*ptcmd = 0x1 << domain;

	while (*ptstat & (0x1 << domain))
		;
	while ((*mdstat & 0x1f) != 0x03)
		;		/* Probably an overkill... */
}

static void dspwake(void)
{
	unsigned *resetvect = (unsigned *)DAVINCI_L3CBARAM_BASE;
	u32 val;

	/* if the device is ARM only, return */
	if ((readl(CHIP_REV_ID_REG) & 0x3f) == 0x10)
		return;

	if (hwconfig_subarg_cmp_f("dsp", "wake", "no", NULL))
		return;

	*resetvect++ = 0x1E000; /* DSP Idle */
	/* clear out the next 10 words as NOP */
	memset(resetvect, 0, sizeof(unsigned) * 10);

	/* setup the DSP reset vector */
	writel(DAVINCI_L3CBARAM_BASE, HOST1CFG);

	dsp_lpsc_on(1, DAVINCI_LPSC_GEM);
	val = readl(PSC0_MDCTL + (15 * 4));
	val |= 0x100;
	writel(val, (PSC0_MDCTL + (15 * 4)));
}

int misc_init_r(void)
{
	dspwake();
	return 0;
}

static const struct pinmux_config gpio_pins[] = {
	/* GP7[14] selects bootmode*/
	{ pinmux(16), 8, 3 },	/* GP7[14] */
};

const struct pinmux_resource pinmuxes[] = {
#ifdef CONFIG_DRIVER_TI_EMAC
	PINMUX_ITEM(emac_pins_mdio),
#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
	PINMUX_ITEM(emac_pins_rmii),
#else
	PINMUX_ITEM(emac_pins_mii),
#endif
#endif
	PINMUX_ITEM(uart2_pins_txrx),
	PINMUX_ITEM(uart2_pins_rtscts),
	PINMUX_ITEM(uart0_pins_txrx),
	PINMUX_ITEM(uart0_pins_rtscts),
#ifdef CONFIG_NAND_DAVINCI
	PINMUX_ITEM(emifa_pins_cs3),
	PINMUX_ITEM(emifa_pins_nand),
#endif
	PINMUX_ITEM(gpio_pins),
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_UART0 },	/* console */
	{ DAVINCI_LPSC_GPIO },
};

const int lpsc_size = ARRAY_SIZE(lpsc);

#ifndef CONFIG_DA850_EVM_MAX_CPU_CLK
#define CONFIG_DA850_EVM_MAX_CPU_CLK	300000000
#endif

#define REV_AM18X_EVM		0x100

/*
 * get_board_rev() - setup to pass kernel board revision information
 * Returns:
 * bit[0-3]	Maximum cpu clock rate supported by onboard SoC
 *		0000b - 300 MHz
 *		0001b - 372 MHz
 *		0010b - 408 MHz
 *		0011b - 456 MHz
 */
u32 get_board_rev(void)
{
	char *s;
	u32 maxcpuclk = CONFIG_DA850_EVM_MAX_CPU_CLK;
	u32 rev = 0;

	s = getenv("maxcpuclk");
	if (s)
		maxcpuclk = simple_strtoul(s, NULL, 10);

	if (maxcpuclk >= 456000000)
		rev = 3;
	else if (maxcpuclk >= 408000000)
		rev = 2;
	else if (maxcpuclk >= 372000000)
		rev = 1;
#ifdef CONFIG_DA850_AM18X_EVM
	rev |= REV_AM18X_EVM;
#endif
	return rev;
}

int board_early_init_f(void)
{
	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	if (da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc)))
		return 1;

	return 0;
}

int board_init(void)
{
	irq_init();

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DA850_EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART0),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

#ifdef CONFIG_DRIVER_TI_EMAC
	davinci_emac_mii_mode_sel(HAS_RMII);
#endif /* CONFIG_DRIVER_TI_EMAC */

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
#if (CONFIG_SYS_NS16550_COM1 == DAVINCI_UART0_BASE)
	       &davinci_uart0_ctrl_regs->pwremu_mgmt);
#else
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);
#endif
	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC
/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */

static int init_led(int gpio, char *name, int val)
{
	int ret;

	ret = gpio_request(gpio, name);
	if (ret)
		return -1;
	ret = gpio_direction_output(gpio, val);
	if (ret)
		return -1;

	return gpio;
}

#define LED_ON	0
#define LED_OFF	1

#if !defined(CONFIG_SPL_BUILD)
#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress(int status)
{
	static int red;
	static int green;

	if (red == 0)
		red = init_led(CONFIG_IPAM390_GPIO_LED_RED, "red", LED_ON);
	if (red != CONFIG_IPAM390_GPIO_LED_RED)
		return;
	if (green == 0)
		green = init_led(CONFIG_IPAM390_GPIO_LED_GREEN, "green",
				 LED_OFF);
	if (green != CONFIG_IPAM390_GPIO_LED_GREEN)
		return;

	switch (status) {
	case BOOTSTAGE_ID_RUN_OS:
		/*
		 * set normal state
		 * LED Red  : on
		 * LED green: off
		 */
		gpio_set_value(red, LED_ON);
		gpio_set_value(green, LED_OFF);
		break;
	case BOOTSTAGE_ID_MAIN_LOOP:
		/*
		 * U-Boot operation
		 * LED Red  : on
		 * LED green: on
		 */
		gpio_set_value(red, LED_ON);
		gpio_set_value(green, LED_ON);
		break;
	}
}
#endif
#endif

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	int ret;
	int bootmode = 0;

	/*
	 * GP7[14] selects bootmode:
	 * 1: boot linux
	 * 0: boot u-boot
	 * if error accessing gpio boot U-Boot
	 *
	 * SPL bootmode
	 * 0: boot linux
	 * 1: boot u-boot
	 */
	ret = gpio_request(CONFIG_IPAM390_GPIO_BOOTMODE , "bootmode");
	if (ret)
		bootmode = 1;
	if (!bootmode) {
		ret = gpio_direction_input(CONFIG_IPAM390_GPIO_BOOTMODE);
		if (ret)
			bootmode = 1;
	}
	if (!bootmode)
		ret = gpio_get_value(CONFIG_IPAM390_GPIO_BOOTMODE);
	if (!bootmode)
		if (ret == 0)
			bootmode = 1;
	/*
	 * LED red  : on
	 * LED green: off
	 */
	init_led(CONFIG_IPAM390_GPIO_LED_RED, "red", LED_ON);
	init_led(CONFIG_IPAM390_GPIO_LED_GREEN, "green", LED_OFF);
	return bootmode;
}
#endif
