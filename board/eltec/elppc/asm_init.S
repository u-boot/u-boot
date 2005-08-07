/*
 * (C) Copyright 2001 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * ELTEC ELPPC RAM initialization
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/processor.h>
#include <version.h>
#include <mpc106.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

.globl board_asm_init
board_asm_init:

/*
 * setup pointer to message block
 */
    mflr    r13                 /* save away link register */
    bl      get_lnk_reg         /* r3=addr of next instruction */
    subi    r4, r3, 8           /* r4=board_asm_init addr */
    addi    r29, r4, (MessageBlock-board_asm_init)

/*
 * dcache_disable
 */
    mfspr   r3, HID0
    li      r4, HID0_DCE
    andc    r3, r3, r4
    mr      r2, r3
    ori     r3, r3, HID0_DCI
    sync
    mtspr   HID0, r3
    mtspr   HID0, r2
    isync
    sync
/*
 * icache_disable
 */
    mfspr   r3, HID0
    li      r4, 0
    ori     r4, r4, HID0_ICE
    andc    r3, r3, r4
    sync
    mtspr   HID0, r3
/*
 * invalidate caches
 */
    ori     r3, r3, (HID0_ICE | HID0_ICFI | HID0_DCI | HID0_DCE)
    or      r4, r4, r3
    isync
    mtspr   HID0, r4
    andc    r4, r4, r3
    isync
    mtspr   HID0, r4
    isync
/*
 * icache_enable
 */
    mfspr   r3, HID0
    ori     r3, r3, (HID0_ICE | HID0_ICFI)
    sync
    mtspr   HID0, r3


/*
 * setup memory controller
 */
    lis     r1, MPC106_REG_ADDR@h
    ori     r1, r1, MPC106_REG_ADDR@l
    lis     r2, MPC106_REG_DATA@h
    ori     r2, r2, MPC106_REG_DATA@l

    /* Configure PICR1 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, PCI_PICR1
    stwbrx  r3, 0, r1
    addis   r3, r0, 0xFF14
    ori     r3, r3, 0x1CC8
    eieio
    stwbrx  r3, 0, r2

    /* Configure PICR2 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, PCI_PICR2
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0000
    ori     r3, r3, 0x0000
    eieio
    stwbrx  r3, 0, r2

    /* Configure EUMBAR */
    lis     r3, MPC106_REG@h
    ori     r3, r3, 0x0078      /* offest of EUMBAR in PCI config space */
    stwbrx  r3, 0, r1
    lis     r3, MPC107_EUMB_ADDR@h
    eieio
    stwbrx  r3, 0, r2

    /* Configure Address Map B Option Reg */
    lis     r3, MPC106_REG@h
    ori     r3, r3, 0x00e0      /* offest of AMBOR in PCI config space */
    stwbrx  r3, 0, r1
    lis     r3, 0
    eieio
    stwbrx  r3, 0, r2

    /* Configure I2C Controller */
    lis     r14, MPC107_I2C_ADDR@h  /* base of I2C controller */
    ori     r14, r14, MPC107_I2C_ADDR@l
    lis     r3, 0x2b10          /* I2C clock = 100MHz/1024 */
    stw     r3, 4(r14)
    li      r3, 0               /* clear arbitration */
    eieio
    stw     r3, 12(r14)

    /* Configure MCCR1 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, MPC106_MCCR1
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0660      /* don't set MEMGO now ! */
    ori     r3, r3, 0x0000
    eieio
    stwbrx  r3, 0, r2

    /* Configure MCCR2 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, MPC106_MCCR2
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0400
    ori     r3, r3, 0x1800
    eieio
    stwbrx  r3, 0, r2


    /* Configure MCCR3 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, MPC106_MCCR3
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0230
    ori     r3, r3, 0x0000
    eieio
    stwbrx  r3, 0, r2

    /* Configure MCCR4 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, MPC106_MCCR4
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x2532
    ori     r3, r3, 0x2220
    eieio
    stwbrx  r3, 0, r2

/*
 * configure memory interface (MICRs)
 */
    addis   r3, r0, 0x8000      /* ADDR_80 */
    ori     r3, r3, 0x0080      /* SMEMADD1 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0xFFFF
    ori     r3, r3, 0x4000
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_84 */
    ori     r3, r3, 0x0084      /* SMEMADD2 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0xFFFF
    ori     r3, r3, 0xFFFF
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_88 */
    ori     r3, r3, 0x0088      /* EXTSMEM1 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0303
    ori     r3, r3, 0x0000
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_8C */
    ori     r3, r3, 0x008c      /* EXTSMEM2 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0303
    ori     r3, r3, 0x0303
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_90 */
    ori     r3, r3, 0x0090      /* EMEMADD1 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0xFFFF
    ori     r3, r3, 0x7F3F
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_94 */
    ori     r3, r3, 0x0094      /* EMEMADD2 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0xFFFF
    ori     r3, r3, 0xFFFF
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_98 */
    ori     r3, r3, 0x0098      /* EXTEMEM1 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0303
    ori     r3, r3, 0x0000
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_9C */
    ori     r3, r3, 0x009c      /* EXTEMEM2 */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0303
    ori     r3, r3, 0x0303
    eieio
    stwbrx  r3, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_A0 */
    ori     r3, r3, 0x00a0      /* MEMBNKEN */
    stwbrx  r3, 0, r1
    addis   r3, r0, 0x0000
    ori     r3, r3, 0x0003
    eieio
    stwbrx  r3, 0, r2

