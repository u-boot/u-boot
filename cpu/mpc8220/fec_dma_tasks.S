/*
 * Copyright (C) 2004, Freescale Semiconductor, Inc.
 *
 * This file contains microcode for the FEC controller of the MPC8220.
 */

#include <config.h>

#if defined(CONFIG_MPC8220)

/* sas/sccg, gas target */
.section        smartdmaInitData,"aw",@progbits	/* Initialized data for task variables */
.section        smartdmaTaskTable,"aw",@progbits	/* Task tables */
.align  9
.globl taskTable
taskTable:
.globl scEthernetRecv_Entry
scEthernetRecv_Entry:		/* Task 0 */
.long   scEthernetRecv_TDT - taskTable	/* Task 0 Descriptor Table */
.long   scEthernetRecv_TDT - taskTable + 0x00000094
.long   scEthernetRecv_VarTab - taskTable	/* Task 0 Variable Table */
.long   scEthernetRecv_FDT - taskTable + 0x03	/* Task 0 Function Descriptor Table & Flags */
.long   0x00000000
.long   0x00000000
.long   scEthernetRecv_CSave - taskTable	/* Task 0 context save space */
.long   0xf0000000
.globl scEthernetXmit_Entry
scEthernetXmit_Entry:		/* Task 1 */
.long   scEthernetXmit_TDT - taskTable	/* Task 1 Descriptor Table */
.long   scEthernetXmit_TDT - taskTable + 0x000000e0
.long   scEthernetXmit_VarTab - taskTable	/* Task 1 Variable Table */
.long   scEthernetXmit_FDT - taskTable + 0x03	/* Task 1 Function Descriptor Table & Flags */
.long   0x00000000
.long   0x00000000
.long   scEthernetXmit_CSave - taskTable	/* Task 1 context save space */
.long   0xf0000000


.globl scEthernetRecv_TDT
scEthernetRecv_TDT:	/* Task 0 Descriptor Table */
.long   0xc4c50000	/* 0000(153):  LCDEXT: idx0 = var9 + var10; idx0 once var0; idx0 += inc0 */
.long   0x84c5e000	/* 0004(153):  LCD: idx1 = var9 + var11; ; idx1 += inc0 */
.long   0x10001f08	/* 0008(156):    DRD1A: var7 = idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x10000380	/* 000C(157):    DRD1A: var0 = *idx0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000f88	/* 0010(158):    DRD1A: var3 = *idx1; FN=0 init=0 WS=0 RS=0 */
.long   0x81980000	/* 0014(162):  LCD: idx0 = var3; idx0 once var0; idx0 += inc0 */
.long   0x10000780	/* 0018(164):    DRD1A: var1 = *idx0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 001C(165):    DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x010cf04c	/* 0020(165):    DRD2B1: var4 = EU3(); EU3(var1,var12)  */
.long   0x82180349	/* 0024(169):  LCD: idx0 = var4; idx0 != var13; idx0 += inc1 */
.long   0x81c68004	/* 0028(172):    LCD: idx1 = var3 + var13 + 4; idx1 once var0; idx1 += inc0 */
.long   0x70000000	/* 002C(174):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x018cf04e	/* 0030(174):      DRD2B1: var6 = EU3(); EU3(var1,var14)  */
.long   0x70000000	/* 0034(175):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x020cf04f	/* 0038(175):      DRD2B1: var8 = EU3(); EU3(var1,var15)  */
.long   0x00000b88	/* 003C(176):      DRD1A: var2 = *idx1; FN=0 init=0 WS=0 RS=0 */
.long   0x80025184	/* 0040(205):    LCDEXT: idx1 = 0xf0009184; ; */
.long   0x86810412	/* 0044(205):    LCD: idx2 = var13, idx3 = var2; idx2 < var16; idx2 += inc2, idx3 += inc2 */
.long   0x0200cf88	/* 0048(209):      DRD1A: *idx3 = *idx1; FN=0 init=16 WS=0 RS=0 */
.long   0x80025184	/* 004C(217):    LCDEXT: idx1 = 0xf0009184; ; */
.long   0x8681845b	/* 0050(217):    LCD: idx2 = var13, idx3 = var3; idx2 < var17; idx2 += inc3, idx3 += inc3 */
.long   0x0000cf88	/* 0054(221):      DRD1A: *idx3 = *idx1; FN=0 init=0 WS=0 RS=0 */
.long   0xc31883a4	/* 0058(225):    LCDEXT: idx1 = var6; idx1 == var14; idx1 += inc4 */
.long   0x80190000	/* 005C(225):    LCD: idx2 = var0; idx2 once var0; idx2 += inc0 */
.long   0x04008468	/* 0060(227):      DRD1A: idx1 = var13; FN=0 INT init=0 WS=0 RS=0 */
.long   0xc4038360	/* 0064(232):    LCDEXT: idx1 = var8, idx2 = var7; idx1 == var13; idx1 += inc4, idx2 += inc0 */
.long   0x81c50000	/* 0068(233):    LCD: idx3 = var3 + var10; idx3 once var0; idx3 += inc0 */
.long   0x1000cb18	/* 006C(235):      DRD1A: *idx2 = idx3; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000f18	/* 0070(236):      DRD1A: var3 = idx3; FN=0 init=0 WS=0 RS=0 */
.long   0xc418836d	/* 0074(238):    LCDEXT: idx1 = var8; idx1 > var13; idx1 += inc5 */
.long   0x83990000	/* 0078(238):    LCD: idx2 = var7; idx2 once var0; idx2 += inc0 */
.long   0x10000c00	/* 007C(240):      DRD1A: var3 = var0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x0000c800	/* 0080(241):      DRD1A: *idx2 = var0; FN=0 init=0 WS=0 RS=0 */
.long   0x81988000	/* 0084(245):    LCD: idx1 = var3; idx1 once var0; idx1 += inc0 */
.long   0x10000788	/* 0088(247):      DRD1A: var1 = *idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 008C(248):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x080cf04c	/* 0090(248):      DRD2B1: idx0 = EU3(); EU3(var1,var12)  */
.long   0x000001f8	/* 0094(:0):    NOP */


