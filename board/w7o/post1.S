/*
 * (C) Copyright 2001
 * Bill Hunter, Wave 7 Optics, william.hunter@mediaone.net
 *  and
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Description:
 *	Routine to exercise memory for the bringing up of our boards.
 */
#include <config.h>
#include <ppc4xx.h>

#define _LINUX_CONFIG_H 1       /* avoid reading Linux autoconf.h file  */

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#include <watchdog.h>

#include "errors.h"

#define _ASMLANGUAGE

	.globl	test_sdram
	.globl	test_led
	.globl	log_stat
	.globl	log_warn
	.globl	log_err
	.globl  temp_uart_init
	.globl  post_puts
	.globl  disp_hex

/*****************************************************
*******   Text Strings for low level printing   ******
*******          In section got2               *******
*****************************************************/

/*
 * Define the text strings for errors and warnings.
 * Switch to .data section.
 */
	.section ".data"
err_str: 	.asciz "*** POST ERROR   = "
warn_str:	.asciz "*** POST WARNING = "
end_str:  .asciz "\r\n"

/*
 * Enter the labels in Global Entry Table (GOT).
 * Switch to .got2 section.
 */
	START_GOT
	GOT_ENTRY(err_str)
	GOT_ENTRY(warn_str)
	GOT_ENTRY(end_str)
	END_GOT

/*
 * Switch  back to .text section.
 */
	.text

/****************************************
 ****************************************
 ********    LED register test   ********
 ****************************************
 ***************************************/
test_led:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -12(r1)		/* Save back chain and move SP */
	stw	r0, +16(r1)		/* Save link register */
	stw	r4, +8(r1)		/* save R4 */

	WATCHDOG_RESET        		/* Reset the watchdog */

	addi    r3, 0, ERR_FF		/* first test value is ffff */
	addi	r4, r3, 0		/* save copy of pattern */
	bl	set_led			/* store first test value */
	bl	get_led			/* read it back */
	xor.	r4, r4, r3		/* compare to original */
#if defined(CONFIG_W7OLMC)
	andi.   r4, r4, 0x00ff		/* lmc has 8 bits */
#else
	andi.   r4, r4, 0xffff		/* lmg has 16 bits */
#endif
	beq     LED2			/* next test */
	addi    r3, 0, ERR_LED		/* error code = 1 */
	bl	log_err			/* display error and halt */
LED2:	addi    r3, 0, ERR_00		/* 2nd test value is 0000 */
	addi	r4, r3, 0		/* save copy of pattern */
	bl	set_led			/* store first test value */
	bl	get_led			/* read it back */
	xor.	r4, r4, r3		/* compare to original */
#if defined(CONFIG_W7OLMC)
	andi.   r4, r4, 0x00ff		/* lmc has 8 bits */
#else
	andi.   r4, r4, 0xffff		/* lmg has 16 bits */
#endif
	beq     LED3			/* next test */
	addi    r3, 0, ERR_LED		/* error code = 1 */
	bl	log_err			/* display error and halt */

LED3:	/* restore stack and return */
	lwz	r0, +16(r1)		/* Get saved link register */
	mtlr	r0			/* Restore link register */
	lwz	r4, +8(r1)		/* restore r4 */
	addi	r1, r1, +12		/* Remove frame from stack */
	blr				/* Return to calling function */

/****************************************
 ****************************************
 ********     SDRAM TESTS        ********
 ****************************************
 ***************************************/
test_sdram:
	/* called with mem size in r3 */
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -16(r1)		/* Save back chain and move SP */
	stw	r0, +20(r1)		/* Save link register */
	stmw	r30, +8(r1)		/* save R30,R31 */
					/* r30 is log2(mem size) */
					/* r31 is mem size */

	/* take log2 of total mem size */
	addi	r31, r3, 0		/* save total mem size */
	addi	r30, 0, 0		/* clear r30 */
