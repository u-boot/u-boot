/*
 *
 * (c) 2009 Emcraft Systems, Ilya Yanok <yanok@emcraft.com>
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
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <nand.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <asm/gpio.h>
#include "qong_fpga.h"
#include <watchdog.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	mxc_hw_watchdog_reset();
}
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

static void qong_fpga_reset(void)
{
	gpio_set_value(QONG_FPGA_RST_PIN, 0);
	udelay(30);
	gpio_set_value(QONG_FPGA_RST_PIN, 1);

	udelay(300);
}

int board_early_init_f(void)
{
#ifdef CONFIG_QONG_FPGA
	/* CS1: FPGA/Network Controller/GPIO, 16-bit, no DTACK */
	static const struct mxc_weimcs cs1 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  0, 10, 0,  0,  1),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(2,  0,   0,   4,  0,  0,  5,  0,  0,  0,   0,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(0,   4,  0,  2,  0,  0,  3,  0,  0,  0,  0,  0,   0,  0)
	};

	mxc_setup_weimcs(1, &cs1);

	/* setup pins for FPGA */
	mx31_gpio_mux(IOMUX_MODE(0x76, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x7e, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x91, MUX_CTL_OUT_FUNC | MUX_CTL_IN_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x92, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x93, MUX_CTL_GPIO));

	/* FPGA reset  Pin */
	/* rstn = 0 */
	gpio_direction_output(QONG_FPGA_RST_PIN, 0);

	/* set interrupt pin as input */
	gpio_direction_input(QONG_FPGA_IRQ_PIN);

	/* FPGA JTAG Interface */
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SFS6, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SCK6, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_CAPTURE, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_COMPARE, MUX_CTL_GPIO));
	gpio_direction_output(QONG_FPGA_TCK_PIN, 0);
	gpio_direction_output(QONG_FPGA_TMS_PIN, 0);
	gpio_direction_output(QONG_FPGA_TDI_PIN, 0);
	gpio_direction_input(QONG_FPGA_TDO_PIN);
#endif

	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_CTS1__UART1_CTS_B);

	/* setup pins for SPI (pmic) */
	mx31_gpio_mux(MUX_CSPI2_SS0__CSPI2_SS0_B);
	mx31_gpio_mux(MUX_CSPI2_MOSI__CSPI2_MOSI);
	mx31_gpio_mux(MUX_CSPI2_MISO__CSPI2_MISO);
	mx31_gpio_mux(MUX_CSPI2_SCLK__CSPI2_CLK);
	mx31_gpio_mux(MUX_CSPI2_SPI_RDY__CSPI2_DATAREADY_B);

	/* Setup pins for USB2 Host */
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_USBH2_CLK, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_USBH2_DIR, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_USBH2_NXT, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_USBH2_STP, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_USBH2_DATA0, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_USBH2_DATA1, MUX_CTL_FUNC));

#define H2_PAD_CFG (PAD_CTL_DRV_MAX | PAD_CTL_SRE_FAST | PAD_CTL_HYS_CMOS | \
			PAD_CTL_ODE_CMOS | PAD_CTL_100K_PU)

	mx31_set_pad(MX31_PIN_USBH2_CLK, H2_PAD_CFG);
	mx31_set_pad(MX31_PIN_USBH2_DIR, H2_PAD_CFG);
	mx31_set_pad(MX31_PIN_USBH2_NXT, H2_PAD_CFG);
	mx31_set_pad(MX31_PIN_USBH2_STP, H2_PAD_CFG);
	mx31_set_pad(MX31_PIN_USBH2_DATA0, H2_PAD_CFG); /* USBH2_DATA0 */
	mx31_set_pad(MX31_PIN_USBH2_DATA1, H2_PAD_CFG); /* USBH2_DATA1 */
	mx31_set_pad(MX31_PIN_SRXD6, H2_PAD_CFG);	/* USBH2_DATA2 */
	mx31_set_pad(MX31_PIN_STXD6, H2_PAD_CFG);	/* USBH2_DATA3 */
	mx31_set_pad(MX31_PIN_SFS3, H2_PAD_CFG);	/* USBH2_DATA4 */
	mx31_set_pad(MX31_PIN_SCK3, H2_PAD_CFG);	/* USBH2_DATA5 */
	mx31_set_pad(MX31_PIN_SRXD3, H2_PAD_CFG);	/* USBH2_DATA6 */
	mx31_set_pad(MX31_PIN_STXD3, H2_PAD_CFG);	/* USBH2_DATA7 */

	mx31_set_gpr(MUX_PGP_UH2, 1);

	return 0;

}

