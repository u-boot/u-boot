/* Memory sub-system initialization code */

#include <config.h>
#include <asm/regdef.h>
#include <asm/au1x00.h>
#include <asm/mipsregs.h>

#define CP0_Config0		$16
#define MEM_1MS			((CFG_MHZ) * 1000)
#define GPIO_RJ1LY     (1<<22)
#define GPIO_CFRESET   (1<<10)

	.text
	.set noreorder
	.set mips32

	.globl	lowlevel_init
lowlevel_init:
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

	/* No workaround if running from ram */
	lui	t0, 0xffc0
	lui	t3, 0xbfc0
	and	t1, ra, t0
	bne	t1, t3, noCacheJump
	nop

	/*** From AMD YAMON ***/
	/*
	 * Step 8) Initialize the caches
	 */
	li		t0, (16*1024)
	li		t1, 32
	li		t2, 0x80000000
	addu	t3, t0, t2
cacheloop:
	cache	0, 0(t2)
	cache	1, 0(t2)
	addu	t2, t1
	bne		t2, t3, cacheloop
	nop

	/* Save return address */
	move		t3, ra

	/* Run from cacheable space now */
	bal		cachehere
	nop
cachehere:
	li		t1, ~0x20000000 /* convert to KSEG0 */
	and		t0, ra, t1
	addi	t0, 5*4			/* 5 insns beyond cachehere */
	jr		t0
	nop

	/* Restore return address */
	move		ra, t3

	/*
	 * Step 9) Initialize the TLB
	 */
	li		t0, 0			# index value
	li		t1, 0x00000000		# entryhi value
	li		t2, 32			# 32 entries

tlbloop:
	/* Probe TLB for matching EntryHi */
	mtc0	t1, CP0_ENTRYHI
	tlbp
	nop

	/* Examine Index[P], 1=no matching entry */
	mfc0	t3, CP0_INDEX
	li	t4, 0x80000000
	and	t3, t4, t3
	addiu	t1, t1, 1		# increment t1 (asid)
	beq	zero, t3, tlbloop
	nop

	/* Initialize the TLB entry */
	mtc0	t0, CP0_INDEX
	mtc0	zero, CP0_ENTRYLO0
	mtc0	zero, CP0_ENTRYLO1
	mtc0	zero, CP0_PAGEMASK
	tlbwi

	/* Do it again */
	addiu	t0, t0, 1
	bne	t0, t2, tlbloop
	nop

	/* First setup pll:s to make serial work ok */
	/* We have a 12.5 MHz crystal */
	li	t0, SYS_CPUPLL
	li	t1, 0x28  /* CPU clock, 500 MHz */
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
	li	t1, 0
	sw	t1, 0(t0) /* aux pll */
	sync

	/*  Static memory controller */
	/* RCE0 - can not change while fetching, do so from icache */
	move		t2, ra /* Store return address */
	bal		getAddr
	nop

getAddr:
	move		t1, ra
	move		ra, t2 /* Move return addess back */

	cache	0x14,0(t1)
	cache	0x14,32(t1)
	/*** /From YAMON ***/

noCacheJump:

	/*  Static memory controller */

	/* RCE0 AMD 29LV800 Flash */
	li	t0, MEM_STCFG0
	li	t1, 0x00000243
	sw	t1, 0(t0)

	li	t0, MEM_STTIME0
	li	t1, 0x040181D7 /* FIXME */
	sw	t1, 0(t0)

	li	t0, MEM_STADDR0
	li	t1, 0x11E03F80
	sw	t1, 0(t0)

	/* RCE1 PCMCIA 250ns */
	li	t0, MEM_STCFG1
	li	t1, 0x00000002
	sw	t1, 0(t0)

	li	t0, MEM_STTIME1
	li	t1, 0x280E3E07
	sw	t1, 0(t0)

	li	t0, MEM_STADDR1
	li	t1, 0x10000000
	sw	t1, 0(t0)

	/* RCE2 CP Altera */
	li	t0, MEM_STCFG2
	li	t1, 0x00000280 /* BE, EW */
	sw	t1, 0(t0)

	li	t0, MEM_STTIME2
	li	t1, 0x0303000c
	sw	t1, 0(t0)

	li	t0, MEM_STADDR2
	li	t1, 0x10c03f80 /* 1 MB */
	sw	t1, 0(t0)

	/* RCE3 DP Altera */
	li	t0, MEM_STCFG3
	li	t1, 0x00000280 /* BE, EW */
	sw	t1, 0(t0)

	li	t0, MEM_STTIME3
	li	t1, 0x0303000c
	sw	t1, 0(t0)

	li	t0, MEM_STADDR3
	li	t1, 0x10e03f80 /* 1 MB */
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


/* SDCS 0 SDRAM */
	li	t0, MEM_SDMODE0
	li	t1, 0x592CD1
	sw	t1, 0(t0)

	li	t0, MEM_SDMODE1
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, MEM_SDMODE2
	li	t1, 0x00000000
	sw	t1, 0(t0)

/* 64 MB SDRAM at addr 0 */
	li	t0, MEM_SDADDR0
	li	t1, 0x001003F0
	sw	t1, 0(t0)


	li	t0, MEM_SDADDR1
	li	t1, 0x00000000
	sw	t1, 0(t0)

	li	t0, MEM_SDADDR2
	li	t1, 0x00000000
	sw	t1, 0(t0)

	sync

	li	t0, MEM_SDREFCFG
	li	t1, 0x880007A1 /* Disable */
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
	li	t1, 0x8A0007A1 /* Enable */
	sw	t1, 0(t0)
	sync

	li	t0, MEM_SDWRMD0
	li	t1, 0x00000023
	sw	t1, 0(t0)
	sync

	/* wait 1mS after setup */
	li	t1, MEM_1MS
1:	add	t1, -1
	bne	t1, zero, 1b
	nop

	/* Setup GPIO pins */

	li	t0, SYS_PINFUNC
	li	t1, 0x00007025 /* 0x8080 */
	sw	t1, 0(t0)

	li	t0, SYS_TRIOUTCLR
	li	t1, 0xFFFFFFFF /* 0x1FFF */
	sw	t1, 0(t0)

	/* Turn yellow front led on */
	/* Release reset on CF */
	li	t0, SYS_OUTPUTCLR
	li	t1, GPIO_RJ1LG
	sw	t1, 0(t0)
	li	t0, SYS_OUTPUTSET
	li	t1, GPIO_RJ1LY|GPIO_CFRESET
	sw	t1, 0(t0)
	sync
	j clearmem
	nop

#if 0
	.globl	memtest
#endif
memtest:
	/* Fill memory with address */
	li	t0, 0x80000000
	li	t1, 0xFFF000 /* 64 MB */
mt0:	sw	t0, 0(t0)
	add	t1, -1
	add	t0, 4
	bne	t1, zero, mt0
	nop
	nop
	/* Verify addr */
	li	t0, 0x80000000
	li	t1, 0xFFF000 /* 64 MB */
mt1:	lw	t2, 0(t0)
	bne	t0, t2, memhang
	add	t1, -1
	add	t0, 4
	bne	t1, zero, mt1
	nop
	nop
#if 0
	.globl	clearmem
#endif
clearmem:
		/* Clear memory */
	li	t0, 0x80000000
	li	t1, 0xFFF000 /* 64 MB */
mtc:	sw	zero, 0(t0)
	add	t1, -1
	add	t0, 4
	bne	t1, zero, mtc
	nop
	nop
memtestend:
	jr	ra
	nop

memhang:
	b	memhang
	nop
