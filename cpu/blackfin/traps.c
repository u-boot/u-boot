/*
 * U-boot - traps.c Routines related to interrupts and exceptions
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * This file is based on
 * No original Copyright holder listed,
 * Probabily original (C) Roman Zippel (assigned DJD, 1999)
 *
 * Copyright 2003 Metrowerks - for Blackfin
 * Copyright 2000-2001 Lineo, Inc. D. Jeff Dionne <jeff@lineo.ca>
 * Copyright 1999-2000 D. Jeff Dionne, <jeff@uclinux.org>
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <linux/types.h>
#include <asm/traps.h>
#include <asm/cplb.h>
#include <asm/io.h>
#include <asm/mach-common/bits/core.h>
#include <asm/mach-common/bits/mpu.h>
#include <asm/mach-common/bits/trace.h>
#include "cpu.h"

#define trace_buffer_save(x) \
	do { \
		(x) = bfin_read_TBUFCTL(); \
		bfin_write_TBUFCTL((x) & ~TBUFEN); \
	} while (0)

#define trace_buffer_restore(x) \
	bfin_write_TBUFCTL((x))

/* The purpose of this map is to provide a mapping of address<->cplb settings
 * rather than an exact map of what is actually addressable on the part.  This
 * map covers all current Blackfin parts.  If you try to access an address that
 * is in this map but not actually on the part, you won't get an exception and
 * reboot, you'll get an external hardware addressing error and reboot.  Since
 * only the ends matter (you did something wrong and the board reset), the means
 * are largely irrelevant.
 */
struct memory_map {
	uint32_t start, end;
	uint32_t data_flags, inst_flags;
};
const struct memory_map const bfin_memory_map[] = {
	{	/* external memory */
		.start = 0x00000000,
		.end   = 0x20000000,
		.data_flags = SDRAM_DGENERIC,
		.inst_flags = SDRAM_IGENERIC,
	},
	{	/* async banks */
		.start = 0x20000000,
		.end   = 0x30000000,
		.data_flags = SDRAM_EBIU,
		.inst_flags = SDRAM_INON_CHBL,
	},
	{	/* everything on chip */
		.start = 0xE0000000,
		.end   = 0xFFFFFFFF,
		.data_flags = L1_DMEMORY,
		.inst_flags = L1_IMEMORY,
	}
};

