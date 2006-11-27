/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

/* This file is largely based on code obtned from AMD. AMD's original
 * copyright is included below
 */

/*
 *  =============================================================================
 *
 *   Copyright 1999 Advanced Micro Devices, Inc.
 *
 *  This software is the property of Advanced Micro Devices, Inc  (AMD)  which
 *  specifically grants the user the right to modify, use and distribute this
 *  software provided this COPYRIGHT NOTICE is not removed or altered.  All
 *  other rights are reserved by AMD.
 *
 *  THE MATERIALS ARE PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY
 *  OF ANY KIND INCLUDING WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT OF
 *  THIRD-PARTY INTELLECTUAL PROPERTY, OR FITNESS FOR ANY PARTICULAR PURPOSE.
 *  IN NO EVENT SHALL AMD OR ITS SUPPLIERS BE LIABLE FOR ANY DAMAGES WHATSOEVER
 *  (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS, BUSINESS
 *  INTERRUPTION, LOSS OF INFORMAITON) ARISING OUT OF THE USE OF OR INABILITY
 *  TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGES.  BECAUSE SOME JURSIDICTIONS PROHIBIT THE EXCLUSION OR
 *  LIMITATION OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE
 *  LIMITATION MAY NOT APPLY TO YOU.
 *
 *  AMD does not assume any responsibility for any errors that may appear in
 *  the Materials nor any responsibility to support or update the Materials.
 *  AMD retains the right to make changes to its test specifications at any
 *  time, without notice.
 *
 *  So that all may benefit from your experience, please report  any  problems
 *  or suggestions about this software back to AMD.  Please include your name,
 *  company,  telephone number,  AMD product requiring support and question or
 *  problem encountered.
 *
 *  Advanced Micro Devices, Inc.         Worldwide support and contact
 *  Embedded Processor Division            information available at:
 *  Systems Engineering                       epd.support@amd.com
 *  5204 E. Ben White Blvd.                          -or-
 *  Austin, TX 78741                http://www.amd.com/html/support/techsup.html
 *  ============================================================================
 */


/*******************************************************************************
 *	 AUTHOR      : Buddy Fey - Original.
 *******************************************************************************
 */


/*******************************************************************************
 *       FUNCTIONAL DESCRIPTION:
 * This routine is called to autodetect the geometry of the DRAM.
 *
 * This routine is called to determine the number of column bits for the DRAM
 * devices in this external bank. This routine assumes that the external bank
 * has been configured for an 11-bit column and for 4 internal banks. This gives
 * us the maximum address reach in memory. By writing a test value to the max
 * address and locating where it aliases to, we can determine the number of valid
 * column bits.
 *
 * This routine is called to determine the number of internal banks each DRAM
 * device has. The external bank (under test) is configured for maximum reach
 * with 11-bit columns and 4 internal banks. This routine will write to a max
 * address (BA1 and BA0 = 1) and then read from an address with BA1=0 to see if
 * that column is a "don't care". If BA1 does not affect write/read of data,
 * then this device has only 2 internal banks.
 *
 * This routine is called to determine the ending address for this external
 * bank of SDRAM. We write to a max address with a data value and then disable
 * row address bits looking for "don't care" locations. Each "don't care" bit
 * represents a dividing of the maximum density (128M) by 2. By dividing the
 * maximum of 32 4M chunks in an external bank down by all the "don't care" bits
 * determined during sizing, we set the proper density.
 *
 * WARNINGS.
 * bp must be preserved because it is used for return linkage.
 *
 * EXIT
 * nothing returned - but the memory subsystem is enabled
 *******************************************************************************
 */

#include <config.h>
#ifdef CONFIG_SC520

.section .text
.equ            DRCCTL,     0x0fffef010   /* DRAM control register */
.equ            DRCTMCTL,   0x0fffef012   /* DRAM timing control register */
.equ            DRCCFG,     0x0fffef014   /* DRAM bank configuration register */
.equ            DRCBENDADR, 0x0fffef018   /* DRAM bank ending address register */
.equ            ECCCTL,     0x0fffef020   /* DRAM ECC control register */
.equ            ECCINT,     0x0fffefd18   /* DRAM ECC nmi-INT mapping */
.equ            DBCTL,      0x0fffef040   /* DRAM buffer control register */