l2_loop:
	srwi.	r31, r31, 1		/* shift right 1 */
	addi	r30, r30, 1		/* count shifts */
	bne	l2_loop			/* loop till done */
	addi	r30, r30, -1		/* correct for over count */
	addi	r31, r3, 0		/* save original size */

	/* now kick the dog and test the mem */
	WATCHDOG_RESET        		/* Reset the watchdog */
	bl	Data_Buster		/* test crossed/shorted data lines */
	addi	r3, r30, 0		/* get log2(memsize) */
	addi	r4, r31, 0		/* get memsize */
	bl	Ghost_Buster		/* test crossed/shorted addr lines */
	addi	r3, r31, 0		/* get mem size */
	bl	Bit_Buster		/* check for bad internal bits */

	/* restore stack and return */
	lmw	r30, +8(r1)		/* Restore r30, r31 */
	lwz	r0, +20(r1)		/* Get saved link register */
	mtlr	r0			/* Restore link register */
	addi	r1, r1, +16		/* Remove frame from stack */
	blr				/* Return to calling function */


/****************************************
 ********  sdram data bus test   ********
 ***************************************/
Data_Buster:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -24(r1)		/* Save back chain and move SP */
	stw	r0, +28(r1)		/* Save link register */
	stmw	r28, 8(r1)		/* save r28 - r31 on stack */
					/* r31 i/o register */
					/* r30 sdram base address */
					/* r29 5555 syndrom */
					/* r28 aaaa syndrom */

	/* set up led register for this test */
	addi    r3, 0, ERR_RAMG		/* set led code to 1 */
	bl	log_stat		/* store test value */
	/* now test the dram data bus */
	xor	r30, r30, r30		/* load r30 with base addr of sdram */
	addis	r31, 0, 0x5555		/* load r31 with test value */
	ori	r31, r31, 0x5555
	stw	r31,0(r30)		/* sto the value */
	lwz	r29,0(r30)		/* read it back */
	xor	r29,r31,r29		/* compare it to original */
	addis	r31, 0, 0xaaaa		/* load r31 with test value */
	ori	r31, r31, 0xaaaa
	stw	r31,0(r30)		/* sto the value */
	lwz	r28,0(r30)		/* read it back */
	xor	r28,r31,r28		/* compare it to original */
	or	r3,r28,r29		/* or together both error terms */
	/*
	 * Now that we have the error bits,
	 * we have to decide which part they are in.
	 */
	bl	get_idx			/* r5 is now index to error */
	addi	r3, r3, ERR_RAMG
	cmpwi	r3, ERR_RAMG		/* check for errors */
	beq	db_done			/* skip if no errors */
	bl	log_err			/* log the error */

db_done:
	lmw	r28, 8(r1)		/* restore r28 - r31 from stack */
	lwz	r0, +28(r1)		/* Get saved link register */
	addi	r1, r1, +24		/* Remove frame from stack */
	mtlr	r0			/* Restore link register */
	blr				/* Return to calling function */


/****************************************************
 ********  test for address ghosting in dram ********
 ***************************************************/

Ghost_Buster:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -36(r1)		/* Save back chain and move SP */
	stw	r0, +40(r1)		/* Save link register */
	stmw	r25, 8(r1)		/* save r25 - r31 on stack */
					/* r31 = scratch register */
					/* r30 is main referance loop counter,
					   0 to 23 */
					/* r29 is ghost loop count, 0 to 22 */
					/* r28 is referance address */
					/* r27 is ghost address */
					/* r26 is log2 (mem size) =
					     number of byte addr bits */
					/* r25 is mem size */

	/* save the log2(mem size) and mem size */
	addi	r26, r3, 0		/* r26 is number of byte addr bits */
	addi	r25, r4, 0		/* r25 is mem size in bytes */

	/* set the leds for address ghost test */
	addi	r3, 0, ERR_ADDG
	bl	set_led

	/* first fill memory with zeros */
	srwi	r31, r25, 2		/* convert bytes to longs */
	mtctr	r31			/* setup byte counter */
	addi	r28, 0, 0		/* start at address at 0 */
	addi	r31, 0, 0		/* data value = 0 */
