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
#include <s6e63d6.h>
#include <netdev.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

int board_init(void)
{

	gd->bd->bi_arch_number = MACH_TYPE_PCM037;	/* board id for linux */
	gd->bd->bi_boot_params = (0x80000100);	/* adress of boot parameters */

	return 0;
}

int board_early_init_f(void)
{
	__REG(CSCR_U(0)) = 0x0000cf03; /* CS0: Nor Flash */
	__REG(CSCR_L(0)) = 0x10000d03;
	__REG(CSCR_A(0)) = 0x00720900;

	__REG(CSCR_U(1)) = 0x0000df06; /* CS1: Network Controller */
	__REG(CSCR_L(1)) = 0x444a4541;
	__REG(CSCR_A(1)) = 0x44443302;

	__REG(CSCR_U(4)) = 0x0000d843; /* CS4: SRAM */
	__REG(CSCR_L(4)) = 0x22252521;
	__REG(CSCR_A(4)) = 0x22220a00;

	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_CTS1__UART1_CTS_B);

	/* setup pins for I2C2 (for EEPROM, RTC) */
	mx31_gpio_mux(MUX_CSPI2_MOSI__I2C2_SCL);
	mx31_gpio_mux(MUX_CSPI2_MISO__I2C2_SDA);

	return 0;
}

#ifdef BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_S6E63D6
	struct s6e63d6 data = {
		/*
		 * See comment in mxc_spi.c::decode_cs() for .cs field format.
		 * We use GPIO 57 as a chipselect for the S6E63D6 and chipselect
		 * 2 of the SPI controller #1, since it is unused.
		 */
		.cs = 2 | (57 << 8),
		.bus = 0,
		.id = 0,
	};
	int ret;

	/* SPI1 */
	mx31_gpio_mux(MUX_CSPI1_SCLK__CSPI1_CLK);
	mx31_gpio_mux(MUX_CSPI1_SPI_RDY__CSPI1_DATAREADY_B);
	mx31_gpio_mux(MUX_CSPI1_MOSI__CSPI1_MOSI);
	mx31_gpio_mux(MUX_CSPI1_MISO__CSPI1_MISO);
	mx31_gpio_mux(MUX_CSPI1_SS0__CSPI1_SS0_B);
	mx31_gpio_mux(MUX_CSPI1_SS1__CSPI1_SS1_B);
	mx31_gpio_mux(MUX_CSPI1_SS2__CSPI1_SS2_B);

	/* start SPI1 clock */
	__REG(CCM_CGR2) = __REG(CCM_CGR2) | (3 << 2);

	/* GPIO 57 */
	/* sw_mux_ctl_key_col4_key_col5_key_col6_key_col7 */
	mx31_gpio_mux(IOMUX_MODE(0x63, MUX_CTL_GPIO));

	/* SPI1 CS2 is free */
	ret = s6e63d6_init(&data);
	if (ret)
		return ret;

	/*
	 * This is a "magic" sequence to initialise a C0240QGLA / C0283QGLC
	 * OLED display connected to a S6E63D6 SPI display controller in the
	 * 18 bit RGB mode
	 */
	s6e63d6_index(&data, 2);
	s6e63d6_param(&data, 0x0182);
	s6e63d6_index(&data, 3);
	s6e63d6_param(&data, 0x8130);
	s6e63d6_index(&data, 0x10);
	s6e63d6_param(&data, 0x0000);
	s6e63d6_index(&data, 5);
	s6e63d6_param(&data, 0x0001);
	s6e63d6_index(&data, 0x22);
#endif
	return 0;
}
#endif

int checkboard (void)
{
	printf("Board: Phytec phyCore i.MX31\n");
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