.equ            CACHELINESZ, 0x00000010   /* size of our cache line (read buffer) */
.equ            COL11_ADR,  0x0e001e00    /* 11 col addrs */
.equ            COL10_ADR,  0x0e000e00    /* 10 col addrs */
.equ            COL09_ADR,  0x0e000600    /*  9 col addrs */
.equ            COL08_ADR,  0x0e000200    /*  8 col addrs */
.equ            ROW14_ADR,  0x0f000000    /* 14 row addrs */
.equ            ROW13_ADR,  0x07000000    /* 13 row addrs */
.equ            ROW12_ADR,  0x03000000    /* 12 row addrs */
.equ            ROW11_ADR,  0x01000000    /* 11 row addrs/also bank switch */
.equ            ROW10_ADR,  0x00000000    /* 10 row addrs/also bank switch */
.equ            COL11_DATA, 0x0b0b0b0b    /* 11 col addrs */
.equ            COL10_DATA, 0x0a0a0a0a    /* 10 col data */
.equ            COL09_DATA, 0x09090909    /*  9 col data */
.equ            COL08_DATA, 0x08080808    /*  8 col data */
.equ            ROW14_DATA, 0x3f3f3f3f    /* 14 row data (MASK) */
.equ            ROW13_DATA, 0x1f1f1f1f    /* 13 row data (MASK) */
.equ            ROW12_DATA, 0x0f0f0f0f    /* 12 row data (MASK) */
.equ            ROW11_DATA, 0x07070707    /* 11 row data/also bank switch (MASK) */
.equ            ROW10_DATA, 0xaaaaaaaa    /* 10 row data/also bank switch (MASK) */


 /*
  * initialize dram controller registers
  */
.globl mem_init
mem_init:
	xorw    %ax,%ax
	movl    $DBCTL, %edi
	movb     %al, (%edi)             /* disable write buffer */

	movl    $ECCCTL, %edi
	movb     %al, (%edi)             /* disable ECC */

	movl    $DRCTMCTL, %edi
	movb    $0x1E,%al                /* Set SDRAM timing for slowest */
	movb     %al, (%edi)

 /*
  * setup loop to do 4 external banks starting with bank 3
  */
	movl    $0xff000000,%eax         /* enable last bank and setup */
	movl    $DRCBENDADR, %edi        /* ending address register */
	movl     %eax, (%edi)

	movl    $DRCCFG, %edi            /* setup */
	movw    $0xbbbb,%ax              /* dram config register for  */
	movw    %ax, (%edi)

 /*
  * issue a NOP to all DRAMs
  */
	movl    $DRCCTL, %edi            /* setup DRAM control register with */
	movb    $0x1,%al                 /* Disable refresh,disable write buffer */
	movb     %al, (%edi)
	movl    $CACHELINESZ, %esi       /* just a dummy address to write for */
	movw     %ax, (%esi)
 /*
  * delay for 100 usec? 200?
  * ******this is a cludge for now *************
  */
	movw    $100,%cx
sizdelay:
	loop    sizdelay                 /* we need 100 usec here */
 /***********************************************/

 /*
  * issue all banks precharge
  */
	movb    $0x2,%al                 /* All banks precharge */
	movb     %al, (%edi)
	movw     %ax, (%esi)

 /*
  * issue 2 auto refreshes to all banks
  */
	movb    $0x4,%al                 /* Auto refresh cmd */
	movb     %al, (%edi)
	movw    $2,%cx
refresh1:
	movw     %ax, (%esi)
	loop    refresh1

 /*
  * issue LOAD MODE REGISTER command
  */
	movb    $0x3,%al                 /* Load mode register cmd */
	movb     %al, (%edi)
	movw     %ax, (%esi)

 /*
  * issue 8 more auto refreshes to all banks
  */
	movb    $0x4,%al                 /* Auto refresh cmd */
	movb     %al, (%edi)
	movw    $8,%cx
