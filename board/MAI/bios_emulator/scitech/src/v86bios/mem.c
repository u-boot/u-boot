/*
 * Copyright 1999 Egbert Eich
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include "debug.h"
#include "v86bios.h"
#include "x86emu.h"

#ifdef __alpha__

void* vram_map = 0;
int sparse_shift = 5;

#define mem_barrier()        __asm__ __volatile__("mb"  : : : "memory")

#define vuip    volatile unsigned int *

CARD8
mem_rb(CARD32 addr)
{
  unsigned long result, shift;
#if 1
  if (addr >= 0xA0000 && addr <= 0xBFFFF) {
    addr -= 0xA0000;
    shift = (addr & 0x3) * 8;
    result = *(vuip) ((unsigned long)vram_map + (addr << sparse_shift));
    result >>= shift;
    return 0xffUL & result;
  } else
#endif
    return rdb(addr);
}

CARD16
mem_rw(CARD32 addr)
{
  unsigned long result, shift;
#if 1
  if (addr >= 0xA0000 && addr <= 0xBFFFF) {
    addr -= 0xA0000;
    shift = (addr & 0x2) * 8;
    result = *(vuip)((unsigned long)vram_map+(addr<<sparse_shift)
	     +(1<<(sparse_shift-2)));
    result >>= shift;
    return 0xffffUL & result;
  } else
#endif
    return rdw(addr);
}

CARD32
mem_rl(CARD32 addr)
{
  unsigned long result;
#if 1
  if (addr >= 0xA0000 && addr <= 0xBFFFF) {
    addr -= 0xA0000;
    result = *(vuip)((unsigned long)vram_map+(addr<<sparse_shift)+(3<<(sparse_shift-2)));
    return result;
  } else
#endif
    return rdl(addr);
}

void
mem_wb(CARD32 addr, CARD8 val)
{
    unsigned int b = val & 0xffU;
#if 1
  if (addr >= 0xA0000 && addr <= 0xBFFFF) {
    addr -= 0xA0000;
    *(vuip) ((unsigned long)vram_map + (addr << sparse_shift)) = b * 0x01010101;
    mem_barrier();
  } else
#endif
    wrb(addr,val);
}

void
mem_ww(CARD32 addr, CARD16 val)
{
  unsigned int w = val & 0xffffU;
#if 1
  if (addr >= 0xA0000 && addr <= 0xBFFFF) {
    addr -= 0xA0000;
    *(vuip)((unsigned long)vram_map+(addr<<sparse_shift)
	+(1<<(sparse_shift-2))) = w * 0x00010001;
    mem_barrier();
  } else
#endif
    wrw(addr,val);
}

void
mem_wl(CARD32 addr, CARD32 val)
{
#if 1
  if (addr >= 0xA0000 && addr <= 0xBFFFF) {
    addr -= 0xA0000;
    *(vuip)((unsigned long)vram_map+(addr<<sparse_shift)
	+(3<<(sparse_shift-2))) = val;
    mem_barrier();
  } else
#endif
    wrl(addr,val);
}
#endif
