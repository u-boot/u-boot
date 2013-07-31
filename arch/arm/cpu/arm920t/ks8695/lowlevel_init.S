/*
 *  lowlevel_init.S - basic hardware initialization for the KS8695 CPU
 *
 *  Copyright (c) 2004-2005, Greg Ungerer <greg.ungerer@opengear.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <version.h>
#include <asm/arch/platform.h>

#ifndef CONFIG_SKIP_LOWLEVEL_INIT

/*
 *************************************************************************
 *
 * Handy dandy macros
 *
 *************************************************************************
 */

/* Delay a bit */
.macro DELAY_FOR cycles, reg0
	ldr     \reg0, =\cycles
	subs    \reg0, \reg0, #1
	subne   pc,  pc, #0xc
.endm

/*
 *************************************************************************
 *
 * Some local storage.
 *
 *************************************************************************
 */

/* Should we boot with an interactive console or not */
.globl serial_console

/*
 *************************************************************************
 *
 * Raw hardware initialization code. The important thing is to get
 * SDRAM setup and running. We do some other basic things here too,
 * like getting the PLL set for high speed, and init the LEDs.
 *
 *************************************************************************
 */

.globl lowlevel_init
lowlevel_init:

#if DEBUG
	/*
	 * enable UART for early debug trace
	 */
	ldr	r1, =(KS8695_IO_BASE+KS8695_UART_DIVISOR)
	mov	r2, #((25000000+CONFIG_BAUDRATE/2) / CONFIG_BAUDRATE)
	str	r2, [r1]
	ldr	r1, =(KS8695_IO_BASE+KS8695_UART_LINE_CTRL)
	mov	r2, #KS8695_UART_LINEC_WLEN8
	str	r2, [r1]		/* 8 data bits, no parity, 1 stop */
	ldr	r1, =(KS8695_IO_BASE+KS8695_UART_TX_HOLDING)
	mov	r2, #0x41
	str	r2, [r1]		/* write 'A' */
#endif
#if DEBUG
	ldr	r1, =(KS8695_IO_BASE+KS8695_UART_TX_HOLDING)
	mov	r2, #0x42
	str	r2, [r1]
#endif

	/*
	 * remap the memory and flash regions. we want to end up with
	 * ram from address 0, and flash at 32MB.
	 */
	ldr	r1, =(KS8695_IO_BASE+KS8695_MEM_CTRL0)
	ldr	r2, =0xbfc00040
	str	r2, [r1]		/* large flash map */
	ldr	pc, =(highflash+0x02000000-0x00f00000)	/* jump to high flash address */
highflash:
	ldr	r2, =0x8fe00040
	str	r2, [r1]		/* remap flash range */

	/*
	 * remap the second select region to the 4MB immediately after
	 * the first region. This way if you have a larger flash (say 8Mb)
	 * then you can have it all mapped nicely. Has no effect if you
	 * only have a 4Mb or smaller flash.
	 */
	ldr	r1, =(KS8695_IO_BASE+KS8695_MEM_CTRL1)
	ldr	r2, =0x9fe40040
	str	r2, [r1]		/* remap flash2 region, contiguous */
	ldr	r1, =(KS8695_IO_BASE+KS8695_MEM_GENERAL)
	ldr	r2, =0x30000005
	str	r2, [r1]		/* enable both flash selects */

#ifdef CONFIG_CM41xx
	/*
	 * map the second flash chip, using the external IO lines.
	 */
	ldr	r1, =(KS8695_IO_BASE+KS8695_IO_CTRL0)
	ldr	r2, =0xafe80b6d
	str	r2, [r1]		/* remap io0 region, contiguous */
	ldr	r1, =(KS8695_IO_BASE+KS8695_IO_CTRL1)
	ldr	r2, =0xbfec0b6d
	str	r2, [r1]		/* remap io1 region, contiguous */
	ldr	r1, =(KS8695_IO_BASE+KS8695_MEM_GENERAL)
	ldr	r2, =0x30050005
	str	r2, [r1]		/* enable second flash */
#endif

	/*
	 * before relocating, we have to setup RAM timing
	 */
	ldr	r1, =(KS8695_IO_BASE+KS8695_SDRAM_CTRL0)
#if (PHYS_SDRAM_1_SIZE == 0x02000000)
	ldr	r2, =0x7fc0000e		/* 32MB */
#else
	ldr	r2, =0x3fc0000e		/* 16MB */
#endif
	str	r2, [r1]		/* configure sdram bank0 setup */
	ldr	r1, =(KS8695_IO_BASE+KS8695_SDRAM_CTRL1)
	mov	r2, #0
	str	r2, [r1]		/* configure sdram bank1 setup */

	ldr	r1, =(KS8695_IO_BASE+KS8695_SDRAM_GENERAL)
	ldr	r2, =0x0000000a
	str	r2, [r1]		/* set RAS/CAS timing */

	ldr	r1, =(KS8695_IO_BASE+KS8695_SDRAM_BUFFER)
	ldr	r2, =0x00030000
	str	r2, [r1]		/* send NOP command */
	DELAY_FOR 0x100, r0
	ldr	r2, =0x00010000
	str	r2, [r1]		/* send PRECHARGE-ALL */
	DELAY_FOR 0x100, r0

	ldr	r1, =(KS8695_IO_BASE+KS8695_SDRAM_REFRESH)
	ldr	r2, =0x00000020
	str	r2, [r1]		/* set for fast refresh */
	DELAY_FOR 0x100, r0
	ldr	r2, =0x00000190
	str	r2, [r1]		/* set normal refresh timing */

	ldr	r1, =(KS8695_IO_BASE+KS8695_SDRAM_BUFFER)
	ldr	r2, =0x00020033
	str	r2, [r1]		/* send mode command */
	DELAY_FOR 0x100, r0
	ldr	r2, =0x01f00000
	str	r2, [r1]		/* enable sdram fifos */

	/*
	 * set pll to top speed
	 */
	ldr	r1, =(KS8695_IO_BASE+KS8695_SYSTEN_BUS_CLOCK)
	mov	r2, #0
	str	r2, [r1]		/* set pll clock to 166MHz */

	ldr	r1, =(KS8695_IO_BASE+KS8695_SWITCH_CTRL0)
	ldr	r2, [r1]		/* Get switch ctrl0 register       */
	and	r2, r2, #0x0fc00000	/* Mask out LED control bits       */
	orr	r2, r2, #0x01800000	/* Set Link/activity/speed actions */
	str	r2, [r1]

#ifdef CONFIG_CM4008
	ldr	r1, =(KS8695_IO_BASE+KS8695_GPIO_MODE)
	ldr	r2, =0x0000fe30
	str	r2, [r1]		/* enable LED's as outputs	    */
	ldr	r1, =(KS8695_IO_BASE+KS8695_GPIO_DATA)
	ldr	r2, =0x0000fe20
	str	r2, [r1]		/* turn on power LED		    */
#endif
#if defined(CONFIG_CM4008) || defined(CONFIG_CM41xx)
	ldr	r2, [r1]		/* get current GPIO input data	    */
	tst	r2, #0x8		/* check if "erase" depressed	    */
	beq	nobutton
	mov	r2, #0			/* be quiet on boot, no console	    */
	ldr	r1, =serial_console
	str	r2, [r1]
nobutton:
#endif

	add	lr, lr, #0x02000000	/* flash is now mapped high */
	add	ip, ip, #0x02000000	/* this is a hack */
	mov	pc, lr			/* all done, return */

#endif /* CONFIG_SKIP_LOWLEVEL_INIT */
