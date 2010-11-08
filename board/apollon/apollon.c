/*
 * (C) Copyright 2005-2007
 * Samsung Electronics.
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Derived from omap2420
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <netdev.h>
#include <asm/arch/omap2420.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/mem.h>
#include <asm/mach-types.h>

void wait_for_command_complete(unsigned int wd_base);

DECLARE_GLOBAL_DATA_PTR;

#define write_config_reg(reg, value)					\
do {									\
	writeb(value, reg);						\
} while (0)

#define mask_config_reg(reg, mask)					\
do {									\
	char value = readb(reg) & ~(mask);				\
	writeb(value, reg);						\
} while (0)

/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops)
{
	__asm__("1:\n" "subs %0, %1, #1\n"
		  "bne 1b":"=r" (loops):"0"(loops));
}

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init(void)
{
	gpmc_init();		/* in SRAM or SDRM, finish GPMC */

	gd->bd->bi_arch_number = 919;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = (OMAP2420_SDRC_CS0 + 0x100);

	return 0;
}

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called path is with sram stack.
 **********************************************************/
void s_init(void)
{
	watchdog_init();
	set_muxconf_regs();
	delay(100);

	peripheral_enable();
	icache_enable();
}

/*******************************************************
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 ********************************************************/
int misc_init_r(void)
{
	return (0);
}

/****************************************
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 *****************************************/
void watchdog_init(void)
{
	/* There are 4 watch dogs.  1 secure, and 3 general purpose.
	 * The ROM takes care of the secure one. Of the 3 GP ones,
	 * 1 can reset us directly, the other 2 only generate MPU interrupts.
	 */
	__raw_writel(WD_UNLOCK1, WD2_BASE + WSPR);
	wait_for_command_complete(WD2_BASE);
	__raw_writel(WD_UNLOCK2, WD2_BASE + WSPR);

#define MPU_WD_CLOCKED 1
#if MPU_WD_CLOCKED
	/* value 0x10 stick on aptix, BIT4 polarity seems oppsite */
	__raw_writel(WD_UNLOCK1, WD3_BASE + WSPR);
	wait_for_command_complete(WD3_BASE);
	__raw_writel(WD_UNLOCK2, WD3_BASE + WSPR);

	__raw_writel(WD_UNLOCK1, WD4_BASE + WSPR);
	wait_for_command_complete(WD4_BASE);
	__raw_writel(WD_UNLOCK2, WD4_BASE + WSPR);
#endif
}

/******************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 ******************************************************/
void wait_for_command_complete(unsigned int wd_base)
{
	int pending = 1;
	do {
		pending = __raw_readl(wd_base + WWPS);
	} while (pending);
}

/*******************************************************************
 * Routine:board_eth_init
 * Description: take the Ethernet controller out of reset and wait
 *		   for the EEPROM load to complete.
 ******************************************************************/
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_LAN91C96
	int cnt = 20;

	__raw_writeb(0x03, OMAP2420_CTRL_BASE + 0x0f2);	/*protect->gpio74 */

	__raw_writew(0x0, LAN_RESET_REGISTER);
	do {
		__raw_writew(0x1, LAN_RESET_REGISTER);
		udelay(100);
		if (cnt == 0)
			goto eth_reset_err_out;
		--cnt;
	} while (__raw_readw(LAN_RESET_REGISTER) != 0x1);

	cnt = 20;

	do {
		__raw_writew(0x0, LAN_RESET_REGISTER);
		udelay(100);
		if (cnt == 0)
			goto eth_reset_err_out;
		--cnt;
	} while (__raw_readw(LAN_RESET_REGISTER) != 0x0000);
	udelay(1000);

	mask_config_reg(ETH_CONTROL_REG, 0x01);
	udelay(1000);
	rc = lan91c96_initialize(0, CONFIG_LAN91C96_BASE);
eth_reset_err_out:
#endif
	return rc;
}

