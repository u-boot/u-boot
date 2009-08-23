/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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
#include <asm/arch/omap2420.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/mem.h>
#include <i2c.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

void wait_for_command_complete(unsigned int wd_base);

/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init (void)
{
	gpmc_init(); /* in SRAM or SDRM, finish GPMC */

	gd->bd->bi_arch_number = MACH_TYPE_OMAP_H4;		/* board id for linux */
	gd->bd->bi_boot_params = (OMAP2420_SDRC_CS0+0x100);	/* adress of boot parameters */

	return 0;
}

/**********************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP type, unlock the SRAM for
 *  general use.
 ***********************************************************/
void try_unlock_sram(void)
{
	/* if GP device unlock device SRAM for general use */
	if (get_device_type() == GP_DEVICE) {
		__raw_writel(0xFF, A_REQINFOPERM0);
		__raw_writel(0xCFDE, A_READPERM0);
		__raw_writel(0xCFDE, A_WRITEPERM0);
	}
}

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called path is with sram stack.
 **********************************************************/
void s_init(void)
{
	int in_sdram = running_in_sdram();

	watchdog_init();
	set_muxconf_regs();
	delay(100);
	try_unlock_sram();

	if(!in_sdram)
		prcm_init();

	peripheral_enable();
	icache_enable();
	if (!in_sdram)
		sdrc_init();
}

/*******************************************************
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 ********************************************************/
int misc_init_r (void)
{
	ether_init(); /* better done here so timers are init'ed */
	return(0);
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
	__raw_writel(WD_UNLOCK1 ,WD2_BASE+WSPR);
	wait_for_command_complete(WD2_BASE);
	__raw_writel(WD_UNLOCK2 ,WD2_BASE+WSPR);

#if MPU_WD_CLOCKED /* value 0x10 stick on aptix, BIT4 polarity seems oppsite*/
	__raw_writel(WD_UNLOCK1 ,WD3_BASE+WSPR);
	wait_for_command_complete(WD3_BASE);
	__raw_writel(WD_UNLOCK2 ,WD3_BASE+WSPR);

	__raw_writel(WD_UNLOCK1 ,WD4_BASE+WSPR);
	wait_for_command_complete(WD4_BASE);
	__raw_writel(WD_UNLOCK2 ,WD4_BASE+WSPR);
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
		pending = __raw_readl(wd_base+WWPS);
	} while (pending);
}

/*******************************************************************
 * Routine:ether_init
 * Description: take the Ethernet controller out of reset and wait
 *		   for the EEPROM load to complete.
 ******************************************************************/
void ether_init (void)
{
#ifdef CONFIG_DRIVER_LAN91C96
	int cnt = 20;

	__raw_writeb(0x3,OMAP2420_CTRL_BASE+0x10a); /*protect->gpio95 */

	__raw_writew(0x0, LAN_RESET_REGISTER);
	do {
		__raw_writew(0x1, LAN_RESET_REGISTER);
		udelay (100);
		if (cnt == 0)
			goto h4reset_err_out;
		--cnt;
	} while (__raw_readw(LAN_RESET_REGISTER) != 0x1);

	cnt = 20;

	do {
		__raw_writew(0x0, LAN_RESET_REGISTER);
		udelay (100);
		if (cnt == 0)
			goto h4reset_err_out;
		--cnt;
	} while (__raw_readw(LAN_RESET_REGISTER) != 0x0000);
	udelay (1000);

	*((volatile unsigned char *) ETH_CONTROL_REG) &= ~0x01;
	udelay (1000);

	h4reset_err_out:
	return;
#endif
}

