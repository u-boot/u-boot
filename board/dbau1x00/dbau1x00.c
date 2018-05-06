// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Thomas.Lange@corelatus.se
 */

#include <common.h>
#include <command.h>
#include <mach/au1x00.h>
#include <asm/mipsregs.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* Sdram is setup by assembler code */
	/* If memory could be changed, we should return the true value here */
	gd->ram_size = MEM_SIZE * 1024 * 1024;

	return 0;
}

#define BCSR_PCMCIA_PC0DRVEN		0x0010
#define BCSR_PCMCIA_PC0RST		0x0080

/* In arch/mips/cpu/cpu.c */
void write_one_tlb( int index, u32 pagemask, u32 hi, u32 low0, u32 low1 );

int checkboard (void)
{
#ifdef CONFIG_IDE_PCMCIA
	u16 status;
	volatile u32 *pcmcia_bcsr = (u32*)(DB1XX0_BCSR_ADDR+0x10);
#endif	/* CONFIG_IDE_PCMCIA */
	volatile u32 *phy = (u32*)(DB1XX0_BCSR_ADDR+0xC);
	volatile u32 *sys_counter = (volatile u32*)SYS_COUNTER_CNTRL;
	u32 proc_id;

	*sys_counter = 0x100; /* Enable 32 kHz oscillator for RTC/TOY */

	proc_id = read_c0_prid();

	switch (proc_id >> 24) {
	case 0:
		puts ("Board: Merlot (DbAu1000)\n");
		printf ("CPU: Au1000 396 MHz, id: 0x%02x, rev: 0x%02x\n",
			(proc_id >> 8) & 0xFF, proc_id & 0xFF);
		break;
	case 1:
		puts ("Board: DbAu1500\n");
		printf ("CPU: Au1500, id: 0x%02x, rev: 0x%02x\n",
			(proc_id >> 8) & 0xFF, proc_id & 0xFF);
		break;
	case 2:
		puts ("Board: DbAu1100\n");
		printf ("CPU: Au1100, id: 0x%02x, rev: 0x%02x\n",
			(proc_id >> 8) & 0xFF, proc_id & 0xFF);
		break;
	case 3:
		puts ("Board: DbAu1550\n");
		printf ("CPU: Au1550, id: 0x%02x, rev: 0x%02x\n",
			(proc_id >> 8) & 0xFF, proc_id & 0xFF);
		break;
	default:
		printf ("Unsupported cpu %d, proc_id=0x%x\n", proc_id >> 24, proc_id);
	}

	set_io_port_base(0);

#ifdef CONFIG_IDE_PCMCIA
	/* Enable 3.3 V on slot 0 ( VCC )
	   No 5V */
	status = 4;
	*pcmcia_bcsr = status;

	status |= BCSR_PCMCIA_PC0DRVEN;
	*pcmcia_bcsr = status;
	au_sync();

	udelay(300*1000);

	status |= BCSR_PCMCIA_PC0RST;
	*pcmcia_bcsr = status;
	au_sync();

	udelay(100*1000);

	/* PCMCIA is on a 36 bit physical address.
	   We need to map it into a 32 bit addresses */

#if 0
	/* We dont need theese unless we run whole pcmcia package */
	write_one_tlb(20,                 /* index */
		      0x01ffe000,         /* Pagemask, 16 MB pages */
		      CONFIG_SYS_PCMCIA_IO_BASE, /* Hi */
		      0x3C000017,         /* Lo0 */
		      0x3C200017);        /* Lo1 */

	write_one_tlb(21,                   /* index */
		      0x01ffe000,           /* Pagemask, 16 MB pages */
		      CONFIG_SYS_PCMCIA_ATTR_BASE, /* Hi */
		      0x3D000017,           /* Lo0 */
		      0x3D200017);          /* Lo1 */
#endif	/* 0 */
	write_one_tlb(22,                   /* index */
		      0x01ffe000,           /* Pagemask, 16 MB pages */
		      CONFIG_SYS_PCMCIA_MEM_ADDR,  /* Hi */
		      0x3E000017,           /* Lo0 */
		      0x3E200017);          /* Lo1 */
#endif	/* CONFIG_IDE_PCMCIA */

	/* Release reset of ethernet PHY chips */
	/* Always do this, because linux does not know about it */
	*phy = 3;

	return 0;
}
