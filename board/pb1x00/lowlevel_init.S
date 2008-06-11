/* Memory sub-system initialization code */

#include <config.h>
#include <asm/regdef.h>
#include <asm/au1x00.h>
#include <asm/mipsregs.h>

#define AU1500_SYS_ADDR		0xB1900000
#define sys_endian		0x0038
#define CP0_Config0		$16
#define MEM_1MS			((396000000/1000000) * 1000)

	.text
	.set noreorder
	.set mips32

	.globl	lowlevel_init
lowlevel_init:
	/*
	 * Step 1) Establish CPU endian mode.
	 * NOTE: A fair amount of code is necessary on the Pb1000 to
	 * obtain the value of Switch S8.1 which is used to determine
	 * endian at run-time.
	 */

	/* RCE1 */
	li		t0, MEM_STCFG1
	li		t1, 0x00000083
	sw		t1, 0(t0)

	li		t0, MEM_STTIME1
	li		t1, 0x33030A10
	sw		t1, 0(t0)

	li		t0, MEM_STADDR1
	li		t1, 0x11803E40
	sw		t1, 0(t0)

	/* Set DSTRB bits so switch will read correctly */
	li		t1, 0xBE00000C
	lw		t2, 0(t1)
	or		t2, t2, 0x00000300
	sw		t2, 0(t1)

	/* Check switch setting */
	li		t1, 0xBE000014
	lw		t2, 0(t1)
	and		t2, t2, 0x00000100
	bne		t2, zero, big_endian
	nop

little_endian:

	/* Change Au1 core to little endian */
	li	t0, AU1500_SYS_ADDR
	li	t1, 1
	sw	t1, sys_endian(t0)
	mfc0	t2, CP0_CONFIG
	mtc0	t2, CP0_CONFIG
	nop
	nop

	/* Big Endian is default so nothing to do but fall through */

big_endian:

	/*
	 * Step 2) Establish Status Register
	 * (set BEV, clear ERL, clear EXL, clear IE)
	 */
	li	t1, 0x00400000
	mtc0	t1, CP0_STATUS

	/*
	 * Step 3) Establish CP0 Config0
	 * (set OD, set K0=3)
	 */
	li	t1, 0x00080003
	mtc0	t1, CP0_CONFIG

	/*
	 * Step 4) Disable Watchpoint facilities
	 */
	li t1, 0x00000000
	mtc0	t1, CP0_WATCHLO
	mtc0	t1, CP0_IWATCHLO
	/*
	 * Step 5) Disable the performance counters
	 */
	mtc0	zero, CP0_PERFORMANCE
	nop

	/*
	 * Step 6) Establish EJTAG Debug register
	 */
	mtc0	zero, CP0_DEBUG
	nop

	/*
	 * Step 7) Establish Cause
	 * (set IV bit)
	 */
	li	t1, 0x00800000
	mtc0	t1, CP0_CAUSE

	/* Establish Wired (and Random) */
	mtc0	zero, CP0_WIRED
	nop

	/* First setup pll:s to make serial work ok */
	/* We have a 12 MHz crystal */
	li	t0, SYS_CPUPLL
	li	t1, 0x21  /* 396 MHz */
	sw	t1, 0(t0)
	sync
	nop
	nop

	/* wait 1mS for clocks to settle */
	li	t1, MEM_1MS