clr_loop:
	stw	r31, 0(r28)		/* Store zero value */
	addi	r28, r28, 4		/* Increment to next word */
	andi.	r27, r28, 0xffff	/* check for 2^16 loops */
	bne	clr_skip		/* if not there, then skip */
	WATCHDOG_RESET   		/* kick the dog every now and then */
clr_skip:
	bdnz	clr_loop		/* Round and round... */

	/* now do main test */
	addi	r30, 0, 0		/* start referance counter at 0 */
outside:
	/*
	 * Calculate the referance address
	 *   the referance address is calculated by setting the (r30-1)
	 *   bit of the base address
	 * when r30=0, the referance address is the base address.
	 * thus the sequence 0,1,2,4,8,..,2^(n-1)
	 * setting the bit is done with the following shift functions.
	 */
	WATCHDOG_RESET   		/* Reset the watchdog */

	addi	r31, 0, 1		/* r31 = 1 */
	slw	r28, r31, r30		/* set bit coresponding to loop cnt */
	srwi	r28, r28, 1		/* then shift it right one so  */
					/*   we start at location 0 */
	/* fill referance address with Fs */
	addi	r31, 0, 0x00ff		/* r31 = one byte of set bits */
	stb     r31,0(r28)		/* save ff in referance address */

	/* ghost (inner) loop, now check all posible ghosted addresses */
	addi	r29, 0, 0		/* start ghosted loop counter at 0 */
inside:
	/*
	 * Calculate the ghost address by flipping one
	 *  bit of referance address.  This gives the
	 *  sequence 1,2,4,8,...,2^(n-1)
	 */
	addi	r31, 0, 1		/* r31 = 1 */
	slw     r27, r31, r29		/* set  bit coresponding to loop cnt */
	xor	r27, r28, r27		/* ghost address = ref addr with
					     bit flipped*/

	/* now check for ghosting */
	lbz     r31,0(r27)		/* get content of ghost addr */
	cmpwi   r31, 0			/* compare read value to 0 */
	bne	Casper			/*   we found a ghost! */

	/* now close ghost ( inner ) loop */
	addi 	r29, r29, 1		/* increment inner loop counter */
	cmpw 	r29, r26		/* check for last inner loop */
	blt		inside		/* do more inner loops */

	/* now close referance ( outer ) loop */
	addi 	r31, 0, 0 		/* r31 = zero */
	stb	r31, 0(28)		/* zero out the altered address loc. */
	/*
	 * Increment and check for end, count is zero based.
	 * With the ble, this gives us one more loops than
	 * address bits for sequence 0,1,2,4,8,...2^(n-1)
	*/
	addi 	r30, r30, 1		/* increment outer loop counter */
	cmpw 	r30, r26		/* check for last inner loop */
	ble	outside			/* do more outer loops */

	/* were done, lets go home */
	b	gb_done
Casper:					/* we found a ghost !! */
	addi	r3, 0, ERR_ADDF		/* get indexed error message */
	bl	log_err			/* log error led error code */
gb_done: /*  pack your bags, and go home */
	lmw     r25, 8(r1)              /* restore r25 - r31 from stack */
	lwz     r0, +40(r1)             /* Get saved link register */
	addi    r1, r1, +36             /* Remove frame from stack */
	mtlr    r0                      /* Restore link register */
	blr                             /* Return to calling function */

/****************************************************
 ********      SDRAM data fill tests       **********
 ***************************************************/