/*
 * must wait at least 100us after HRESET to issue a MEMGO
 */
    lis     r0, 1
    mtctr   r0
memStartWait:
    bdnz    memStartWait

/*
 * enable RAM Operations through MCCR1 (MEMGO)
 */
    lis     r3, 0x8000
    ori     r3, r3, 0x00f0
    stwbrx  r3, r0, r1
    sync
    lwbrx   r3, 0, r2
    lis     r0, 0x0008
    or      r3, r0, r3
    stwbrx  r3, 0, r2
    sync

/*
 * set LEDs first time
 */
    li      r3, 0x1
    lis     r30, CFG_USR_LED_BASE@h
    stb     r3, 2(r30)
    sync

/*
 * init COM1 for polled output
 */
    lis     r8, CFG_NS16550_COM1@h  /* COM1 base address*/
    ori     r8, r8, CFG_NS16550_COM1@l
    li      r9, 0x00
    stb     r9, 1(r8)           /* int disabled */
    eieio
    li      r9, 0x00
    stb     r9, 4(r8)           /* modem ctrl */
    eieio
    li      r9, 0x80
    stb     r9, 3(r8)           /* link ctrl */
    eieio
    li      r9, (CFG_NS16550_CLK / 16 / CONFIG_BAUDRATE)
    stb     r9, 0(r8)           /* baud rate (LSB)*/
    eieio
    li      r9, ((CFG_NS16550_CLK / 16 / CONFIG_BAUDRATE) >> 8)
    stb     r9, 1(r8)           /* baud rate (MSB) */
    eieio
    li      r9, 0x07
    stb     r9, 3(r8)           /* 8 data bits, 2 stop bit, no parity */
    eieio
    li      r9, 0x0b
    stb     r9, 4(r8)           /* enable the receiver and transmitter (modem ctrl) */
    eieio
waitEmpty:
    lbz     r9, 5(r8)           /* transmit empty */
    andi.   r9, r9, 0x40
    beq     waitEmpty
    li      r9, 0x47
    stb     r9, 3(r8)           /* send break, 8 data bits, 2 stop bit, no parity */
    eieio

    lis     r0, 0x0001
    mtctr   r0
waitCOM1:
    lwz     r0, 5(r8)           /* load from port for delay */
    bdnz    waitCOM1

waitEmpty1:
    lbz     r9, 5(r8)           /* transmit empty */
    andi.   r9, r9, 0x40
    beq     waitEmpty1
    li      r9, 0x07
    stb     r9, 3(r8)           /* 8 data bits, 2 stop bit, no parity */
    eieio

/*
 * intro message from message block
 */
    addi    r3, r29, (MnewLine-MessageBlock)
    bl      Printf
    addi    r3, r29, (MinitLogo-MessageBlock)
    bl      Printf

/*
 * memory cofiguration using SPD information stored on the SODIMMs
 */
    addi    r3, r29, (Mspd01-MessageBlock)
    bl      Printf

    li      r17, 0

    li      r3, 0x0002          /* get RAM type from spd for bank0/1 */
    bl      spdRead

    cmpi    0, 0, r3, -1        /* error ? */
    bne     noSpdError

    addi    r3, r29, (Mfail-MessageBlock)
    bl      Printf

    li      r6, 0xe             /* error codes in r6 and r7  */
    li      r7, 0x0
    b       toggleError         /* fail - loop forever */