/**********************************************
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 **********************************************/
int dram_init (void)
{
	unsigned int size0=0,size1=0;
	u32 mtype, btype, rev, cpu;
	u8 chg_on = 0x5; /* enable charge of back up battery */
	u8 vmode_on = 0x8C;
	#define NOT_EARLY 0

	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE); /* need this a bit early */

	btype = get_board_type();
	mtype = get_mem_type();
	rev = get_cpu_rev();
	cpu = get_cpu_type();

	display_board_info(btype);
	if (btype == BOARD_H4_MENELAUS){
		update_mux(btype,mtype); /* combo part on menelaus */
		i2c_write(I2C_MENELAUS, 0x20, 1, &chg_on, 1); /*fix POR reset bug */
		i2c_write(I2C_MENELAUS, 0x2, 1, &vmode_on, 1); /* VCORE change on VMODE */
	}

	if ((mtype == DDR_COMBO) || (mtype == DDR_STACKED)) {
		do_sdrc_init(SDRC_CS1_OSET, NOT_EARLY);	/* init other chip select */
	}
	size0 = get_sdr_cs_size(SDRC_CS0_OSET);
	size1 = get_sdr_cs_size(SDRC_CS1_OSET);

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = size0;
	if(rev == CPU_2420_2422_ES1) /* ES1's 128MB remap granularity isn't worth doing */
		gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	else /* ES2 and above can remap at 32MB granularity */
		gd->bd->bi_dram[1].start = PHYS_SDRAM_1+size0;
	gd->bd->bi_dram[1].size = size1;

	return 0;
}

/**********************************************************
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers
 *              specific to the hardware
 *********************************************************/
void set_muxconf_regs (void)
{
	muxSetupSDRC();
	muxSetupGPMC();
	muxSetupUsb0();
	muxSetupUart3();
	muxSetupI2C1();
	muxSetupUART1();
	muxSetupLCD();
	muxSetupCamera();
	muxSetupMMCSD();
	muxSetupTouchScreen();
	muxSetupHDQ();
}

/*****************************************************************
 * Routine: peripheral_enable
 * Description: Enable the clks & power for perifs (GPT2, UART1,...)
 ******************************************************************/
void peripheral_enable(void)
{
	unsigned int v, if_clks=0, func_clks=0;

	/* Enable GP2 timer.*/
	if_clks |= BIT4;
	func_clks |= BIT4;
	v = __raw_readl(CM_CLKSEL2_CORE) | 0x4;	/* Sys_clk input OMAP2420_GPT2 */
	__raw_writel(v, CM_CLKSEL2_CORE);
	__raw_writel(0x1, CM_CLKSEL_WKUP);

#ifdef CONFIG_SYS_NS16550
	/* Enable UART1 clock */
	func_clks |= BIT21;
	if_clks |= BIT21;
#endif
	v = __raw_readl(CM_ICLKEN1_CORE) | if_clks;	/* Interface clocks on */
	__raw_writel(v,CM_ICLKEN1_CORE );
	v = __raw_readl(CM_FCLKEN1_CORE) | func_clks; /* Functional Clocks on */
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
	volatile uint8   *MuxConfigReg;
	volatile uint32  *otgCtrlReg;

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_USB0_PUEN;
	*MuxConfigReg &= (uint8)(~0x1F);

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_USB0_VP;
	*MuxConfigReg &= (uint8)(~0x1F);

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_USB0_VM;
	*MuxConfigReg &= (uint8)(~0x1F);

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_USB0_RCV;
	*MuxConfigReg &= (uint8)(~0x1F);

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_USB0_TXEN;
	*MuxConfigReg &= (uint8)(~0x1F);

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_USB0_SE0;
	*MuxConfigReg &= (uint8)(~0x1F);

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_USB0_DAT;
	*MuxConfigReg &= (uint8)(~0x1F);

	/* setup for USB VBus detection */
	otgCtrlReg = (volatile uint32 *)USB_OTG_CTRL;
	*otgCtrlReg |= 0x00040000; /* bit 18 */
}

/****************************************
 * Routine: muxSetupUart3   (ostboot)
 * Description: Setup uart3 muxing
 *****************************************/
void muxSetupUart3(void)
{
	volatile uint8 *MuxConfigReg;

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_UART3_TX_IRTX;
	*MuxConfigReg &= (uint8)(~0x1F);

	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_UART3_RX_IRRX;
	*MuxConfigReg &= (uint8)(~0x1F);
}

/****************************************
 * Routine: muxSetupI2C1   (ostboot)
 * Description: Setup i2c muxing
 *****************************************/
void muxSetupI2C1(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* I2C1 Clock pin configuration, PIN = M19 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_I2C1_SCL;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* I2C1 Data pin configuration, PIN = L15 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_I2C1_SDA;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* Pull-up required on data line */
	/* external pull-up already present. */
	/* *MuxConfigReg |= 0x18 ;*/ /* Mode = 0, PullTypeSel=PU, PullUDEnable=Enabled */
}

