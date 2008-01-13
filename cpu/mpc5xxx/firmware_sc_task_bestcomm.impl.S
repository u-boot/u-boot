/*
 * Copyright (C) 2001, Software Center, Motorola China.
 *
 * This file contains microcode for the FEC controller of the MPC5200 CPU.
 */

#include <config.h>

#if defined(CONFIG_MPC5200)

/* sas/sccg, gas target */
.section        smartdmaInitData,"aw",@progbits	/* Initialized data for task variables */
.section        smartdmaTaskTable,"aw",@progbits	/* Task tables */
.align  9
.globl taskTable
taskTable:
.globl scEthernetRecv_Entry
scEthernetRecv_Entry:		/* Task 0 */
.long   scEthernetRecv_TDT - taskTable	/* Task 0 Descriptor Table */
.long   scEthernetRecv_TDT - taskTable + 0x000000a4
.long   scEthernetRecv_VarTab - taskTable	/* Task 0 Variable Table */
.long   scEthernetRecv_FDT - taskTable + 0x03	/* Task 0 Function Descriptor Table & Flags */
.long   0x00000000
.long   0x00000000
.long   scEthernetRecv_CSave - taskTable	/* Task 0 context save space */
.long   CFG_MBAR
.globl scEthernetXmit_Entry
scEthernetXmit_Entry:		/* Task 1 */
.long   scEthernetXmit_TDT - taskTable	/* Task 1 Descriptor Table */
.long   scEthernetXmit_TDT - taskTable + 0x000000d0
.long   scEthernetXmit_VarTab - taskTable	/* Task 1 Variable Table */
.long   scEthernetXmit_FDT - taskTable + 0x03	/* Task 1 Function Descriptor Table & Flags */
.long   0x00000000
.long   0x00000000
.long   scEthernetXmit_CSave - taskTable	/* Task 1 context save space */
.long   CFG_MBAR


