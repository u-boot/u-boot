/*
 * U-boot - start.S Startup file for Blackfin u-boot
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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
#include <asm/mach-common/bits/dma.h>
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
	sp.l = LO(L1_SRAM_SCRATCH + L1_SRAM_SCRATCH_SIZE);
	sp.h = HI(L1_SRAM_SCRATCH + L1_SRAM_SCRATCH_SIZE);

#ifdef CONFIG_HW_WATCHDOG
# ifndef CONFIG_HW_WATCHDOG_TIMEOUT_START
#  define CONFIG_HW_WATCHDOG_TIMEOUT_START 5000
# endif
	/* Program the watchdog with an initial timeout of ~5 seconds.
	 * That should be long enough to bootstrap ourselves up and
	 * then the common u-boot code can take over.
	 */
	P0.L = LO(WDOG_CNT);
	P0.H = HI(WDOG_CNT);
	R0.L = 0;
	R0.H = HI(MSEC_TO_SCLK(CONFIG_HW_WATCHDOG_TIMEOUT_START));
	[P0] = R0;
	/* fire up the watchdog - R0.L above needs to be 0x0000 */
	W[P0 + (WDOG_CTL - WDOG_CNT)] = R0;
#endif

	/* Turn on the serial for debugging the init process */
	serial_early_init
	serial_early_set_baud

	serial_early_puts("Init Registers");

	/* Disable nested interrupts and enable CYCLES for udelay() */
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

#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS)
	/* In bypass mode, we don't have an LDR with an init block
	 * so we need to explicitly call it ourselves.  This will
	 * reprogram our clocks and setup our async banks.
	 */
	/* XXX: we should DMA this into L1, put external memory into
	 *      self refresh, and then jump there ...
	 */
	call _get_pc;
	r3 = 0x0;
	r3.h = 0x2000;
	cc = r0 < r3 (iu);
	if cc jump .Lproc_initialized;

	serial_early_puts("Program Clocks");

	call _initcode;

	/* Since we reprogrammed SCLK, we need to update the serial divisor */
	serial_early_set_baud

.Lproc_initialized:
#endif

	/* Inform upper layers if we had to do the relocation ourselves.
	 * This allows us to detect whether we were loaded by 'go 0x1000'
	 * or by the bootrom from an LDR.  "r6" is "loaded_from_ldr".
	 */
	r6 = 1 (x);

	/* Relocate from wherever are (FLASH/RAM/etc...) to the
	 * hardcoded monitor location in the end of RAM.
	 */
	serial_early_puts("Relocate");
	call _get_pc;
.Loffset:
	r2.l = .Loffset;
	r2.h = .Loffset;
	r3.l = _start;
	r3.h = _start;
	r1 = r2 - r3;

	r0 = r0 - r1;

	cc = r0 == r3;
	if cc jump .Lnorelocate;

	r6 = 0 (x);
	p1 = r0;

	p2.l = LO(CFG_MONITOR_BASE);
	p2.h = HI(CFG_MONITOR_BASE);

	p3 = 0x04;
	p4.l = LO(CFG_MONITOR_BASE + CFG_MONITOR_LEN);
	p4.h = HI(CFG_MONITOR_BASE + CFG_MONITOR_LEN);
.Lloop1:
	r1 = [p1 ++ p3];
	[p2 ++ p3] = r1;
	cc=p2==p4;
	if !cc jump .Lloop1;

	/* Initialize BSS section ... we know that memset() does not
	 * use the BSS, so it is safe to call here.  The bootrom LDR
	 * takes care of clearing things for us.
	 */
	serial_early_puts("Zero BSS");
	r0.l = __bss_start;
	r0.h = __bss_start;
	r1 = 0 (x);
	r2.l = __bss_end;
	r2.h = __bss_end;
	r2 = r2 - r0;
	call _memset;

.Lnorelocate:

	/* Setup the actual stack in external memory */
	r0.h = HI(CONFIG_STACKBASE);
	r0.l = LO(CONFIG_STACKBASE);
	sp = r0;
	fp = sp;

	/* Now lower ourselves from the highest interrupt level to
	 * the lowest.  We do this by masking all interrupts but 15,
	 * setting the 15 handler to "board_init_f", raising the 15
	 * interrupt, and then returning from the highest interrupt
	 * level to the dummy "jump" until the interrupt controller
	 * services the pending 15 interrupt.
	 */
	serial_early_puts("Lower to 15");
	r0 = r7;
	r1 = r6;
	p0.l = LO(EVT15);
	p0.h = HI(EVT15);
	p1.l = _cpu_init_f;
	p1.h = _cpu_init_f;
	[p0] = p1;
	p2.l = LO(IMASK);
	p2.h = HI(IMASK);
	p3.l = LO(EVT_IVG15);
	p3.h = HI(EVT_IVG15);
	[p2] = p3;
	raise 15;
	p4.l = .LWAIT_HERE;
	p4.h = .LWAIT_HERE;
	reti = p4;
	rti;

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