/****************************************
 * Routine: muxSetupUART1  (ostboot)
 * Description: Set up uart1 muxing
 *****************************************/
void muxSetupUART1(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* UART1_CTS pin configuration, PIN = D21 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_CTS;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* UART1_RTS pin configuration, PIN = H21 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_RTS;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* UART1_TX pin configuration, PIN = L20 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_TX;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* UART1_RX pin configuration, PIN = T21 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_UART1_RX;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
}

/****************************************
 * Routine: muxSetupLCD   (ostboot)
 * Description: Setup lcd muxing
 *****************************************/
void muxSetupLCD(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* LCD_D0 pin configuration, PIN = Y7  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D0;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D1 pin configuration, PIN = P10 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D1;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D2 pin configuration, PIN = V8  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D2;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D3 pin configuration, PIN = Y8  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D3;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D4 pin configuration, PIN = W8  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D4;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D5 pin configuration, PIN = R10 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D5;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D6 pin configuration, PIN = Y9  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D6;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D7 pin configuration, PIN = V9  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D7;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D8 pin configuration, PIN = W9  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D8;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D9 pin configuration, PIN = P11 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D9;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D10 pin configuration, PIN = V10 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D10;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D11 pin configuration, PIN = Y10 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D11;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D12 pin configuration, PIN = W10 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D12;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D13 pin configuration, PIN = R11 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D13;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D14 pin configuration, PIN = V11 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D14;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D15 pin configuration, PIN = W11 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D15;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D16 pin configuration, PIN = P12 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D16;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_D17 pin configuration, PIN = R12 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_D17;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_PCLK pin configuration,   PIN = W6   */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_PCLK;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_VSYNC pin configuration,  PIN = V7  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_VSYNC;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_HSYNC pin configuration,  PIN = Y6  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_HSYNC;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* LCD_ACBIAS pin configuration, PIN = W7 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_DSS_ACBIAS;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
}

/****************************************
 * Routine: muxSetupCamera  (ostboot)
 * Description: Setup camera muxing
 *****************************************/