.globl scEthernetRecv_TDT
scEthernetRecv_TDT:	/* Task 0 Descriptor Table */
.long   0xc4c50000	/* 0000:  LCDEXT: idx0 = var9 + var10; idx0 once var0; idx0 += inc0 */
.long   0x84c5e000	/* 0004:  LCD: idx1 = var9 + var11; ; idx1 += inc0 */
.long   0x10001f08	/* 0008:    DRD1A: var7 = idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x10000380	/* 000C:    DRD1A: var0 = *idx0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000f88	/* 0010:    DRD1A: var3 = *idx1; FN=0 init=0 WS=0 RS=0 */
.long   0x81980000	/* 0014:  LCD: idx0 = var3; idx0 once var0; idx0 += inc0 */
.long   0x10000780	/* 0018:    DRD1A: var1 = *idx0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 001C:    DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x010cf04c	/* 0020:    DRD2B1: var4 = EU3(); EU3(var1,var12)  */
.long   0x82180349	/* 0024:  LCD: idx0 = var4; idx0 != var13; idx0 += inc1 */
.long   0x81c68004	/* 0028:    LCD: idx1 = var3 + var13 + 4; idx1 once var0; idx1 += inc0 */
.long   0x70000000	/* 002C:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x018cf04e	/* 0030:      DRD2B1: var6 = EU3(); EU3(var1,var14)  */
.long   0x70000000	/* 0034:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x020cf04f	/* 0038:      DRD2B1: var8 = EU3(); EU3(var1,var15)  */
.long   0x00000b88	/* 003C:      DRD1A: var2 = *idx1; FN=0 init=0 WS=0 RS=0 */
.long   0x8000d184	/* 0040:    LCDEXT: idx1 = 0xf0003184; ; */
.long   0xc6990452	/* 0044:    LCDEXT: idx2 = var13; idx2 < var17; idx2 += inc2 */
.long   0x81486010	/* 0048:    LCD: idx3 = var2 + var16; ; idx3 += inc2 */
.long   0x006acf88	/* 004C:      DRD1A: *idx3 = *idx1; FN=0 init=3 WS=1 RS=1 */
.long   0x8000d184	/* 0050:    LCDEXT: idx1 = 0xf0003184; ; */
.long   0x86810492	/* 0054:    LCD: idx2 = var13, idx3 = var2; idx2 < var18; idx2 += inc2, idx3 += inc2 */
.long   0x006acf88	/* 0058:      DRD1A: *idx3 = *idx1; FN=0 init=3 WS=1 RS=1 */
.long   0x8000d184	/* 005C:    LCDEXT: idx1 = 0xf0003184; ; */
.long   0x868184d2	/* 0060:    LCD: idx2 = var13, idx3 = var3; idx2 < var19; idx2 += inc2, idx3 += inc2 */
.long   0x000acf88	/* 0064:      DRD1A: *idx3 = *idx1; FN=0 init=0 WS=1 RS=1 */
.long   0xc318839b	/* 0068:    LCDEXT: idx1 = var6; idx1 == var14; idx1 += inc3 */
.long   0x80190000	/* 006C:    LCD: idx2 = var0; idx2 once var0; idx2 += inc0 */
.long   0x04008468	/* 0070:      DRD1A: idx1 = var13; FN=0 INT init=0 WS=0 RS=0 */
.long   0xc4038358	/* 0074:    LCDEXT: idx1 = var8, idx2 = var7; idx1 == var13; idx1 += inc3, idx2 += inc0 */
.long   0x81c50000	/* 0078:    LCD: idx3 = var3 + var10; idx3 once var0; idx3 += inc0 */
.long   0x1000cb18	/* 007C:      DRD1A: *idx2 = idx3; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000f18	/* 0080:      DRD1A: var3 = idx3; FN=0 init=0 WS=0 RS=0 */
.long   0xc4188364	/* 0084:    LCDEXT: idx1 = var8; idx1 > var13; idx1 += inc4 */
.long   0x83990000	/* 0088:    LCD: idx2 = var7; idx2 once var0; idx2 += inc0 */
.long   0x10000c00	/* 008C:      DRD1A: var3 = var0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x0000c800	/* 0090:      DRD1A: *idx2 = var0; FN=0 init=0 WS=0 RS=0 */
.long   0x81988000	/* 0094:    LCD: idx1 = var3; idx1 once var0; idx1 += inc0 */
.long   0x10000788	/* 0098:      DRD1A: var1 = *idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 009C:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x080cf04c	/* 00A0:      DRD2B1: idx0 = EU3(); EU3(var1,var12)  */
.long   0x000001f8	/* 00A4(:0):    NOP */


