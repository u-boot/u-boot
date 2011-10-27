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
#include <netdev.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

int board_early_init_f(void)
{
	/* CS0: Nor Flash */
	static const struct mxc_weimcs cs0 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  3, 15, 0,  0,  3),
		/*    oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(10,  0,   3,   3,  0,  1,  5,  0,  0,  0,   0,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(0,   0,  2,  2,  0,  0,  2,  0,  0,  0,  0,  0,   0,  0)
	};

	/* CS4: Network Controller */
	static const struct mxc_weimcs cs4 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  3, 28, 1,  7,  6),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(4,  4,   4,  10,  4,  0,  5,  4,  0,  0,   0,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(4,   4,  4,  4,  0,  1,  4,  3,  0,  0,  0,  0,   1,  0)
	};

	mxc_setup_weimcs(0, &cs0);
	mxc_setup_weimcs(4, &cs4);

	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_CTS1__UART1_CTS_B);

	/* SPI2 */
	mx31_gpio_mux(MUX_CSPI2_SS2__CSPI2_SS2_B);
	mx31_gpio_mux(MUX_CSPI2_SCLK__CSPI2_CLK);
	mx31_gpio_mux(MUX_CSPI2_SPI_RDY__CSPI2_DATAREADY_B);
	mx31_gpio_mux(MUX_CSPI2_MOSI__CSPI2_MOSI);
	mx31_gpio_mux(MUX_CSPI2_MISO__CSPI2_MISO);
	mx31_gpio_mux(MUX_CSPI2_SS0__CSPI2_SS0_B);
	mx31_gpio_mux(MUX_CSPI2_SS1__CSPI2_SS1_B);

	/* start SPI2 clock */
	__REG(CCM_CGR2) = __REG(CCM_CGR2) | (3 << 4);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = (0x80000100);	/* adress of boot parameters */

	return 0;
}

int checkboard(void)
{
	printf("Board: i.MX31 Litekit\n");
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
