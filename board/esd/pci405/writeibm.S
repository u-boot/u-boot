/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */
/*----------------------------------------------------------------------------- */
/* Function:     ext_bus_cntlr_init */
/* Description:  Initializes the External Bus Controller for the external */
/*		peripherals. IMPORTANT: For pass1 this code must run from */
/*		cache since you can not reliably change a peripheral banks */
/*		timing register (pbxap) while running code from that bank. */
/*		For ex., since we are running from ROM on bank 0, we can NOT */
/*		execute the code that modifies bank 0 timings from ROM, so */
/*		we run it from cache. */
/*	Bank 0 - Flash and SRAM */
/*	Bank 1 - NVRAM/RTC */
/*	Bank 2 - Keyboard/Mouse controller */
/*	Bank 3 - IR controller */
/*	Bank 4 - not used */
/*	Bank 5 - not used */
/*	Bank 6 - not used */
/*	Bank 7 - FPGA registers */
/*----------------------------------------------------------------------------- */
#include <asm/ppc4xx.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>


	.globl	write_without_sync
write_without_sync:
		/*
		 * Write one values to host via pci busmastering
		 * ptr = 0xc0000000 -> 0x01000000 (PCI)
		 * *ptr = 0x01234567;
		 */
	addi    r31,0,0
	lis     r31,0xc000

start1:
	lis     r0,0x0123
	ori     r0,r0,0x4567
	stw     r0,0(r31)

		/*
		 * Read one value back
		 * ptr = (volatile unsigned long *)addr;
		 * val = *ptr;
		 */

	lwz     r0,0(r31)

		/*
		 * One pci config write
		 * ibmPciConfigWrite(0x2e, 2, 0x1234);
		 */
		/* subsystem id */

	li      r4,0x002C
	oris    r4,r4,0x8000
	lis     r3,0xEEC0
	stwbrx  r4,0,r3

	li      r5,0x1234
	ori     r3,r3,0x4
	stwbrx  r5,0,r3

	b       start1

	blr	/* never reached !!!! */

	.globl	write_with_sync
write_with_sync:
		/*
		 * Write one values to host via pci busmastering
		 * ptr = 0xc0000000 -> 0x01000000 (PCI)
		 * *ptr = 0x01234567;
		 */
	addi    r31,0,0
	lis     r31,0xc000

start2:
	lis     r0,0x0123
	ori     r0,r0,0x4567
	stw     r0,0(r31)

		/*
		 * Read one value back
		 * ptr = (volatile unsigned long *)addr;
		 * val = *ptr;
		 */

	lwz     r0,0(r31)

		/*
		 * One pci config write
		 * ibmPciConfigWrite(0x2e, 2, 0x1234);
		 */
		/* subsystem id */

	li      r4,0x002C
	oris    r4,r4,0x8000
	lis     r3,0xEEC0
	stwbrx  r4,0,r3
	sync

	li      r5,0x1234
	ori     r3,r3,0x4
	stwbrx  r5,0,r3
	sync

	b       start2

	blr	/* never reached !!!! */

	.globl	write_with_less_sync
write_with_less_sync:
		/*
		 * Write one values to host via pci busmastering
		 * ptr = 0xc0000000 -> 0x01000000 (PCI)
		 * *ptr = 0x01234567;
		 */
	addi    r31,0,0
	lis     r31,0xc000

start2b:
	lis     r0,0x0123
	ori     r0,r0,0x4567
	stw     r0,0(r31)

		/*
		 * Read one value back
		 * ptr = (volatile unsigned long *)addr;
		 * val = *ptr;
		 */

	lwz     r0,0(r31)

		/*
		 * One pci config write
		 * ibmPciConfigWrite(0x2e, 2, 0x1234);
		 */
		/* subsystem id */

	li      r4,0x002C
	oris    r4,r4,0x8000
	lis     r3,0xEEC0
	stwbrx  r4,0,r3
	sync

	li      r5,0x1234
	ori     r3,r3,0x4
	stwbrx  r5,0,r3
/*        sync */

	b       start2b

	blr	/* never reached !!!! */

	.globl	write_with_more_sync
write_with_more_sync:
		/*
		 * Write one values to host via pci busmastering
		 * ptr = 0xc0000000 -> 0x01000000 (PCI)
		 * *ptr = 0x01234567;
		 */
	addi    r31,0,0
	lis     r31,0xc000

start3:
	lis     r0,0x0123
	ori     r0,r0,0x4567
	stw     r0,0(r31)
	sync

		/*
		 * Read one value back
		 * ptr = (volatile unsigned long *)addr;
		 * val = *ptr;
		 */

	lwz     r0,0(r31)
	sync

		/*
		 * One pci config write
		 * ibmPciConfigWrite(0x2e, 2, 0x1234);
		 */
		/* subsystem id (PCIC0_SBSYSVID)*/

	li      r4,0x002C
	oris    r4,r4,0x8000
	lis     r3,0xEEC0
	stwbrx  r4,0,r3
	sync

	li      r5,0x1234
	ori     r3,r3,0x4
	stwbrx  r5,0,r3
	sync

	b       start3

	blr	/* never reached !!!! */
