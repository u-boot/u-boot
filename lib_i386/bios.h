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

#ifndef _BIOS_H_
#define _BIOS_H_

#define OFFS_ES      0     /* 16bit */
#define OFFS_GS      2     /* 16bit */
#define OFFS_DS      4     /* 16bit */
#define OFFS_EDI     6     /* 32bit */
#define OFFS_DI      6     /* low 16 bits of EDI */
#define OFFS_ESI     10    /* 32bit */
#define OFFS_SI      10    /* low 16 bits of ESI */
#define OFFS_EBP     14    /* 32bit */
#define OFFS_BP      14    /* low 16 bits of EBP */
#define OFFS_ESP     18    /* 32bit */
#define OFFS_SP      18    /* low 16 bits of ESP */
#define OFFS_EBX     22    /* 32bit */
#define OFFS_BX      22    /* low 16 bits of EBX */
#define OFFS_BL      22    /* low  8 bits of BX */
#define OFFS_BH      23    /* high 8 bits of BX */
#define OFFS_EDX     26    /* 32bit */
#define OFFS_DX      26    /* low 16 bits of EBX */
#define OFFS_DL      26    /* low  8 bits of BX */
#define OFFS_DH      27    /* high 8 bits of BX */
#define OFFS_ECX     30    /* 32bit */
#define OFFS_CX      30    /* low 16 bits of EBX */
#define OFFS_CL      30    /* low  8 bits of BX */
#define OFFS_CH      31    /* high 8 bits of BX */
#define OFFS_EAX     34    /* 32bit */
#define OFFS_AX      34    /* low 16 bits of EBX */
#define OFFS_AL      34    /* low  8 bits of BX */
#define OFFS_AH      35    /* high 8 bits of BX */
#define OFFS_VECTOR  38    /* 16bit */
#define OFFS_IP      40    /* 16bit */
#define OFFS_CS      42    /* 16bit */
#define OFFS_FLAGS   44    /* 16bit */

#define SEGMENT      0x40
#define STACK	     0x800  			/* stack at 0x40:0x800 -> 0x800 */

/* save general registers */
/* save some segments     */
/* save callers stack segment .. */
/* ... in gs */
	/* setup my segments */
	/* setup BIOS stackpointer */

#define MAKE_BIOS_STACK \
	pushal		; \
	pushw	%ds	; \
	pushw	%gs	; \
	pushw	%es	; \
	pushw	%ss	; \
	popw	%gs	; \
	movw	$SEGMENT,%ax ; \
	movw	%ax,%ds	; \
	movw	%ax,%es	; \
	movw	%ax,%ss	; \
	movw	%sp,%bp	; \
	movw	$STACK,%sp

#define RESTORE_CALLERS_STACK \
	pushw	%gs     ;			/* restore callers stack segment */ \
	popw	%ss     ; \
	movw	%bp,%sp	;			/* restore stackpointer */ \
		\
	popw	%es	;			/* restore segment selectors */ \
	popw	%gs     ; \
	popw	%ds     ; \
		\
	popal					/* restore GP registers */

#endif
