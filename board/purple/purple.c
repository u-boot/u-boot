/*
 * (C) Copyright 2003
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
#include <command.h>
#include <asm/inca-ip.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>

#include "sconsole.h"

#define cache_unroll(base,op)	        	\
	__asm__ __volatile__("	         	\
		.set noreorder;		        \
		.set mips3;		        \
                cache %1, (%0);	                \
		.set mips0;			\
		.set reorder"			\
		:				\
		: "r" (base),			\
		  "i" (op));

typedef void (*FUNCPTR)(ulong *source, ulong *destination, ulong nlongs);

extern void	asc_serial_init		(void);
extern void	asc_serial_putc 	(char);
extern void	asc_serial_puts 	(const char *);
extern int	asc_serial_getc 	(void);
extern int	asc_serial_tstc 	(void);
extern void	asc_serial_setbrg 	(void);


long int initdram(int board_type)
{
	/* The only supported number of SDRAM banks is 4.
	 */
#define CFG_NB	4

	ulong	cfgpb0	= *INCA_IP_SDRAM_MC_CFGPB0;
	ulong	cfgdw	= *INCA_IP_SDRAM_MC_CFGDW;
	int	cols	= cfgpb0 & 0xF;
	int	rows	= (cfgpb0 & 0xF0) >> 4;
	int	dw	= cfgdw & 0xF;
	ulong	size	= (1 << (rows + cols)) * (1 << (dw - 1)) * CFG_NB;

	return size;
}

int checkboard (void)
{

	unsigned long chipid = *(unsigned long *)0xB800C800;

	printf ("Board: Purple PLB 2800 chip version %ld, ", chipid & 0xF);

	printf("CPU Speed %d MHz\n", CPU_CLOCK_RATE/1000000);

	return 0;
}

int misc_init_r (void)
{
	asc_serial_init ();

	sconsole_putc   = asc_serial_putc;
	sconsole_puts   = asc_serial_puts;
	sconsole_getc   = asc_serial_getc;
	sconsole_tstc   = asc_serial_tstc;
	sconsole_setbrg = asc_serial_setbrg;

	sconsole_flush ();
	return (0);
}

/*******************************************************************************
*
* copydwords - copy one buffer to another a long at a time
*
* This routine copies the first <nlongs> longs from <source> to <destination>.
*/
static void copydwords (ulong *source, ulong *destination, ulong nlongs)
{
	ulong temp,temp1;
	ulong *dstend = destination + nlongs;

	while (destination < dstend)
	{
		temp = *source++;
		/* dummy read from sdram */
		temp1 = *(ulong *)0xa0000000;
		/* avoid optimization from compliler */
		*(ulong *)0xbf0081f8 = temp1 + temp;
		*destination++ = temp;

	} 
}

/*******************************************************************************
*
* copyLongs - copy one buffer to another a long at a time
*
* This routine copies the first <nlongs> longs from <source> to <destination>.
*/
static void copyLongs (ulong *source, ulong *destination, ulong nlongs)
{
	FUNCPTR absEntry;

	absEntry = (FUNCPTR)(0xbf008000+((ulong)copydwords & 0x7));
	absEntry(source, destination, nlongs);
}

/*******************************************************************************
*
* programLoad - load program into ram
*
* This routine load copydwords into ram
*
*/
static void programLoad(void)
{
	FUNCPTR absEntry;
	ulong *src,*dst;

	src = (ulong *)(TEXT_BASE + 0x428);
	dst = (ulong *)0xbf0081d0;

	absEntry = (FUNCPTR)(TEXT_BASE + 0x400);   
	absEntry(src,dst,0x6);     	

	src = (ulong *)((ulong)copydwords & 0xfffffff8);
	dst = (ulong *)0xbf008000;

	absEntry(src,dst,0x38);
}

/*******************************************************************************
*
* copy_code - copy u-boot image from flash to RAM
*
* This routine is needed to solve flash problems on this board
*
*/
void copy_code (ulong dest_addr)
{
	unsigned long start;
	unsigned long end;

	/* load copydwords into ram
	 */
	programLoad();

	/* copy u-boot code
	 */
	copyLongs((ulong *)CFG_MONITOR_BASE,
		  (ulong *)dest_addr,
		  (CFG_MONITOR_LEN + 3) / 4);


	/* flush caches
	 */

	start = KSEG0;
	end = start + CFG_DCACHE_SIZE;
	while(start < end) {
		cache_unroll(start,Index_Writeback_Inv_D);
		start += CFG_CACHELINE_SIZE;
	}

	start = KSEG0;
	end = start + CFG_ICACHE_SIZE;
	while(start < end) {
		cache_unroll(start,Index_Invalidate_I);
		start += CFG_CACHELINE_SIZE;
	}
}
