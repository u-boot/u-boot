/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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


/*
 * Partly based on msbios.c from rolo 1.6:
 *----------------------------------------------------------------------
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions GmbH
 * Klein-Winternheim, Germany
 *----------------------------------------------------------------------
 */

#include <common.h>
#include <pci.h>
#include <asm/realmode.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define NUMVECTS	256

#define BIOS_DATA        ((char*)0x400)
#define BIOS_DATA_SIZE   256
#define BIOS_BASE        ((char*)0xf0000)
#define BIOS_CS          0xf000

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

static int set_jmp_vector(int entry_point, void *target)
{
	if (entry_point & ~0xffff) {
		return -1;
	}

	if (((u32)target-0xf0000) & ~0xffff) {
		return -1;
	}
	printf("set_jmp_vector: 0xf000:%04x -> %p\n",
	       entry_point, target);

	/* jmp opcode */
	writeb(0xea, 0xf0000 + entry_point);

	/* offset */
	writew(((u32)target-0xf0000), 0xf0000 + entry_point + 1);

	/* segment */
	writew(0xf000, 0xf0000 + entry_point + 3);

	return 0;
}


/*
 ************************************************************
 * Install an interrupt vector
 ************************************************************
 */

static void setvector(int vector, u16 segment, void *handler)
{
	u16 *ptr = (u16*)(vector*4);
	ptr[0] = ((u32)handler - (segment << 4))&0xffff;
	ptr[1] = segment;

#if 0
	printf("setvector: int%02x -> %04x:%04x\n",
	       vector, ptr[1], ptr[0]);
#endif
}

#define RELOC_16_LONG(seg, off) *(u32*)(seg << 4 | (u32)&off)
#define RELOC_16_WORD(seg, off) *(u16*)(seg << 4 | (u32)&off)
#define RELOC_16_BYTE(seg, off) *(u8*)(seg << 4 | (u32)&off)

int bios_setup(void)
{
	static int done=0;
	int vector;
	struct pci_controller *pri_hose;

	if (done) {
		return 0;
	}
	done = 1;

	if (i386boot_bios_size > 65536) {
		printf("BIOS too large (%ld bytes, max is 65536)\n",
		       i386boot_bios_size);
		return -1;
	}

	memcpy(BIOS_BASE, (void*)i386boot_bios, i386boot_bios_size);

	/* clear bda */
	memset(BIOS_DATA, 0, BIOS_DATA_SIZE);

	/* enter some values to the bda */
	writew(0x3f8, BIOS_DATA);   /* com1 addr */
	writew(0x2f8, BIOS_DATA+2); /* com2 addr */
	writew(0x3e8, BIOS_DATA+4); /* com3 addr */
	writew(0x2e8, BIOS_DATA+6); /* com4 addr */
	writew(0x278, BIOS_DATA+8); /* lpt1 addr */
	/*
	 * The kernel wants to read the base memory size
	 * from 40:13. Put a zero there to avoid an error message
	 */
	writew(0, BIOS_DATA+0x13);  /* base memory size */


	/* setup realmode interrupt vectors */
	for (vector = 0; vector < NUMVECTS; vector++) {
		setvector(vector, BIOS_CS, &rm_def_int);
	}

	setvector(0x00, BIOS_CS, &rm_int00);
	setvector(0x01, BIOS_CS, &rm_int01);
	setvector(0x02, BIOS_CS, &rm_int02);
	setvector(0x03, BIOS_CS, &rm_int03);
	setvector(0x04, BIOS_CS, &rm_int04);
	setvector(0x05, BIOS_CS, &rm_int05);
	setvector(0x06, BIOS_CS, &rm_int06);
	setvector(0x07, BIOS_CS, &rm_int07);
	setvector(0x08, BIOS_CS, &rm_int08);
	setvector(0x09, BIOS_CS, &rm_int09);
	setvector(0x0a, BIOS_CS, &rm_int0a);
	setvector(0x0b, BIOS_CS, &rm_int0b);
	setvector(0x0c, BIOS_CS, &rm_int0c);
	setvector(0x0d, BIOS_CS, &rm_int0d);
	setvector(0x0e, BIOS_CS, &rm_int0e);
	setvector(0x0f, BIOS_CS, &rm_int0f);
	setvector(0x10, BIOS_CS, &rm_int10);
	setvector(0x11, BIOS_CS, &rm_int11);
	setvector(0x12, BIOS_CS, &rm_int12);
	setvector(0x13, BIOS_CS, &rm_int13);
	setvector(0x14, BIOS_CS, &rm_int14);
	setvector(0x15, BIOS_CS, &rm_int15);
	setvector(0x16, BIOS_CS, &rm_int16);
	setvector(0x17, BIOS_CS, &rm_int17);
	setvector(0x18, BIOS_CS, &rm_int18);
	setvector(0x19, BIOS_CS, &rm_int19);
	setvector(0x1a, BIOS_CS, &rm_int1a);
	setvector(0x1b, BIOS_CS, &rm_int1b);
	setvector(0x1c, BIOS_CS, &rm_int1c);
	setvector(0x1d, BIOS_CS, &rm_int1d);
	setvector(0x1e, BIOS_CS, &rm_int1e);
	setvector(0x1f, BIOS_CS, &rm_int1f);

	set_jmp_vector(0xfff0, &realmode_reset);
	set_jmp_vector(0xfe6e, &realmode_pci_bios_call_entry);

	/* fill in data area */
	RELOC_16_WORD(0xf000, ram_in_64kb_chunks) = gd->ram_size >> 16;
	RELOC_16_WORD(0xf000, bios_equipment) = 0; /* FixMe */

	/* If we assume only one PCI hose, this PCI hose
	 * will own PCI bus #0, and the last PCI bus of
	 * that PCI hose will be the last PCI bus in the
	 * system.
	 * (This, ofcause break on multi hose systems,
	 *  but our PCI BIOS only support one hose anyway)
	 */
	pri_hose = pci_bus_to_hose(0);
	if (NULL != pri_hose) {
		/* fill in last pci bus number for use by the realmode
		 * PCI BIOS */
		RELOC_16_BYTE(0xf000, pci_last_bus) = pri_hose->last_busno;
	}

	return 0;
}
