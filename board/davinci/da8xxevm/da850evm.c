/*
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <hwconfig.h>

DECLARE_GLOBAL_DATA_PTR;

#define pinmux(x)	(&davinci_syscfg_regs->pinmux[x])

/* SPI0 pin muxer settings */
static const struct pinmux_config spi1_pins[] = {
	{ pinmux(5), 1, 1 },
	{ pinmux(5), 1, 2 },
	{ pinmux(5), 1, 4 },
	{ pinmux(5), 1, 5 }
};

/* UART pin muxer settings */
static const struct pinmux_config uart_pins[] = {
	{ pinmux(0), 4, 6 },
	{ pinmux(0), 4, 7 },
	{ pinmux(4), 2, 4 },
	{ pinmux(4), 2, 5 }
};

#ifdef CONFIG_DRIVER_TI_EMAC
static const struct pinmux_config emac_pins[] = {
#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
	{ pinmux(14), 8, 2 },
	{ pinmux(14), 8, 3 },
	{ pinmux(14), 8, 4 },
	{ pinmux(14), 8, 5 },
	{ pinmux(14), 8, 6 },
	{ pinmux(14), 8, 7 },
	{ pinmux(15), 8, 1 },
#else /* ! CONFIG_DRIVER_TI_EMAC_USE_RMII */
	{ pinmux(2), 8, 1 },
	{ pinmux(2), 8, 2 },
	{ pinmux(2), 8, 3 },
	{ pinmux(2), 8, 4 },
	{ pinmux(2), 8, 5 },
	{ pinmux(2), 8, 6 },
	{ pinmux(2), 8, 7 },
	{ pinmux(3), 8, 0 },
	{ pinmux(3), 8, 1 },
	{ pinmux(3), 8, 2 },
	{ pinmux(3), 8, 3 },
	{ pinmux(3), 8, 4 },
	{ pinmux(3), 8, 5 },
	{ pinmux(3), 8, 6 },
	{ pinmux(3), 8, 7 },
#endif /* CONFIG_DRIVER_TI_EMAC_USE_RMII */
	{ pinmux(4), 8, 0 },
	{ pinmux(4), 8, 1 }
};

/* I2C pin muxer settings */
static const struct pinmux_config i2c_pins[] = {
	{ pinmux(4), 2, 2 },
	{ pinmux(4), 2, 3 }
};

#ifdef CONFIG_NAND_DAVINCI
const struct pinmux_config nand_pins[] = {
	{ pinmux(7), 1, 1 },
	{ pinmux(7), 1, 2 },
	{ pinmux(7), 1, 4 },
	{ pinmux(7), 1, 5 },
	{ pinmux(9), 1, 0 },
	{ pinmux(9), 1, 1 },
	{ pinmux(9), 1, 2 },
	{ pinmux(9), 1, 3 },
	{ pinmux(9), 1, 4 },
	{ pinmux(9), 1, 5 },
	{ pinmux(9), 1, 6 },
	{ pinmux(9), 1, 7 },
	{ pinmux(12), 1, 5 },
	{ pinmux(12), 1, 6 }
};
#elif defined(CONFIG_USE_NOR)
/* NOR pin muxer settings */
const struct pinmux_config nor_pins[] = {
	{ pinmux(5), 1, 6 },
	{ pinmux(6), 1, 6 },
	{ pinmux(7), 1, 0 },
	{ pinmux(7), 1, 4 },
	{ pinmux(7), 1, 5 },
	{ pinmux(8), 1, 0 },
	{ pinmux(8), 1, 1 },
	{ pinmux(8), 1, 2 },
	{ pinmux(8), 1, 3 },
	{ pinmux(8), 1, 4 },
	{ pinmux(8), 1, 5 },
	{ pinmux(8), 1, 6 },
	{ pinmux(8), 1, 7 },
	{ pinmux(9), 1, 0 },
	{ pinmux(9), 1, 1 },
	{ pinmux(9), 1, 2 },
	{ pinmux(9), 1, 3 },
	{ pinmux(9), 1, 4 },
	{ pinmux(9), 1, 5 },
	{ pinmux(9), 1, 6 },
	{ pinmux(9), 1, 7 },
	{ pinmux(10), 1, 0 },
	{ pinmux(10), 1, 1 },
	{ pinmux(10), 1, 2 },
	{ pinmux(10), 1, 3 },
	{ pinmux(10), 1, 4 },
	{ pinmux(10), 1, 5 },
	{ pinmux(10), 1, 6 },
	{ pinmux(10), 1, 7 },
	{ pinmux(11), 1, 0 },
	{ pinmux(11), 1, 1 },
	{ pinmux(11), 1, 2 },
	{ pinmux(11), 1, 3 },
	{ pinmux(11), 1, 4 },
	{ pinmux(11), 1, 5 },
	{ pinmux(11), 1, 6 },
	{ pinmux(11), 1, 7 },
	{ pinmux(12), 1, 0 },
	{ pinmux(12), 1, 1 },
	{ pinmux(12), 1, 2 },
	{ pinmux(12), 1, 3 },
	{ pinmux(12), 1, 4 },
	{ pinmux(12), 1, 5 },
	{ pinmux(12), 1, 6 },
	{ pinmux(12), 1, 7 }
};
#endif

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
	memset(resetvect, 0, sizeof(unsigned) *10);

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

