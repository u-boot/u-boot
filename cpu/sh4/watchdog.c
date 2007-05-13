/*
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
#include <asm/processor.h>

#define WDT_BASE	WTCNT

static unsigned char cnt_read (void){
	return *((volatile unsigned char *)(WDT_BASE + 0x00));
}

static unsigned char csr_read (void){
	return *((volatile unsigned char *)(WDT_BASE + 0x04));
}

static void cnt_write (unsigned char value){
	while (csr_read() & (1 << 5)) {
		/* delay */
	}
	*((volatile unsigned short *)(WDT_BASE + 0x00)) = ((unsigned short) value) | 0x5A00;
}

static void csr_write (unsigned char value){
	*((volatile unsigned short *)(WDT_BASE + 0x04)) = ((unsigned short) value) | 0xA500;
}


int watchdog_init (void){ return 0; }

void reset_cpu (unsigned long ignored)
{
	while(1);
}


