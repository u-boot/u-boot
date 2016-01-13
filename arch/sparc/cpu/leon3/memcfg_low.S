/* This is the memory initialization functions, the function
 * implemented below initializes each memory controller
 * found and specified by the input grlib_mctrl_handler structure.
 *
 * After the memory controllers have been initialized the stack
 * can be used.
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <ambapp.h>
#include "memcfg.h"
#include <config.h>

	.seg	"text"
	.globl	_nomem_memory_ctrl_init
	.globl	_nomem_mctrl_init, _nomem_ahbmctrl_init
	.extern	_nomem_find_apb
	.extern	_nomem_find_ahb


/* FUNCTION
 *   _nomem_memory_controller_init(struct grlib_mctrl_handler *mem_handlers)
 *
 * Initialize AMBA devices, _nomem_amba_init() has prepared i0-i5
 * with the AHB buses on the system.
 *
 * For each entry in mem_handlers find the VENDOR:DEVICE and handle it
 * by calling the handler function pointer.
 *
 * Constraints:
 *  i6, i7, o6, l7, l6, g3, g4, g5, g6, g7 is used by caller
 *  o7 is return address
 *  l5 reserved for this function for future use.
 *
 * Arguments
 *  - o0 Pointer to memory handler array
 *
 * Results
 *  - o0 Number of memory controllers found
 *
 * Clobbered
 *  - o0 (Current AHB slave conf address)
 *  - l0 (mem handler entry address)
 *  - l1 (Return value, number of memory controllers found)
 *  - o7 (function pointer)
 *  - l0, l1, l2, l3, l4, g1, g2 (used by _nomem_ambapp_find_buses)
 *  - o0, o1, o2, o3, o4, o5 (Used as arguments)
 *
 *  - g1 ( level 1 return address)
 *  - g2 ( level 2 return address)
 */

_nomem_memory_ctrl_init:
	/* At this point all AHB buses has been found and the I/O Areas of
	 * all AHB buses is stored in the i0-i5 registers. Max 6 buses. Next,
	 * memory controllers are found by searching all buses for matching
	 * VENDOR:DEVICE. The VENDOR:DEVICE to search for are taken from the
	 * mem_handlers array. For each match the function pointer stored in
	 * the mem_handler entry is called to handle the hardware setup.
	 */
	mov	%o7, %g1	/* Save return address */
	mov	%o0, %l0
	mov	%g0, %l1	/* The return value */

.L_do_one_mem_handler:
	ld	[%l0 + MH_FUNC], %o7
	cmp	%o7, %g0
	be	.L_all_mctrl_handled
	 nop

	/*** Scan for memory controller ***/

	/* Set up argments, o5 not used by _nomem_find_apb */
	ldub	[%l0 + MH_TYPE], %o5
	clr	%o4
	clr	%o3
	ldub	[%l0 + MH_INDEX], %o2
	ld	[%l0 + MH_VENDOR_DEVICE], %o1

	/* An empty config? */
	cmp	%o5, DEV_NONE
	beq	.L_all_mctrl_next

	/* Select function (APB or AHB) */
	 cmp	%o5, DEV_APB_SLV
	bne	.L_find_ahb_memctrl
	 clr	%o0
.L_find_apb_memctrl:
	call	_nomem_find_apb			/* Scan for APB slave device */
	 nop

	/* o3 = iobar address
	 * o4 = AHB Bus index
	 *
	 * REG ADR = ((iobar >> 12) & (iobar << 4) & 0xfff00) | "APB Base"
	 */
	ld	[%o3 + AMBA_APB_IOBAR_OFS], %o5
	srl	%o5, 12, %o2
	sll	%o5, 4, %o5
	and	%o2, %o5, %o5
	set	0xfff00, %o2
	and	%o2, %o5, %o5
	sethi	%hi(0xfff00000), %o2
	and	%o3, %o2, %o2
	or	%o5, %o2, %o5	/* Register base address */

	ba	.L_call_one_mem_handler
	 nop

.L_find_ahb_memctrl:
	call	_nomem_find_ahb		/* Scan for AHB Slave or Master.
					 * o5 determine type. */
	 nop
	clr	%o5

	/* Call the handler function if the hardware was found
	 *
	 * o0 = mem_handler
	 * o1 = Configuration address
	 * o2 = AHB Bus index
	 * o3 = APB Base register (if APB Slave)
	 *
	 * Constraints:
	 * i0-i7, l0, l1, l5, g1, g3-g7 may no be used.
	 */