noSpdError:
    mr      r15, r3             /* save r3 */

    addi    r3, r29, (Mok-MessageBlock)
    bl      Printf

    cmpli   0, 0, r15, 0x0004   /* SDRAM ? */
    beq     isSDRAM

    addi    r3, r29, (MramTyp-MessageBlock)
    bl      Printf

    li      r6, 0xd             /* error codes in r6 and r7  */
    li      r7, 0x0
    b       toggleError         /* fail - loop forever */

isSDRAM:
    li      r3, 0x0012          /* get supported CAS latencies from byte 18 */
    bl      spdRead
    mr      r15, r3
    li      r3, 0x09
    andi.   r0, r15, 0x04
    bne     maxCLis3
    li      r3, 0x17
maxCLis3:
    andi.   r0, r15, 0x02
    bne     CL2

    addi    r3, r29, (MramTyp-MessageBlock)
    bl      Printf

    li      r6, 0xc             /* error codes in r6 and r7  */
    li      r7, 0x0
    b       toggleError         /* fail - loop forever */
CL2:
    bl      spdRead
    cmpli   0, 0, r3, 0xa1      /* cycle time must be 10ns max. */
    blt     speedOk

    addi    r3, r29, (MramTyp-MessageBlock)
    bl      Printf

    li      r6, 0xb             /* error codes in r6 and r7  */
    li      r7, 0x0
    b       toggleError         /* fail - loop forever */
speedOk:
    lis     r20, 0x06e8         /* preset MCR1 value */

    li      r3, 0x0011          /* get number of internal banks from spd for bank0/1 */
    bl      spdRead

    cmpli   0, 0, r3, 0x02
    beq     SD_2B
    cmpli   0, 0, r3, 0x04
    beq     SD_4B
memConfErr:
    addi    r3, r29, (MramConfErr-MessageBlock)
    bl      Printf

    li      r6, 0xa             /* error codes in r6 and r7  */
    li      r7, 0x0
    b       toggleError         /* fail - loop forever */

SD_2B:
    li      r3, 0x0003          /* get number of row bits from spd for bank0/1 */
    bl      spdRead
    cmpli   0, 0, r3, 0x0b
    beq     row11x2
    cmpli   0, 0, r3, 0x0c
    beq     row12x2or13x2
    cmpli   0, 0, r3, 0x0d
    beq     row12x2or13x2
    b       memConfErr
SD_4B:
    li      r3, 0x0003          /* get number of row bits from spd for bank0/1 */
    bl      spdRead
    cmpli   0, 0, r3, 0x0b
    beq     row11x4or12x4
    cmpli   0, 0, r3, 0x0c
    beq     row11x4or12x4
    cmpli   0, 0, r3, 0x0d
    beq     row13x4
    b       memConfErr
row12x2or13x2:
    ori     r20, r20, 0x05
    b       row11x4or12x4
row13x4:
    ori     r20, r20, 0x0a
    b       row11x4or12x4
row11x2:
    ori     r20, r20, 0x0f
row11x4or12x4:
    /* get the size of bank 0-1 */

    li      r3, 0x001f          /* get bank size from spd for bank0/1 */
    bl      spdRead

    rlwinm  r16, r3, 2, 24, 29  /* calculate size in MByte (128 MB max.) */

    li      r3, 0x0005          /* get number of banks from spd for bank0/1 */
    bl      spdRead

    cmpi    0, 0, r3, 2         /* 2 banks ? */
    bne     SDRAMnobank1

    mr      r17, r16

SDRAMnobank1:
    li      r3, 0x000c          /* get refresh from spd for bank0/1 */
    bl      spdRead
    andi.   r3, r3, 0x007f      /* mask selfrefresh bit */
    li      r4, 0x1800          /* refesh cycle 1536 clocks left shifted 2 */
    cmpli   0, 0, r3, 0x0000    /* 15.6 us ? */
    beq     writeRefresh

    li      r4, 0x0c00          /* refesh cycle 768 clocks left shifted 2 */
    cmpli   0, 0, r3, 0x0002    /* 7.8 us ? */
    beq     writeRefresh

    li      r4, 0x3000          /* refesh cycle 3072 clocks left shifted 2 */
    cmpli   0, 0, r3, 0x0003    /* 31.3 us ? */
    beq     writeRefresh

    li      r4, 0x6000          /* refesh cycle 6144 clocks left shifted 2 */
    cmpli   0, 0, r3, 0x0004    /* 62.5 us ? */
    beq     writeRefresh

    li      r4, 0
    ori     r4, r4, 0xc000      /* refesh cycle 8224 clocks left shifted 2 */
    cmpli   0, 0, r3, 0x0005    /* 125 us ? */
    beq     writeRefresh

    b       memConfErr

