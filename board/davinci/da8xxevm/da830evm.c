/*
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 *
 * Base on code from TI. Original Notices follow:
 *
 * (C) Copyright 2008, Texas Instruments, Inc. http://www.ti.com/
 *
 * Modified for DA8xx EVM.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts are shamelessly stolen from various TI sources, original copyright
 * follows:
 * -----------------------------------------------------------------
 *
 * Copyright (C) 2004 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 * SPDX-License-Identifier:	GPL-2.0+
 * ----------------------------------------------------------------------------
 */

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/davinci_misc.h>

#ifdef CONFIG_DAVINCI_MMC
#include <mmc.h>
#include <asm/arch/sdmmc_defs.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static const struct pinmux_resource pinmuxes[] = {
#ifdef CONFIG_SPI_FLASH
	PINMUX_ITEM(spi0_pins_base),
	PINMUX_ITEM(spi0_pins_scs0),
	PINMUX_ITEM(spi0_pins_ena),
#endif
	PINMUX_ITEM(uart2_pins_txrx),
	PINMUX_ITEM(i2c0_pins),
#ifdef CONFIG_USB_DA8XX
	PINMUX_ITEM(usb_pins),
#endif
#ifdef CONFIG_USE_NAND
	PINMUX_ITEM(emifa_pins),
	PINMUX_ITEM(emifa_pins_cs0),
	PINMUX_ITEM(emifa_pins_cs2),
	PINMUX_ITEM(emifa_pins_cs3),
#endif
#if defined(CONFIG_DRIVER_TI_EMAC)
	PINMUX_ITEM(emac_pins_rmii),
	PINMUX_ITEM(emac_pins_mdio),
	PINMUX_ITEM(emac_pins_rmii_clk_source),
#endif
#ifdef CONFIG_DAVINCI_MMC
	PINMUX_ITEM(mmc0_pins_8bit)
#endif
};

static const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI0 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
#ifdef CONFIG_DAVINCI_MMC
	{ DAVINCI_LPSC_MMC_SD },
#endif

};

#ifdef CONFIG_DAVINCI_MMC
static struct davinci_mmc mmc_sd0 = {
	.reg_base = (struct davinci_mmc_regs *)DAVINCI_MMC_SD0_BASE,
	.host_caps = MMC_MODE_8BIT,
	.voltages = MMC_VDD_32_33 | MMC_VDD_33_34,
	.version = MMC_CTLR_VERSION_2,
};

int board_mmc_init(bd_t *bis)
{
	mmc_sd0.input_clk = clk_get(DAVINCI_MMCSD_CLKID);

	printf("%x\n", mmc_sd0.input_clk);

	/* Add slot-0 to mmc subsystem */
	return davinci_mmc_init(bis, &mmc_sd0);
}
#endif

int board_init(void)
{
#ifndef CONFIG_USE_IRQ
	irq_init();
#endif

#ifdef CONFIG_NAND_DAVINCI
	/* EMIFA 100MHz clock select */
	writel(readl(&davinci_syscfg_regs->cfgchip3) & ~2,
	       &davinci_syscfg_regs->cfgchip3);
	/* NAND CS setup */
	writel((DAVINCI_ABCR_WSETUP(0) |
		DAVINCI_ABCR_WSTROBE(2) |
		DAVINCI_ABCR_WHOLD(0) |
		DAVINCI_ABCR_RSETUP(0) |
		DAVINCI_ABCR_RSTROBE(2) |
		DAVINCI_ABCR_RHOLD(0) |
		DAVINCI_ABCR_TA(2) |
		DAVINCI_ABCR_ASIZE_8BIT),
	       &davinci_emif_regs->ab2cr);
#endif

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DA830_EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	if (da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc)))
		return 1;

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI0 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART2),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);

	return(0);
}


#ifdef CONFIG_NAND_DAVINCI
int board_nand_init(struct nand_chip *nand)
{
	davinci_nand_init(nand);

	return 0;
}
#endif

#if defined(CONFIG_DRIVER_TI_EMAC)

#define PHY_SW_I2C_ADDR	0x5f /* Address of PHY on i2c bus */

/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
	u_int8_t mac_addr[6];
	u_int8_t switch_start_cmd[2] = { 0x01, 0x23 };
	struct eth_device *dev;

	/* Read Ethernet MAC address from EEPROM */
	if (dvevm_read_mac_address(mac_addr))
		/* set address env if not already set */
		davinci_sync_env_enetaddr(mac_addr);

	/* read the address back from env */
	if (!eth_getenv_enetaddr("ethaddr", mac_addr))
		return -1;

	/* enable the Ethernet switch in the 3 port PHY */
	if (i2c_write(PHY_SW_I2C_ADDR, 0, 0,
			switch_start_cmd, sizeof(switch_start_cmd))) {
		printf("Ethernet switch start failed!\n");
		return -1;
	}

	/* finally, initialise the driver */
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	dev = eth_get_dev();

	/* provide the resulting addr to the driver */
	memcpy(dev->enetaddr, mac_addr, 6);
	dev->write_hwaddr(dev);

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */
