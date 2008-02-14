/*
 * U-boot - cpu.c CPU specific functions
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <asm/blackfin.h>
#include <command.h>
#include <asm/entry.h>
#include <asm/cplb.h>
#include <asm/io.h>

#define CACHE_ON 1
#define CACHE_OFF 0

extern unsigned int icplb_table[page_descriptor_table_size][2];
extern unsigned int dcplb_table[page_descriptor_table_size][2];

int do_reset(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	__asm__ __volatile__("cli r3;" "P0 = %0;" "JUMP (P0);"::"r"(L1_INST_SRAM)
	    );

	return 0;
}

/* These functions are just used to satisfy the linker */
int cpu_init(void)
{
	return 0;
}

int cleanup_before_linux(void)
{
	return 0;
}

void icache_enable(void)
{
	unsigned int *I0, *I1;
	int i, j = 0;

	/* Before enable icache, disable it first */
	icache_disable();
	I0 = (unsigned int *)ICPLB_ADDR0;
	I1 = (unsigned int *)ICPLB_DATA0;

	/* make sure the locked ones go in first */
	for (i = 0; i < page_descriptor_table_size; i++) {
		if (CPLB_LOCK & icplb_table[i][1]) {
			debug("adding %02i %02i 0x%08x 0x%08x\n", i, j,
			      icplb_table[i][0], icplb_table[i][1]);
			*I0++ = icplb_table[i][0];
			*I1++ = icplb_table[i][1];
			j++;
		}
	}

	for (i = 0; i < page_descriptor_table_size; i++) {
		if (!(CPLB_LOCK & icplb_table[i][1])) {
			debug("adding %02i %02i 0x%08x 0x%08x\n", i, j,
			      icplb_table[i][0], icplb_table[i][1]);
			*I0++ = icplb_table[i][0];
			*I1++ = icplb_table[i][1];
			j++;
			if (j == 16) {
				break;
			}
		}
	}

	/* Fill the rest with invalid entry */
	if (j <= 15) {
		for (; j < 16; j++) {
			debug("filling %i with 0", j);
			*I1++ = 0x0;
		}

	}

	SSYNC();
	asm(" .align 8; ");
	*(unsigned int *)IMEM_CONTROL = IMC | ENICPLB;
	SSYNC();
}

void icache_disable(void)
{
	SSYNC();
	asm(" .align 8; ");
	*(unsigned int *)IMEM_CONTROL &= ~(IMC | ENICPLB);
	SSYNC();
}

int icache_status(void)
{
	unsigned int value;
	value = *(unsigned int *)IMEM_CONTROL;

	if (value & (IMC | ENICPLB))
		return CACHE_ON;
	else
		return CACHE_OFF;
}

void dcache_enable(void)
{
	unsigned int *I0, *I1;
	unsigned int temp;
	int i, j = 0;

	/* Before enable dcache, disable it first */
	dcache_disable();
	I0 = (unsigned int *)DCPLB_ADDR0;
	I1 = (unsigned int *)DCPLB_DATA0;

	/* make sure the locked ones go in first */
	for (i = 0; i < page_descriptor_table_size; i++) {
		if (CPLB_LOCK & dcplb_table[i][1]) {
			debug("adding %02i %02i 0x%08x 0x%08x\n", i, j,
			      dcplb_table[i][0], dcplb_table[i][1]);
			*I0++ = dcplb_table[i][0];
			*I1++ = dcplb_table[i][1];
			j++;
		} else {
			debug("skip   %02i %02i 0x%08x 0x%08x\n", i, j,
			      dcplb_table[i][0], dcplb_table[i][1]);
		}
	}

	for (i = 0; i < page_descriptor_table_size; i++) {
		if (!(CPLB_LOCK & dcplb_table[i][1])) {
			debug("adding %02i %02i 0x%08x 0x%08x\n", i, j,
			      dcplb_table[i][0], dcplb_table[i][1]);
			*I0++ = dcplb_table[i][0];
			*I1++ = dcplb_table[i][1];
			j++;
			if (j == 16) {
				break;
			}
		}
	}

	/* Fill the rest with invalid entry */
	if (j <= 15) {
		for (; j < 16; j++) {
			debug("filling %i with 0", j);
			*I1++ = 0x0;
		}
	}

	temp = *(unsigned int *)DMEM_CONTROL;
	SSYNC();
	asm(" .align 8; ");
	*(unsigned int *)DMEM_CONTROL =
	    ACACHE_BCACHE | ENDCPLB | PORT_PREF0 | temp;
	SSYNC();
}

void dcache_disable(void)
{

	unsigned int *I0, *I1;
	int i;

	SSYNC();
	asm(" .align 8; ");
	*(unsigned int *)DMEM_CONTROL &=
	    ~(ACACHE_BCACHE | ENDCPLB | PORT_PREF0);
	SSYNC();

	/* after disable dcache, clear it so we don't confuse the next application */
	I0 = (unsigned int *)DCPLB_ADDR0;
	I1 = (unsigned int *)DCPLB_DATA0;

	for (i = 0; i < 16; i++) {
		*I0++ = 0x0;
		*I1++ = 0x0;
	}
}

int dcache_status(void)
{
	unsigned int value;
	value = *(unsigned int *)DMEM_CONTROL;
	if (value & (ENDCPLB))
		return CACHE_ON;
	else
		return CACHE_OFF;
}