.globl scEthernetXmit_TDT
scEthernetXmit_TDT:	/* Task 1 Descriptor Table */
.long   0x80095b00	/* 0000(280):  LCDEXT: idx0 = 0xf0025b00; ; */
.long   0x85c60004	/* 0004(280):  LCD: idx1 = var11 + var12 + 4; idx1 once var0; idx1 += inc0 */
.long   0x10002308	/* 0008(283):    DRD1A: var8 = idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x10000f88	/* 000C(284):    DRD1A: var3 = *idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000380	/* 0010(285):    DRD1A: var0 = *idx0; FN=0 init=0 WS=0 RS=0 */
.long   0x81980000	/* 0014(288):  LCD: idx0 = var3; idx0 once var0; idx0 += inc0 */
.long   0x10000780	/* 0018(290):    DRD1A: var1 = *idx0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 001C(291):    DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x024cf04d	/* 0020(291):    DRD2B1: var9 = EU3(); EU3(var1,var13)  */
.long   0x84980309	/* 0024(294):  LCD: idx0 = var9; idx0 != var12; idx0 += inc1 */
.long   0xc0004003	/* 0028(297):    LCDEXT: idx1 = 0x00000003; ; */
.long   0x81c60004	/* 002C(297):    LCD: idx2 = var3 + var12 + 4; idx2 once var0; idx2 += inc0 */
.long   0x70000000	/* 0030(299):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x010cf04e	/* 0034(299):      DRD2B1: var4 = EU3(); EU3(var1,var14)  */
.long   0x70000000	/* 0038(300):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x014cf04f	/* 003C(300):      DRD2B1: var5 = EU3(); EU3(var1,var15)  */
.long   0x70000000	/* 0040(301):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x028cf050	/* 0044(301):      DRD2B1: var10 = EU3(); EU3(var1,var16)  */
.long   0x70000000	/* 0048(302):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT MORE init=0 WS=0 RS=0 */
.long   0x018cf051	/* 004C(302):      DRD2B1: var6 = EU3(); EU3(var1,var17)  */
.long   0x10000b90	/* 0050(303):      DRD1A: var2 = *idx2; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 0054(304):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x01ccf0a1	/* 0058(304):      DRD2B1: var7 = EU3(); EU3(var2,idx1)  */
.long   0xc2988312	/* 005C(308):    LCDEXT: idx1 = var5; idx1 > var12; idx1 += inc2 */
.long   0x83490000	/* 0060(308):    LCD: idx2 = var6 + var18; idx2 once var0; idx2 += inc0 */
.long   0x00001b10	/* 0064(310):      DRD1A: var6 = idx2; FN=0 init=0 WS=0 RS=0 */
.long   0x800251a4	/* 0068(315):    LCDEXT: idx1 = 0xf00091a4; ; */
.long   0xc30104dc	/* 006C(315):    LCDEXT: idx2 = var6, idx3 = var2; idx2 >= var19; idx2 += inc3, idx3 += inc4 */
.long   0x839a032d	/* 0070(316):    LCD: idx4 = var7; idx4 == var12; idx4 += inc5 */
.long   0x0220c798	/* 0074(321):      DRD1A: *idx1 = *idx3; FN=0 init=17 WS=0 RS=0 */
.long   0x800251a4	/* 0078(329):    LCDEXT: idx1 = 0xf00091a4; ; */
.long   0x99198337	/* 007C(329):    LCD: idx2 = idx2, idx3 = idx3; idx2 > var12; idx2 += inc6, idx3 += inc7 */
.long   0x022ac798	/* 0080(333):      DRD1A: *idx1 = *idx3; FN=0 init=17 WS=1 RS=1 */
.long   0x800251a4	/* 0084(350):    LCDEXT: idx1 = 0xf00091a4; ; */
.long   0xc1430000	/* 0088(350):    LCDEXT: idx2 = var2 + var6; idx2 once var0; idx2 += inc0 */
.long   0x82998312	/* 008C(351):    LCD: idx3 = var5; idx3 > var12; idx3 += inc2 */
.long   0x0a2ac790	/* 0090(354):      DRD1A: *idx1 = *idx2; FN=0 TFD init=17 WS=1 RS=1 */
.long   0x81988000	/* 0094(359):    LCD: idx1 = var3; idx1 once var0; idx1 += inc0 */
.long   0x60000002	/* 0098(361):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=2 EXT init=0 WS=0 RS=0 */
.long   0x0c4cfc4d	/* 009C(361):      DRD2B1: *idx1 = EU3(); EU3(*idx1,var13)  */
.long   0xc21883ad	/* 00A0(365):    LCDEXT: idx1 = var4; idx1 == var14; idx1 += inc5 */
.long   0x80190000	/* 00A4(365):    LCD: idx2 = var0; idx2 once var0; idx2 += inc0 */
.long   0x04008460	/* 00A8(367):      DRD1A: idx1 = var12; FN=0 INT init=0 WS=0 RS=0 */
.long   0xc4052305	/* 00AC(371):    LCDEXT: idx1 = var8, idx2 = var10; idx2 == var12; idx1 += inc0, idx2 += inc5 */
.long   0x81ca0000	/* 00B0(372):    LCD: idx3 = var3 + var20; idx3 once var0; idx3 += inc0 */
.long   0x1000c718	/* 00B4(374):      DRD1A: *idx1 = idx3; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00000f18	/* 00B8(375):      DRD1A: var3 = idx3; FN=0 init=0 WS=0 RS=0 */
.long   0xc4188000	/* 00BC(378):    LCDEXT: idx1 = var8; idx1 once var0; idx1 += inc0 */
.long   0x85190312	/* 00C0(378):    LCD: idx2 = var10; idx2 > var12; idx2 += inc2 */
.long   0x10000c00	/* 00C4(380):      DRD1A: var3 = var0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x1000c400	/* 00C8(381):      DRD1A: *idx1 = var0; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x00008860	/* 00CC(382):      DRD1A: idx2 = var12; FN=0 init=0 WS=0 RS=0 */
.long   0x81988000	/* 00D0(386):    LCD: idx1 = var3; idx1 once var0; idx1 += inc0 */
.long   0x10000788	/* 00D4(388):      DRD1A: var1 = *idx1; FN=0 MORE init=0 WS=0 RS=0 */
.long   0x60000000	/* 00D8(389):      DRD2A: EU0=0 EU1=0 EU2=0 EU3=0 EXT init=0 WS=0 RS=0 */
.long   0x080cf04d	/* 00DC(389):      DRD2B1: idx0 = EU3(); EU3(var1,var13)  */
.long   0x000001f8	/* 00E0(:0):    NOP */

