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

#include <common.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <asm/realmode.h>


#define REALMODE_BASE    ((char*)0x7c0)
#define REALMODE_MAILBOX ((char*)0xe00)


extern char realmode_enter;


int enter_realmode(u16 seg, u16 off, struct pt_regs *in, struct pt_regs *out)
{
	
	/* setup out thin bios emulation */
	if (bios_setup()) {
		return -1;
	}
		
	/* copy the realmode switch code */
	if (i386boot_realmode_size > (REALMODE_MAILBOX-REALMODE_BASE)) {
		printf("realmode switch too large (%ld bytes, max is %d)\n", 
		       i386boot_realmode_size, (int)(REALMODE_MAILBOX-REALMODE_BASE));
		return -1;
	}
	
	memcpy(REALMODE_BASE, (void*)i386boot_realmode, i386boot_realmode_size);
		
	
	in->eip = off;
	in->xcs = seg;
	if (3>(in->esp & 0xffff)) {
		printf("Warning: entering realmode with sp < 4 will fail\n");
	}
	
	memcpy(REALMODE_MAILBOX, in, sizeof(struct pt_regs));
	
	__asm__ volatile ( 
		 "lcall $0x20,%0\n"  : :  "i" (&realmode_enter) );

	memcpy(out, REALMODE_MAILBOX, sizeof(struct pt_regs));

	return out->eax;
}

