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

#include <stdio.h>
#if defined(__alpha__) || defined (__ia64__)
#include <sys/io.h>
#endif
#include "AsmMacros.h"
#include "v86bios.h"
#include "pci.h"

int r_inb = 0, r_inw = 0, r_inl = 0, r_outb = 0, r_outw = 0, r_outl = 0;
int in_b = 0, in_w = 0, in_l = 0, out_b = 0, out_w = 0, out_l = 0;


int
port_rep_inb(CARD16 port, CARD8 *base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD8 *dst = base;

    p_printf(" rep_insb(%#x) %d bytes at %p %s",
	     port, count, base, d_f?"up":"down");
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    r_inb++;
    while (count--) {
	*dst = inb(port);
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_inw(CARD16 port, CARD16 *base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD16 *dst = base;

    p_printf(" rep_insw(%#x) %d bytes at %p %s",
	     port, count, base, d_f?"up":"down");
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    r_inw++;
    while (count--) {
	*dst = inw(port);
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_inl(CARD16 port, CARD32 *base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD32 *dst = base;

    p_printf(" rep_insl(%#x) %d bytes at %p %s",
	     port, count, base, d_f?"up":"down");
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    r_inl++;
    while (count--) {
	*dst = inl(port);
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_outb(CARD16 port, CARD8 *base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD8 *dst = base;

    p_printf(" rep_outb(%#x) %d bytes at %p %s",
	     port, count, base, d_f?"up":"down");
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    r_outb++;
    while (count--) {
	outb(port,*dst);
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_outw(CARD16 port, CARD16 *base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD16 *dst = base;

    p_printf(" rep_outw(%#x) %d bytes at %p %s",
	     port, count, base, d_f?"up":"down");
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    r_outw++;
    while (count--) {
	outw(port,*dst);
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_outl(CARD16 port, CARD32 *base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD32 *dst = base;

    p_printf(" rep_outl(%#x) %d bytes at %p %s",
	     port, count, base, d_f?"up":"down");
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    r_outl++;
    while (count--) {
	outl(port,*dst);
	dst += inc;
    }
    return (dst-base);
}

CARD8
p_inb(CARD16 port)
{
    CARD8 val = 0;
    in_b++;
    val = inb(port);
    p_printf(" inb(%#x) = %2.2x",port,val);
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    return val;
}

CARD16
p_inw(CARD16 port)
{
    CARD16 val = 0;
    in_w++;
    val = inw(port);
    p_printf(" inw(%#x) = %4.4x",port,val);
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    return val;
}

CARD32
p_inl(CARD16 port)
{
    CARD32 val = 0;
    in_l++;
#ifdef NEED_PCI_IO
    if (cfg1in(port,&val))
	return val;
    else
#endif
    val = inl(port);
    p_printf(" inl(%#x) = %8.8x",port,val);
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    return val;
}

void
p_outb(CARD16 port, CARD8 val)
{
    out_b++;
    p_printf(" outb(%#x, %2.2x)",port,val);
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    outb(port,val);
}

void
p_outw(CARD16 port, CARD16 val)
{
    out_w++;
    p_printf(" outw(%#x, %4.4x)",port,val);
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

    outw(port,val);
}

void
p_outl(CARD16 port, CARD32 val)
{
    out_l++;
    p_printf(" outl(%#x, %8.8x)",port,val);
    if (Config.PrintIp)
	p_printf(" %x\n",getIP());
    else p_printf("\n");

#ifdef NEED_PCI_IO
    if (cfg1out(port,val))
	return;
#endif
    outl(port,val);
}

void
io_statistics(void)
{
    p_printf("rep: inb: %i, inw: %i, inl: %i, outb: %i, outw: %i, outl: %i\n",
	 r_inb,r_inw,r_inl,r_outb,r_outw,r_outl);
    p_printf("inb: %i, inw: %i, inl: %i, outb: %i, outw: %i, outl: %i\n",
	 in_b,in_w,in_l,out_b,out_w,out_l);
}

void
clear_stat(void)
{
    r_inb = r_inw = r_inl = r_outb = r_outw = r_outl = 0;
    in_b = in_w = in_l = out_b = out_w = out_l = 0;
}
