#include <config.h>
#include <74xx_7xx.h>
#include "version.h"

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#include "../include/mv_gen_reg.h"

#ifdef CONFIG_ECC
	/* Galileo specific asm code for initializing ECC */
	.globl board_relocate_rom
board_relocate_rom:
	mflr	r7
	/* update the location of the GT registers */
	lis	r11, CONFIG_SYS_GT_REGS@h
	/* if we're using ECC, we must use the DMA engine to copy ourselves */
	bl	start_idma_transfer_0
	bl	wait_for_idma_0
	bl	stop_idma_engine_0

	mtlr	r7
	blr

	.globl board_init_ecc
board_init_ecc:
	mflr	r7
	/* NOTE: r10 still contains the location we've been relocated to
	 * which happens to be TOP_OF_RAM - CONFIG_SYS_MONITOR_LEN */

	/* now that we're running from ram, init the rest of main memory
	 * for ECC use */
	lis	r8, CONFIG_SYS_MONITOR_LEN@h
	ori	r8, r8, CONFIG_SYS_MONITOR_LEN@l

	divw	r3, r10, r8

	/* set up the counter, and init the starting address */
	mtctr	r3
	li	r12, 0

	/* bytes per transfer */
	mr	r5, r8
about_to_init_ecc:
1:	mr	r3, r12
	mr	r4, r12
	bl	start_idma_transfer_0
	bl	wait_for_idma_0
	bl	stop_idma_engine_0
	add	r12, r12, r8
	bdnz	1b

	mtlr	r7
	blr

	/* r3:	dest addr
	 * r4:	source addr
	 * r5:	byte count
	 * r11: gt regbase
	 * trashes:	 r6, r5
	 */
start_idma_transfer_0:
	/* set the byte count, including the OWN bit */
	mr	r6, r11
	ori	r6, r6, CHANNEL0_DMA_BYTE_COUNT
	stwbrx	r5, 0, (r6)

	/* set the source address */
	mr	r6, r11
	ori	r6, r6, CHANNEL0_DMA_SOURCE_ADDRESS
	stwbrx	r4, 0, (r6)

	/* set the dest address */
	mr	r6, r11
	ori	r6, r6, CHANNEL0_DMA_DESTINATION_ADDRESS
	stwbrx	r3, 0, (r6)

	/* set the next record pointer */
	li	r5, 0
	mr	r6, r11
	ori	r6, r6, CHANNEL0NEXT_RECORD_POINTER
	stwbrx	r5, 0, (r6)

	/* set the low control register */
	/* bit 9 is NON chained mode, bit 31 is new style descriptors.
	   bit 12 is channel enable */
	ori	r5, r5, (1 << 12) | (1 << 12) | (1 << 11)
	/* 15 shifted by 16 (oris) == bit 31 */
	oris	r5, r5, (1 << 15)
	mr	r6, r11
	ori	r6, r6, CHANNEL0CONTROL
	stwbrx	r5, 0, (r6)

	blr

	/* this waits for the bytecount to return to zero, indicating
	 * that the trasfer is complete */
wait_for_idma_0:
	mr	r5, r11
	lis	r6, 0xff
	ori	r6, r6, 0xffff
	ori	r5, r5, CHANNEL0_DMA_BYTE_COUNT
1:	lwbrx	r4, 0, (r5)
	and.	r4, r4, r6
	bne	1b

	blr

	/* this turns off channel 0 of the idma engine */
stop_idma_engine_0:
	/* shut off the DMA engine */
	li	r5, 0
	mr	r6, r11
	ori	r6, r6, CHANNEL0CONTROL
	stwbrx	r5, 0, (r6)

	blr
#endif

#ifdef CONFIG_SYS_BOARD_ASM_INIT
	/* NOTE: trashes r3-r7 */
	.globl board_asm_init
board_asm_init:
	/* just move the GT registers to where they belong */
	lis	r3, CONFIG_SYS_DFL_GT_REGS@h
	ori	r3, r3, CONFIG_SYS_DFL_GT_REGS@l
	lis	r4, CONFIG_SYS_GT_REGS@h
	ori	r4, r4, CONFIG_SYS_GT_REGS@l
	li	r5, INTERNAL_SPACE_DECODE

	/* test to see if we've already moved */
	lwbrx	r6, r5, r4
	andi.	r6, r6, 0xffff
	/* check loading of R7 is: 0x0F80 should: 0xf800: DONE */
/*	rlwinm	r7, r4, 8, 16, 31
	rlwinm	r7, r4, 12, 16, 31	*/ /* original */
	rlwinm	r7, r4, 16, 16, 31
	/* -----------------------------------------------------*/
	cmp	cr0, r7, r6
	beqlr

	/* nope, have to move the registers */
	lwbrx	r6, r5, r3
	andis.	r6, r6, 0xffff
	or	r6, r6, r7
	stwbrx	r6, r5, r3

	/* now, poll for the change */
1:	lwbrx	r7, r5, r4
	cmp	cr0, r7, r6
	bne	1b

	/* done! */
	blr
#endif

/* For use of the debug LEDs */
	.global led_on0_relocated
led_on0_relocated:
	xor	r21, r21, r21
	xor	r18, r18, r18
	lis	r18, 0xFC80
	ori	r18, r18, 0x8000
	stw	r21, 0x0(r18)
/*	  stw	  r18, 0x0(r18) */
	sync
	blr

	.global led_off0_relocated
led_off0_relocated:
	xor	r21, r21, r21
	xor	r18, r18, r18
	lis	r18, 0xFC81
	ori	r18, r18, 0x4000
	stw	r21, 0x0(r18)
/*	  stw	  r18, 0x0(r18) */
	sync
	blr

	.global led_on0
led_on0:
	xor	r18, r18, r18
	lis	r18, 0x1c80
	ori	r18, r18, 0x8000
	stw	r18, 0x0(r18)
	sync
	blr

	.global led_off0
led_off0:
	xor	r18, r18, r18
	lis	r18, 0x1c81
	ori	r18, r18, 0x4000
	stw	r18, 0x0(r18)
	sync
	blr

	.global led_on1
led_on1:
	xor	r18, r18, r18
	lis	r18, 0x1c80
	ori	r18, r18, 0xc000
	stw	r18, 0x0(r18)
	sync
	blr

	.global led_off1
led_off1:
	xor	r18, r18, r18
	lis	r18, 0x1c81
	ori	r18, r18, 0x8000
	stw	r18, 0x0(r18)
	sync
	blr

	.global led_on2
led_on2:
	xor	r18, r18, r18
	lis	r18, 0x1c81
	ori	r18, r18, 0x0000
	stw	r18, 0x0(r18)
	sync
	blr

	.global led_off2
led_off2:
	xor	r18, r18, r18
	lis	r18, 0x1c81
	ori	r18, r18, 0xc000
	stw	r18, 0x0(r18)
	sync
	blr