Bit_Buster:
	/* called with mem size in r3 */
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -16(r1)		/* Save back chain and move SP */
	stw	r0, +20(r1)		/* Save link register */
	stw	r4, +8(r1)		/* save R4 */
	stw	r5, +12(r1)		/* save r5 */

	addis	r5, r3, 0		/* save mem size */

	/* Test 55555555 */
	addi	r3, 0, ERR_R55G		/* set up error code in case we fail */
	bl	log_stat		/* store test value */
	addis	r4, 0, 0x5555
	ori	r4, r4, 0x5555
	bl	fill_test

	/* Test aaaaaaaa  */
	addi	r3, 0, ERR_RAAG		/* set up error code in case we fail */
	bl	log_stat		/* store test value */
	addis	r4, 0, 0xAAAA
	ori	r4, r4, 0xAAAA
	bl	fill_test

	/* Test 00000000  */
	addi	r3, 0, ERR_R00G		/* set up error code in case we fail */
	bl	log_stat		/* store test value */
	addis	r4, 0, 0
	ori	r4, r4, 0
	bl	fill_test

	/* restore stack and return */
	lwz	r5, +12(r1)		/* restore r4 */
	lwz	r4, +8(r1)		/* restore r4 */
	lwz	r0, +20(r1)		/* Get saved link register */
	addi	r1, r1, +16		/* Remove frame from stack */
	mtlr	r0			/* Restore link register */
	blr				/* Return to calling function */


/****************************************************
 ********             fill test              ********
 ***************************************************/
/*	tests memory by filling with value, and reading back */
/*	r5 = Size of memory in bytes */
/*	r4 = Value to write */
/*	r3 = Error code */
fill_test:
	mflr    r0                      /* Get link register */
	stwu    r1, -32(r1)             /* Save back chain and move SP */
	stw     r0, +36(r1)             /* Save link register */
	stmw    r27, 8(r1)              /* save r27 - r31 on stack */
					/* r31 - scratch register */
					/* r30 - memory address */
	mr	r27, r3
	mr	r28, r4
	mr	r29, r5

	WATCHDOG_RESET   		/* Reset the watchdog */

	/* first fill memory with Value */
	srawi	r31, r29, 2		/* convert bytes to longs */
	mtctr	r31			/* setup counter */
	addi	r30, 0, 0		/* Make r30 = addr 0 */
ft_0:	stw	r28, 0(r30)		/* Store value */
	addi	r30, r30, 4		/* Increment to next word */
	andi.	r31, r30, 0xffff	/* check for 2^16 loops */
	bne	ft_0a			/* if not there, then skip */
	WATCHDOG_RESET   		/* kick the dog every now and then */
ft_0a:	bdnz	ft_0			/* Round and round... */

	WATCHDOG_RESET   		/* Reset the watchdog */

	/* Now confirm Value is in memory */
	srawi	r31, r29, 2		/* convert bytes to longs */
	mtctr	r31			/* setup counter */
	addi	r30, 0, 0		/* Make r30 = addr 0 */
ft_1:	lwz	r31, 0(r30)		/* get value from memory */
	xor.	r31, r31, r28		/* Writen = Read ? */
	bne	ft_err			/* If bad, than halt */
	addi	r30, r30, 4		/* Increment to next word */
	andi.	r31, r30, 0xffff	/* check for 2^16 loops*/
	bne	ft_1a			/* if not there, then skip */
	WATCHDOG_RESET			/* kick the dog every now and then */
ft_1a:	bdnz	ft_1			/* Round and round... */

	WATCHDOG_RESET   		/* Reset the watchdog */

	b	fill_done		/* restore and return */

ft_err:	addi	r29, r27, 0		/* save current led code */
	addi	r27, r31, 0		/* get pattern in r27 */
	bl	get_idx			/* get index from r27 */
	add	r27, r27, r29		/* add index to old led code */
	bl	log_err			/* output led err code, halt CPU */

fill_done:
	lmw     r27, 8(r1)              /* restore r27 - r31 from stack */
	lwz     r0, +36(r1)             /* Get saved link register */
	addi    r1, r1, +32             /* Remove frame from stack */
	mtlr    r0                      /* Restore link register */
	blr                             /* Return to calling function */


/****************************************************
 *******  get error index from r3 pattern    ********
 ***************************************************/