void trap_c(struct pt_regs *regs)
{
	uint32_t trapnr = (regs->seqstat & EXCAUSE);
	bool data = false;

	switch (trapnr) {
	/* 0x26 - Data CPLB Miss */
	case VEC_CPLB_M:

		if (ANOMALY_05000261) {
			static uint32_t last_cplb_fault_retx;
			/*
			 * Work around an anomaly: if we see a new DCPLB fault,
			 * return without doing anything. Then,
			 * if we get the same fault again, handle it.
			 */
			if (last_cplb_fault_retx != regs->retx) {
				last_cplb_fault_retx = regs->retx;
				return;
			}
		}

		data = true;
		/* fall through */

	/* 0x27 - Instruction CPLB Miss */
	case VEC_CPLB_I_M: {
		volatile uint32_t *CPLB_ADDR_BASE, *CPLB_DATA_BASE, *CPLB_ADDR, *CPLB_DATA;
		uint32_t new_cplb_addr = 0, new_cplb_data = 0;
		static size_t last_evicted;
		size_t i;

		new_cplb_addr = (data ? bfin_read_DCPLB_FAULT_ADDR() : bfin_read_ICPLB_FAULT_ADDR()) & ~(4 * 1024 * 1024 - 1);

		for (i = 0; i < ARRAY_SIZE(bfin_memory_map); ++i) {
			/* if the exception is inside this range, lets use it */
			if (new_cplb_addr >= bfin_memory_map[i].start &&
			    new_cplb_addr < bfin_memory_map[i].end)
				break;
		}
		if (i == ARRAY_SIZE(bfin_memory_map)) {
			printf("%cCPLB exception outside of memory map at 0x%p\n",
				(data ? 'D' : 'I'), new_cplb_addr);
			bfin_panic(regs);
		} else
			debug("CPLB addr %p matches map 0x%p - 0x%p\n", new_cplb_addr, bfin_memory_map[i].start, bfin_memory_map[i].end);
		new_cplb_data = (data ? bfin_memory_map[i].data_flags : bfin_memory_map[i].inst_flags);

		/* Turn the cache off */
		SSYNC();
		if (data) {
			asm(" .align 8; ");
			*pDMEM_CONTROL &= ~ENDCPLB;
		} else {
			asm(" .align 8; ");
			*pIMEM_CONTROL &= ~ENICPLB;
		}
		SSYNC();

		if (data) {
			CPLB_ADDR_BASE = (uint32_t *)DCPLB_ADDR0;
			CPLB_DATA_BASE = (uint32_t *)DCPLB_DATA0;
		} else {
			CPLB_ADDR_BASE = (uint32_t *)ICPLB_ADDR0;
			CPLB_DATA_BASE = (uint32_t *)ICPLB_DATA0;
		}

		/* find the next unlocked entry and evict it */
		i = last_evicted & 0xF;
		debug("last evicted = %i\n", i);
		CPLB_DATA = CPLB_DATA_BASE + i;
		while (*CPLB_DATA & CPLB_LOCK) {
			debug("skipping %i %p - %08X\n", i, CPLB_DATA, *CPLB_DATA);
			i = (i + 1) & 0xF;	/* wrap around */
			CPLB_DATA = CPLB_DATA_BASE + i;
		}
		CPLB_ADDR = CPLB_ADDR_BASE + i;

		debug("evicting entry %i: 0x%p 0x%08X\n", i, *CPLB_ADDR, *CPLB_DATA);
		last_evicted = i + 1;
		*CPLB_ADDR = new_cplb_addr;
		*CPLB_DATA = new_cplb_data;

		/* dump current table for debugging purposes */
		CPLB_ADDR = CPLB_ADDR_BASE;
		CPLB_DATA = CPLB_DATA_BASE;
		for (i = 0; i < 16; ++i)
			debug("%2i 0x%p 0x%08X\n", i, *CPLB_ADDR++, *CPLB_DATA++);

		/* Turn the cache back on */
		SSYNC();
		if (data) {
			asm(" .align 8; ");
			*pDMEM_CONTROL |= ENDCPLB;
		} else {
			asm(" .align 8; ");
			*pIMEM_CONTROL |= ENICPLB;
		}
		SSYNC();

		break;
	}

	default:
		/* All traps come here */
		bfin_panic(regs);
	}
}

#ifdef CONFIG_DEBUG_DUMP
# define ENABLE_DUMP 1
#else
# define ENABLE_DUMP 0
#endif

#ifdef CONFIG_DEBUG_DUMP_SYMS
# define ENABLE_DUMP_SYMS 1
#else
# define ENABLE_DUMP_SYMS 0
#endif

