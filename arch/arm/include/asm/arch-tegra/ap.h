/*
 * (C) Copyright 2010-2011
 * NVIDIA Corporation <www.nvidia.com>
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
#include <asm/types.h>

/* Stabilization delays, in usec */
#define PLL_STABILIZATION_DELAY (300)
#define IO_STABILIZATION_DELAY	(1000)

#define NVBL_PLLP_KHZ	(216000)

#define PLLX_ENABLED		(1 << 30)
#define CCLK_BURST_POLICY	0x20008888
#define SUPER_CCLK_DIVIDER	0x80000000

/* Calculate clock fractional divider value from ref and target frequencies */
#define CLK_DIVIDER(REF, FREQ)  ((((REF) * 2) / FREQ) - 2)

/* Calculate clock frequency value from reference and clock divider value */
#define CLK_FREQUENCY(REF, REG)  (((REF) * 2) / (REG + 2))

/* AVP/CPU ID */
#define PG_UP_TAG_0_PID_CPU	0x55555555	/* CPU aka "a9" aka "mpcore" */
#define PG_UP_TAG_0             0x0

#define CORESIGHT_UNLOCK	0xC5ACCE55;

/* AP20-Specific Base Addresses */

/* AP20 Base physical address of SDRAM. */
#define AP20_BASE_PA_SDRAM      0x00000000
/* AP20 Base physical address of internal SRAM. */
#define AP20_BASE_PA_SRAM       0x40000000
/* AP20 Size of internal SRAM (256KB). */
#define AP20_BASE_PA_SRAM_SIZE  0x00040000
/* AP20 Base physical address of flash. */
#define AP20_BASE_PA_NOR_FLASH  0xD0000000
/* AP20 Base physical address of boot information table. */
#define AP20_BASE_PA_BOOT_INFO  AP20_BASE_PA_SRAM

/*
 * Super-temporary stacks for EXTREMELY early startup. The values chosen for
 * these addresses must be valid on ALL SOCs because this value is used before
 * we are able to differentiate between the SOC types.
 *
 * NOTE: The since CPU's stack will eventually be moved from IRAM to SDRAM, its
 *       stack is placed below the AVP stack. Once the CPU stack has been moved,
 *       the AVP is free to use the IRAM the CPU stack previously occupied if
 *       it should need to do so.
 *
 * NOTE: In multi-processor CPU complex configurations, each processor will have
 *       its own stack of size CPU_EARLY_BOOT_STACK_SIZE. CPU 0 will have a
 *       limit of CPU_EARLY_BOOT_STACK_LIMIT. Each successive CPU will have a
 *       stack limit that is CPU_EARLY_BOOT_STACK_SIZE less then the previous
 *       CPU.
 */

/* Common AVP early boot stack limit */
#define AVP_EARLY_BOOT_STACK_LIMIT	\
	(AP20_BASE_PA_SRAM + (AP20_BASE_PA_SRAM_SIZE/2))
/* Common AVP early boot stack size */
#define AVP_EARLY_BOOT_STACK_SIZE	0x1000
/* Common CPU early boot stack limit */
#define CPU_EARLY_BOOT_STACK_LIMIT	\
	(AVP_EARLY_BOOT_STACK_LIMIT - AVP_EARLY_BOOT_STACK_SIZE)
/* Common CPU early boot stack size */
#define CPU_EARLY_BOOT_STACK_SIZE	0x1000

#define EXCEP_VECTOR_CPU_RESET_VECTOR	(NV_PA_EVP_BASE + 0x100)
#define CSITE_CPU_DBG0_LAR		(NV_PA_CSITE_BASE + 0x10FB0)
#define CSITE_CPU_DBG1_LAR		(NV_PA_CSITE_BASE + 0x12FB0)

#define FLOW_CTLR_HALT_COP_EVENTS	(NV_PA_FLOW_BASE + 4)
#define FLOW_MODE_STOP			2
#define HALT_COP_EVENT_JTAG		(1 << 28)
#define HALT_COP_EVENT_IRQ_1		(1 << 11)
#define HALT_COP_EVENT_FIQ_1		(1 << 9)

/* This is the main entry into U-Boot, used by the Cortex-A9 */
extern void _start(void);

/**
 * Works out the SOC type used for clocks settings
 *
 * @return	SOC type - see TEGRA_SOC...
 */
int tegra_get_chip_type(void);