get_idx:				/* r3 = (MSW(r3) !=0)*2 +
					    (LSW(r3) !=0) */
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -12(r1)		/* Save back chain and move SP */
	stw	r0, +16(r1)		/* Save link register */
	stw	r4, +8(r1)		/* save R4 */

	andi.	r4, r3, 0xffff		/* check for lower bits */
	beq	gi2			/* skip if no bits set */
	andis.	r4, r3, 0xffff		/* check for upper bits */
	beq	gi3			/* skip if no bits set */
	addi	r3, 0, 3		/* both upper and lower bits set */
	b	gi_done
gi2:	andis.	r4, r3, 0xffff		/* check for upper bits*/
	beq	gi4			/* skip if no bits set */
	addi	r3, 0, 2		/* only upper bits set */
	b	gi_done
gi3:	addi	r3, 0, 1		/* only lower bits set */
	b	gi_done
gi4:	addi	r3, 0, 0		/* no bits set */
gi_done:
	/* restore stack and return */
	lwz	r0, +16(r1)		/* Get saved link register */
	mtlr	r0			/* Restore link register */
	lwz	r4, +8(r1)		/* restore r4 */
	addi	r1, r1, +12		/* Remove frame from stack */
	blr				/* Return to calling function */

/****************************************************
 ********       set LED to R5 and hang       ********
 ***************************************************/
log_stat:				/* output a led code and continue */
set_led:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -12(r1)		/* Save back chain and move SP */
	stw	r0, +16(r1)		/* Save link register */
	stw	r4, +8(r1)		/* save R4 */

	addis	r4, 0, 0xfe00		/* LED buffer is at 0xfe000000 */
#if defined(CONFIG_W7OLMG)		/* only on gateway, invert outputs */
	xori	r3,r3, 0xffff		/* complement led code, active low */
	sth	r3, 0(r4)		/* store first test value */
	xori	r3,r3, 0xffff		/* complement led code, active low */
#else					/* if not gateway, then don't invert */
	sth	r3, 0(r4)		/* store first test value */
#endif

	/* restore stack and return */
	lwz	r0, +16(r1)		/* Get saved link register */
	mtlr	r0			/* Restore link register */
	lwz	r4, +8(r1)		/* restore r4 */
	addi	r1, r1, +12		/* Remove frame from stack */
	blr				/* Return to calling function */

get_led:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -12(r1)		/* Save back chain and move SP */
	stw	r0, +16(r1)		/* Save link register */
	stw	r4, +8(r1)		/* save R4 */

	addis	r4, 0, 0xfe00		/* LED buffer is at 0xfe000000 */
	lhz	r3, 0(r4)		/* store first test value */
#if defined(CONFIG_W7OLMG)		/* only on gateway, invert inputs */
	xori	r3,r3, 0xffff		/* complement led code, active low */
#endif

	/* restore stack and return */
	lwz	r0, +16(r1)		/* Get saved link register */
	mtlr	r0			/* Restore link register */
	lwz	r4, +8(r1)		/* restore r4 */
	addi	r1, r1, +12		/* Remove frame from stack */
	blr				/* Return to calling function */

log_err:	/* output the error and hang the board ( for now ) */
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -12(r1)		/* Save back chain and move SP */
	stw	r0, +16(r1)		/* Save link register */
	stw	r3, +8(r1)		/* save a copy of error code */
	bl	set_led			/* set the led pattern */
	GET_GOT				/* get GOT address in r14 */
	lwz	r3,GOT(err_str)		/* get address of string */
	bl	post_puts		/* output the warning string */
	lwz	r3, +8(r1)		/* get error code */
	addi	r4, 0, 2		/* set disp length to 2 nibbles */
	bl	disp_hex		/* output the error code */
	lwz	r3,GOT(end_str)		/* get address of string */
	bl	post_puts		/* output the warning string */
halt:
	b	halt			/* hang */

	/* restore stack and return */
	lwz	r0, +16(r1)		/* Get saved link register */
	mtlr	r0			/* Restore link register */
	addi	r1, r1, +12		/* Remove frame from stack */
	blr				/* Return to calling function */

