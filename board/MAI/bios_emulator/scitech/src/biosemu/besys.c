/****************************************************************************
*
*                        BIOS emulator and interface
*                      to Realmode X86 Emulator Library
*
*               Copyright (C) 1996-1999 SciTech Software, Inc.
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.  The authors makes no
*  representations about the suitability of this software for any purpose.
*  It is provided "as is" without express or implied warranty.
*
*  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
*  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
*  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
*  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
*  PERFORMANCE OF THIS SOFTWARE.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
* Developer:    Kendall Bennett
*
* Description:  This file includes BIOS emulator I/O and memory access
*               functions.
*
****************************************************************************/

#include "biosemui.h"

/*------------------------------- Macros ----------------------------------*/

/* Macros to read and write values to x86 bus memory. Replace these as
 * necessary if you need to do something special to access memory over
 * the bus on a particular processor family.
 */

#define readb(base,off)     *((u8*)((u32)(base) + (off)))
#define readw(base,off)     *((u16*)((u32)(base) + (off)))
#define readl(base,off)     *((u32*)((u32)(base) + (off)))
#define writeb(v,base,off)  *((u8*)((u32)(base) + (off))) = (v)
#define writew(v,base,off)  *((u16*)((u32)(base) + (off))) = (v)
#define writel(v,base,off)  *((u32*)((u32)(base) + (off))) = (v)

/*----------------------------- Implementation ----------------------------*/

#ifdef DEBUG
# define DEBUG_MEM()        (M.x86.debug & DEBUG_MEM_TRACE_F)
#else
# define DEBUG_MEM()
#endif

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Byte value read from emulator memory.

REMARKS:
Reads a byte value from the emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
u8 X86API BE_rdb(
    u32 addr)
{
    u8 val = 0;

    if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
	val = *(u8*)(_BE_env.biosmem_base + addr - 0xC0000);
	}
    else if (addr >= 0xA0000 && addr <= 0xFFFFF) {
	val = readb(_BE_env.busmem_base, addr - 0xA0000);
	}
    else if (addr > M.mem_size - 1) {
DB(     printk("mem_read: address %#lx out of range!\n", addr);)
	HALT_SYS();
	}
    else {
	val = *(u8*)(M.mem_base + addr);
	}
DB( if (DEBUG_MEM())
	printk("%#08x 1 -> %#x\n", addr, val);)
    return val;
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Word value read from emulator memory.

REMARKS:
Reads a word value from the emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
u16 X86API BE_rdw(
    u32 addr)
{
    u16 val = 0;

    if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    addr -= 0xC0000;
	    val = ( *(u8*)(_BE_env.biosmem_base + addr) |
		   (*(u8*)(_BE_env.biosmem_base + addr + 1) << 8));
	    }
	else
#endif
	    val = *(u16*)(_BE_env.biosmem_base + addr - 0xC0000);
	}
    else if (addr >= 0xA0000 && addr <= 0xFFFFF) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    addr -= 0xA0000;
	    val = ( readb(_BE_env.busmem_base, addr) |
		   (readb(_BE_env.busmem_base, addr + 1) << 8));
	    }
	else
#endif
	    val = readw(_BE_env.busmem_base, addr - 0xA0000);
	}
    else if (addr > M.mem_size - 2) {
DB(     printk("mem_read: address %#lx out of range!\n", addr);)
	HALT_SYS();
	}
    else {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    val = ( *(u8*)(M.mem_base + addr) |
		   (*(u8*)(M.mem_base + addr + 1) << 8));
	    }
	else
#endif
	    val = *(u16*)(M.mem_base + addr);
	}
DB( if (DEBUG_MEM())
	printk("%#08x 2 -> %#x\n", addr, val);)
    return val;
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Long value read from emulator memory.