.globl scEthernetXmit_TDT
scEthernetXmit_TDT:	/* Task 1 Descriptor Table */
.long   0x80024800	/* 0000:  LCDEXT: idx0 = 0xf0008800; ; */
.long   0x85c60004	/* 0004:  LCD: idx1 = var11 + var12 + 4; idx1 once var0; idx1 += inc0 */
.long   0x10002308	/* 0008:    DRD1A: var8 = idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x10000f88	/* 000C:    DRD1A: var3 = *idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000380	/* 0010:    DRD1A: var0 = *idx0; FN=0 init=0 WS=0 RS=0 */
.long   0x81980000	/* 0014:  LCD: idx0 = var3; idx0 once var0; idx0 += inc0 */
.long   0x10000780	/* 0018:    DRD1A: var1 = *idx0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 001C:    DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x024cf04d	/* 0020:    DRD2B1: var9 = EU3(); EU3(var1,var13)  */
.long   0x84980309	/* 0024:  LCD: idx0 = var9; idx0 != var12; idx0 += inc1 */
.long   0xc0004003	/* 0028:    LCDEXT: idx1 = 0x00000003; ; */
.long   0x81c60004	/* 002C:    LCD: idx2 = var3 + var12 + 4; idx2 once var0; idx2 += inc0 */
.long   0x70000000	/* 0030:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x010cf04e	/* 0034:      DRD2B1: var4 = EU3(); EU3(var1,var14)  */
.long   0x70000000	/* 0038:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x014cf04f	/* 003C:      DRD2B1: var5 = EU3(); EU3(var1,var15)  */
.long   0x70000000	/* 0040:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x028cf050	/* 0044:      DRD2B1: var10 = EU3(); EU3(var1,var16)  */
.long   0x70000000	/* 0048:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x018cf051	/* 004C:      DRD2B1: var6 = EU3(); EU3(var1,var17)  */
.long   0x10000b90	/* 0050:      DRD1A: var2 = *idx2; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 0054:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x01ccf0a1	/* 0058:      DRD2B1: var7 = EU3(); EU3(var2,idx1)  */
.long   0xc2988312	/* 005C:    LCDEXT: idx1 = var5; idx1 > var12; idx1 += inc2 */
.long   0x83490000	/* 0060:    LCD: idx2 = var6 + var18; idx2 once var0; idx2 += inc0 */
.long   0x00001b10	/* 0064:      DRD1A: var6 = idx2; FN=0 init=0 WS=0 RS=0 */
.long   0x8000d1a4	/* 0068:    LCDEXT: idx1 = 0xf00031a4; ; */
.long   0x8301031c	/* 006C:    LCD: idx2 = var6, idx3 = var2; idx2 > var12; idx2 += inc3, idx3 += inc4 */
.long   0x008ac798	/* 0070:      DRD1A: *idx1 = *idx3; FN=0 init=4 WS=1 RS=1 */
.long   0x8000d1a4	/* 0074:    LCDEXT: idx1 = 0xf00031a4; ; */
.long   0xc1430000	/* 0078:    LCDEXT: idx2 = var2 + var6; idx2 once var0; idx2 += inc0 */
.long   0x82998312	/* 007C:    LCD: idx3 = var5; idx3 > var12; idx3 += inc2 */
.long   0x088ac790	/* 0080:      DRD1A: *idx1 = *idx2; FN=0 TFD init=4 WS=1 RS=1 */
.long   0x81988000	/* 0084:    LCD: idx1 = var3; idx1 once var0; idx1 += inc0 */
.long   0x60000001	/* 0088:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=1 EXT init=0 WS=0 RS=0 */
.long   0x0c4cfc4d	/* 008C:      DRD2B1: *idx1 = EU3(); EU3(*idx1,var13)  */
.long   0xc21883ad	/* 0090:    LCDEXT: idx1 = var4; idx1 == var14; idx1 += inc5 */
.long   0x80190000	/* 0094:    LCD: idx2 = var0; idx2 once var0; idx2 += inc0 */
.long   0x04008460	/* 0098:      DRD1A: idx1 = var12; FN=0 INT init=0 WS=0 RS=0 */
.long   0xc4052305	/* 009C:    LCDEXT: idx1 = var8, idx2 = var10; idx2 == var12; idx1 += inc0, idx2 += inc5 */
.long   0x81c98000	/* 00A0:    LCD: idx3 = var3 + var19; idx3 once var0; idx3 += inc0 */
.long   0x1000c718	/* 00A4:      DRD1A: *idx1 = idx3; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000f18	/* 00A8:      DRD1A: var3 = idx3; FN=0 init=0 WS=0 RS=0 */
.long   0xc4188000	/* 00AC:    LCDEXT: idx1 = var8; idx1 once var0; idx1 += inc0 */
.long   0x85190312	/* 00B0:    LCD: idx2 = var10; idx2 > var12; idx2 += inc2 */
.long   0x10000c00	/* 00B4:      DRD1A: var3 = var0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x1000c400	/* 00B8:      DRD1A: *idx1 = var0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00008860	/* 00BC:      DRD1A: idx2 = var12; FN=0 init=0 WS=0 RS=0 */
.long   0x81988000	/* 00C0:    LCD: idx1 = var3; idx1 once var0; idx1 += inc0 */
.long   0x10000788	/* 00C4:      DRD1A: var1 = *idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 00C8:      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x080cf04d	/* 00CC:      DRD2B1: idx0 = EU3(); EU3(var1,var13)  */
.long   0x000001f8	/* 00D0(:0):    NOP */

