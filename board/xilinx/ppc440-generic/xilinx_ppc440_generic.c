/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@gmail.com
 * This work has been supported by: QTechnology  http://qtec.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
*/

#include <config.h>
#include <common.h>
#include <netdev.h>
#include <asm/processor.h>

int checkboard(void)
{
	puts("Xilinx PPC440 Generic Board\n");
	return 0;
}

phys_size_t initdram(int board_type)
{
	return get_ram_size(XPAR_DDR2_SDRAM_MEM_BASEADDR,
			    CONFIG_SYS_SDRAM_SIZE_MB * 1024 * 1024);
}

void get_sys_info(sys_info_t *sys_info)
{
	sys_info->freqProcessor = XPAR_CORE_CLOCK_FREQ_HZ;
	sys_info->freqPLB = XPAR_PLB_CLOCK_FREQ_HZ;
	sys_info->freqPCI = 0;

	return;
}

int get_serial_clock(void){
	return XPAR_UARTNS550_0_CLOCK_FREQ_HZ;
}

int board_eth_init(bd_t *bis)
{
	int ret = 0;

	puts("Init xilinx temac\n");
#ifdef XPAR_LLTEMAC_0_BASEADDR
	ret |= xilinx_ll_temac_eth_init(bis, XPAR_LLTEMAC_0_BASEADDR,
			XILINX_LL_TEMAC_M_SDMA_DCR | XILINX_LL_TEMAC_M_SDMA_PLB,
			XPAR_LLTEMAC_0_LLINK_CONNECTED_BASEADDR);

#endif

#ifdef XPAR_LLTEMAC_1_BASEADDR
	ret |= xilinx_ll_temac_eth_init(bis, XPAR_LLTEMAC_1_BASEADDR,
			XILINX_LL_TEMAC_M_SDMA_DCR | XILINX_LL_TEMAC_M_SDMA_PLB,
			XPAR_LLTEMAC_1_LLINK_CONNECTED_BASEADDR);
#endif

	return ret;
}
