/*
 * Board specific setup info
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/arch/omap.h>
#include <linux/linkage.h>

ENTRY(save_boot_params)
	/*
	 * See if the rom code passed pointer is valid:
	 * It is not valid if it is not in non-secure SRAM
	 * This may happen if you are booting with the help of
	 * debugger
	 */
	ldr     r2, =NON_SECURE_SRAM_START
	cmp	r2, r0
	bgt	1f
	ldr	r2, =NON_SECURE_SRAM_END
	cmp	r2, r0
	blt	1f

	/*
	 * store the boot params passed from rom code or saved
	 * and passed by SPL
	 */
	cmp	r0, #0
	beq	1f
	ldr	r1, =boot_params
	str	r0, [r1]
#ifdef CONFIG_SPL_BUILD
	/* Store the boot device in omap_boot_device */
	ldrb	r2, [r0, #BOOT_DEVICE_OFFSET]	@ r1 <- value of boot device
	and	r2, #BOOT_DEVICE_MASK
	ldr	r3, =boot_params
	strb	r2, [r3, #BOOT_DEVICE_OFFSET]	@ omap_boot_device <- r1

	/* boot mode is passed only for devices that can raw/fat mode */
	cmp	r2, #2
	blt	2f
	cmp	r2, #7
	bgt	2f
	/* Store the boot mode (raw/FAT) in omap_boot_mode */
	ldr	r2, [r0, #DEV_DESC_PTR_OFFSET]	@ get the device descriptor ptr
	ldr	r2, [r2, #DEV_DATA_PTR_OFFSET]	@ get the pDeviceData ptr
	ldr	r2, [r2, #BOOT_MODE_OFFSET]	@ get the boot mode
	ldr	r3, =omap_bootmode
	str	r2, [r3]
#endif
2:
	ldrb	r2, [r0, #CH_FLAGS_OFFSET]
	ldr	r3, =boot_params
	strb	r2, [r3, #CH_FLAGS_OFFSET]
1:
	bx	lr
ENDPROC(save_boot_params)

ENTRY(lowlevel_init)
	/*
	 * Setup a temporary stack
	 */
	ldr	sp, =LOW_LEVEL_SRAM_STACK

	/*
	 * Save the old lr(passed in ip) and the current lr to stack
	 */
	push	{ip, lr}

	/*
	 * go setup pll, mux, memory
	 */
	bl	s_init
	pop	{ip, pc}
ENDPROC(lowlevel_init)

ENTRY(set_pl310_ctrl_reg)
	PUSH	{r4-r11, lr}	@ save registers - ROM code may pollute
				@ our registers
	LDR	r12, =0x102	@ Set PL310 control register - value in R0
	.word	0xe1600070	@ SMC #0 - hand assembled because -march=armv5
				@ call ROM Code API to set control register
	POP	{r4-r11, pc}
ENDPROC(set_pl310_ctrl_reg)
