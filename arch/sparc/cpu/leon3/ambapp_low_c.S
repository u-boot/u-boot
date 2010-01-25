/* C-interface for AMBA PnP scanning functions implemented in
 * ambapp_low.S. At the point the memory and stack can be
 * used.
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


	.seg	"text"
	.extern	_nomem_ambapp_find_buses
	.extern	_nomem_find_apb
	.extern	_nomem_find_ahb

	.globl	ambapp_find_buses
	.globl	ambapp_find_apb
	.globl	ambapp_find_ahb


/* C-interface for _nomem_ambapp_find_buses used when memory is available.
 */
ambapp_find_buses:
	save	%sp, -104, %sp
	mov	%i1, %l7	/* Save second argument */
	call _nomem_ambapp_find_buses
	 mov	%i0, %o0

	/* Store result */
	st	%g0, [%l7+0x00]
	st	%i0, [%l7+0x04]
	st	%i1, [%l7+0x08]
	st	%i2, [%l7+0x0c]
	st	%i3, [%l7+0x10]
	st	%i4, [%l7+0x14]
	st	%i5, [%l7+0x18]

	ret
	 restore

/* C-interface for _nomem_find_apb used when memory is available.
 *
 * void ambapp_find_apb(
 *	struct ambapp_bus *abus,
 *	unsigned int dev_vend,
 *	int index,
 *	struct ambapp_find_apb_info *result
 *	);
 *
 */
ambapp_find_apb:
	save	%sp, -104, %sp

	mov	%i3, %l7	/* Save second argument */
	mov	%i1, %o1
	mov	%i2, %o2

	/* Initialize buses available in system */
	ld	[%i0+0x08], %i1
	ld	[%i0+0x0c], %i2
	ld	[%i0+0x10], %i3
	ld	[%i0+0x14], %i4
	ld	[%i0+0x18], %i5

	call _nomem_find_apb
	 ld	[%i0+0x04], %i0

	st	%o2, [%l7+0x08]	/* Decremented Index */
	st	%o3, [%l7]	/* PnP configuration address of APB Device */
	st	%o4, [%l7+0x04]	/* AHB Bus Index of AHB/APB bridge and APB Device */
	mov	%o0, %i0
	ret
	 restore

/* C-interface for _nomem_find_ahb used when memory is available.
 *
 * void ambapp_find_ahb(
 *	struct ambapp_bus *abus,
 *	unsigned int dev_vend,
 *	int index,
 *	int type,
 *	struct ambapp_find_ahb_info *result
 *	);
 *
 */
ambapp_find_ahb:
	save	%sp, -104, %sp

	mov	%i4, %l7	/* Save second argument */
	clr	%o0
	mov	%i1, %o1
	mov	%i2, %o2
	clr	%o3
	clr	%o4
	mov	%i3, %o5

	/* Initialize buses available in system */
	ld	[%i0+0x08], %i1
	ld	[%i0+0x0c], %i2
	ld	[%i0+0x10], %i3
	ld	[%i0+0x14], %i4
	ld	[%i0+0x18], %i5

	call _nomem_find_ahb
	 ld	[%i0+0x04], %i0

	st	%o2, [%l7+0x08]	/* Decremented Index */
	st	%o3, [%l7]	/* PnP configuration address of AHB Device */
	st	%o4, [%l7+0x04]	/* AHB Bus Index of AHB Device */
	mov	%o0, %i0
	ret
	 restore
