
/*
 * This black box is a direct translation of init_ddrc.tcl.
 * The Zynq DDR controller is initialized herein.
 * It needs lots of love and attention some day.
 */

#include <config.h>

.globl lowlevel_init
lowlevel_init:

	# unlock SLCR
	ldr r1, =(XPSS_SYS_CTRL_BASEADDR + 8)
	ldr r2, =0xDF0D
	str r2, [r1]

	# remap DDR to zero
	# FILTERSTART
	ldr r1, =(XPSS_SCU_BASEADDR + 0x40)
	ldr r2, =0
	str r2, [r1]

	# Device config APB
	# unlock the PCAP
	ldr r1, =(XPSS_DEV_CFG_APB_BASEADDR + 0x34)
	ldr r2, =0x757BDF0D
	str r2, [r1]
	ldr r1, =(XPSS_DEV_CFG_APB_BASEADDR + 0x28)
	ldr r2, =0xFFFFFFFF
	str r2, [r1]

	# OCM_CFG
	# Mask out the ROM
	# map ram into upper addresses
	ldr r1, =(XPSS_SYS_CTRL_BASEADDR +0x910)
	ldr r2, =0x1F
	str r2, [r1]

	# FPGA_RST_CTRL
	# clear resets on AXI fabric ports
	ldr r1, =(XPSS_SYS_CTRL_BASEADDR + 0x240)
	ldr r2, =0x0
	str r2, [r1]

	# TZ_DDR_RAM
	# Set DDR trust zone non-secure
	ldr r1, =(XPSS_SYS_CTRL_BASEADDR + 0x430)
	ldr r2, =0xFFFFFFFF
	str r2, [r1]

	# set urgent bits with register
	ldr r1, =(XPSS_SYS_CTRL_BASEADDR + 0x61C)
	ldr r2, =0
	str r2, [r1]

	# urgent write, ports S2/S3
	ldr r1, =(XPSS_SYS_CTRL_BASEADDR + 0x600)
	ldr r2, =0xC
	str r2, [r1]

	# relock SLCR
	ldr r1, =(XPSS_SYS_CTRL_BASEADDR + 0x4)
	ldr r2, =0x767B
	str r2, [r1]

	mov pc, lr

doit:
	# Reset DDR controller
	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0)
	ldr r2, =0x200
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x4)
	ldr r2, =0x000C1061
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0xC)
	ldr r2, =0x03001001
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x10)
	ldr r2, =0x00014001
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x14)
	ldr r2, =0x0004e020
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x18)
	ldr r2, =0x36264ccf
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x1C)
	ldr r2, =0x820158a4
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x20)
	ldr r2, =0x250882c4
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x28)
	ldr r2, =0x00809004
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x2C)
	ldr r2, =0x00000000
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x30)
	ldr r2, =0x00040952
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x34)
	ldr r2, =0x00020022
	str r2, [r1]

#if (XPAR_MEMORY_MB_SIZE == 256)
/*
 * starting with PEEP8 designs, there is 256 MB
 */
	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x3C)
	ldr r2, =0x00000F88
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x40)
	ldr r2, =0xFF000000
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x44)
	ldr r2, =0x0FF66666
	str r2, [r1]
#endif

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x50)
	ldr r2, =0x00000256
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x5C)
	ldr r2, =0x00002223
	str r2, [r1]

	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0x64)
	ldr r2, =0x00020FE0
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

# Delay spin loop
	ldr r4, =0x1000000
loop:
	sub r4, r4, #1
	cmp r4, #0
	bne loop

	mov pc, lr