log_warn:	/* output a warning, then continue with operations */
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -16(r1)		/* Save back chain and move SP */
	stw	r0, +20(r1)		/* Save link register */
	stw	r3, +8(r1)		/* save a copy of error code */
	stw	r14, +12(r1)		/* save a copy of r14 (used by GOT) */

	bl	set_led			/* set the led pattern */
	GET_GOT				/* get GOT address in r14 */
	lwz	r3,GOT(warn_str)	/* get address of string */
	bl	post_puts		/* output the warning string */
	lwz	r3, +8(r1)		/* get error code */
	addi	r4, 0, 2		/* set disp length to 2 nibbles */
	bl	disp_hex		/* output the error code */
	lwz	r3,GOT(end_str)		/* get address of string */
	bl	post_puts		/* output the warning string */

	addis	r3, 0, 64		/* has a long delay */
	mtctr	r3
log_2:
	WATCHDOG_RESET			/* this keeps dog from barking, */
					/*   and takes time */
	bdnz	log_2			/* loop till time expires */

	/* restore stack and return */
	lwz	r0, +20(r1)		/* Get saved link register */
	lwz	r14, +12(r1)		/* restore r14 */
	mtlr	r0			/* Restore link register */
	addi	r1, r1, +16		/* Remove frame from stack */
	blr				/* Return to calling function */

/*******************************************************************
 *	temp_uart_init
 *	Temporary UART initialization routine
 *	Sets up UART0 to run at 9600N81 off of the internal clock.
 *	R3-R4 are used.
 ******************************************************************/
temp_uart_init:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	stw	r0, +12(r1)		/* Save link register */

	addis   r3, 0, 0xef60
	ori     r3, r3, 0x0303          /* r3 = UART0_LCR */
	addi    r4, 0, 0x83             /* n81 format, divisor regs enabled */
	stb     r4, 0(r3)

	/* set baud rate to use internal clock,
	   baud = (200e6/16)/31/42 = 9600 */

	addis   r3, 0, 0xef60		/* Address of baud divisor reg */
	ori     r3, r3, 0x0300		/*   UART0_DLM */
	addi    r4, 0, +42		/* uart baud divisor LSB = 93 */
	stb     r4, 0(r3)               /* baud = (200 /16)/14/93 */

	addi    r3, r3, 0x0001		/* uart baud divisor addr */
	addi    r4, 0, 0
	stb     r4, 0(r3)               /* Divisor Latch MSB = 0 */

	addis   r3, 0, 0xef60
	ori     r3, r3, 0x0303          /* r3 = UART0_LCR */
	addi    r4, 0, 0x03             /* n81 format, tx/rx regs enabled */
	stb     r4, 0(r3)

	/* output a few line feeds */
	addi	r3, 0, '\n'		/* load line feed */
	bl 	post_putc		/* output the char */
	addi	r3, 0, '\n'		/* load line feed */
	bl 	post_putc		/* output the char */

	/* restore stack and return */
	lwz	r0, +12(r1)		/* Get saved link register */
	mtlr	r0			/* Restore link register */
	addi	r1, r1, +8		/* Remove frame from stack */
	blr				/* Return to calling function */

/**********************************************************************
 **	post_putc
 **	outputs charactor in R3
 **	r3 returns the error code ( -1 if there is an error )
 *********************************************************************/

post_putc:

	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -20(r1)		/* Save back chain and move SP */
	stw	r0, +24(r1)		/* Save link register */
	stmw	r29, 8(r1)		/* save	r29 - r31 on stack
					   r31 - uart base address
					   r30 - delay counter
					   r29 - scratch reg */

     addis   r31, 0, 0xef60		/* Point to uart base */
     ori     r31, r31, 0x0300
     addis   r30, 0, 152 		/* Load about 10,000,000 ticks. */
