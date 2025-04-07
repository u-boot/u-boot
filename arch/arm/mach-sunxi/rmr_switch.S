@
@ ARMv8 RMR reset sequence on Allwinner SoCs.
@
@ All 64-bit capable Allwinner SoCs reset in AArch32 (and continue to
@ exectute the Boot ROM in this state), so we need to switch to AArch64
@ at some point.
@ Section G6.2.133 of the ARMv8 ARM describes the Reset Management Register
@ (RMR), which triggers a warm-reset of a core and can request to switch
@ into a different execution state (AArch32 or AArch64).
@ The address at which execution starts after the reset is held in the
@ RVBAR system register, which is architecturally read-only.
@ Allwinner provides a writable alias of this register in MMIO space, so
@ we can easily set the start address of AArch64 code.
@ This code below switches to AArch64 and starts execution at the specified
@ start address. It needs to be assembled by an ARM(32) assembler and
@ the machine code must be inserted as verbatim .word statements into the
@ beginning of the AArch64 U-Boot code.
@ To get the encoded bytes, use:
@ ${CROSS_COMPILE}gcc -c -Iinclude -Iarch/arm/include	\
@   -D__ASSEMBLY__ -DCONFIG_ARM64			\
@   -o rmr_switch.o arch/arm/mach-sunxi/rmr_switch.S
@ ${CROSS_COMPILE}objdump -d rmr_switch.o
@
@ The resulting words should be inserted into the U-Boot file at
@ arch/arm/include/asm/arch-sunxi/boot0.h.
@
@ This file is not build by the U-Boot build system, but provided only as a
@ reference and to be able to regenerate a (probably fixed) version of this
@ code found in encoded form in boot0.h.

#include <config.h>

.text
	b	start32			// this is "tst x0, x0" in AArch64
	.word	0x14000047		// this is "b   reset"  in AArch64

	.space  0x78			// gap distance set by the common
					// encoding of the first instruction
fel_stash_addr:
	.word	fel_stash - .		// distance to fel_stash buffer

start32:
	adr	r0, fel_stash_addr	// absolute location of fel_stash_addr
	ldr	r1, fel_stash_addr	// distance to actual fel_stash
	add	r0, r0, r1		// real address of fel_stash

	/* save the current state as needed by the BROM for a later return */
	str	sp, [r0]
	str	lr, [r0, #4]
	mrs	lr, CPSR
	str	lr, [r0, #8]
	mrc	p15, 0, lr, cr1, cr0, 0		// SCTLR
	str	lr, [r0, #12]
	mrc	p15, 0, lr, cr12, cr0, 0	// VBAR
	str	lr, [r0, #16]

	ldr	r1, =CONFIG_SUNXI_RVBAR_ADDRESS
	ldr	r0, =SUNXI_SRAMC_BASE
	ldr	r0, [r0, #36]			// SRAM_VER_REG
	ands	r0, r0, #0xff
	ldrne	r1, =CONFIG_SUNXI_RVBAR_ALTERNATIVE
#ifdef CONFIG_XPL_BUILD
	ldr	r0, =CONFIG_SPL_TEXT_BASE
#else
	ldr	r0, =CONFIG_TEXT_BASE
#endif
	str	r0, [r1]			// store start address in RVBAR
	dsb	sy
	isb	sy
	mrc	15, 0, r0, cr12, cr0, 2	@ read RMR register
	orr	r0, r0, #3		@ request reset in AArch64
	mcr	15, 0, r0, cr12, cr0, 2 @ write RMR register
	isb	sy
1:	wfi
	b	1b
