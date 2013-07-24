/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <config.h>
#include <gt64120.h>

#include <asm/addrspace.h>
#include <asm/regdef.h>
#include <asm/malta.h>

#ifdef CONFIG_SYS_BIG_ENDIAN
#define CPU_TO_GT32(_x)		((_x))
#else
#define CPU_TO_GT32(_x) (					\
	(((_x) & 0xff) << 24) | (((_x) & 0xff00) << 8) |	\
	(((_x) & 0xff0000) >> 8) | (((_x) & 0xff000000) >> 24))
#endif

	.text
	.set noreorder
	.set mips32

	.globl	lowlevel_init
lowlevel_init:

	/*
	 * Load BAR registers of GT64120 as done by YAMON
	 *
	 * based on a patch sent by Antony Pavlov <antonynpavlov@gmail.com>
	 * to the barebox mailing list.
	 * The subject of the original patch:
	 *   'MIPS: qemu-malta: add YAMON-style GT64120 memory map'
	 * URL:
	 * http://www.mail-archive.com/barebox@lists.infradead.org/msg06128.html
	 *
	 * based on write_bootloader() in qemu.git/hw/mips_malta.c
	 * see GT64120 manual and qemu.git/hw/gt64xxx.c for details
	 */

	/* move GT64120 registers from 0x14000000 to 0x1be00000 */
	li	t1, KSEG1ADDR(GT_DEF_BASE)
	li	t0, CPU_TO_GT32(0xdf000000)
	sw	t0, GT_ISD_OFS(t1)

	/* setup MEM-to-PCI0 mapping */
	li	t1, KSEG1ADDR(MALTA_GT_BASE)

	/* setup PCI0 io window to 0x18000000-0x181fffff */
	li	t0, CPU_TO_GT32(0xc0000000)
	sw	t0, GT_PCI0IOLD_OFS(t1)
	li	t0, CPU_TO_GT32(0x40000000)
	sw	t0, GT_PCI0IOHD_OFS(t1)

	/* setup PCI0 mem windows */
	li	t0, CPU_TO_GT32(0x80000000)
	sw	t0, GT_PCI0M0LD_OFS(t1)
	li	t0, CPU_TO_GT32(0x3f000000)
	sw	t0, GT_PCI0M0HD_OFS(t1)

	li	t0, CPU_TO_GT32(0xc1000000)
	sw	t0, GT_PCI0M1LD_OFS(t1)
	li	t0, CPU_TO_GT32(0x5e000000)
	sw	t0, GT_PCI0M1HD_OFS(t1)

	jr	ra
	 nop