int board_init(void)
{
	/* Chip selects */
	/* CS0: Nor Flash #0 - it must be init'ed when executing from DDR */
	/* Assumptions: HCLK = 133 MHz, tACC = 130ns */
	static const struct mxc_weimcs cs0 = {
		/*     sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  3, 21, 0,  0,  6),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(0,  1,   3,   3,  1,  1,  5,  1,  0,  0,   0,  1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(0,   1,  2,  2,  0,  0,  2,  0,  0,  0,  0,  0,   0,  0)
	};

	mxc_setup_weimcs(0, &cs0);

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_QONG;
	gd->bd->bi_boot_params = (0x80000100);	/* adress of boot parameters */

	qong_fpga_init();

	return 0;
}

int board_late_init(void)
{
	u32 val;
	struct pmic *p;
	int ret;

	ret = pmic_init(I2C_PMIC);
	if (ret)
		return ret;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return -ENODEV;
	/* Enable RTC battery */
	pmic_reg_read(p, REG_POWER_CTL0, &val);
	pmic_reg_write(p, REG_POWER_CTL0, val | COINCHEN);
	pmic_reg_write(p, REG_INT_STATUS1, RTCRSTI);

#ifdef CONFIG_HW_WATCHDOG
	mxc_hw_watchdog_enable();
#endif

	return 0;
}

int checkboard(void)
{
	printf("Board: DAVE/DENX Qong\n");
	return 0;
}

int misc_init_r(void)
{
#ifdef CONFIG_QONG_FPGA
	u32 tmp;

	tmp = *(volatile u32*)QONG_FPGA_CTRL_VERSION;
	printf("FPGA:  ");
	printf("version register = %u.%u.%u\n",
		(tmp & 0xF000) >> 12, (tmp & 0x0F00) >> 8, tmp & 0x00FF);
#endif
	return 0;
}

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_QONG_FPGA) && defined(CONFIG_DNET)
	return dnet_eth_initialize(0, (void *)CONFIG_DNET_BASE, -1);
#else
	return 0;
#endif
}

#if defined(CONFIG_QONG_FPGA) && defined(CONFIG_NAND_PLAT)
static void board_nand_setup(void)
{
	/* CS3: NAND 8-bit */
	static const struct mxc_weimcs cs3 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  1, 15, 0,  0,  0),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(2,  0,   0,   1,  3,  1,  3,  3,  0,  0,   0,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(0,   0,  0,  2,  0,  0,  2,  0,  0,  0,  0,  0,  0,   0)
	};

	mxc_setup_weimcs(3, &cs3);

	mx31_set_gpr(MUX_SDCTL_CSD1_SEL, 1);

	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_NFC_WP, MUX_CTL_IN_GPIO));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_NFC_CE, MUX_CTL_IN_GPIO));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_NFC_RB, MUX_CTL_IN_GPIO));

	/* Make sure to reset the fpga else you cannot access NAND */
	qong_fpga_reset();

	/* Enable NAND flash */
	gpio_set_value(15, 1);
	gpio_set_value(14, 1);
	gpio_direction_output(15, 0);
	gpio_direction_input(16);
	gpio_direction_input(14);

}

int qong_nand_rdy(void *chip)
{
	udelay(1);
	return gpio_get_value(16);
}

void qong_nand_select_chip(struct mtd_info *mtd, int chip)
{
	if (chip >= 0)
		gpio_set_value(15, 0);
	else
		gpio_set_value(15, 1);

}

void qong_nand_plat_init(void *chip)
{
	struct nand_chip *nand = (struct nand_chip *)chip;
	nand->chip_delay = 20;
	nand->select_chip = qong_nand_select_chip;
	nand->options &= ~NAND_BUSWIDTH_16;
	board_nand_setup();
}

#endif