refresh2:
	movw     %ax, (%esi)
	loop    refresh2

 /*
  * set control register to NORMAL mode
  */
	movb    $0x0,%al                 /* Normal mode value */
	movb     %al, (%edi)

 /*
  * size dram starting with external bank 3 moving to external bank 0
  */
	movl    $0x3,%ecx                /* start with external bank 3 */

nextbank:

 /*
  * write col 11 wrap adr
  */
	movl    $COL11_ADR, %esi         /* set address to max col (11) wrap addr */
	movl    $COL11_DATA, %eax        /* pattern for max supported columns(11) */
	movl    %eax, (%esi)             /* write max col pattern at max col adr */
	movl    (%esi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write col 10 wrap adr
  */

	movl    $COL10_ADR, %esi         /* set address to 10 col wrap address */
	movl    $COL10_DATA, %eax        /* pattern for 10 col wrap */
	movl    %eax, (%esi)             /* write 10 col pattern @ 10 col wrap adr */
	movl    (%esi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write col 9 wrap adr
  */
	movl    $COL09_ADR, %esi         /* set address to 9 col wrap address */
	movl    $COL09_DATA, %eax        /* pattern for 9 col wrap */
	movl    %eax, (%esi)             /* write 9 col pattern @ 9 col wrap adr */
	movl    (%esi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write col 8 wrap adr
  */
	movl    $COL08_ADR, %esi         /* set address to min(8) col wrap address */
	movl    $COL08_DATA, %eax        /* pattern for min (8) col wrap */
	movl    %eax, (%esi)             /* write min col pattern @ min col adr */
	movl    (%esi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write row 14 wrap adr
  */
	movl    $ROW14_ADR, %esi         /* set address to max row (14) wrap addr */
	movl    $ROW14_DATA, %eax        /* pattern for max supported rows(14) */
	movl    %eax, (%esi)             /* write max row pattern at max row adr */
	movl    (%esi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write row 13 wrap adr
  */
	movl    $ROW13_ADR, %esi         /* set address to 13 row wrap address */
	movl    $ROW13_DATA, %eax        /* pattern for 13 row wrap */
	movl    %eax, (%esi)             /* write 13 row pattern @ 13 row wrap adr */
	movl    (%esi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write row 12 wrap adr
  */
	movl    $ROW12_ADR, %esi         /* set address to 12 row wrap address */
	movl    $ROW12_DATA, %eax        /* pattern for 12 row wrap */
	movl    %eax, (%esi)             /* write 12 row pattern @ 12 row wrap adr */
	movl    (%esi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write row 11 wrap adr
  */
	movl    $ROW11_ADR, %edi         /* set address to 11 row wrap address */
	movl    $ROW11_DATA, %eax        /* pattern for 11 row wrap */
	movl    %eax, (%edi)             /* write 11 row pattern @ 11 row wrap adr */
	movl    (%edi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * write row 10 wrap adr --- this write is really to determine number of banks
  */
	movl    $ROW10_ADR, %edi         /* set address to 10 row wrap address */
	movl    $ROW10_DATA, %eax        /* pattern for 10 row wrap (AA) */
	movl    %eax, (%edi)             /* write 10 row pattern @ 10 row wrap adr */
	movl    (%edi), %ebx             /* optional read */
	cmpl    %ebx,%eax                /* to verify write */
	jnz     bad_ram                  /* this ram is bad */
 /*
  * read data @ row 12 wrap adr to determine  * banks,
  * and read data @ row 14 wrap adr to determine  * rows.
  * if data @ row 12 wrap adr is not AA, 11 or 12 we have bad RAM.
  * if data @ row 12 wrap == AA, we only have 2 banks, NOT 4
  * if data @ row 12 wrap == 11 or 12, we have 4 banks,
  */
	xorw    %di,%di                  /* value for 2 banks in DI */
	movl    (%esi), %ebx             /* read from 12 row wrap to check banks
					  * (esi is setup from the write to row 12 wrap) */
	cmpl    %ebx,%eax                /* check for AA pattern  (eax holds the aa pattern) */
	jz      only2                    /* if pattern == AA, we only have 2 banks */

	/* 4 banks */

	movw    $8,%di                   /* value for 4 banks in DI (BNK_CNT bit) */
	cmpl    $ROW11_DATA, %ebx        /* only other legitimate values are 11 */
	jz      only2
	cmpl    $ROW12_DATA, %ebx        /* and 12 */
	jnz     bad_ram                  /* its bad if not 11 or 12! */

	/* fall through */
only2:
 /*
  * validate row mask
  */
	movl    $ROW14_ADR, %esi         /* set address back to max row wrap addr */
	movl    (%esi), %eax             /* read actual number of rows @ row14 adr */

	cmpl    $ROW11_DATA, %eax        /* row must be greater than 11 pattern */
	jb      bad_ram

	cmpl    $ROW14_DATA, %eax        /* and row must be less than 14 pattern */
	ja      bad_ram

	cmpb    %ah,%al                  /* verify all 4 bytes of dword same */
	jnz     bad_ram
	movl    %eax,%ebx
	shrl    $16,%ebx
	cmpw    %bx,%ax
	jnz     bad_ram
 /*
  * read col 11 wrap adr for real column data value
  */
	movl    $COL11_ADR, %esi         /* set address to max col (11) wrap addr */
	movl    (%esi), %eax             /* read real col number at max col adr */
 /*
  * validate column data
  */
	cmpl    $COL08_DATA, %eax        /* col must be greater than 8 pattern */
	jb      bad_ram

	cmpl    $COL11_DATA, %eax        /* and row must be less than 11 pattern */
	ja      bad_ram

	subl    $COL08_DATA, %eax        /* normalize column data to zero */
	jc      bad_ram
	cmpb    %ah,%al                  /* verify all 4 bytes of dword equal */
	jnz     bad_ram
	movl    %eax,%edx
	shrl    $16,%edx
	cmpw    %dx,%ax
	jnz     bad_ram
 /*
  * merge bank and col data together
  */
	addw    %di,%dx                  /* merge of bank and col info in dl */
 /*
  * fix ending addr mask based upon col info
  */
	movb    $3,%al
	subb    %dh,%al                  /* dh contains the overflow from the bank/col merge  */
	movb    %bl,%dh                  /* bl contains the row mask (aa, 07, 0f, 1f or 3f) */
	xchgw   %cx,%ax                  /* cx = ax = 3 or 2 depending on 2 or 4 bank device */
	shrb    %cl,%dh	                 /*  */
	incb    %dh                      /* ending addr is 1 greater than real end */
	xchgw   %cx,%ax                  /* cx is bank number again */
 /*
  * issue all banks precharge
  */
bad_reint:
	movl    $DRCCTL, %esi            /* setup DRAM control register with */
	movb    $0x2,%al                 /* All banks precharge */
	movb     %al, (%esi)
	movl    $CACHELINESZ, %esi       /* address to init read buffer */
	movw     %ax, (%esi)

 /*
  * update ENDING ADDRESS REGISTER
  */
	movl    $DRCBENDADR, %edi        /* DRAM ending address register */
	movl    %ecx,%ebx
	addl	%ebx, %edi
	movb    %dh, (%edi)
 /*
  * update CONFIG REGISTER
  */
	xorb    %dh,%dh
	movw    $0x00f,%bx
	movw    %cx,%ax
	shlw    $2,%ax
	xchgw   %cx,%ax
	shlw    %cl,%dx
	shlw    %cl,%bx
	notw    %bx
	xchgw   %cx,%ax
	movl    $DRCCFG, %edi
	mov     (%edi), %ax
	andw    %bx,%ax
	orw     %dx,%ax
	movw    %ax, (%edi)
	jcxz    cleanup

	decw    %cx
	movl    %ecx,%ebx
	movl    $DRCBENDADR, %edi        /* DRAM ending address register */
	movb    $0xff,%al
	addl	%ebx, %edi
	movb    %al, (%edi)
 /*
  * set control register to NORMAL mode
  */
	movl    $DRCCTL, %esi            /* setup DRAM control register with */
	movb    $0x0,%al                 /* Normal mode value */
	movb    %al, (%esi)
	movl    $CACHELINESZ, %esi       /* address to init read buffer */
	movw    %ax, (%esi)
	jmp     nextbank

cleanup:
	movl    $DRCBENDADR, %edi        /* DRAM ending address register  */
	movw    $4,%cx
	xorw    %ax,%ax
cleanuplp:
	movb   (%edi), %al
	orb     %al,%al
	jz      emptybank

	addb    %ah,%al
	jns     nottoomuch

	movb    $0x7f,%al
nottoomuch:
	movb    %al,%ah
	orb     $0x80,%al
	movb    %al, (%edi)
emptybank:
	incl    %edi
	loop    cleanuplp

#if defined CFG_SDRAM_DRCTMCTL
	/* just have your hardware desinger _GIVE_ you what you need here! */
	movl    $DRCTMCTL, %edi
	movb    $CFG_SDRAM_DRCTMCTL,%al
	movb    (%edi), %al
#else
#if defined(CFG_SDRAM_CAS_LATENCY_2T) || defined(CFG_SDRAM_CAS_LATENCY_3T)
	/* set the CAS latency now since it is hard to do
	 * when we run from the RAM */
	movl    $DRCTMCTL, %edi          /* DRAM timing register */
	movb    (%edi), %al
#ifdef CFG_SDRAM_CAS_LATENCY_2T
	andb    $0xef, %al
#endif
#ifdef CFG_SDRAM_CAS_LATENCY_3T
	orb     $0x10, %al
#endif
	movb    %al, (%edi)
#endif
#endif
	movl    $DRCCTL, %edi            /* DRAM Control register */
	movb    $0x3,%al                 /* Load mode register cmd */
	movb     %al, (%edi)
	movw     %ax, (%esi)


	movl    $DRCCTL, %edi            /* DRAM Control register */
	movb    $0x18,%al                /*  Enable refresh and NORMAL mode */
	movb    %al, (%edi)

	jmp     dram_done

bad_ram:
	xorl    %edx,%edx
	xorl    %edi,%edi
	jmp     bad_reint

dram_done:

	/* readback DRCBENDADR and return the number
	 * of available ram bytes in %eax */

	movl    $DRCBENDADR, %edi        /* DRAM ending address register  */

	movl	(%edi), %eax
	movl	%eax, %ecx
	andl	$0x80000000, %ecx
	jz	bank2
	andl	$0x7f000000, %eax
	shrl	$2, %eax
	movl	%eax, %ebx

bank2: 	movl	(%edi), %eax
	movl	%eax, %ecx
	andl	$0x00800000, %ecx
	jz	bank1
	andl	$0x007f0000, %eax
	shll	$6, %eax
	movl	%eax, %ebx

bank1: 	movl	(%edi), %eax
	movl	%eax, %ecx
	andl	$0x00008000, %ecx
	jz	bank0
	andl	$0x00007f00, %eax
	shll	$14, %eax
	movl	%eax, %ebx

bank0: 	movl	(%edi), %eax
	movl	%eax, %ecx
	andl	$0x00000080, %ecx
	jz	done
	andl	$0x0000007f, %eax
	shll	$22, %eax
	movl	%eax, %ebx


done:
	movl	%ebx, %eax

#if CFG_SDRAM_ECC_ENABLE
	/* A nominal memory test: just a byte at each address line */
	movl    %eax, %ecx
	shrl    $0x1, %ecx
	movl	$0x1, %edi
memtest0:
	movb	$0xa5, (%edi)
	cmpb    $0xa5, (%edi)
	jne	out
	shrl	$1, %ecx
	andl	%ecx,%ecx
	jz	set_ecc
	shll	$1, %edi
	jmp	memtest0

set_ecc:
	/* clear all ram with a memset */
	movl	%eax, %ecx
	xorl	%esi, %esi
	xorl	%edi, %edi
	xorl	%eax, %eax
	shrl	$2, %ecx
	cld
	rep 	stosl
			/* enable read, write buffers */
	movb    $0x11, %al
	movl    $DBCTL, %edi
	movb    %al, (%edi)
			/* enable NMI mapping for ECC */
	movl    $ECCINT, %edi
	mov	$0x10, %al
	movb    %al, (%edi)
			/* Turn on ECC */
	movl    $ECCCTL, %edi
	mov	$0x05, %al
	movb    %al, (%edi)
#endif
out:
	movl	%ebx, %eax
	jmp	*%ebp

#endif /* CONFIG_SC520 */