1:	add	t1, -1
	bne	t1, zero, 1b
	nop
	/* Setup AUX PLL */
	li	t0, SYS_AUXPLL
	li	t1, 8 /* 96 MHz */
	sw	t1, 0(t0) /* aux pll */
	sync

	/*  Static memory controller */

	/* RCE0 8MB AMD29D323 Flash */
	li	t0, MEM_STCFG0
	li	t1, 0x00001403
	sw	t1, 0(t0)

	li	t0, MEM_STTIME0
	li	t1, 0xFFFFFFDD
	sw	t1, 0(t0)

	li	t0, MEM_STADDR0
	li	t1, 0x11F83FE0
	sw	t1, 0(t0)

	/* RCE1 CPLD Board Logic */
	li	t0, MEM_STCFG1
	li	t1, 0x00000083
	sw	t1, 0(t0)

	li	t0, MEM_STTIME1
	li	t1, 0x33030A10
	sw	t1, 0(t0)

	li	t0, MEM_STADDR1
	li	t1, 0x11803E40
	sw	t1, 0(t0)

	/* RCE2 CPLD Board Logic */
	li	t0, MEM_STCFG2
	li	t1, 0x00000004
	sw	t1, 0(t0)

	li	t0, MEM_STTIME2
	li	t1, 0x08061908
	sw	t1, 0(t0)

	li	t0, MEM_STADDR2
	li	t1, 0x12A03FC0
	sw	t1, 0(t0)

	/* RCE3 PCMCIA 250ns */
	li	t0, MEM_STCFG3
	li	t1, 0x00000002
	sw	t1, 0(t0)

	li	t0, MEM_STTIME3
	li	t1, 0x280E3E07
	sw	t1, 0(t0)

	li	t0, MEM_STADDR3
	li	t1, 0x10000000
	sw	t1, 0(t0)

	sync

	/* Set peripherals to a known state */
	li	t0, IC0_CFG0CLR
	li	t1, 0xFFFFFFFF
	sw	t1, 0(t0)

	li	t0, IC0_CFG0CLR
	sw	t1, 0(t0)

	li	t0, IC0_CFG1CLR
	sw	t1, 0(t0)

	li	t0, IC0_CFG2CLR
	sw	t1, 0(t0)

	li	t0, IC0_SRCSET
	sw	t1, 0(t0)

	li	t0, IC0_ASSIGNSET
	sw	t1, 0(t0)

	li	t0, IC0_WAKECLR
	sw	t1, 0(t0)

	li	t0, IC0_RISINGCLR
	sw	t1, 0(t0)

	li	t0, IC0_FALLINGCLR
	sw	t1, 0(t0)

	li	t0, IC0_TESTBIT
	li	t1, 0x00000000
	sw	t1, 0(t0)
	sync

	li	t0, IC1_CFG0CLR
	li	t1, 0xFFFFFFFF
	sw	t1, 0(t0)

	li	t0, IC1_CFG0CLR
	sw	t1, 0(t0)

	li	t0, IC1_CFG1CLR
	sw	t1, 0(t0)

	li	t0, IC1_CFG2CLR
	sw	t1, 0(t0)

	li	t0, IC1_SRCSET
	sw	t1, 0(t0)

	li	t0, IC1_ASSIGNSET
	sw	t1, 0(t0)

	li	t0, IC1_WAKECLR
	sw	t1, 0(t0)

	li	t0, IC1_RISINGCLR
	sw	t1, 0(t0)

	li	t0, IC1_FALLINGCLR
	sw	t1, 0(t0)

	li	t0, IC1_TESTBIT
	li	t1, 0x00000000
	sw	t1, 0(t0)
	sync

	li	t0, SYS_FREQCTRL0
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, SYS_FREQCTRL1
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, SYS_CLKSRC
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, SYS_PININPUTEN
	li	t1, 0x00000000
	sw	t1, 0(t0)
	sync

	li	t0, 0xB1100100
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, 0xB1400100
	li	t1, 0x00000000
	sw	t1, 0(t0)


	li	t0, SYS_WAKEMSK
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, SYS_WAKESRC
	li	t1, 0x00000000
	sw	t1, 0(t0)

	/* wait 1mS before setup */
	li	t1, MEM_1MS
1:	add	t1, -1
	bne	t1, zero, 1b
	nop

	/*
	 * Skip memory setup if we are running from memory
	 */
	li		t0, 0x90000000
	sub		t0, ra, t0
	bltz		t0, skip_memsetup
	nop

	/*
	 * SDCS0 - Not used, for SMROM
	 * SDCS1 - 32MB Micron 48LCBM16A2
	 * SDCS2 - 32MB Micron 48LCBM16A2
	 */
	li	t0, MEM_SDMODE0
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, MEM_SDMODE1
	li	t1, 0x00552229
	sw	t1, 0(t0)

	li	t0, MEM_SDMODE2
	li	t1, 0x00552229
	sw	t1, 0(t0)

	li	t0, MEM_SDADDR0
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, MEM_SDADDR1
	li	t1, 0x001003F8
	sw	t1, 0(t0)

	li	t0, MEM_SDADDR2
	li	t1, 0x001023F8
	sw	t1, 0(t0)

	sync

	li	t0, MEM_SDREFCFG
	li	t1, 0x74000c30 /* Disable */
	sw	t1, 0(t0)
	sync

	li	t0, MEM_SDPRECMD
	sw	zero, 0(t0)
	sync

	li	t0, MEM_SDAUTOREF
	sw	zero, 0(t0)
	sync
	sw	zero, 0(t0)
	sync

	li	t0, MEM_SDREFCFG
	li	t1, 0x76000c30 /* Enable */
	sw	t1, 0(t0)
	sync

	li	t0, MEM_SDWRMD0
	li	t1, 0x00000023
	sw	t1, 0(t0)
	sync

	li	t0, MEM_SDWRMD1
	li	t1, 0x00000023
	sw	t1, 0(t0)
	sync

	li	t0, MEM_SDWRMD2
	li	t1, 0x00000023
	sw	t1, 0(t0)
	sync

	/* wait 1mS after setup */
	li	t1, MEM_1MS
1:	add	t1, -1
	bne	t1, zero, 1b
	nop

skip_memsetup:

	li	t0, SYS_PINFUNC
	li	t1, 0/*0x00008080*/
	sw	t1, 0(t0)

	/*
	li	t0, SYS_TRIOUTCLR
	li	t1, 0x00001FFF
	sw	t1, 0(t0)

	li	t0, SYS_OUTPUTCLR
	li	t1, 0x00008000
	sw	t1, 0(t0)
	*/
	sync

	jr	ra
	nop