writeRefresh:
    lis     r21, 0x0400         /* preset MCCR2 value */
    or      r21, r21, r4

    /* Overwrite MCCR1 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, MPC106_MCCR1
    stwbrx  r3, 0, r1
    eieio
    stwbrx  r20, 0, r2

    /* Overwrite MCCR2 */
    lis     r3, MPC106_REG@h
    ori     r3, r3, MPC106_MCCR2
    stwbrx  r3, 0, r1
    eieio
    stwbrx  r21, 0, r2

    /* set the memory boundary registers for bank 0-3 */
    li      r20, 0
    lis     r23, 0x0303
    lis     r24, 0x0303
    subi    r21, r16, 1         /* calculate end address bank0 */
    li      r22, 1

    cmpi    0, 0, r17, 0        /* bank1 present ? */
    beq     nobank1

    andi.   r3, r16, 0x00ff     /* calculate start address of bank1 */
    andi.   r4, r16, 0x0300
    rlwinm  r3, r3, 8, 16, 23
    or      r20, r20, r3
    or      r23, r23, r4

    add     r16, r16, r17       /* add to total memory size */

    subi    r3, r16, 1          /* calculate end address of bank1 */
    andi.   r4, r3, 0x0300
    andi.   r3, r3, 0x00ff
    rlwinm  r3, r3, 8, 16, 23
    or      r21, r21, r3
    or      r24, r24, r4

    ori     r22, r22, 2         /* enable bank1 */
    b       bankOk
nobank1:
    ori     r23, r23, 0x0300    /* set bank1 start to unused area */
    ori     r24, r24, 0x0300    /* set bank1 end to unused area */
bankOk:
    addi    r3, r29, (Mactivate-MessageBlock)
    bl      Printf
    mr      r3, r16
    bl      OutDec
    addi    r3, r29, (Mact0123e-MessageBlock)
    bl      Printf

/*
 * overwrite MSAR1, MEAR1, EMSAR1, and EMEAR1
 */
    addis   r3, r0, 0x8000      /* ADDR_80 */
    ori     r3, r3, 0x0080      /* MSAR1 */
    stwbrx  r3, 0, r1
    eieio
    stwbrx  r20, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_88 */
    ori     r3, r3, 0x0088      /* EMSAR1 */
    stwbrx  r3, 0, r1
    eieio
    stwbrx  r23, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_90 */
    ori     r3, r3, 0x0090      /* MEAR1 */
    stwbrx  r3, 0, r1
    eieio
    stwbrx  r21, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_98 */
    ori     r3, r3, 0x0098      /* EMEAR1 */
    stwbrx  r3, 0, r1
    eieio
    stwbrx  r24, 0, r2

    addis   r3, r0, 0x8000      /* ADDR_A0 */
    ori     r3, r3, 0x00a0      /* MBER */
    stwbrx  r3, 0, r1
    eieio
    stwbrx  r22, 0, r2

/*
 * delay to let SDRAM go through several initialization/refresh cycles
 */
    lis     r3, 3
    mtctr   r3
memStartWait_1:
    bdnz    memStartWait_1
    eieio

/*
 * set LEDs end
 */
    li      r3, 0xf
    lis     r30, CFG_USR_LED_BASE@h
    stb     r3, 2(r30)
    sync

    mtlr    r13
    blr                         /* EXIT board_asm_init ... */

/*----------------------------------------------------------------------------*/
/*
 * print a message to COM1 in polling mode (r10=COM1 port, r3=(char*)string)
 */

Printf:
    lis     r10, CFG_NS16550_COM1@h /* COM1 base address*/
    ori     r10, r10, CFG_NS16550_COM1@l
WaitChr:
    lbz     r0, 5(r10)          /* read link status */
    eieio
    andi.   r0, r0, 0x40        /* mask transmitter empty bit */
    beq     cr0, WaitChr        /* wait till empty */
    lbzx    r0, r0, r3          /* get char */
    stb     r0, 0(r10)          /* write to transmit reg */
    eieio
    addi    r3, r3, 1           /* next char */
    lbzx    r0, r0, r3          /* get char */
    cmpwi   cr1, r0, 0          /* end of string ? */
    bne     cr1, WaitChr
    blr

/*
 * print a char to COM1 in polling mode (r10=COM1 port, r3=char)
 */
OutChr:
    lis     r10, CFG_NS16550_COM1@h /* COM1 base address*/
    ori     r10, r10, CFG_NS16550_COM1@l
OutChr1:
    lbz     r0, 5(r10)          /* read link status */
    eieio
    andi.   r0, r0, 0x40        /* mask transmitter empty bit */
    beq     cr0, OutChr1        /* wait till empty */
    stb     r3, 0(r10)          /* write to transmit reg */
    eieio
    blr

/*
 * print 8/4/2 digits hex value to COM1 in polling mode (r10=COM1 port, r3=val)
 */
OutHex2:
    li      r9, 4               /* shift reg for 2 digits */
    b       OHstart
OutHex4:
    li      r9, 12              /* shift reg for 4 digits */
    b       OHstart
OutHex:
    li      r9, 28              /* shift reg for 8 digits */
OHstart:
    lis     r10, CFG_NS16550_COM1@h /* COM1 base address*/
    ori     r10, r10, CFG_NS16550_COM1@l
OutDig:
    lbz     r0, 0(r29)          /* slow down dummy read */
    lbz     r0, 5(r10)          /* read link status */
    eieio
    andi.   r0, r0, 0x40        /* mask transmitter empty bit */
    beq     cr0, OutDig
    sraw    r0, r3, r9
    clrlwi  r0, r0, 28
    cmpwi   cr1, r0, 9
    ble     cr1, digIsNum
    addic   r0, r0, 55
    b       nextDig
digIsNum:
    addic   r0, r0, 48
nextDig:
    stb     r0, 0(r10)          /* write to transmit reg */
    eieio
    addic.  r9, r9, -4
    bge     OutDig
    blr

/*
 * print 3 digits hdec value to COM1 in polling mode
 * (r10=COM1 port, r3=val, r7=x00, r8=x0, r9=x, r0, r6=scratch)
 */
OutDec:
    li      r6, 10
    divwu   r0, r3, r6          /* r0 = r3 / 10, r9 = r3 mod 10 */
    mullw   r10, r0, r6
    subf    r9, r10, r3
    mr      r3, r0
    divwu   r0, r3, r6          /* r0 = r3 / 10, r8 = r3 mod 10 */
    mullw   r10, r0, r6
    subf    r8, r10, r3
    mr      r3, r0
    divwu   r0, r3, r6          /* r0 = r3 / 10, r7 = r3 mod 10 */
    mullw   r10, r0, r6
    subf    r7, r10, r3
    lis     r10, CFG_NS16550_COM1@h /* COM1 base address*/
    ori     r10, r10, CFG_NS16550_COM1@l
    or.     r7, r7, r7
    bne     noblank1
    li      r3, 0x20
    b       OutDec4
noblank1:
    addi    r3, r7, 48          /* convert to ASCII */
OutDec4:
    lbz     r0, 0(r29)          /* slow down dummy read */
    lbz     r0, 5(r10)          /* read link status */
    eieio
    andi.   r0, r0, 0x40        /* mask transmitter empty bit */
    beq     cr0, OutDec4
    stb     r3, 0(r10)          /* x00 to transmit */
    eieio
    or.     r7, r7, r8
    beq     OutDec5
    addi    r3, r8, 48          /* convert to ASCII */
OutDec5:
    lbz     r0, 0(r29)          /* slow down dummy read */
    lbz     r0, 5(r10)          /* read link status */
    eieio
    andi.   r0, r0, 0x40        /* mask transmitter empty bit */
    beq     cr0, OutDec5
    stb     r3, 0(r10)          /* x0  to transmit */
    eieio
    addi    r3, r9, 48          /* convert to ASCII */
OutDec6:
    lbz     r0, 0(r29)          /* slow down dummy read */
    lbz     r0, 5(r10)          /* read link status */
    eieio
    andi.   r0, r0, 0x40        /* mask transmitter empty bit */
    beq     cr0, OutDec6
    stb     r3, 0(r10)          /* x   to transmit */
    eieio
    blr

/*
 * hang endless loop
 */
toggleError:                    /* fail type in r6, r7=0xff, toggle LEDs */
    stb     r7, 2(r30)          /* r7 to LED */
    li      r0, 0
    lis     r9, 127
    ori     r9, r9, 65535
toggleError1:
    addic   r0, r0, 1
    cmpw    cr1, r0, r9
    ble     cr1, toggleError1
    stb     r6, 2(r30)          /* r6 to LED */
    li      r0, 0
    lis     r9, 127
    ori     r9, r9, 65535
toggleError2:
    addic   r0, r0, 1
    cmpw    cr1, r0, r9
    ble     cr1, toggleError2
    b       toggleError

/*
 * routines to read from ram spd
 */
spdWaitIdle:
    lis     r0, 0x1             /* timeout for about 100us */
    mtctr   r0
iSpd:
    lbz     r10, 12(r14)
    andi.   r10, r10, 0x20      /* mask and test MBB */
    beq     idle
    bdnz    iSpd
    orc.    r10, r0, r0         /* return -1 to caller */
idle:
    bclr    20, 0               /* return to caller */

waitSpd:
    lis     r0, 0x10            /* timeout for about 1.5ms */
    mtctr   r0
wSpd:
    lbz     r10, 12(r14)
    andi.   r10, r10, 0x82
    cmpli   0, 0, r10, 0x82     /* test MCF and MIF set */
    beq     wend
    bdnz    wSpd
    orc.    r10, r0, r0         /* return -1 to caller */
    bclr    20, 0               /* return to caller */

wend:
    li      r10, 0
    stb     r10, 12(r14)        /* clear status */
    bclr    20, 0               /* return to caller */

/*
 * spdread
 * in:  r3 adr to read
 * out: r3 val or -1 for error
 * uses r10, assumes that r14 points to I2C controller
 */
spdRead:
    mfspr   r25, 8              /* save link register */

    bl      spdWaitIdle
    bne     spdErr

    li      r10, 0x80           /* start with MEN */
    stb     r10, 8(r14)
    eieio

    li      r10, 0xb0           /* start as master */
    stb     r10, 8(r14)
    eieio

    li      r10, 0xa0           /* write device 0xA0 */
    stb     r10, 16(r14)
    eieio
    bl      waitSpd
    bne     spdErr

    lbz     r10, 12(r14)        /* test ACK */
    andi.   r10, r10, 0x01
    bne     gotNoAck

    stb     r3, 16(r14)         /* data address */
    eieio
    bl      waitSpd
    bne     spdErr


    li      r10, 0xb4           /* switch to read - restart */
    stb     r10, 8(r14)
    eieio

    li      r10, 0xa1           /* read device 0xA0 */
    stb     r10, 16(r14)
    eieio
    bl      waitSpd
    bne     spdErr

    li      r10, 0xa8           /* no ACK */
    stb     r10, 8(r14)
    eieio

    lbz     r10, 16(r14)        /* trigger read next byte */
    eieio
    bl      waitSpd
    bne     spdErr

    li      r10, 0x88           /* generate STOP condition */
    stb     r10, 8(r14)
    eieio

    lbz     r3, 16(r14)         /* return read byte */

    mtspr   8, r25              /* restore link register */
    blr

gotNoAck:
    li      r10, 0x80           /* generate STOP condition */
    stb     r10, 8(r14)
    eieio
spdErr:
    orc     r3, r0, r0          /* return -1 */
    mtspr   8, r25              /* restore link register */
    blr

get_lnk_reg:
    mflr    r3                  /* return link reg */
    blr

MessageBlock:

MinitLogo:
    .ascii  "\015\012*** ELTEC Elektronik, Mainz ***\015\012"
    .ascii  "\015\012Initialising RAM\015\012\000"
Mspd01:
    .ascii  "       Reading SPD of SODIMM ...... \000"
MramTyp:
    .ascii  "\015\012\SDRAM with CL=2 at 100 MHz required!\015\012\000"
MramConfErr:
    .ascii  "\015\012\Unsupported SODIMM Configuration!\015\012\000"
Mactivate:
    .ascii  "       Activating \000"
Mact0123e:
    .ascii  " MByte.\015\012\000"
Mok:
    .ascii  "OK \015\012\000"
Mfail:
    .ascii  "FAILED \015\012\000"
MnewLine:
    .ascii  "\015\012\000"
    .align 4
