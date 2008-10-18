/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>
#include <common.h>
#include <asm/processor.h>

int __board_pre_init(void)
{
	return 0;
}
int board_pre_init(void) __attribute__((weak, alias("__board_pre_init")));

int __checkboard(void)
{
	puts("Xilinx PPC440 Generic Board\n");
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
