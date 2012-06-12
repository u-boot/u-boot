
/*
 * This black box is a direct translation of init_ddrc.tcl.
 * The Zynq DDR controller is initialized herein.
 * It needs lots of love and attention some day.
 */

#include <asm/arch/xparameters.h>
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

#ifdef CONFIG_EP107
	# this should not be needed after EP107

	# Do nothing if DDR already running
	ldr r1, =(XPSS_DDR_CTRL_BASEADDR + 0)
	ldr r2, [r1]
	ldr r3, =0x201
	cmp r2, r3
	bne doit
#endif

#ifdef CONFIG_ZYNQ_DDR_533_INIT
   
    LDR     r0,   =0xf8000008   	/*DDR PLL Settings */ 
    LDR     r4,   = 0xdf0d 
    STR     r4,   [r0]  
    
    NOP					/* 11 cRYSTAL Clocks  */
    NOP 	
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP					/* 11 cRYSTAL Clocks  */
    NOP 	
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    


    LDR     r0,   =0xf8000124
  	
     	

    LDR     r0,   =0xf8000124     
    LDR     r4,   =0x18200003 
    STR     r4,   [r0]  
 
    NOP					/* 11 cRYSTAL Clocks  */
    NOP 	
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP					/* 11 cRYSTAL Clocks  */
    NOP 	
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP


    LDR     r0,   =0xf8000124

    LDR     r0,   =0xf8000104


    LDR     r0,   =0xf8000104      
    LDR     r4,   =0x20008 
    STR     r4,   [r0]  

 
    NOP					/* 11 cRYSTAL Clocks */
    NOP 	
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP					/* 11 cRYSTAL Clocks  */
    NOP 	
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP

    LDR     r0,   =0xf8000104

    LDR     r0,   =0xf8000128 
    LDR     r4,   =0x3500201 
    STR     r4,   [r0]  
    
    
    NOP					/* 11 cRYSTAL Clocks see if it locks ? */
    NOP 	
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
 
    
    LDR     r0,   = 0xf8000000  /*SLCR Base APB Address */ 
    LDR     r4,   = 0x00000600 
    STR     r4,   [r0,  #0xB40]  
    LDR     r5,   [r0,  #0xB40]  
     
      
    LDR     r4,   = 0x00000600 
    STR     r4,   [r0,  #0xB44]  
    LDR     r5,   [r0,  #0xB44]  
     
      
    LDR     r4,   = 0x00000672 
    STR     r4,   [r0,  #0xB48]  
    LDR     r5,   [r0,  #0xB48]  
     
      
    LDR     r4,   = 0x00000672 
    STR     r4,   [r0,  #0xB4C]  
    LDR     r5,   [r0,  #0xB4C]  
     
      
    LDR     r4,   = 0x00000674 
    STR     r4,   [r0,  #0xB50]  
    LDR     r5,   [r0,  #0xB50]  
     
      
    LDR     r4,   = 0x00000674 
    STR     r4,   [r0,  #0xB54]  
    LDR     r5,   [r0,  #0xB54]  
     
      
    LDR     r4,   = 0x00000600 
    STR     r4,   [r0,  #0xB58]  
    LDR     r5,   [r0,  #0xB58]  
     
      
    LDR     r4,   = 0x00D6861C 
    STR     r4,   [r0,  #0xB5C]  
    LDR     r5,   [r0,  #0xB5C]  
     
      
    LDR     r4,   = 0x00F9861C 
    STR     r4,   [r0,  #0xB60]  
    LDR     r5,   [r0,  #0xB60]  
     
      
    LDR     r4,   = 0x00F9861C 
    STR     r4,   [r0,  #0xB64]  
    LDR     r5,   [r0,  #0xB64]  
     
      
    LDR     r4,   = 0x00D6861C 
    STR     r4,   [r0,  #0xB68]  
    LDR     r5,   [r0,  #0xB68]  
     
      
    LDR     r4,   = 0x00000E09 
    STR     r4,   [r0,  #0xB6C]  
    LDR     r5,   [r0,  #0xB6C]  
     
      
    LDR     r4,   = 0x00000021 
    STR     r4,   [r0,  #0xB70]  
    LDR     r5,   [r0,  #0xB70]  
     
      
    LDR     r4,   = 0x00000020 
    STR     r4,   [r0,  #0xB70]  
    LDR     r5,   [r0,  #0xB70]  
     
      
    LDR     r4,   = 0x00000823 
    STR     r4,   [r0,  #0xB70]  
    LDR     r5,   [r0,  #0xB70]  
     
      
    LDR     r0,   = 0xf8006000  /*DRAM Base APB Address */ 
    LDR     r4,   = 0x00000080 
    STR     r4,   [r0,  #0x0]  
    LDR     r5,   [r0,  #0x0]  
     
      
    LDR     r4,   = 0x00081081 
    STR     r4,   [r0,  #0x4]  
    LDR     r5,   [r0,  #0x4]  
     
      
    LDR     r4,   = 0x03C0780F 
    STR     r4,   [r0,  #0x8]  
    LDR     r5,   [r0,  #0x8]  
     
      
    LDR     r4,   = 0x02001001 
    STR     r4,   [r0,  #0xC]  
    LDR     r5,   [r0,  #0xC]  
     
      
    LDR     r4,   = 0x00014001 
    STR     r4,   [r0,  #0x10]  
    LDR     r5,   [r0,  #0x10]  
     
      
    LDR     r4,   = 0x0004281A 
    STR     r4,   [r0,  #0x14]  
    LDR     r5,   [r0,  #0x14]  
     
      
    LDR     r4,   = 0x44E458D2 
    STR     r4,   [r0,  #0x18]  
    LDR     r5,   [r0,  #0x18]  
     
      
    LDR     r4,   = 0x720238E5 
    STR     r4,   [r0,  #0x1C]  
    LDR     r5,   [r0,  #0x1C]  
     
      
    LDR     r4,   = 0x272872D0 
    STR     r4,   [r0,  #0x20]  
    LDR     r5,   [r0,  #0x20]  
     
      
    LDR     r4,   = 0x0000003C 
    STR     r4,   [r0,  #0x24]  
    LDR     r5,   [r0,  #0x24]  
     
      
    LDR     r4,   = 0x00002007 
    STR     r4,   [r0,  #0x28]  
    LDR     r5,   [r0,  #0x28]  
     
      
    LDR     r4,   = 0x00000008 
    STR     r4,   [r0,  #0x2C]  
    LDR     r5,   [r0,  #0x2C]  
     
      
    LDR     r4,   = 0x00040930 
    STR     r4,   [r0,  #0x30]  
    LDR     r5,   [r0,  #0x30]  
     
      
    LDR     r4,   = 0x00010694 
    STR     r4,   [r0,  #0x34]  
    LDR     r5,   [r0,  #0x34]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0x38]  
    LDR     r5,   [r0,  #0x38]  
     
      
    LDR     r4,   = 0x00000777 
    STR     r4,   [r0,  #0x3C]  
    LDR     r5,   [r0,  #0x3C]  
     
      
    LDR     r4,   = 0xFFF00000 
    STR     r4,   [r0,  #0x40]  
    LDR     r5,   [r0,  #0x40]  
     
      
    LDR     r4,   = 0x0F666666 
    STR     r4,   [r0,  #0x44]  
    LDR     r5,   [r0,  #0x44]  
     
      
    LDR     r4,   = 0x0003C248 
    STR     r4,   [r0,  #0x48]  
    LDR     r5,   [r0,  #0x48]  
     
      
    LDR     r4,   = 0x77010800 
    STR     r4,   [r0,  #0x50]  
    LDR     r5,   [r0,  #0x50]  
     
      
    LDR     r4,   = 0x00000101 
    STR     r4,   [r0,  #0x58]  
    LDR     r5,   [r0,  #0x58]  
     
      
    LDR     r4,   = 0x00005003 
    STR     r4,   [r0,  #0x5C]  
    LDR     r5,   [r0,  #0x5C]  
     
      
    LDR     r4,   = 0x0000003E 
    STR     r4,   [r0,  #0x60]  
    LDR     r5,   [r0,  #0x60]  
     
      
    LDR     r4,   = 0x00020000 
    STR     r4,   [r0,  #0x64]  
    LDR     r5,   [r0,  #0x64]  
     
      
    LDR     r4,   = 0x00284141 
    STR     r4,   [r0,  #0x68]  
    LDR     r5,   [r0,  #0x68]  
     
      
    LDR     r4,   = 0x00001610 
    STR     r4,   [r0,  #0x6C]  
    LDR     r5,   [r0,  #0x6C]  
     
      
    LDR     r4,   = 0x00008000 
    STR     r4,   [r0,  #0xA0]  
    LDR     r5,   [r0,  #0xA0]  
     
      
    LDR     r4,   = 0x10200802 
    STR     r4,   [r0,  #0xA4]  
    LDR     r5,   [r0,  #0xA4]  
     
      
    LDR     r4,   = 0x0690CB73 
    STR     r4,   [r0,  #0xA8]  
    LDR     r5,   [r0,  #0xA8]  
     
      
    LDR     r4,   = 0x000001FE 
    STR     r4,   [r0,  #0xAC]  
    LDR     r5,   [r0,  #0xAC]  
     
      
    LDR     r4,   = 0x1CFFFFFF 
    STR     r4,   [r0,  #0xB0]  
    LDR     r5,   [r0,  #0xB0]  
     
      
    LDR     r4,   = 0x00000200 
    STR     r4,   [r0,  #0xB4]  
    LDR     r5,   [r0,  #0xB4]  
     
      
    LDR     r4,   = 0x00200066 
    STR     r4,   [r0,  #0xB8]  
    LDR     r5,   [r0,  #0xB8]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0xBC]  
    LDR     r5,   [r0,  #0xBC]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0xC4]  
    LDR     r5,   [r0,  #0xC4]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0xC8]  
    LDR     r5,   [r0,  #0xC8]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0xDC]  
    LDR     r5,   [r0,  #0xDC]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0xF0]  
    LDR     r5,   [r0,  #0xF0]  
     
      
    LDR     r4,   = 0x00000008 
    STR     r4,   [r0,  #0xF4]  
    LDR     r5,   [r0,  #0xF4]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0x114]  
    LDR     r5,   [r0,  #0x114]  
     
      
    LDR     r4,   = 0x40000001 
    STR     r4,   [r0,  #0x118]  
    LDR     r5,   [r0,  #0x118]  
     
      
    LDR     r4,   = 0x40000001 
    STR     r4,   [r0,  #0x11C]  
    LDR     r5,   [r0,  #0x11C]  
     
      
    LDR     r4,   = 0x40000001 
    STR     r4,   [r0,  #0x120]  
    LDR     r5,   [r0,  #0x120]  
     
      
    LDR     r4,   = 0x40000001 
    STR     r4,   [r0,  #0x124]  
    LDR     r5,   [r0,  #0x124]  
     
      
    LDR     r4,   = 0x00036022 
    STR     r4,   [r0,  #0x12C]  
    LDR     r5,   [r0,  #0x12C]  
     
      
    LDR     r4,   = 0x00038419 
    STR     r4,   [r0,  #0x130]  
    LDR     r5,   [r0,  #0x130]  
     
      
    LDR     r4,   = 0x00038818 
    STR     r4,   [r0,  #0x134]  
    LDR     r5,   [r0,  #0x134]  
     
      
    LDR     r4,   = 0x00036C1F 
    STR     r4,   [r0,  #0x138]  
    LDR     r5,   [r0,  #0x138]  
     
      
    LDR     r4,   = 0x00000035 
    STR     r4,   [r0,  #0x140]  
    LDR     r5,   [r0,  #0x140]  
     
      
    LDR     r4,   = 0x00000035 
    STR     r4,   [r0,  #0x144]  
    LDR     r5,   [r0,  #0x144]  
     
      
    LDR     r4,   = 0x00000035 
    STR     r4,   [r0,  #0x148]  
    LDR     r5,   [r0,  #0x148]  
     
      
    LDR     r4,   = 0x00000035 
    STR     r4,   [r0,  #0x14C]  
    LDR     r5,   [r0,  #0x14C]  
     
      
    LDR     r4,   = 0x00000022 
    STR     r4,   [r0,  #0x154]  
    LDR     r5,   [r0,  #0x154]  
     
      
    LDR     r4,   = 0x00000019 
    STR     r4,   [r0,  #0x158]  
    LDR     r5,   [r0,  #0x158]  
     
      
    LDR     r4,   = 0x00000018 
    STR     r4,   [r0,  #0x15C]  
    LDR     r5,   [r0,  #0x15C]  
     
      
    LDR     r4,   = 0x0000001F 
    STR     r4,   [r0,  #0x160]  
    LDR     r5,   [r0,  #0x160]  
     
      
    LDR     r4,   = 0x0000012D 
    STR     r4,   [r0,  #0x168]  
    LDR     r5,   [r0,  #0x168]  
     
      
    LDR     r4,   = 0x00000136 
    STR     r4,   [r0,  #0x16C]  
    LDR     r5,   [r0,  #0x16C]  
     
      
    LDR     r4,   = 0x00000137 
    STR     r4,   [r0,  #0x170]  
    LDR     r5,   [r0,  #0x170]  
     
      
    LDR     r4,   = 0x00000130 
    STR     r4,   [r0,  #0x174]  
    LDR     r5,   [r0,  #0x174]  
     
      
    LDR     r4,   = 0x00000062 
    STR     r4,   [r0,  #0x17C]  
    LDR     r5,   [r0,  #0x17C]  
     
      
    LDR     r4,   = 0x00000059 
    STR     r4,   [r0,  #0x180]  
    LDR     r5,   [r0,  #0x180]  
     
      
    LDR     r4,   = 0x00000058 
    STR     r4,   [r0,  #0x184]  
    LDR     r5,   [r0,  #0x184]  
     
      
    LDR     r4,   = 0x0000005F 
    STR     r4,   [r0,  #0x188]  
    LDR     r5,   [r0,  #0x188]  
     
      
    LDR     r4,   = 0x10040080 
    STR     r4,   [r0,  #0x190]  
    LDR     r5,   [r0,  #0x190]  
     
      
    LDR     r4,   = 0x0001FC82 
    STR     r4,   [r0,  #0x194]  
    LDR     r5,   [r0,  #0x194]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0x204]  
    LDR     r5,   [r0,  #0x204]  
     
      
    LDR     r4,   = 0x000803FF 
    STR     r4,   [r0,  #0x208]  
    LDR     r5,   [r0,  #0x208]  
     
      
    LDR     r4,   = 0x000803FF 
    STR     r4,   [r0,  #0x20C]  
    LDR     r5,   [r0,  #0x20C]  
     
      
    LDR     r4,   = 0x000803FF 
    STR     r4,   [r0,  #0x210]  
    LDR     r5,   [r0,  #0x210]  
     
      
    LDR     r4,   = 0x000803FF 
    STR     r4,   [r0,  #0x214]  
    LDR     r5,   [r0,  #0x214]  
     
      
    LDR     r4,   = 0x000003FF 
    STR     r4,   [r0,  #0x218]  
    LDR     r5,   [r0,  #0x218]  
     
      
    LDR     r4,   = 0x000003FF 
    STR     r4,   [r0,  #0x21C]  
    LDR     r5,   [r0,  #0x21C]  
     
      
    LDR     r4,   = 0x000003FF 
    STR     r4,   [r0,  #0x220]  
    LDR     r5,   [r0,  #0x220]  
     
      
    LDR     r4,   = 0x000003FF 
    STR     r4,   [r0,  #0x224]  
    LDR     r5,   [r0,  #0x224]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0x2A8]  
    LDR     r5,   [r0,  #0x2A8]  
     
      
    LDR     r4,   = 0x00000000 
    STR     r4,   [r0,  #0x2AC]  
    LDR     r5,   [r0,  #0x2AC]  
     
      
    LDR     r4,   = 0x00005125 
    STR     r4,   [r0,  #0x2B0]  
    LDR     r5,   [r0,  #0x2B0]  
     
      
    LDR     r4,   = 0x000012A8 
    STR     r4,   [r0,  #0x2B4]  
    LDR     r5,   [r0,  #0x2B4]  
     
      
    LDR     r4,   = 0x00000081 
    STR     r4,   [r0,  #0x0]  
    LDR     r5,   [r0,  #0x0]  
     
#endif  
	
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