static const struct pinmux_resource pinmuxes[] = {
#ifdef CONFIG_SPI_FLASH
	PINMUX_ITEM(spi1_pins),
#endif
	PINMUX_ITEM(uart_pins),
	PINMUX_ITEM(i2c_pins),
#ifdef CONFIG_NAND_DAVINCI
	PINMUX_ITEM(nand_pins),
#elif defined(CONFIG_USE_NOR)
	PINMUX_ITEM(nor_pins),
#endif
};

static const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI1 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
};

#ifndef CONFIG_DA850_EVM_MAX_CPU_CLK
#define CONFIG_DA850_EVM_MAX_CPU_CLK	300000000
#endif

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

	return rev;
}

int board_init(void)
{
#ifndef CONFIG_USE_IRQ
	irq_init();
#endif


#ifdef CONFIG_NAND_DAVINCI
	/*
	 * NAND CS setup - cycle counts based on da850evm NAND timings in the
	 * Linux kernel @ 25MHz EMIFA
	 */
	writel((DAVINCI_ABCR_WSETUP(0) |
		DAVINCI_ABCR_WSTROBE(1) |
		DAVINCI_ABCR_WHOLD(0) |
		DAVINCI_ABCR_RSETUP(0) |
		DAVINCI_ABCR_RSTROBE(1) |
		DAVINCI_ABCR_RHOLD(0) |
		DAVINCI_ABCR_TA(1) |
		DAVINCI_ABCR_ASIZE_8BIT),
	       &davinci_emif_regs->ab2cr); /* CS3 */
#endif

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DA850_EVM;

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
		 DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART2),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

#ifdef CONFIG_DRIVER_TI_EMAC
	if (davinci_configure_pin_mux(emac_pins, ARRAY_SIZE(emac_pins)) != 0)
		return 1;

	davinci_emac_mii_mode_sel(HAS_RMII);
#endif /* CONFIG_DRIVER_TI_EMAC */

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC

#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
/**
 * rmii_hw_init
 *
 * DA850/OMAP-L138 EVM can interface to a daughter card for
 * additional features. This card has an I2C GPIO Expander TCA6416
 * to select the required functions like camera, RMII Ethernet,
 * character LCD, video.
 *
 * Initialization of the expander involves configuring the
 * polarity and direction of the ports. P07-P05 are used here.
 * These ports are connected to a Mux chip which enables only one
 * functionality at a time.
 *
 * For RMII phy to respond, the MII MDIO clock has to be  disabled
 * since both the PHY devices have address as zero. The MII MDIO
 * clock is controlled via GPIO2[6].
 *
 * This code is valid for Beta version of the hardware
 */
int rmii_hw_init(void)
{
	const struct pinmux_config gpio_pins[] = {
		{ pinmux(6), 8, 1 }
	};
	u_int8_t buf[2];
	unsigned int temp;
	int ret;

	/* PinMux for GPIO */
	if (davinci_configure_pin_mux(gpio_pins, ARRAY_SIZE(gpio_pins)) != 0)
		return 1;

	/* I2C Exapnder configuration */
	/* Set polarity to non-inverted */
	buf[0] = 0x0;
	buf[1] = 0x0;
	ret = i2c_write(CONFIG_SYS_I2C_EXPANDER_ADDR, 4, 1, buf, 2);
	if (ret) {
		printf("\nExpander @ 0x%02x write FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
		return ret;
	}

	/* Configure P07-P05 as outputs */
	buf[0] = 0x1f;
	buf[1] = 0xff;
	ret = i2c_write(CONFIG_SYS_I2C_EXPANDER_ADDR, 6, 1, buf, 2);
	if (ret) {
		printf("\nExpander @ 0x%02x write FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
	}

	/* For Ethernet RMII selection
	 * P07(SelA)=0
	 * P06(SelB)=1
	 * P05(SelC)=1
	 */
	if (i2c_read(CONFIG_SYS_I2C_EXPANDER_ADDR, 2, 1, buf, 1)) {
		printf("\nExpander @ 0x%02x read FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
	}

	buf[0] &= 0x1f;
	buf[0] |= (0 << 7) | (1 << 6) | (1 << 5);
	if (i2c_write(CONFIG_SYS_I2C_EXPANDER_ADDR, 2, 1, buf, 1)) {
		printf("\nExpander @ 0x%02x write FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
	}

	/* Set the output as high */
	temp = REG(GPIO_BANK2_REG_SET_ADDR);
	temp |= (0x01 << 6);
	REG(GPIO_BANK2_REG_SET_ADDR) = temp;

	/* Set the GPIO direction as output */
	temp = REG(GPIO_BANK2_REG_DIR_ADDR);
	temp &= ~(0x01 << 6);
	REG(GPIO_BANK2_REG_DIR_ADDR) = temp;

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC_USE_RMII */

/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
	/* Select RMII fucntion through the expander */
	if (rmii_hw_init())
		printf("RMII hardware init failed!!!\n");
#endif
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */
