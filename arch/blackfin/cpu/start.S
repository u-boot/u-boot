/*
 * U-boot - start.S Startup file for Blackfin u-boot
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * This file is based on head.S
 * Copyright (c) 2003  Metrowerks/Motorola
 * Copyright (C) 1998  D. Jeff Dionne <jeff@ryeham.ee.ryerson.ca>,
 *                     Kenneth Albanowski <kjahds@kjahds.com>,
 *                     The Silver Hammer Group, Ltd.
 * (c) 1995, Dionne & Associates
 * (c) 1995, DKG Display Tech.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <config.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/core.h>
#include <asm/mach-common/bits/pll.h>

#include "serial.h"

/* It may seem odd that we make calls to functions even though we haven't
 * relocated ourselves yet out of {flash,ram,wherever}.  This is OK because
 * the "call" instruction in the Blackfin architecture is actually PC
 * relative.  So we can call functions all we want and not worry about them
 * not being relocated yet.
 */

.text
ENTRY(_start)

	/* Set our initial stack to L1 scratch space */
	sp.l = LO(L1_SRAM_SCRATCH_END - 20);
	sp.h = HI(L1_SRAM_SCRATCH_END - 20);

	/* Optimization register tricks: keep a base value in the
	 * reserved P registers so we use the load/store with an
	 * offset syntax.  R0 = [P5 + <constant>];
	 *   P4 - system MMR base
	 *   P5 - core MMR base
	 */
#ifdef CONFIG_HW_WATCHDOG
	p4.l = 0;
	p4.h = HI(SYSMMR_BASE);
#endif
	p5.l = 0;
	p5.h = HI(COREMMR_BASE);

#ifdef CONFIG_HW_WATCHDOG
# ifndef CONFIG_HW_WATCHDOG_TIMEOUT_START
#  define CONFIG_HW_WATCHDOG_TIMEOUT_START 5000
# endif
	/* Program the watchdog with an initial timeout of ~5 seconds.
	 * That should be long enough to bootstrap ourselves up and
	 * then the common u-boot code can take over.
	 */
	r0 = 0;
	r0.h = HI(MSEC_TO_SCLK(CONFIG_HW_WATCHDOG_TIMEOUT_START));
	[p4 + (WDOG_CNT - SYSMMR_BASE)] = r0;
	/* fire up the watchdog - R0.L above needs to be 0x0000 */
	W[p4 + (WDOG_CTL - SYSMMR_BASE)] = r0;
#endif

	/* Turn on the serial for debugging the init process */
	serial_early_init
	serial_early_set_baud

	serial_early_puts("Init Registers");

	/* Disable self-nested interrupts and enable CYCLES for udelay() */
	R0 = CCEN | 0x30;
	SYSCFG = R0;

	/* Zero out registers required by Blackfin ABI.
	 * http://docs.blackfin.uclinux.org/doku.php?id=application_binary_interface
	 */
	r1 = 0 (x);
	/* Disable circular buffers */
	l0 = r1;
	l1 = r1;
	l2 = r1;
	l3 = r1;
	/* Disable hardware loops in case we were started by 'go' */
	lc0 = r1;
	lc1 = r1;

	/* Save RETX so we can pass it while booting Linux */
	r7 = RETX;

#if CONFIG_MEM_SIZE
	/* Figure out where we are currently executing so that we can decide
	 * how to best reprogram and relocate things.  We'll pass below:
	 *  R4: load address of _start
	 *  R5: current (not load) address of _start
	 */
	serial_early_puts("Find ourselves");

	call _get_pc;
.Loffset:
	r1.l = .Loffset;
	r1.h = .Loffset;
	r4.l = _start;
	r4.h = _start;
	r3 = r1 - r4;
	r5 = r0 - r3;

	/* Inform upper layers if we had to do the relocation ourselves.
	 * This allows us to detect whether we were loaded by 'go 0x1000'
	 * or by the bootrom from an LDR.  "R6" is "loaded_from_ldr".
	 */
	r6 = 1 (x);
	cc = r4 == r5;
	if cc jump .Lnorelocate;
	r6 = 0 (x);

	/* Turn off caches as they require CPLBs and a CPLB miss requires
	 * a software exception handler to process it.  But we're about to
	 * clobber any previous executing software (like U-Boot that just
	 * launched a new U-Boot via 'go'), so any handler state will be
	 * unreliable after the memcpy below.
	 */
	serial_early_puts("Kill Caches");
	r0 = 0;
	[p5 + (IMEM_CONTROL - COREMMR_BASE)] = r0;
	[p5 + (DMEM_CONTROL - COREMMR_BASE)] = r0;
	ssync;

	/* In bypass mode, we don't have an LDR with an init block
	 * so we need to explicitly call it ourselves.  This will
	 * reprogram our clocks, memory, and setup our async banks.
	 */
	serial_early_puts("Program Clocks");

	/* if we're executing >=0x20000000, then we dont need to dma */
	r3 = 0x0;
	r3.h = 0x2000;
	cc = r5 < r3 (iu);
	if cc jump .Ldma_and_reprogram;