void muxSetupCamera(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* CAMERA_RSTZ  pin configuration, PIN = Y16 */
	/* CAM_RST is connected through the I2C IO expander.*/
	/* MuxConfigReg = (volatile unsigned char *), CONTROL_PADCONF_SYS_NRESWARM*/
	/* *MuxConfigReg = 0x00 ; / * Mode = 0, PUPD=Disabled   */

	/* CAMERA_XCLK  pin configuration, PIN = U3 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_XCLK;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_LCLK  pin configuration, PIN = V5 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_LCLK;
	*MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_VSYNC pin configuration, PIN = U2 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_VS,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_HSYNC pin configuration, PIN = T3 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_HS,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT0 pin configuration, PIN = T4 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D0,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT1 pin configuration, PIN = V2 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D1,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT2 pin configuration, PIN = V3 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D2,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT3 pin configuration, PIN = U4 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D3,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT4 pin configuration, PIN = W2 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D4,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT5 pin configuration, PIN = V4 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D5,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT6 pin configuration, PIN = W3 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D6,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT7 pin configuration, PIN = Y2 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D7,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT8 pin configuration, PIN = Y4 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D8,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* CAMERA_DAT9 pin configuration, PIN = V6 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_CAM_D9,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
}

/****************************************
 * Routine: muxSetupMMCSD (ostboot)
 * Description: set up MMC muxing
 *****************************************/
void muxSetupMMCSD(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* SDMMC_CLKI pin configuration,  PIN = H15 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_CLKI,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SDMMC_CLKO pin configuration,  PIN = G19 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_CLKO,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SDMMC_CMD pin configuration,   PIN = H18 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_CMD,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
	/* External pull-ups are present. */
	/* *MuxConfigReg |= 0x18 ; #/ PullUDEnable=Enabled, PullTypeSel=PU */

	/* SDMMC_DAT0 pin configuration,  PIN = F20 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT0,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
	/* External pull-ups are present. */
	/* *MuxConfigReg |= 0x18 ; #/ PullUDEnable=Enabled, PullTypeSel=PU */

	/* SDMMC_DAT1 pin configuration,  PIN = H14 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT1,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
	/* External pull-ups are present. */
	/* *MuxConfigReg |= 0x18 ; #/ PullUDEnable=Enabled, PullTypeSel=PU */

	/* SDMMC_DAT2 pin configuration,  PIN = E19 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT2,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
	/* External pull-ups are present. */
	/* *MuxConfigReg |= 0x18 ; #/ PullUDEnable=Enabled, PullTypeSel=PU */

	/* SDMMC_DAT3 pin configuration,  PIN = D19 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT3,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
	/* External pull-ups are present. */
	/* *MuxConfigReg |= 0x18 ; #/ PullUDEnable=Enabled, PullTypeSel=PU */

	/* SDMMC_DDIR0 pin configuration, PIN = F19 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT_DIR0,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SDMMC_DDIR1 pin configuration, PIN = E20 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT_DIR1,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SDMMC_DDIR2 pin configuration, PIN = F18 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT_DIR2,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SDMMC_DDIR3 pin configuration, PIN = E18 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_DAT_DIR3,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SDMMC_CDIR pin configuration,  PIN = G18 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MMC_CMD_DIR,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* MMC_CD pin configuration,      PIN = B3  ---2420IP ONLY---*/
	/* MMC_CD for 2422IP=K1 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_SDRC_A14,
				   *MuxConfigReg = 0x03 ; /* Mode = 3, PUPD=Disabled */

	/* MMC_WP pin configuration,      PIN = B4  */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_SDRC_A13,
				   *MuxConfigReg = 0x03 ; /* Mode = 3, PUPD=Disabled */
}

/******************************************
 * Routine: muxSetupTouchScreen (ostboot)
 * Description:  Set up touch screen muxing
 *******************************************/
void muxSetupTouchScreen(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* SPI1_CLK pin configuration,  PIN = U18 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_SPI1_CLK,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SPI1_MOSI pin configuration, PIN = V20 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_SPI1_SIMO,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SPI1_MISO pin configuration, PIN = T18 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_SPI1_SOMI,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* SPI1_nCS0 pin configuration, PIN = U19 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_SPI1_NCS0,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */

	/* PEN_IRQ pin configuration,   PIN = P20 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_MCBSP1_FSR,
				   *MuxConfigReg = 0x03 ; /* Mode = 3, PUPD=Disabled */
}

/****************************************
 * Routine: muxSetupHDQ (ostboot)
 * Description: setup 1wire mux
 *****************************************/
void muxSetupHDQ(void)
{
	volatile unsigned char  *MuxConfigReg;

	/* HDQ_SIO pin configuration,  PIN = N18 */
	MuxConfigReg = (volatile unsigned char *)CONTROL_PADCONF_HDQ_SIO,
				   *MuxConfigReg = 0x00 ; /* Mode = 0, PUPD=Disabled */
}

/***************************************************************
 * Routine: muxSetupGPMC (ostboot)
 * Description: Configures balls which cam up in protected mode
 ***************************************************************/
void muxSetupGPMC(void)
{
	volatile uint8 *MuxConfigReg;
	volatile unsigned int *MCR = (volatile unsigned int *)0x4800008C;

	/* gpmc_io_dir */
	*MCR = 0x19000000;

	/* NOR FLASH CS0 */
	/* signal - Gpmc_clk; pin - J4; offset - 0x0088; mode - 0; Byte-3	Pull/up - N/A */
	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_GPMC_D2_BYTE3,
				   *MuxConfigReg = 0x00 ;

	/* signal - Gpmc_iodir; pin - n2; offset - 0x008C; mode - 1; Byte-3	Pull/up - N/A */
	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_GPMC_NCS0_BYTE3,
				   *MuxConfigReg = 0x01 ;

	/* MPDB(Multi Port Debug Port) CS1 */
	/* signal - gpmc_ncs1; pin - N8; offset - 0x008C; mode - 0; Byte-1	Pull/up - N/A */
	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_GPMC_NCS0_BYTE1,
				   *MuxConfigReg = 0x00 ;

	/* signal - Gpmc_ncs2; pin - E2; offset - 0x008C; mode - 0; Byte-2	Pull/up - N/A */
	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_GPMC_NCS0_BYTE2,
				   *MuxConfigReg = 0x00 ;
}

/****************************************************************
 * Routine: muxSetupSDRC  (ostboot)
 * Description: Configures balls which come up in protected mode
 ****************************************************************/
void muxSetupSDRC(void)
{
	volatile uint8 *MuxConfigReg;

	/* signal - sdrc_ncs1; pin - C12; offset - 0x00A0; mode - 0; Byte-1	Pull/up - N/A */
	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_SDRC_NCS0_BYTE1,
				   *MuxConfigReg = 0x00 ;

	/* signal - sdrc_a12; pin - D11; offset - 0x0030; mode - 0; Byte-2	Pull/up - N/A */
	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_SDRC_A14_BYTE2,
				   *MuxConfigReg = 0x00 ;

	/* signal - sdrc_cke1; pin - B13; offset - 0x00A0; mode - 0; Byte-3	Pull/up - N/A */
	MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_SDRC_NCS0_BYTE3,
				   *MuxConfigReg = 0x00;

	if (get_cpu_type() == CPU_2422) {
		MuxConfigReg = (volatile uint8 *)CONTROL_PADCONF_SDRC_A14_BYTE0,
					   *MuxConfigReg = 0x1b;
	}
}

/*****************************************************************************
 * Routine: update_mux()
 * Description: Update balls which are different beween boards.  All should be
 *              updated to match functionaly.  However, I'm only updating ones
 *              which I'll be using for now.  When power comes into play they
 *              all need updating.
 *****************************************************************************/
void update_mux(u32 btype,u32 mtype)
{
	u32 cpu, base = OMAP2420_CTRL_BASE;
	cpu = get_cpu_type();

	if (btype == BOARD_H4_MENELAUS) {
		if (cpu == CPU_2420) {
			/* PIN = B3,  GPIO.0->KBR5,      mode 3,  (pun?),-DO-*/
			__raw_writeb(0x3, base+0x30);
			/* PIN = B13, GPIO.38->KBC6,     mode 3,  (pun?)-DO-*/
			__raw_writeb(0x3, base+0xa3);
			/* PIN = F1, GPIO.25->HSUSBxx    mode 3,  (for external HS USB)*/
			/* PIN = H1, GPIO.26->HSUSBxx    mode 3,  (for external HS USB)*/
			/* PIN = K1, GPMC_ncs6           mode 0,  (on board nand access)*/
			/* PIN = L2, GPMC_ncs67          mode 0,  (for external HS USB)*/
			/* PIN = M1 (HSUSBOTG) */
			/* PIN = P1, GPIO.35->MEN_POK    mode 3,  (menelaus powerok)-DO-*/
			__raw_writeb(0x3, base+0x9d);
			/* PIN = U32, (WLAN_CLKREQ) */
			/* PIN = Y11, WLAN */
			/* PIN = AA4, GPIO.15->KBC2,     mode 3,  -DO- */
			__raw_writeb(0x3, base+0xe7);
			/* PIN = AA8, mDOC */
			/* PIN = AA10, BT */
			/* PIN = AA13, WLAN */
			/* PIN = M18 GPIO.96->MMC2_WP    mode 3   -DO- */
			__raw_writeb(0x3, base+0x10e);
			/* PIN = N19 GPIO.98->WLAN_INT   mode 3   -DO- */
			__raw_writeb(0x3, base+0x110);
			/* PIN = J15 HHUSB */
			/* PIN = H19 HSUSB */
			/* PIN = W13, P13, R13, W16 ... */
			/* PIN = V12 GPIO.25->I2C_CAMEN  mode 3   -DO- */
			__raw_writeb(0x3, base+0xde);
			/* PIN = W19 sys_nirq->MENELAUS_INT mode 0 -DO- */
			__raw_writeb(0x0, base+0x12c);
			/* PIN = AA17->sys_clkreq        mode 0   -DO- */
			__raw_writeb(0x0, base+0x136);
		} else if (cpu == CPU_2422) {
			/* PIN = B3,  GPIO.0->nc,        mode 3,  set above (pun?)*/
			/* PIN = B13, GPIO.cke1->nc,     mode 0,  set above, (pun?)*/
			/* PIN = F1, GPIO.25->HSUSBxx    mode 3,  (for external HS USB)*/
			/* PIN = H1, GPIO.26->HSUSBxx    mode 3,  (for external HS USB)*/
			/* PIN = K1, GPMC_ncs6           mode 0,  (on board nand access)*/
			__raw_writeb(0x0, base+0x92);
			/* PIN = L2, GPMC_ncs67          mode 0,  (for external HS USB)*/
			/* PIN = M1 (HSUSBOTG) */
			/* PIN = P1, GPIO.35->MEN_POK    mode 3,  (menelaus powerok)-DO-*/
			__raw_writeb(0x3, base+0x10c);
			/* PIN = U32, (WLAN_CLKREQ) */
			/* PIN = AA4, GPIO.15->KBC2,     mode 3,  -DO- */
			__raw_writeb(0x3, base+0x30);
			/* PIN = AA8, mDOC */
			/* PIN = AA10, BT */
			/* PIN = AA12, WLAN */
			/* PIN = M18 GPIO.96->MMC2_WP    mode 3   -DO- */
			__raw_writeb(0x3, base+0x10e);
			/* PIN = N19 GPIO.98->WLAN_INT   mode 3   -DO- */
			__raw_writeb(0x3, base+0x110);
			/* PIN = J15 HHUSB */
			/* PIN = H19 HSUSB */
			/* PIN = W13, P13, R13, W16 ... */
			/* PIN = V12 GPIO.25->I2C_CAMEN  mode 3   -DO- */
			__raw_writeb(0x3, base+0xde);
			/* PIN = W19 sys_nirq->MENELAUS_INT mode 0 -DO- */
			__raw_writeb(0x0, base+0x12c);
			/* PIN = AA17->sys_clkreq        mode 0   -DO- */
			__raw_writeb(0x0, base+0x136);
		}

	} else if (btype == BOARD_H4_SDP) {
		if (cpu == CPU_2420) {
			/* PIN = B3,  GPIO.0->nc         mode 3,  set above (pun?)*/
			/* PIN = B13, GPIO.cke1->nc,     mode 0,  set above, (pun?)*/
			/* Pin = Y11 VLNQ */
			/* Pin = AA4 VLNQ */
			/* Pin = AA6 VLNQ */
			/* Pin = AA8 VLNQ */
			/* Pin = AA10 VLNQ */
			/* Pin = AA12 VLNQ */
			/* PIN = M18 GPIO.96->KBR5       mode 3   -DO- */
			__raw_writeb(0x3, base+0x10e);
			/* PIN = N19 GPIO.98->KBC6       mode 3   -DO- */
			__raw_writeb(0x3, base+0x110);
			/* PIN = J15 MDOC_nDMAREQ */
			/* PIN = H19 GPIO.100->KBC2      mode 3   -DO- */
			__raw_writeb(0x3, base+0x114);
			/* PIN = W13, V12, P13, R13, W19, W16 ... */
			/* PIN = AA17 sys_clkreq->bt_clk_req  mode 0  */
		} else if (cpu == CPU_2422) {
			/* PIN = B3,  GPIO.0->MMC_CD,    mode 3,  set above */
			/* PIN = B13, GPIO.38->wlan_int, mode 3,  (pun?)*/
			/* Pin = Y11 VLNQ */
			/* Pin = AA4 VLNQ */
			/* Pin = AA6 VLNQ */
			/* Pin = AA8 VLNQ */
			/* Pin = AA10 VLNQ */
			/* Pin = AA12 VLNQ */
			/* PIN = M18 GPIO.96->KBR5       mode 3   -DO- */
			__raw_writeb(0x3, base+0x10e);
			/* PIN = N19 GPIO.98->KBC6       mode 3   -DO- */
			__raw_writeb(0x3, base+0x110);
			/* PIN = J15 MDOC_nDMAREQ */
			/* PIN = H19 GPIO.100->KBC2      mode 3   -DO- */
			__raw_writeb(0x3, base+0x114);
			/* PIN = W13, V12, P13, R13, W19, W16 ... */
			/* PIN = AA17 sys_clkreq->bt_clk_req  mode 0 */
		}
	}
}