.L_call_one_mem_handler:
	cmp	%o0, %g0
	be	.L_all_mctrl_next
	 mov	%l0, %o0			/* Mem handler pointer */
	mov	%o3, %o1			/* AMBA PnP Configuration address */
	mov	%o4, %o2			/* AHB Bus index */
	ld	[%l0 + MH_FUNC], %o7	/* Get Function pointer */
	call	%o7
	 mov	%o5, %o3			/* APB Register Base Address */

	inc	%l1				/* Number of Memory controllers
						 * handled. */

	/* Do next entry in mem_handlers */
.L_all_mctrl_next:
	ba	.L_do_one_mem_handler
	 add	%l0, MH_STRUCT_SIZE, %l0

.L_all_mctrl_handled:
	mov	%g1, %o7	/* Restore return address */
	retl
	 mov	%l1, %o0



/* Generic Memory controller initialization routine (APB Registers)
 *
 * o0 = mem_handler structure pointer
 * o1 = Configuration address
 * o2 = AHB Bus index
 * o3 = APB Base register
 *
 * Clobbered
 *  o0-o4
 */
_nomem_mctrl_init:
	ld	[%o0 + MH_PRIV], %o0	/* Get Private structure */
	ld	[%o0], %o1		/* Get Reg Mask */
	and	%o1, 0xff, %o1
	add	%o0, REGS_OFS, %o0	/* Point to first reg */
.L_do_one_reg:
	andcc	%o1, 0x1, %g0
	beq	.L_do_next_reg
	 ld	[%o0], %o2
	ld	[%o3], %o4
	and	%o4, %o2, %o4
	ld	[%o0 + 4], %o2
	or	%o4, %o2, %o4
	st	%o4, [%o3]

.L_do_next_reg:
	add	%o0, REGS_SIZE, %o0
	add	%o3, 4, %o3
	srl	%o1, 1, %o1
	cmp	%o1, 0
	bne	.L_do_one_reg
	 nop

	/* No more registers to write */
	retl
	 nop



/* Generic Memory controller initialization routine (AHB Registers)
 *
 * o0 = mem_handler structure pointer
 * o1 = Configuration address of memory controller
 * o2 = AHB Bus index
 *
 * Clobbered
 *  o0-o5
 */
_nomem_ahbmctrl_init:
	ld	[%o0 + MH_PRIV], %o0		/* Get Private structure */

	/* Get index of AHB MBAR to get registers from */
	ld	[%o0], %o5
	add	%o0, 4, %o0

	/* Get Address of MBAR in PnP info */
	add	%o5, 4, %o5
	sll	%o5, 2, %o5
	add	%o5, %o1, %o5			/* Address of MBAR */

	/* Get Address of registers from PnP information
	 * Address is in AHB I/O format, i.e. relative to bus
	 *
	 * ADR = (iobar & (iobar << 16) & 0xfff00000)
	 * IOADR = (ADR >> 12) | "APB Base"
	 */
	ld	[%o5], %o5
	sll	%o5, 16, %o4
	and	%o5, %o4, %o5
	sethi	%hi(0xfff00000), %o4
	and	%o5, %o4, %o5			/* ADR */
	and	%o4, %o1, %o4
	srl	%o5, 12, %o5
	or	%o5, %o4, %o3			/* IOADR in o3 */

	ld	[%o0], %o1			/* Get Reg Mask */
	and	%o1, 0xff, %o1
	add	%o0, REGS_OFS, %o0		/* Point to first reg */
.L_do_one_ahbreg:
	andcc	%o1, 0x1, %g0
	beq	.L_do_next_reg
	 ld	[%o0], %o2
	ld	[%o3], %o4
	and	%o4, %o2, %o4
	ld	[%o0 + 4], %o2
	or	%o4, %o2, %o4
	st	%o4, [%o3]

.L_do_next_ahbreg:
	add	%o0, REGS_SIZE, %o0
	add	%o3, 4, %o3
	srl	%o1, 1, %o1
	cmp	%o1, 0
	bne	.L_do_one_reg
	 nop

	/* No more registers to write */
	retl
	 nop
