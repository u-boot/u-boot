/*
 * Memory Setup stuff - taken from Linux
 *
 * Copyright (c) 2002	Stephan Linz <linz@mazet.de>, <linz@li-pro.net>
 * (c) 2004 IMMS gGmbH <www.imms.de>, Thomas Elste <info@elste.org>
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


#include <config.h>
#include <version.h>
#include <asm/arch/netarm_registers.h>


/* some parameters for the board */
#define FLASH_90ns_WAIT_STATES	((NETARM_PLL_COUNT_VAL + 2) / 3)
#define FLASH_70ns_WAIT_STATES	4

#define NETARM_MMAP_CS0_BASE	(PHYS_FLASH_1)
#if 1
#define NETARM_MMAP_CS0_MASK	(~(PHYS_FLASH_1_SIZE - 1))
#else
#define NETARM_MMAP_CS0_MASK	(~(1000000 - 1))
#endif
#define NETARM_MMAP_CS1_BASE	(PHYS_SDRAM_1)
#define NETARM_MMAP_CS1_MASK	(~(PHYS_SDRAM_1_SIZE - 1))
#define NETARM_MMAP_CS2_BASE	(PHYS_SDRAM_2)
#define NETARM_MMAP_CS2_MASK	(~(PHYS_SDRAM_2_SIZE - 1))
#if defined(CONFIG_NETARM_EEPROM) && defined(PHYS_NVRAM_1) && defined(PHYS_NVRAM_SIZE)
#define NETARM_MMAP_CS3_BASE	(PHYS_NVRAM_1)
#define NETARM_MMAP_CS3_MASK	(~(PHYS_NVRAM_SIZE - 1))
#endif
#define NETARM_MMAP_CS4_BASE	(PHYS_EXT_1)
#define NETARM_MMAP_CS4_MASK	(~(PHYS_EXT_SIZE - 1))

/* setting up the memory */
.globl lowlevel_init
lowlevel_init:

#if defined(CONFIG_MODNET50)
	ldr	pc, =(_jump_to_high + NETARM_MMAP_CS0_BASE - TEXT_BASE)