static const char *symbol_lookup(unsigned long addr, unsigned long *caddr)
{
	if (!ENABLE_DUMP_SYMS)
		return NULL;

	extern const char system_map[] __attribute__((__weak__));
	const char *sym, *csym;
	char *esym;
	unsigned long sym_addr;

	sym = system_map;
	csym = NULL;
	*caddr = 0;

	while (*sym) {
		sym_addr = simple_strtoul(sym, &esym, 16);
		sym = esym + 1;
		if (sym_addr > addr)
			break;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	return csym;
}

static void decode_address(char *buf, unsigned long address)
{
	unsigned long sym_addr;
	const char *sym = symbol_lookup(address, &sym_addr);

	if (sym) {
		sprintf(buf, "<0x%p> { %s + 0x%x }", address, sym, address - sym_addr);
		return;
	}

	if (!address)
		sprintf(buf, "<0x%p> /* Maybe null pointer? */", address);
	else if (address >= CFG_MONITOR_BASE &&
	         address < CFG_MONITOR_BASE + CFG_MONITOR_LEN)
		sprintf(buf, "<0x%p> /* somewhere in u-boot */", address);
	else
		sprintf(buf, "<0x%p> /* unknown address */", address);
}

void dump(struct pt_regs *fp)
{
	char buf[150];
	size_t i;

	if (!ENABLE_DUMP)
		return;

	printf("SEQUENCER STATUS:\n");
	printf(" SEQSTAT: %08lx  IPEND: %04lx  SYSCFG: %04lx\n",
		fp->seqstat, fp->ipend, fp->syscfg);
	printf("  HWERRCAUSE: 0x%lx\n", (fp->seqstat & HWERRCAUSE) >> HWERRCAUSE_P);
	printf("  EXCAUSE   : 0x%lx\n", (fp->seqstat & EXCAUSE) >> EXCAUSE_P);
	for (i = 6; i <= 15; ++i) {
		if (fp->ipend & (1 << i)) {
			decode_address(buf, bfin_read32(EVT0 + 4*i));
			printf("  physical IVG%i asserted : %s\n", i, buf);
		}
	}
	decode_address(buf, fp->rete);
	printf(" RETE: %s\n", buf);
	decode_address(buf, fp->retn);
	printf(" RETN: %s\n", buf);
	decode_address(buf, fp->retx);
	printf(" RETX: %s\n", buf);
	decode_address(buf, fp->rets);
	printf(" RETS: %s\n", buf);
	decode_address(buf, fp->pc);
	printf(" PC  : %s\n", buf);

	if (fp->seqstat & EXCAUSE) {
		decode_address(buf, bfin_read_DCPLB_FAULT_ADDR());
		printf("DCPLB_FAULT_ADDR: %s\n", buf);
		decode_address(buf, bfin_read_ICPLB_FAULT_ADDR());
		printf("ICPLB_FAULT_ADDR: %s\n", buf);
	}

	printf("\nPROCESSOR STATE:\n");
	printf(" R0 : %08lx    R1 : %08lx    R2 : %08lx    R3 : %08lx\n",
		fp->r0, fp->r1, fp->r2, fp->r3);
	printf(" R4 : %08lx    R5 : %08lx    R6 : %08lx    R7 : %08lx\n",
		fp->r4, fp->r5, fp->r6, fp->r7);
	printf(" P0 : %08lx    P1 : %08lx    P2 : %08lx    P3 : %08lx\n",
		fp->p0, fp->p1, fp->p2, fp->p3);
	printf(" P4 : %08lx    P5 : %08lx    FP : %08lx    SP : %08lx\n",
		fp->p4, fp->p5, fp->fp, fp);
	printf(" LB0: %08lx    LT0: %08lx    LC0: %08lx\n",
		fp->lb0, fp->lt0, fp->lc0);
	printf(" LB1: %08lx    LT1: %08lx    LC1: %08lx\n",
		fp->lb1, fp->lt1, fp->lc1);
	printf(" B0 : %08lx    L0 : %08lx    M0 : %08lx    I0 : %08lx\n",
		fp->b0, fp->l0, fp->m0, fp->i0);
	printf(" B1 : %08lx    L1 : %08lx    M1 : %08lx    I1 : %08lx\n",
		fp->b1, fp->l1, fp->m1, fp->i1);
	printf(" B2 : %08lx    L2 : %08lx    M2 : %08lx    I2 : %08lx\n",
		fp->b2, fp->l2, fp->m2, fp->i2);
	printf(" B3 : %08lx    L3 : %08lx    M3 : %08lx    I3 : %08lx\n",
		fp->b3, fp->l3, fp->m3, fp->i3);
	printf("A0.w: %08lx   A0.x: %08lx   A1.w: %08lx   A1.x: %08lx\n",
		fp->a0w, fp->a0x, fp->a1w, fp->a1x);

	printf("USP : %08lx  ASTAT: %08lx\n",
		fp->usp, fp->astat);

	printf("\n");
}

void dump_bfin_trace_buffer(void)
{
	char buf[150];
	unsigned long tflags;
	size_t i = 0;

	if (!ENABLE_DUMP)
		return;

	trace_buffer_save(tflags);

	printf("Hardware Trace:\n");

	if (bfin_read_TBUFSTAT() & TBUFCNT) {
		for (; bfin_read_TBUFSTAT() & TBUFCNT; i++) {
			decode_address(buf, bfin_read_TBUF());
			printf("%4i Target : %s\n", i, buf);
			decode_address(buf, bfin_read_TBUF());
			printf("     Source : %s\n", buf);
		}
	}

	trace_buffer_restore(tflags);
}

void bfin_panic(struct pt_regs *regs)
{
	if (ENABLE_DUMP) {
		unsigned long tflags;
		trace_buffer_save(tflags);
	}

	puts(
		"\n"
		"\n"
		"\n"
		"Ack! Something bad happened to the Blackfin!\n"
		"\n"
	);
	dump(regs);
	dump_bfin_trace_buffer();
	printf(
		"\n"
		"Please reset the board\n"
		"\n"
	);
	bfin_reset_or_hang();
}
