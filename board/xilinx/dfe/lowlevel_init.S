
/*
 * This black box is a direct translation of init_ddrc.tcl. The Pele DDR controller
 * is initialized herein.
 * It needs lots of love and attention some day.
 */

#undef XPSS_DDR_CTRL_BASEADDR
/* ARCH SPEC 1.1 */
#define XPSS_DDR_CTRL_BASEADDR                  0xFE006000
/* ARCH SPEC 1.2 */
//#define XPSS_DDR_CTRL_BASEADDR		0xF8006000


.globl lowlevel_init
lowlevel_init:

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0)
	ldr r2, =0x200
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x4)
	ldr r2, =0x000C1061
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x14)
	ldr r2, =0x00040CD6
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x18)
	ldr r2, =0x049B48CD
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x1C)
	ldr r2, =0x80615884
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x20)
	ldr r2, =0x250842CB
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x28)
	ldr r2, =0x00809004
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x2C)
	ldr r2, =0x00000000
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x30)
	ldr r2, =0x00060B62
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x34)
	ldr r2, =0x00020022
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x50)
	ldr r2, =0x00000256
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x5C)
	ldr r2, =0x00002223
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0xA4)
	ldr r2, =0x10200800
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0xB8)
	ldr r2, =0x00200065
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x17C)
	ldr r2, =0x00000050
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x180)
	ldr r2, =0x00000050
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x184)
	ldr r2, =0x00000050
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x188)
	ldr r2, =0x00000050
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x200)
	ldr r2, =0x00000000
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x0)
	ldr r2, =0x201
	str r2, [r1]

	mov pc, lr