#else
	r6 = 1 (x);	/* fake loaded_from_ldr = 1 */
#endif
	r0 = 0 (x);	/* set bootstruct to NULL */
	call _initcode;
	jump .Lprogrammed;

	/* we're sitting in external memory, so dma into L1 and reprogram */
.Ldma_and_reprogram:
	r0.l = LO(L1_INST_SRAM);
	r0.h = HI(L1_INST_SRAM);
	r1.l = __initcode_lma;
	r1.h = __initcode_lma;
	r2.l = __initcode_len;
	r2.h = __initcode_len;
	r1 = r1 - r4;	/* convert r1 from load address of initcode ... */
	r1 = r1 + r5;	/* ... to current (not load) address of initcode */
	p3 = r0;
	call _dma_memcpy_nocache;
	r0 = 0 (x);	/* set bootstruct to NULL */
	call (p3);

	/* Since we reprogrammed SCLK, we need to update the serial divisor */
.Lprogrammed:
	serial_early_set_baud

#if CONFIG_MEM_SIZE
	/* Relocate from wherever we are (FLASH/RAM/etc...) to the hardcoded
	 * monitor location in the end of RAM.  We know that memcpy() only
	 * uses registers, so it is safe to call here.  Note that this only
	 * copies to external memory ... we do not start executing out of
	 * it yet (see "lower to 15" below).
	 */
	serial_early_puts("Relocate");
	r0 = r4;
	r1 = r5;
	r2.l = LO(CONFIG_SYS_MONITOR_LEN);
	r2.h = HI(CONFIG_SYS_MONITOR_LEN);
	call _memcpy_ASM;
#endif

	/* Initialize BSS section ... we know that memset() does not
	 * use the BSS, so it is safe to call here.  The bootrom LDR
	 * takes care of clearing things for us.
	 */
	serial_early_puts("Zero BSS");
	r0.l = __bss_vma;
	r0.h = __bss_vma;
	r1 = 0 (x);
	r2.l = __bss_len;
	r2.h = __bss_len;
	call _memset;

.Lnorelocate:

	/* Setup the actual stack in external memory */
	sp.h = HI(CONFIG_STACKBASE);
	sp.l = LO(CONFIG_STACKBASE);
	fp = sp;

	/* Now lower ourselves from the highest interrupt level to
	 * the lowest.  We do this by masking all interrupts but 15,
	 * setting the 15 handler to ".Lenable_nested", raising the 15
	 * interrupt, and then returning from the highest interrupt
	 * level to the dummy "jump" until the interrupt controller
	 * services the pending 15 interrupt.  If executing out of
	 * flash, these steps also changes the code flow from flash
	 * to external memory.
	 */
	serial_early_puts("Lower to 15");
	r0 = r7;
	r1 = r6;
	p1.l = .Lenable_nested;
	p1.h = .Lenable_nested;
	[p5 + (EVT15 - COREMMR_BASE)] = p1;
	r7 = EVT_IVG15 (z);
	sti r7;
	raise 15;
	p3.l = .LWAIT_HERE;
	p3.h = .LWAIT_HERE;
	reti = p3;
	rti;

	/* Enable nested interrupts before continuing with cpu init */
.Lenable_nested:
	cli r7;
	[--sp] = reti;
	jump.l _cpu_init_f;

.LWAIT_HERE:
	jump .LWAIT_HERE;
ENDPROC(_start)

LENTRY(_get_pc)
	r0 = rets;
#if ANOMALY_05000371
	NOP;
	NOP;
	NOP;
#endif
	rts;
ENDPROC(_get_pc)