_jump_to_high:
	/*
	 * MEM Config Reg
	 * ---------------------------------------------------
	 */
	ldr	r0, =NETARM_MEM_MODULE_BASE
	ldr	r1, =(	NETARM_MEM_REFR_PERIOD_USEC(16) | \
			NETARM_MEM_CFG_REFRESH_EN | \
			NETARM_MEM_CFG_REFR_CYCLE_5CLKS )
	str	r1, [r0, #+NETARM_MEM_MODULE_CONFIG]


memsetup_cs0:
	/*
	 * Base Addr / Option Reg 0 (Flash)
	 * ---------------------------------------------------
	 */
	ldr	r1, =(	NETARM_MEM_BAR_BASE(NETARM_MMAP_CS0_BASE) | \
			NETARM_MEM_BAR_DRAM_FP | \
			NETARM_MEM_BAR_DRAM_MUX_INT | \
			NETARM_MEM_BAR_DRAM_MUX_BAL | \
			NETARM_MEM_BAR_VALID )
	str	r1, [r0, #+NETARM_MEM_CS0_BASE_ADDR]

	/* trust that the bus size for flash was strapped correctly */
	/* this saves the bus width in r2 and then ORs it back in */
	/* it's pretty safe assumption, otherwise it wouldn't boot */
	ldr	r2, [r0, #+NETARM_MEM_CS0_OPTIONS]
	and	r2, r2, #NETARM_MEM_OPT_BUS_SIZE_MASK

/*	just a test: assume 32 bit flash mem */
/*	mov	r2, #NETARM_MEM_OPT_32BIT */

	ldr	r1, =(	NETARM_MEM_OPT_BASE_USE(NETARM_MMAP_CS0_MASK) | \
			NETARM_MEM_OPT_WAIT_STATES(FLASH_70ns_WAIT_STATES) | \
			NETARM_MEM_OPT_BCYC_4 | \
			NETARM_MEM_OPT_BSIZE_16 | \
			NETARM_MEM_OPT_16BIT | \
			NETARM_MEM_OPT_READ_ASYNC | \
			NETARM_MEM_OPT_WRITE_ASYNC )

	orr	r1, r1, r2
	str	r1, [r0, #+NETARM_MEM_CS0_OPTIONS]


memsetup_cs1:
	/*
	 * Base Addr / Option Reg 1 (DRAM #1)
	 * ---------------------------------------------------
	 */
#ifdef CONFIG_NETARM_NET40_REV2
	/* we have to config SDRAM in burst mode */
	ldr	r1, =(	NETARM_MEM_OPT_BASE_USE(NETARM_MMAP_CS1_MASK) | \
			NETARM_MEM_OPT_BCYC_2 | \
			NETARM_MEM_OPT_BSIZE_16 | \
			NETARM_MEM_OPT_WAIT_STATES(0) | \
			NETARM_MEM_OPT_32BIT | \
			NETARM_MEM_OPT_READ_ASYNC | \
			NETARM_MEM_OPT_WRITE_ASYNC )
	str	r1, [r0, #+NETARM_MEM_CS1_OPTIONS]

	ldr	r1, =(	NETARM_MEM_BAR_BASE(NETARM_MMAP_CS1_BASE) | \
			NETARM_MEM_BAR_DRAM_SYNC | \
			NETARM_MEM_BAR_DRAM_MUX_INT | \
			NETARM_MEM_BAR_DRAM_MUX_UNBAL | \
			NETARM_MEM_BAR_DRAM_SEL | \
			NETARM_MEM_BAR_BURST_EN | \
			NETARM_MEM_BAR_VALID )
	str	r1, [r0, #+NETARM_MEM_CS1_BASE_ADDR]
#else
	/* we have to config FPDRAM in burst mode with smaller burst access size */
	ldr	r1, =(	NETARM_MEM_OPT_BASE_USE(NETARM_MMAP_CS1_MASK) | \
			NETARM_MEM_OPT_BCYC_2 | \
			NETARM_MEM_OPT_BSIZE_16 | \
			NETARM_MEM_OPT_WAIT_STATES(0) | \
			NETARM_MEM_OPT_32BIT | \
			NETARM_MEM_OPT_READ_ASYNC | \
			NETARM_MEM_OPT_WRITE_ASYNC )
	str	r1, [r0, #+NETARM_MEM_CS1_OPTIONS]

	ldr	r1, =(	NETARM_MEM_BAR_BASE(NETARM_MMAP_CS1_BASE) | \
			NETARM_MEM_BAR_DRAM_SYNC | \
			NETARM_MEM_BAR_DRAM_MUX_INT | \
			NETARM_MEM_BAR_DRAM_MUX_UNBAL | \
			NETARM_MEM_BAR_DRAM_SEL | \
			NETARM_MEM_BAR_BURST_EN | \
			NETARM_MEM_BAR_VALID )
	str	r1, [r0, #+NETARM_MEM_CS1_BASE_ADDR]

#endif /* CONFIG_NETARM_NET40_REV2 */


memsetup_cs3:
	/*
	 * Base Addr / Option Reg 3 (EEPROM, NVRAM)
	 * ---------------------------------------------------
	 */
#if defined(CONFIG_NETARM_EEPROM) && defined(PHYS_NVRAM_1) && defined(PHYS_NVRAM_SIZE)
	ldr	r1, =(	NETARM_MEM_OPT_BASE_USE(NETARM_MMAP_CS3_MASK) | \
			NETARM_MEM_OPT_BCYC_3 | \
			NETARM_MEM_OPT_BSIZE_2 | \
			NETARM_MEM_OPT_WAIT_STATES(10) | \
			NETARM_MEM_OPT_8BIT | \
			NETARM_MEM_OPT_READ_ASYNC | \
			NETARM_MEM_OPT_WRITE_ASYNC )
	str	r1, [r0, #+NETARM_MEM_CS3_OPTIONS]

	ldr	r1, =(	NETARM_MEM_BAR_BASE(NETARM_MMAP_CS3_BASE) | \
			NETARM_MEM_BAR_DRAM_FP | \
			NETARM_MEM_BAR_DRAM_MUX_INT | \
			NETARM_MEM_BAR_DRAM_MUX_BAL | \
			NETARM_MEM_BAR_VALID )
	str	r1, [r0, #+NETARM_MEM_CS3_BASE_ADDR]
#else
	/* we don't need EEPROM --> no config */
	ldr	r1, =( 0 )
	str	r1, [r0, #+NETARM_MEM_CS3_OPTIONS]

	ldr	r1, =( 0 )
	str	r1, [r0, #+NETARM_MEM_CS3_BASE_ADDR]
#endif


#else
/*
#error "missing CONFIG_MODNET50 (see your config.h)"
*/
#endif /* CONFIG_MODNET50 */


lowlevel_init_end:
	/*
	 * manipulate address in lr and ip to match new
	 * address space
	 */
	ldr	r3, =(NETARM_MMAP_CS0_BASE)
	mov	r0, lr
	add	r0, r3, r0
	mov	lr, r0
	mov	r0, ip
	add	r0, r3, r0
	mov	ip, r0

	/* everything is fine now */
	mov	pc, lr