.align  8

.globl scEthernetRecv_VarTab
scEthernetRecv_VarTab:	/* Task 0 Variable Table */
.long   0x00000000	/* var[0] */
.long   0x00000000	/* var[1] */
.long   0x00000000	/* var[2] */
.long   0x00000000	/* var[3] */
.long   0x00000000	/* var[4] */
.long   0x00000000	/* var[5] */
.long   0x00000000	/* var[6] */
.long   0x00000000	/* var[7] */
.long   0x00000000	/* var[8] */
.long   (CFG_MBAR + 0x8800)	/* var[9] */
.long   0x00000008	/* var[10] */
.long   0x0000000c	/* var[11] */
.long   0x80000000	/* var[12] */
.long   0x00000000	/* var[13] */
.long   0x10000000	/* var[14] */
.long   0x20000000	/* var[15] */
.long   0x000005e4	/* var[16] */
.long   0x0000000e	/* var[17] */
.long   0x000005e0	/* var[18] */
.long   0x00000004	/* var[19] */
.long   0x00000000	/* var[20] */
.long   0x00000000	/* var[21] */
.long   0x00000000	/* var[22] */
.long   0x00000000	/* var[23] */
.long   0x00000000	/* inc[0] */
.long   0x60000000	/* inc[1] */
.long   0x20000001	/* inc[2] */
.long   0x80000000	/* inc[3] */
.long   0x40000000	/* inc[4] */
.long   0x00000000	/* inc[5] */
.long   0x00000000	/* inc[6] */
.long   0x00000000	/* inc[7] */

.align  8

.globl scEthernetXmit_VarTab
scEthernetXmit_VarTab:	/* Task 1 Variable Table */
.long   0x00000000	/* var[0] */
.long   0x00000000	/* var[1] */
.long   0x00000000	/* var[2] */
.long   0x00000000	/* var[3] */
.long   0x00000000	/* var[4] */
.long   0x00000000	/* var[5] */
.long   0x00000000	/* var[6] */
.long   0x00000000	/* var[7] */
.long   0x00000000	/* var[8] */
.long   0x00000000	/* var[9] */
.long   0x00000000	/* var[10] */
.long   (CFG_MBAR + 0x8800)	/* var[11] */
.long   0x00000000	/* var[12] */
.long   0x80000000	/* var[13] */
.long   0x10000000	/* var[14] */
.long   0x08000000	/* var[15] */
.long   0x20000000	/* var[16] */
.long   0x0000ffff	/* var[17] */
.long   0xffffffff	/* var[18] */
.long   0x00000008	/* var[19] */
.long   0x00000000	/* var[20] */
.long   0x00000000	/* var[21] */
.long   0x00000000	/* var[22] */
.long   0x00000000	/* var[23] */
.long   0x00000000	/* inc[0] */
.long   0x60000000	/* inc[1] */
.long   0x40000000	/* inc[2] */
.long   0x4000ffff	/* inc[3] */
.long   0xe0000001	/* inc[4] */
.long   0x80000000	/* inc[5] */
.long   0x00000000	/* inc[6] */
.long   0x00000000	/* inc[7] */

.align  8

.globl scEthernetRecv_FDT
scEthernetRecv_FDT:	/* Task 0 Function Descriptor Table */
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x21800000	/* and(), EU# 3 */
.long   0x21400000	/* andn(), EU# 3 */
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000

.align  8

.globl scEthernetXmit_FDT
scEthernetXmit_FDT:	/* Task 1 Function Descriptor Table */
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x21800000	/* and(), EU# 3 */
.long   0x21400000	/* andn(), EU# 3 */
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000
.long   0x00000000


.globl scEthernetRecv_CSave
scEthernetRecv_CSave:	/* Task 0 context save space */
.space  128, 0x0


.globl scEthernetXmit_CSave
scEthernetXmit_CSave:	/* Task 1 context save space */
.space  128, 0x0

#endif /* CONFIG_MPC5200 */