REMARKS:
Reads a long value from the emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
u32 X86API BE_rdl(
    u32 addr)
{
    u32 val = 0;

    if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x3) {
	    addr -= 0xC0000;
	    val = ( *(u8*)(_BE_env.biosmem_base + addr + 0) |
		   (*(u8*)(_BE_env.biosmem_base + addr + 1) << 8) |
		   (*(u8*)(_BE_env.biosmem_base + addr + 2) << 16) |
		   (*(u8*)(_BE_env.biosmem_base + addr + 3) << 24));
	    }
	else
#endif
	    val = *(u32*)(_BE_env.biosmem_base + addr - 0xC0000);
	}
    else if (addr >= 0xA0000 && addr <= 0xFFFFF) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x3) {
	    addr -= 0xA0000;
	    val = ( readb(_BE_env.busmem_base, addr) |
		   (readb(_BE_env.busmem_base, addr + 1) <<  8) |
		   (readb(_BE_env.busmem_base, addr + 2) << 16) |
		   (readb(_BE_env.busmem_base, addr + 3) << 24));
	    }
	else
#endif
	    val = readl(_BE_env.busmem_base, addr - 0xA0000);
	}
    else if (addr > M.mem_size - 4) {
DB(     printk("mem_read: address %#lx out of range!\n", addr);)
	HALT_SYS();
	}
    else {
#ifdef __BIG_ENDIAN__
	if (addr & 0x3) {
	    val = ( *(u8*)(M.mem_base + addr + 0) |
		   (*(u8*)(M.mem_base + addr + 1) << 8) |
		   (*(u8*)(M.mem_base + addr + 2) << 16) |
		   (*(u8*)(M.mem_base + addr + 3) << 24));
	    }
	else
#endif
	    val = *(u32*)(M.mem_base + addr);
	}
DB( if (DEBUG_MEM())
	printk("%#08x 4 -> %#x\n", addr, val);)
    return val;
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a byte value to emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
void X86API BE_wrb(
    u32 addr,
    u8 val)
{
DB( if (DEBUG_MEM())
	printk("%#08x 1 <- %#x\n", addr, val);)
    if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
	*(u8*)(_BE_env.biosmem_base + addr - 0xC0000) = val;
	}
    else if (addr >= 0xA0000 && addr <= 0xFFFFF) {
	writeb(val, _BE_env.busmem_base, addr - 0xA0000);
	}
    else if (addr > M.mem_size-1) {
DB(     printk("mem_write: address %#lx out of range!\n", addr);)
	HALT_SYS();
	}
    else {
	*(u8*)(M.mem_base + addr) = val;
	}
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a word value to emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
void X86API BE_wrw(
    u32 addr,
    u16 val)
{
DB( if (DEBUG_MEM())
	printk("%#08x 2 <- %#x\n", addr, val);)
    if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    addr -= 0xC0000;
	    *(u8*)(_BE_env.biosmem_base + addr + 0) = (val >> 0) & 0xff;
	    *(u8*)(_BE_env.biosmem_base + addr + 1) = (val >> 8) & 0xff;
	    }
	else
#endif
	    *(u16*)(_BE_env.biosmem_base + addr - 0xC0000) = val;
	}
    else if (addr >= 0xA0000 && addr <= 0xFFFFF) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    addr -= 0xA0000;
	    writeb(val >> 0, _BE_env.busmem_base, addr);
	    writeb(val >> 8, _BE_env.busmem_base, addr + 1);
	    }
	else
#endif
	    writew(val, _BE_env.busmem_base, addr - 0xA0000);
	}
    else if (addr > M.mem_size-2) {
DB(     printk("mem_write: address %#lx out of range!\n", addr);)
	HALT_SYS();
	}
    else {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    *(u8*)(M.mem_base + addr + 0) = (val >> 0) & 0xff;
	    *(u8*)(M.mem_base + addr + 1) = (val >> 8) & 0xff;
	    }
	else
