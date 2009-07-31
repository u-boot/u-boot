/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 *
 * This file is derived from cpu/mpc85xx/cpu.c and cpu/mpc86xx/cpu.c.
 * Basically this file contains cpu specific common code for 85xx/86xx
 * processors.
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

#include <config.h>
#include <common.h>
#include <command.h>
#include <tsec.h>
#include <netdev.h>
#include <asm/cache.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct cpu_type cpu_type_list [] = {
#if defined(CONFIG_MPC85xx)
	CPU_TYPE_ENTRY(8533, 8533),
	CPU_TYPE_ENTRY(8533, 8533_E),
	CPU_TYPE_ENTRY(8535, 8535),
	CPU_TYPE_ENTRY(8535, 8535_E),
	CPU_TYPE_ENTRY(8536, 8536),
	CPU_TYPE_ENTRY(8536, 8536_E),
	CPU_TYPE_ENTRY(8540, 8540),
	CPU_TYPE_ENTRY(8541, 8541),
	CPU_TYPE_ENTRY(8541, 8541_E),
	CPU_TYPE_ENTRY(8543, 8543),
	CPU_TYPE_ENTRY(8543, 8543_E),
	CPU_TYPE_ENTRY(8544, 8544),
	CPU_TYPE_ENTRY(8544, 8544_E),
	CPU_TYPE_ENTRY(8545, 8545),
	CPU_TYPE_ENTRY(8545, 8545_E),
	CPU_TYPE_ENTRY(8547, 8547_E),
	CPU_TYPE_ENTRY(8548, 8548),
	CPU_TYPE_ENTRY(8548, 8548_E),
	CPU_TYPE_ENTRY(8555, 8555),
	CPU_TYPE_ENTRY(8555, 8555_E),
	CPU_TYPE_ENTRY(8560, 8560),
	CPU_TYPE_ENTRY(8567, 8567),
	CPU_TYPE_ENTRY(8567, 8567_E),
	CPU_TYPE_ENTRY(8568, 8568),
	CPU_TYPE_ENTRY(8568, 8568_E),
	CPU_TYPE_ENTRY(8569, 8569),
	CPU_TYPE_ENTRY(8569, 8569_E),
	CPU_TYPE_ENTRY(8572, 8572),
	CPU_TYPE_ENTRY(8572, 8572_E),
	CPU_TYPE_ENTRY(P2020, P2020),
	CPU_TYPE_ENTRY(P2020, P2020_E),
#elif defined(CONFIG_MPC86xx)
	CPU_TYPE_ENTRY(8610, 8610),
	CPU_TYPE_ENTRY(8641, 8641),
	CPU_TYPE_ENTRY(8641D, 8641D),
#endif
};

struct cpu_type *identify_cpu(u32 ver)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++) {
		if (cpu_type_list[i].soc_ver == ver)
			return &cpu_type_list[i];
	}

	return NULL;
}

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_ETHER_ON_FCC)
	fec_initialize(bis);
#endif

#if defined(CONFIG_UEC_ETH)
	uec_standard_init(bis);
#endif

#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_MPC85XX_FEC)
	tsec_standard_init(bis);
#endif

	return 0;
}