pputc_lp:
	lbz     r29, 5(r31) 		/* Read Line Status Register */
	andi.   r29, r29, 0x20		/* Check THRE status */
	bne     thre_set		/* Branch if FIFO empty */
	addic.  r30, r30, -1		/* Decrement and check if empty. */
	bne     pputc_lp		/* Try, try again */
	addi    r3, 0, -1		/* Load error code for timeout */
	b       pputc_done 		/* Bail out with error code set */
thre_set:
	stb     r3, 0(r31)		/* Store character to UART */
	addi	r3, 0, 0		/* clear error code */
pputc_done:
	lmw	r29, 8(r1)		/*restore r29 - r31 from stack */
	lwz	r0, +24(r1)		/* Get saved link register */
	addi	r1, r1, +20		/* Remove frame from stack */
	mtlr	r0			/* Restore link register */
	blr				/* Return to calling function */


/****************************************************************
    post_puts
    Accepts a null-terminated string pointed to by R3
    Outputs to the serial port until 0x00 is found.
    r3 returns the error code ( -1 if there is an error )
*****************************************************************/
post_puts:

	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -12(r1)		/* Save back chain and move SP */
	stw	r0, +16(r1)		/* Save link register */
	stw	r31, 8(r1)		/* save r31 - char pointer */

	addi 	r31, r3, 0              /* move pointer to R31 */
pputs_nxt:
	lbz	r3, 0(r31)		/* Get next character */
	addic.  r3, r3, 0		/* Check for zero */
	beq	pputs_term		/* bail out if zero */
	bl	post_putc		/* output the char */
	addic.	r3, r3, 0		/* check for error */
	bne	pputs_err
	addi 	r31, r31, 1		/* point to next char */
	b	pputs_nxt 		/* loop till term */
pputs_err:
	addi 	r3, 0, -1		/* set error code */
	b	pputs_end		/* were outa here */
pputs_term:
	addi 	r3, 0, 1		/* set success code */
	/* restore stack and return */
pputs_end:
	lwz	r31, 8(r1)		/* restore r27 - r31 from stack */
	lwz	r0, +16(r1)		/* Get saved link register */
	addi	r1, r1, +12		/* Remove frame from stack */
	mtlr	r0			/* Restore link register */
	blr				/* Return to calling function */


/********************************************************************
 *****	disp_hex
 *****	Routine to display a hex value from a register.
 *****	R3 is value to display
 *****	R4 is number of nibbles to display ie 2 for byte 8 for (long)word
 *****	Returns -1 in R3 if there is an error ( ie serial port hangs )
 *****	Returns 0 in R3 if no error
 *******************************************************************/
disp_hex:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -16(r1)		/* Save back chain and move SP */
	stw	r0, +20(r1)		/* Save link register */
	stmw	r30, 8(r1)		/* save r30 - r31 on stack */
					/* r31 output char */
					/* r30 uart base address */
	addi 	r30, 0, 8               /* Go through 8 nibbles. */
	addi 	r31, r3, 0
pputh_nxt:
	rlwinm	r31, r31, 4, 0, 31	/* Rotate next nibble into position */
	andi. 	r3, r31, 0x0f		/* Get nibble. */
	addi 	r3, r3, 0x30		/* Add zero's ASCII code. */
	cmpwi	r3, 0x03a
	blt	pputh_out
	addi 	r3, r3, 0x07            /* 0x27 for lower case. */
pputh_out:
	cmpw 	r30, r4
	bgt	pputh_skip
	bl	post_putc
	addic. 	r3, r3, 0 		/* check for error */
	bne	pputh_err
pputh_skip:
	addic.	r30, r30, -1
	bne	pputh_nxt
	xor	r3, r3, r3		/* Clear error code */
	b	pputh_done
pputh_err:
	addi 	r3, 0, -1 		/* set error code */
pputh_done:
	/* restore stack and return */
	lmw	r30, 8(r1)		/*  restore r30 - r31 from stack */
	lwz	r0, +20(r1)		/* Get saved link register */
	addi	r1, r1, +16		/* Remove frame from stack */
	mtlr	r0			/* Restore link register */
	blr				/* Return to calling function */