/**********************************************
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 **********************************************/
int dram_init(void)
{
	unsigned int size0 = 0, size1 = 0;
	u32 mtype, btype, rev = 0, cpu = 0;
#define NOT_EARLY 0

	btype = get_board_type();
	mtype = get_mem_type();
	rev = get_cpu_rev();
	cpu = get_cpu_type();

	display_board_info(btype);

	if ((mtype == DDR_COMBO) || (mtype == DDR_STACKED)) {
		/* init other chip select */
		do_sdrc_init(SDRC_CS1_OSET, NOT_EARLY);
	}

	size0 = get_sdr_cs_size(SDRC_CS0_OSET);
	size1 = get_sdr_cs_size(SDRC_CS1_OSET);

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = size0;
#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = PHYS_SDRAM_1 + size0;
	gd->bd->bi_dram[1].size = size1;
#endif

	return 0;
}

/**********************************************************
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers
 *              specific to the hardware
 *********************************************************/
void set_muxconf_regs(void)
{
	muxSetupSDRC();
	muxSetupGPMC();
	muxSetupUsb0();		/* USB Device */
	muxSetupUsbHost();	/* USB Host */
	muxSetupUART1();
	muxSetupLCD();
	muxSetupMMCSD();
	muxSetupTouchScreen();
}

/*****************************************************************
 * Routine: peripheral_enable
 * Description: Enable the clks & power for perifs (GPT2, UART1,...)
 ******************************************************************/
void peripheral_enable(void)
{
	unsigned int v, if_clks = 0, func_clks = 0;

	/* Enable GP2 timer. */
	if_clks |= BIT4 | BIT3;
	func_clks |= BIT4 | BIT3;
	/* Sys_clk input OMAP2420_GPT2 */
	v = __raw_readl(CM_CLKSEL2_CORE) | 0x4 | 0x2;
	__raw_writel(v, CM_CLKSEL2_CORE);
	__raw_writel(0x1, CM_CLKSEL_WKUP);

#ifdef CONFIG_SYS_NS16550
	/* Enable UART1 clock */
	func_clks |= BIT21;
	if_clks |= BIT21;
#endif
	/* Interface clocks on */
	v = __raw_readl(CM_ICLKEN1_CORE) | if_clks;
	__raw_writel(v, CM_ICLKEN1_CORE);
	/* Functional Clocks on */
	v = __raw_readl(CM_FCLKEN1_CORE) | func_clks;
	__raw_writel(v, CM_FCLKEN1_CORE);
	delay(1000);

#ifndef KERNEL_UPDATED
	{
#define V1 0xffffffff
#define V2 0x00000007

		__raw_writel(V1, CM_FCLKEN1_CORE);
		__raw_writel(V2, CM_FCLKEN2_CORE);
		__raw_writel(V1, CM_ICLKEN1_CORE);
		__raw_writel(V1, CM_ICLKEN2_CORE);
	}
#endif
}

/****************************************
 * Routine: muxSetupUsb0   (ostboot)
 * Description: Setup usb muxing
 *****************************************/
void muxSetupUsb0(void)
{
	mask_config_reg(CONTROL_PADCONF_USB0_PUEN, 0x1f);
	mask_config_reg(CONTROL_PADCONF_USB0_VP, 0x1f);
	mask_config_reg(CONTROL_PADCONF_USB0_VM, 0x1f);
	mask_config_reg(CONTROL_PADCONF_USB0_RCV, 0x1f);
	mask_config_reg(CONTROL_PADCONF_USB0_TXEN, 0x1f);
	mask_config_reg(CONTROL_PADCONF_USB0_SE0, 0x1f);
	mask_config_reg(CONTROL_PADCONF_USB0_DAT, 0x1f);
}

/****************************************
 * Routine: muxSetupUSBHost   (ostboot)
 * Description: Setup USB Host muxing
 *****************************************/
