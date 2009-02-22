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
#include "qong_fpga.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((volatile void *)PHYS_SDRAM_1,
			PHYS_SDRAM_1_SIZE);

	return 0;
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
#endif

	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_CTS1__UART1_CTS_B);

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_QONG;
	gd->bd->bi_boot_params = (0x80000100);	/* adress of boot parameters */

	return 0;
}

int checkboard (void)
{
	printf("Board: DAVE/DENX QongEVB-LITE\n");
	return 0;
}

int misc_init_r (void)
{
#ifdef CONFIG_QONG_FPGA
	u32 tmp;

	/* FPGA reset */
	/* rstn = 0 */
	tmp = __REG(GPIO2_BASE + GPIO_DR);
	tmp &= (~(1 << QONG_FPGA_RST_PIN));
	__REG(GPIO2_BASE + GPIO_DR) = tmp;
	/* set the GPIO as output */
	tmp = __REG(GPIO2_BASE + GPIO_GDIR);
	tmp |= (1 << QONG_FPGA_RST_PIN);
	__REG(GPIO2_BASE + GPIO_GDIR) = tmp;
	/* wait */
	udelay(30);
	/* rstn = 1 */
	tmp = __REG(GPIO2_BASE + GPIO_DR);
	tmp |= (1 << QONG_FPGA_RST_PIN);
	__REG(GPIO2_BASE + GPIO_DR) = tmp;
	/* set interrupt pin as input */
	__REG(GPIO2_BASE + GPIO_GDIR) = tmp | (1 << QONG_FPGA_IRQ_PIN);
	/* wait while the FPGA starts */
	udelay(300);

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