#endif
	    *(u16*)(M.mem_base + addr) = val;
	}
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a long value to emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
void X86API BE_wrl(
    u32 addr,
    u32 val)
{
DB( if (DEBUG_MEM())
	printk("%#08x 4 <- %#x\n", addr, val);)
    if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    addr -= 0xC0000;
	    *(u8*)(M.mem_base + addr + 0) = (val >>  0) & 0xff;
	    *(u8*)(M.mem_base + addr + 1) = (val >>  8) & 0xff;
	    *(u8*)(M.mem_base + addr + 2) = (val >> 16) & 0xff;
	    *(u8*)(M.mem_base + addr + 3) = (val >> 24) & 0xff;
	    }
	else
#endif
	    *(u32*)(M.mem_base + addr - 0xC0000) = val;
	}
    else if (addr >= 0xA0000 && addr <= 0xFFFFF) {
#ifdef __BIG_ENDIAN__
	if (addr & 0x3) {
	    addr -= 0xA0000;
	    writeb(val >>  0, _BE_env.busmem_base, addr);
	    writeb(val >>  8, _BE_env.busmem_base, addr + 1);
	    writeb(val >> 16, _BE_env.busmem_base, addr + 1);
	    writeb(val >> 24, _BE_env.busmem_base, addr + 1);
	    }
	else
#endif
	    writel(val, _BE_env.busmem_base, addr - 0xA0000);
	}
    else if (addr > M.mem_size-4) {
DB(     printk("mem_write: address %#lx out of range!\n", addr);)
	HALT_SYS();
	}
    else {
#ifdef __BIG_ENDIAN__
	if (addr & 0x1) {
	    *(u8*)(M.mem_base + addr + 0) = (val >>  0) & 0xff;
	    *(u8*)(M.mem_base + addr + 1) = (val >>  8) & 0xff;
	    *(u8*)(M.mem_base + addr + 2) = (val >> 16) & 0xff;
	    *(u8*)(M.mem_base + addr + 3) = (val >> 24) & 0xff;
	    }
	else
#endif
	    *(u32*)(M.mem_base + addr) = val;
	}
}

/* Debug functions to do ISA/PCI bus port I/O */

#ifdef  DEBUG
#define DEBUG_IO()          (M.x86.debug & DEBUG_IO_TRACE_F)

u8 X86API BE_inb(int port)
{
    u8 val = PM_inpb(port);
    if (DEBUG_IO())
	printk("%04X:%04X:  inb.%04X -> %02X\n",M.x86.saved_cs, M.x86.saved_ip, (ushort)port, val);
    return val;
}

u16 X86API BE_inw(int port)
{
    u16 val = PM_inpw(port);
    if (DEBUG_IO())
	printk("%04X:%04X:  inw.%04X -> %04X\n",M.x86.saved_cs, M.x86.saved_ip, (ushort)port, val);
    return val;
}

u32 X86API BE_inl(int port)
{
    u32 val = PM_inpd(port);
    if (DEBUG_IO())
	printk("%04X:%04X:  inl.%04X -> %08X\n",M.x86.saved_cs, M.x86.saved_ip, (ushort)port, val);
    return val;
}

void X86API BE_outb(int port, u8 val)
{
    if (DEBUG_IO())
	printk("%04X:%04X: outb.%04X <- %02X\n",M.x86.saved_cs, M.x86.saved_ip, (ushort)port, val);
    PM_outpb(port,val);
}

void X86API BE_outw(int port, u16 val)
{
    if (DEBUG_IO())
	printk("%04X:%04X: outw.%04X <- %04X\n",M.x86.saved_cs, M.x86.saved_ip, (ushort)port, val);
    PM_outpw(port,val);
}

void X86API BE_outl(int port, u32 val)
{
    if (DEBUG_IO())
	printk("%04X:%04X: outl.%04X <- %08X\n",M.x86.saved_cs, M.x86.saved_ip, (ushort)port, val);
    PM_outpd(port,val);
}
#endif
