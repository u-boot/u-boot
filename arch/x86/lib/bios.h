/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
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

#ifndef _BIOS_H_
#define _BIOS_H_

#define OFFS_ES		0	/* 16bit */
#define OFFS_GS		2	/* 16bit */
#define OFFS_DS		4	/* 16bit */
#define OFFS_EDI	6	/* 32bit */
#define OFFS_DI		6	/* low 16 bits of EDI */
#define OFFS_ESI	10	/* 32bit */
#define OFFS_SI		10	/* low 16 bits of ESI */
#define OFFS_EBP	14	/* 32bit */
#define OFFS_BP		14	/* low 16 bits of EBP */
#define OFFS_ESP	18	/* 32bit */
#define OFFS_SP		18	/* low 16 bits of ESP */
#define OFFS_EBX	22	/* 32bit */
#define OFFS_BX		22	/* low 16 bits of EBX */
#define OFFS_BL		22	/* low  8 bits of BX */
#define OFFS_BH		23	/* high 8 bits of BX */
#define OFFS_EDX	26	/* 32bit */
#define OFFS_DX		26	/* low 16 bits of EBX */
#define OFFS_DL		26	/* low  8 bits of BX */
#define OFFS_DH		27	/* high 8 bits of BX */
#define OFFS_ECX	30	/* 32bit */
#define OFFS_CX		30	/* low 16 bits of EBX */
#define OFFS_CL		30	/* low  8 bits of BX */
#define OFFS_CH		31	/* high 8 bits of BX */
#define OFFS_EAX	34	/* 32bit */
#define OFFS_AX		34	/* low 16 bits of EBX */
#define OFFS_AL		34	/* low  8 bits of BX */
#define OFFS_AH		35	/* high 8 bits of BX */
#define OFFS_VECTOR	38	/* 16bit */
#define OFFS_IP		40	/* 16bit */
#define OFFS_CS		42	/* 16bit */
#define OFFS_FLAGS	44	/* 16bit */

/* stack at 0x40:0x800 -> 0x800 */
#define SEGMENT		0x40
#define STACK		0x800

/*
 * save general registers
 * save some segments
 * save callers stack segment
 * setup BIOS segments
 * setup BIOS stackpointer
 */
#define MAKE_BIOS_STACK		\
	pushal;			\
	pushw	%ds;		\
	pushw	%gs;		\
	pushw	%es;		\
	pushw	%ss;		\
	popw	%gs;		\
	movw	$SEGMENT, %ax;	\
	movw	%ax, %ds;	\
	movw	%ax, %es;	\
	movw	%ax, %ss;	\
	movw	%sp, %bp;	\
	movw	$STACK, %sp

/*
 * restore callers stack segment
 * restore some segments
 * restore general registers
 */
#define RESTORE_CALLERS_STACK	\
	pushw	%gs;		\
	popw	%ss;		\
	movw	%bp, %sp;	\
	popw	%es;		\
	popw	%gs;		\
	popw	%ds;		\
	popal

#ifndef __ASSEMBLY__
#define BIOS_DATA	((char *)0x400)
#define BIOS_DATA_SIZE	256
#define BIOS_BASE	((char *)0xf0000)
#define BIOS_CS		0xf000

extern ulong __bios_start;
extern ulong __bios_size;

/* these are defined in a 16bit segment and needs
 * to be accessed with the RELOC_16_xxxx() macros below
 */
extern u16 ram_in_64kb_chunks;
extern u16 bios_equipment;
extern u8  pci_last_bus;

extern void *rm_int00;
extern void *rm_int01;
extern void *rm_int02;
extern void *rm_int03;
extern void *rm_int04;
extern void *rm_int05;
extern void *rm_int06;
extern void *rm_int07;
extern void *rm_int08;
extern void *rm_int09;
extern void *rm_int0a;
extern void *rm_int0b;
extern void *rm_int0c;
extern void *rm_int0d;
extern void *rm_int0e;
extern void *rm_int0f;
extern void *rm_int10;
extern void *rm_int11;
extern void *rm_int12;
extern void *rm_int13;
extern void *rm_int14;
extern void *rm_int15;
extern void *rm_int16;
extern void *rm_int17;
extern void *rm_int18;
extern void *rm_int19;
extern void *rm_int1a;
extern void *rm_int1b;
extern void *rm_int1c;
extern void *rm_int1d;
extern void *rm_int1e;
extern void *rm_int1f;
extern void *rm_def_int;

extern void *realmode_reset;
extern void *realmode_pci_bios_call_entry;

#define RELOC_16_LONG(seg, off) (*(u32 *)(seg << 4 | (u32)&off))
#define RELOC_16_WORD(seg, off) (*(u16 *)(seg << 4 | (u32)&off))
#define RELOC_16_BYTE(seg, off) (*(u8 *)(seg << 4 | (u32)&off))

#ifdef PCI_BIOS_DEBUG
extern u32 num_pci_bios_present;
extern u32 num_pci_bios_find_device;
extern u32 num_pci_bios_find_class;
extern u32 num_pci_bios_generate_special_cycle;
extern u32 num_pci_bios_read_cfg_byte;
extern u32 num_pci_bios_read_cfg_word;
extern u32 num_pci_bios_read_cfg_dword;
extern u32 num_pci_bios_write_cfg_byte;
extern u32 num_pci_bios_write_cfg_word;
extern u32 num_pci_bios_write_cfg_dword;
extern u32 num_pci_bios_get_irq_routing;
extern u32 num_pci_bios_set_irq;
extern u32 num_pci_bios_unknown_function;
#endif

#endif

#endif
