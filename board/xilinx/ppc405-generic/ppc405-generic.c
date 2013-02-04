/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This is a board specific file.  It's OK to include board specific
 * header files */

#include <common.h>
#include <config.h>
#include <netdev.h>

ulong __get_PCI_freq(void)
{
	return 0;
}

ulong get_PCI_freq(void) __attribute__((weak, alias("__get_PCI_freq")));

phys_size_t __initdram(int board_type)
{
	return get_ram_size(XILINX_RAM_START, XILINX_RAM_SIZE);
}
phys_size_t initdram(int) __attribute__((weak, alias("__initdram")));

void __get_sys_info(sys_info_t *sysInfo)
{
	/* FIXME */
	sysInfo->freqProcessor = XILINX_CLOCK_FREQ;
	sysInfo->freqPLB = XILINX_CLOCK_FREQ;
	sysInfo->freqPCI = 0;

	return;
}

void get_sys_info(sys_info_t *) __attribute__((weak, alias("__get_sys_info")));

int __checkboard(void)
{
	puts(__stringify(XILINX_BOARD_NAME) "\n");
	return 0;
}
int checkboard(void) __attribute__((weak, alias("__checkboard")));


int gpio_init (void)
{
#ifdef CONFIG_SYS_GPIO_0
	*((unsigned long *)(CONFIG_SYS_GPIO_0_ADDR)) = 0xFFFFFFFF;
#endif
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = 0;

#ifdef CONFIG_XILINX_EMACLITE
	u32 txpp = 0;
	u32 rxpp = 0;
# ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
	txpp = 1;
# endif
# ifdef CONFIG_XILINX_EMACLITE_RX_PING_PONG
	rxpp = 1;
# endif
	ret |= xilinx_emaclite_initialize(bis, XILINX_EMACLITE_BASEADDR,
			txpp, rxpp);
#endif
#ifdef CONFIG_XILINX_LL_TEMAC
# ifdef XILINX_LLTEMAC_FIFO_BASEADDR
	ret |= xilinx_ll_temac_initialize(bis, XILINX_LLTEMAC_BASEADDR, 0,
					XILINX_LLTEMAC_FIFO_BASEADDR);
# elif XILINX_LLTEMAC_SDMA_CTRL_BASEADDR
#  if XILINX_LLTEMAC_SDMA_USE_DCR == 1
	ret |= xilinx_ll_temac_initialize(bis, XILINX_LLTEMAC_BASEADDR, 3,
					XILINX_LLTEMAC_SDMA_CTRL_BASEADDR);
#  else
	ret |= xilinx_ll_temac_initialize(bis, XILINX_LLTEMAC_BASEADDR, 1,
					XILINX_LLTEMAC_SDMA_CTRL_BASEADDR);
#  endif
# endif
#endif
	return ret;
}
