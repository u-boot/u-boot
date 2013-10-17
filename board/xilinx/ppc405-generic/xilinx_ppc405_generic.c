/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
*/

#include <config.h>
#include <common.h>
#include <asm/processor.h>

ulong __get_PCI_freq(void)
{
	return 0;
}

ulong get_PCI_freq(void) __attribute__((weak, alias("__get_PCI_freq")));

int __board_pre_init(void)
{
	return 0;
}
int board_pre_init(void) __attribute__((weak, alias("__board_pre_init")));

int __checkboard(void)
{
	puts("Xilinx PPC405 Generic Board\n");
	return 0;
}
int checkboard(void) __attribute__((weak, alias("__checkboard")));

phys_size_t __initdram(int board_type)
{
	return get_ram_size(XPAR_DDR2_SDRAM_MEM_BASEADDR,
			    CONFIG_SYS_SDRAM_SIZE_MB * 1024 * 1024);
}
phys_size_t initdram(int) __attribute__((weak, alias("__initdram")));

void __get_sys_info(sys_info_t *sysInfo)
{
	sysInfo->freqProcessor = XPAR_CORE_CLOCK_FREQ_HZ;
	sysInfo->freqPLB = XPAR_PLB_CLOCK_FREQ_HZ;
	sysInfo->freqPCI = 0;

	return;
}
void get_sys_info(sys_info_t *) __attribute__((weak, alias("__get_sys_info")));
