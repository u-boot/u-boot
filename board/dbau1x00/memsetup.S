/* Memory sub-system initialization code */

#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/au1x00.h>

	.globl	memsetup
memsetup:
	/* First setup pll:s to make serial work ok */
	/* We have a 12 MHz crystal */
	li              t0, SYS_CPUPLL
	li              t1, 0x21 /* 396 MHz */
	sw              t1, 0(t0)
	sync
	nop

	/* Setup AUX PLL */
	li              t0, SYS_AUXPLL
	li              t1, 8 /* 96 MHz */
	sw              t1, 0(t0) /* aux pll */
	sync

/* SDCS 0,1 SDRAM */
	li		t0, MEM_SDMODE0
	li		t1, 0x005522AA
	sw		t1, 0(t0)

	li              t0, MEM_SDMODE1
	li		t1, 0x005522AA
	sw		t1, 0(t0)

	li              t0, MEM_SDADDR0
	li		t1, 0x001003F8
	sw		t1, 0(t0)


	li              t0, MEM_SDADDR1
	li		t1, 0x001023F8
	sw		t1, 0(t0)
	sync

	li              t0, MEM_SDREFCFG
	li		t1, 0x64000C24 /* Disable */
	sw		t1, 0(t0)
	sync

	li              t0, MEM_SDPRECMD
	sw		zero, 0(t0)
	sync

	li              t0, MEM_SDAUTOREF
	sw		zero, 0(t0)
	sync
	sw		zero, 0(t0)
	sync

	li              t0, MEM_SDREFCFG
	li		t1, 0x66000C24 /* Enable */
	sw		t1, 0(t0)
	sync

	li              t0, MEM_SDWRMD0
	li		t1, 0x00000033
	sw		t1, 0(t0)
	sync

	li              t0, MEM_SDWRMD1
	li		t1, 0x00000033
	sw		t1, 0(t0)
	sync

	/*  Static memory controller */

	/* RCE0 AMD 29LV640M MirrorBit Flash */
	li              t0, MEM_STCFG0
	li		t1, 0x00000003
	sw		t1, 0(t0)

	li		t0, MEM_STTIME0
	li		t1, 0x22080b20
	sw		t1, 0(t0)

	li		t0, MEM_STADDR0
	li		t1, 0x11E03F80
	sw		t1, 0(t0)

	/* RCE1 CPLD Board Logic */
	li		t0, MEM_STCFG1
	li		t1, 0x00000080
	sw		t1, 0(t0)

	li		t0, MEM_STTIME1
	li		t1, 0x22080a20
	sw		t1, 0(t0)

	li		t0, MEM_STADDR1
	li		t1, 0x10c03f00
	sw		t1, 0(t0)

	/* RCE3 PCMCIA 250ns */
	li		t0, MEM_STCFG3
	li		t1, 0x00000002
	sw		t1, 0(t0)


	li		t0, MEM_STTIME3
	li		t1, 0x280E3E07
	sw		t1, 0(t0)

	li		t0, MEM_STADDR3
	li		t1, 0x10000000
	sw		t1, 0(t0)

	sync

	j	ra
	nop
