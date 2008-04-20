/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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
#include <asm/arch/mx31.h>
#include <asm/arch/mx31-regs.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

int board_init (void)
{
	__REG(CSCR_U(0)) = 0x0000cf03; /* CS0: Nor Flash */
	__REG(CSCR_L(0)) = 0xa0330d01;
	__REG(CSCR_A(0)) = 0x00220800;

	__REG(CSCR_U(4)) = 0x0000dcf6; /* CS4: Network Controller */
	__REG(CSCR_L(4)) = 0x444a4541;
	__REG(CSCR_A(4)) = 0x44443302;

	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_RTS1__UART1_CTS_B);

	/* SPI2 */
	mx31_gpio_mux((MUX_CTL_FUNC << 8) | MUX_CTL_CSPI2_SS2);
	mx31_gpio_mux((MUX_CTL_FUNC << 8) | MUX_CTL_CSPI2_SCLK);
	mx31_gpio_mux((MUX_CTL_FUNC << 8) | MUX_CTL_CSPI2_SPI_RDY);
	mx31_gpio_mux((MUX_CTL_FUNC << 8) | MUX_CTL_CSPI2_MOSI);
	mx31_gpio_mux((MUX_CTL_FUNC << 8) | MUX_CTL_CSPI2_MISO);
	mx31_gpio_mux((MUX_CTL_FUNC << 8) | MUX_CTL_CSPI2_SS0);
	mx31_gpio_mux((MUX_CTL_FUNC << 8) | MUX_CTL_CSPI2_SS1);

	/* start SPI2 clock */
	__REG(CCM_CGR2) = __REG(CCM_CGR2) | (3 << 4);

	gd->bd->bi_arch_number = MACH_TYPE_MX31LITE; /* board id for linux */
	gd->bd->bi_boot_params = (0x80000100);	/* adress of boot parameters */

	return 0;
}

int checkboard (void)
{
	printf("Board: i.MX31 Litekit\n");
	return 0;
}