.align 8

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
.long   0xf0025b00	/* var[9] */
.long   0x00000008	/* var[10] */
.long   0x0000000c	/* var[11] */
.long   0x80000000	/* var[12] */
.long   0x00000000	/* var[13] */
.long   0x10000000	/* var[14] */
.long   0x20000000	/* var[15] */
.long   0x00000800	/* var[16] */
.long   0x00000001	/* var[17] */
.long   0x00000000	/* var[18] */
.long   0x00000000	/* var[19] */
.long   0x00000000	/* var[20] */
.long   0x00000000	/* var[21] */
.long   0x00000000	/* var[22] */
.long   0x00000000	/* var[23] */
.long   0x00000000	/* inc[0] */
.long   0x60000000	/* inc[1] */
.long   0x20000004	/* inc[2] */
.long   0x20000001	/* inc[3] */
.long   0x80000000	/* inc[4] */
.long   0x40000000	/* inc[5] */
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
.long   0xf0025b00	/* var[11] */
.long   0x00000000	/* var[12] */
.long   0x80000000	/* var[13] */
.long   0x10000000	/* var[14] */
.long   0x08000000	/* var[15] */
.long   0x20000000	/* var[16] */
.long   0x0000ffff	/* var[17] */
.long   0xffffffff	/* var[18] */
.long   0x00000004	/* var[19] */
.long   0x00000008	/* var[20] */
.long   0x00000000	/* var[21] */
.long   0x00000000	/* var[22] */
.long   0x00000000	/* var[23] */
.long   0x00000000	/* inc[0] */
.long   0x60000000	/* inc[1] */
.long   0x40000000	/* inc[2] */
.long   0xc000fffc	/* inc[3] */
.long   0xe0000004	/* inc[4] */
.long   0x80000000	/* inc[5] */
.long   0x4000ffff	/* inc[6] */
.long   0xe0000001	/* inc[7] */

.align 8

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
.long   0x21e00000	/* or(), EU# 3 */
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

.align 8

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
.long   0x21e00000	/* or(), EU# 3 */
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


.globl scEthernetRecv_CSave
scEthernetRecv_CSave:	/* Task 0 context save space */
.space  128, 0x0


.globl scEthernetXmit_CSave
scEthernetXmit_CSave:	/* Task 1 context save space */
.space  128, 0x0

#endif