void muxSetupUsbHost(void)
{
	/* V19 */
	write_config_reg(CONTROL_PADCONF_USB1_RCV, 1);
	/* W20 */
	write_config_reg(CONTROL_PADCONF_USB1_TXEN, 1);
	/* N14 */
	write_config_reg(CONTROL_PADCONF_GPIO69, 3);
	/* P15 */
	write_config_reg(CONTROL_PADCONF_GPIO70, 3);
	/* L18 */
	write_config_reg(CONTROL_PADCONF_GPIO102, 3);
	/* L19 */
	write_config_reg(CONTROL_PADCONF_GPIO103, 3);
	/* K15 */
	write_config_reg(CONTROL_PADCONF_GPIO104, 3);
	/* K14 */
	write_config_reg(CONTROL_PADCONF_GPIO105, 3);
}

/****************************************
 * Routine: muxSetupUART1  (ostboot)
 * Description: Set up uart1 muxing
 *****************************************/
void muxSetupUART1(void)
{
	/* UART1_CTS pin configuration, PIN = D21, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_UART1_CTS, 0);
	/* UART1_RTS pin configuration, PIN = H21, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_UART1_RTS, 0);
	/* UART1_TX pin configuration, PIN = L20, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_UART1_TX, 0);
	/* UART1_RX pin configuration, PIN = T21, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_UART1_RX, 0);
}

/****************************************
 * Routine: muxSetupLCD   (ostboot)
 * Description: Setup lcd muxing
 *****************************************/
void muxSetupLCD(void)
{
	/* LCD_D0 pin configuration, PIN = Y7, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D0, 0);
	/* LCD_D1 pin configuration, PIN = P10 , Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D1, 0);
	/* LCD_D2 pin configuration, PIN = V8, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D2, 0);
	/* LCD_D3 pin configuration, PIN = Y8, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D3, 0);
	/* LCD_D4 pin configuration, PIN = W8, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D4, 0);
	/* LCD_D5 pin configuration, PIN = R10, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D5, 0);
	/* LCD_D6 pin configuration, PIN = Y9, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D6, 0);
	/* LCD_D7 pin configuration, PIN = V9, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D7, 0);
	/* LCD_D8 pin configuration, PIN = W9, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D8, 0);
	/* LCD_D9 pin configuration, PIN = P11, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D9, 0);
	/* LCD_D10 pin configuration, PIN = V10, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D10, 0);
	/* LCD_D11 pin configuration, PIN = Y10, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D11, 0);
	/* LCD_D12 pin configuration, PIN = W10, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D12, 0);
	/* LCD_D13 pin configuration, PIN = R11, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D13, 0);
	/* LCD_D14 pin configuration, PIN = V11, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D14, 0);
	/* LCD_D15 pin configuration, PIN = W11, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D15, 0);
	/* LCD_D16 pin configuration, PIN = P12, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D16, 0);
	/* LCD_D17 pin configuration, PIN = R12, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_D17, 0);
	/* LCD_PCLK pin configuration, PIN = W6, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_PCLK, 0);
	/* LCD_VSYNC pin configuration, PIN = V7, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_VSYNC, 0);
	/* LCD_HSYNC pin configuration, PIN = Y6, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_HSYNC, 0);
	/* LCD_ACBIAS pin configuration, PIN = W7, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_DSS_ACBIAS, 0);
}

/****************************************
 * Routine: muxSetupMMCSD (ostboot)
 * Description: set up MMC muxing
 *****************************************/
void muxSetupMMCSD(void)
{
	/* SDMMC_CLKI pin configuration,  PIN = H15, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_CLKI, 0);
	/* SDMMC_CLKO pin configuration,  PIN = G19, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_CLKO, 0);
	/* SDMMC_CMD pin configuration,   PIN = H18, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_CMD, 0);
	/* SDMMC_DAT0 pin configuration,  PIN = F20, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT0, 0);
	/* SDMMC_DAT1 pin configuration,  PIN = H14, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT1, 0);
	/* SDMMC_DAT2 pin configuration,  PIN = E19, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT2, 0);
	/* SDMMC_DAT3 pin configuration,  PIN = D19, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT3, 0);
	/* SDMMC_DDIR0 pin configuration, PIN = F19, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT_DIR0, 0);
	/* SDMMC_DDIR1 pin configuration, PIN = E20, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT_DIR1, 0);
	/* SDMMC_DDIR2 pin configuration, PIN = F18, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT_DIR2, 0);
	/* SDMMC_DDIR3 pin configuration, PIN = E18, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_DAT_DIR3, 0);
	/* SDMMC_CDIR pin configuration,  PIN = G18, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_MMC_CMD_DIR, 0);
}

/******************************************
 * Routine: muxSetupTouchScreen (ostboot)
 * Description:  Set up touch screen muxing
 *******************************************/
void muxSetupTouchScreen(void)
{
	/* SPI1_CLK pin configuration,  PIN = U18, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_SPI1_CLK, 0);
	/* SPI1_MOSI pin configuration, PIN = V20, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_SPI1_SIMO, 0);
	/* SPI1_MISO pin configuration, PIN = T18, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_SPI1_SOMI, 0);
	/* SPI1_nCS0 pin configuration, PIN = U19, Mode = 0, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_SPI1_NCS0, 0);
#define CONTROL_PADCONF_GPIO85	CONTROL_PADCONF_SPI1_NCS1
	/* PEN_IRQ pin configuration,   PIN = N15, Mode = 3, PUPD=Disabled */
	write_config_reg(CONTROL_PADCONF_GPIO85, 3);
}

/***************************************************************
 * Routine: muxSetupGPMC (ostboot)
 * Description: Configures balls which cam up in protected mode
 ***************************************************************/
void muxSetupGPMC(void)
{
	/* gpmc_io_dir, MCR */
	volatile unsigned int *MCR = (unsigned int *) 0x4800008C;
	*MCR = 0x19000000;

	/* NOR FLASH CS0 */
	/* signal - Gpmc_clk; pin - J4; offset - 0x0088; mode 0; Byte-3 */
	write_config_reg(CONTROL_PADCONF_GPMC_D2_BYTE3, 0);
	/* MPDB(Multi Port Debug Port) CS1 */
	/* signal - gpmc_ncs1; pin - N8; offset - 0x008D; mode 0; Byte-1 */
	write_config_reg(CONTROL_PADCONF_GPMC_NCS0_BYTE1, 0);
	/* signal - Gpmc_ncs2; pin - E2; offset - 0x008E; mode 0; Byte-2 */
	write_config_reg(CONTROL_PADCONF_GPMC_NCS0_BYTE2, 0);
	/* signal - Gpmc_ncs3; pin - N2; offset - 0x008F; mode 0; Byte-3 */
	write_config_reg(CONTROL_PADCONF_GPMC_NCS0_BYTE3, 0);
	/* signal - Gpmc_ncs4; pin - ??; offset - 0x0090; mode 0; Byte-4 */
	write_config_reg(CONTROL_PADCONF_GPMC_NCS0_BYTE4, 0);
	/* signal - Gpmc_ncs5; pin - ??; offset - 0x0091; mode 0; Byte-5 */
	write_config_reg(CONTROL_PADCONF_GPMC_NCS0_BYTE5, 0);
	/* signal - Gpmc_ncs6; pin - ??; offset - 0x0092; mode 0; Byte-6 */
	write_config_reg(CONTROL_PADCONF_GPMC_NCS0_BYTE6, 0);
	/* signal - Gpmc_ncs7; pin - ??; offset - 0x0093; mode 0; Byte-7 */
	write_config_reg(CONTROL_PADCONF_GPMC_NCS0_BYTE7, 0);
}

/****************************************************************
 * Routine: muxSetupSDRC  (ostboot)
 * Description: Configures balls which come up in protected mode
 ****************************************************************/
void muxSetupSDRC(void)
{
	/* It's set by IPL */
}
