/*
 * U-boot - cpu.c CPU specific functions
 *
 * Copyright (c) 2005 blackfin.uclinux.org
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/blackfin.h>
#include <command.h>
#include <asm/entry.h>

#define SSYNC() asm("ssync;")
#define CACHE_ON 1
#define CACHE_OFF 0

/* Data Attibutes*/

#define SDRAM_IGENERIC		(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_USER_RD | CPLB_VALID)
#define SDRAM_IKERNEL		(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_USER_RD | CPLB_VALID | CPLB_LOCK)
#define L1_IMEMORY            	(PAGE_SIZE_1MB | CPLB_L1_CHBL | CPLB_USER_RD | CPLB_VALID | CPLB_LOCK)
#define SDRAM_INON_CHBL		(PAGE_SIZE_4MB | CPLB_USER_RD | CPLB_VALID)

#define ANOMALY_05000158		0x200
#define SDRAM_DGENERIC          (PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_WT | CPLB_L1_AOW | CPLB_SUPV_WR | CPLB_USER_RD | CPLB_USER_WR | CPLB_VALID | ANOMALY_05000158)
#define SDRAM_DNON_CHBL         (PAGE_SIZE_4MB | CPLB_WT | CPLB_L1_AOW | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_USER_RD | CPLB_VALID | ANOMALY_05000158)
#define SDRAM_DKERNEL           (PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_WT | CPLB_L1_AOW | CPLB_USER_RD | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_VALID | CPLB_LOCK | ANOMALY_05000158)
#define L1_DMEMORY              (PAGE_SIZE_4KB | CPLB_L1_CHBL | CPLB_L1_AOW | CPLB_WT | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_VALID | ANOMALY_05000158)
#define SDRAM_EBIU              (PAGE_SIZE_4MB | CPLB_WT | CPLB_L1_AOW | CPLB_USER_RD | CPLB_USER_WR | CPLB_SUPV_WR | CPLB_VALID | ANOMALY_05000158)

static unsigned int icplb_table[16][2]={
			{0xFFA00000, L1_IMEMORY},
			{0x00000000, SDRAM_IKERNEL},	/*SDRAM_Page1*/
			{0x00400000, SDRAM_IKERNEL},	/*SDRAM_Page1*/
			{0x07C00000, SDRAM_IKERNEL},    /*SDRAM_Page14*/
			{0x00800000, SDRAM_IGENERIC},	/*SDRAM_Page2*/
			{0x00C00000, SDRAM_IGENERIC},	/*SDRAM_Page2*/
			{0x01000000, SDRAM_IGENERIC},	/*SDRAM_Page4*/
			{0x01400000, SDRAM_IGENERIC},	/*SDRAM_Page5*/
			{0x01800000, SDRAM_IGENERIC},	/*SDRAM_Page6*/
			{0x01C00000, SDRAM_IGENERIC},	/*SDRAM_Page7*/
			{0x02000000, SDRAM_IGENERIC},	/*SDRAM_Page8*/
			{0x02400000, SDRAM_IGENERIC},	/*SDRAM_Page9*/
			{0x02800000, SDRAM_IGENERIC},	/*SDRAM_Page10*/
			{0x02C00000, SDRAM_IGENERIC},	/*SDRAM_Page11*/
			{0x03000000, SDRAM_IGENERIC},	/*SDRAM_Page12*/
			{0x03400000, SDRAM_IGENERIC},	/*SDRAM_Page13*/
};

static unsigned int dcplb_table[16][2]={
			{0xFFA00000,L1_DMEMORY},
			{0x00000000,SDRAM_DKERNEL},	/*SDRAM_Page1*/
			{0x00400000,SDRAM_DKERNEL},	/*SDRAM_Page1*/
			{0x07C00000,SDRAM_DKERNEL},	/*SDRAM_Page15*/
			{0x00800000,SDRAM_DGENERIC},	/*SDRAM_Page2*/
			{0x00C00000,SDRAM_DGENERIC},	/*SDRAM_Page3*/
			{0x01000000,SDRAM_DGENERIC},	/*SDRAM_Page4*/
			{0x01400000,SDRAM_DGENERIC},	/*SDRAM_Page5*/
			{0x01800000,SDRAM_DGENERIC},	/*SDRAM_Page6*/
			{0x01C00000,SDRAM_DGENERIC},	/*SDRAM_Page7*/
			{0x02000000,SDRAM_DGENERIC},	/*SDRAM_Page8*/
			{0x02400000,SDRAM_DGENERIC},	/*SDRAM_Page9*/
			{0x02800000,SDRAM_DGENERIC},	/*SDRAM_Page10*/
			{0x02C00000,SDRAM_DGENERIC},	/*SDRAM_Page11*/
			{0x03000000,SDRAM_DGENERIC},	/*SDRAM_Page12*/
			{0x20000000,SDRAM_EBIU},	/*For Network */
};

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	__asm__ __volatile__
	("cli r3;"
	"P0 = %0;"
	"JUMP (P0);"
	:
	: "r" (L1_ISRAM)
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
	unsigned int *I0,*I1;
	int i;

	I0 = (unsigned int *)ICPLB_ADDR0;
	I1 = (unsigned int *)ICPLB_DATA0;

	for(i=0;i<16;i++){
		*I0++ = icplb_table[i][0];
		*I1++ = icplb_table[i][1];
		}
	cli();
	SSYNC();
	*(unsigned int *)IMEM_CONTROL = IMC | ENICPLB;
	SSYNC();
	sti();
}

void icache_disable(void)
{
	cli();
	SSYNC();
	*(unsigned int *)IMEM_CONTROL &= ~(IMC | ENICPLB);
	SSYNC();
	sti();
}

int icache_status(void)
{
	unsigned int value;
	value = *(unsigned int *)IMEM_CONTROL;

	if( value & (IMC|ENICPLB) )
		return CACHE_ON;
	else
		return CACHE_OFF;
}

void dcache_enable(void)
{
	unsigned int *I0,*I1;
	unsigned int temp;
	int i;
	I0 = (unsigned int *)DCPLB_ADDR0;
	I1 = (unsigned int *)DCPLB_DATA0;

	for(i=0;i<16;i++){
		*I0++ = dcplb_table[i][0];
		*I1++ = dcplb_table[i][1];
		}
	cli();
	temp = *(unsigned int *)DMEM_CONTROL;
	SSYNC();
	*(unsigned int *)DMEM_CONTROL = ACACHE_BCACHE |ENDCPLB |PORT_PREF0|temp;
	SSYNC();
	sti();
}

void dcache_disable(void)
{
	cli();
	SSYNC();
	*(unsigned int *)DMEM_CONTROL &= ~(ACACHE_BCACHE |ENDCPLB |PORT_PREF0);
	SSYNC();
	sti();
}

int dcache_status(void)
{
	unsigned int value;
	value = *(unsigned int *)DMEM_CONTROL;
	if( value & (ENDCPLB))
		return CACHE_ON;
	else
		return CACHE_OFF;
}
