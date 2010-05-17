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
#include <asm/arch/mx31.h>
#include <asm/arch/mx31-regs.h>
#include <nand.h>
#include <fsl_pmic.h>
#include "qong_fpga.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((volatile void *)PHYS_SDRAM_1,
			PHYS_SDRAM_1_SIZE);

	return 0;
}

static void qong_fpga_reset(void)
{
	mx31_gpio_set(QONG_FPGA_RST_PIN, 0);
	udelay(30);
	mx31_gpio_set(QONG_FPGA_RST_PIN, 1);

	udelay(300);
}

int board_init (void)
{
	/* Chip selects */
	/* CS0: Nor Flash #0 - it must be init'ed when executing from DDR */
	/* Assumptions: HCLK = 133 MHz, tACC = 130ns */
	__REG(CSCR_U(0)) = ((0 << 31)	| /* SP */
						(0 << 30)	| /* WP */
						(0 << 28)	| /* BCD */
						(0 << 24)	| /* BCS */
						(0 << 22)	| /* PSZ */
						(0 << 21)	| /* PME */
						(0 << 20)	| /* SYNC */
						(0 << 16)	| /* DOL */
						(3 << 14)	| /* CNC */
						(21 << 8)	| /* WSC */
						(0 << 7)	| /* EW */
						(0 << 4)	| /* WWS */
						(6 << 0)	  /* EDC */
					   );

	__REG(CSCR_L(0)) = ((2 << 28)	| /* OEA */
						(1 << 24)	| /* OEN */
						(3 << 20)	| /* EBWA */
						(3 << 16)	| /* EBWN */
						(1 << 12)	| /* CSA */
						(1 << 11)	| /* EBC */
						(5 << 8)	| /* DSZ */
						(1 << 4)	| /* CSN */
						(0 << 3)	| /* PSR */
						(0 << 2)	| /* CRE */
						(0 << 1)	| /* WRAP */
						(1 << 0)	  /* CSEN */
					   );

	__REG(CSCR_A(0)) = ((2 << 28)	| /* EBRA */
						(1 << 24)	| /* EBRN */
						(2 << 20)	| /* RWA */
						(2 << 16)	| /* RWN */
						(0 << 15)	| /* MUM */
						(0 << 13)	| /* LAH */
						(2 << 10)	| /* LBN */
						(0 << 8)	| /* LBA */
						(0 << 6)	| /* DWW */
						(0 << 4)	| /* DCT */
						(0 << 3)	| /* WWU */
						(0 << 2)	| /* AGE */
						(0 << 1)	| /* CNC2 */
						(0 << 0)	  /* FCE */
					   );

#ifdef CONFIG_QONG_FPGA
	/* CS1: FPGA/Network Controller/GPIO */
	/* 16-bit, no DTACK */
	__REG(CSCR_U(1)) = 0x00000A01;
	__REG(CSCR_L(1)) = 0x20040501;
	__REG(CSCR_A(1)) = 0x04020C00;

	/* setup pins for FPGA */
	mx31_gpio_mux(IOMUX_MODE(0x76, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x7e, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x91, MUX_CTL_OUT_FUNC | MUX_CTL_IN_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x92, MUX_CTL_GPIO));
	mx31_gpio_mux(IOMUX_MODE(0x93, MUX_CTL_GPIO));

	/* FPGA reset  Pin */
	/* rstn = 0 */
	mx31_gpio_set(QONG_FPGA_RST_PIN, 0);
	mx31_gpio_direction(QONG_FPGA_RST_PIN, MX31_GPIO_DIRECTION_OUT);

	/* set interrupt pin as input */
	mx31_gpio_direction(QONG_FPGA_IRQ_PIN, MX31_GPIO_DIRECTION_IN);

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

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_QONG;
	gd->bd->bi_boot_params = (0x80000100);	/* adress of boot parameters */

	return 0;
}

int board_late_init(void)
{
	u32 val;

	/* Enable RTC battery */
	val = pmic_reg_read(REG_POWER_CTL0);
	pmic_reg_write(REG_POWER_CTL0, val | COINCHEN);
	pmic_reg_write(REG_INT_STATUS1, RTCRSTI);

	return 0;
}

int checkboard (void)
{
	printf("Board: DAVE/DENX Qong\n");
	return 0;
}

int misc_init_r (void)
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
	__REG(CSCR_U(3)) = 0x00004f00;
	__REG(CSCR_L(3)) = 0x20013b31;
	__REG(CSCR_A(3)) = 0x00020800;
	__REG(IOMUXC_GPR) |= 1 << 13;

	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_NFC_WP, MUX_CTL_IN_GPIO));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_NFC_CE, MUX_CTL_IN_GPIO));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_NFC_RB, MUX_CTL_IN_GPIO));

	/* Make sure to reset the fpga else you cannot access NAND */
	qong_fpga_reset();

	/* Enable NAND flash */
	mx31_gpio_set(15, 1);
	mx31_gpio_set(14, 1);
	mx31_gpio_direction(15, MX31_GPIO_DIRECTION_OUT);
	mx31_gpio_direction(16, MX31_GPIO_DIRECTION_IN);
	mx31_gpio_direction(14, MX31_GPIO_DIRECTION_IN);
	mx31_gpio_set(15, 0);

}

int qong_nand_rdy(void *chip)
{
	udelay(1);
	return mx31_gpio_get(16);
}

void qong_nand_select_chip(struct mtd_info *mtd, int chip)
{
	if (chip >= 0)
		mx31_gpio_set(15, 0);
	else
		mx31_gpio_set(15, 1);

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
