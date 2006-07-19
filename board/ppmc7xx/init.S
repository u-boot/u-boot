/*
 * init.S
 * ------
 *
 * Wind River PPMC 7xx/74xx init code.
 *
 * By Richard Danter (richard.danter@windriver.com)
 * Copyright (C) 2005 Wind River Systems
 *
 * NOTE: The following code was generated automatically by Workbench
 *       from the ppmc7400_107.reg register file.
 */

#include <ppc_asm.tmpl>


.globl board_asm_init
board_asm_init:

      lis    r4,0xFEC0
      ori    r4,r4,0x0000
      lis    r5,0xFEE0
      ori    r5,r5,0x0000
      lis    r3,0x8000          # ADDR_00
      ori    r3,r3,0x0000
      stwbrx    r3,0,r4
      li     r3,0x1057          # VENDOR
      li    r8, 0x0
      sthbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_02
      ori    r3,r3,0x0002
      stwbrx    r3,0,r4
      li     r3,0x0004          # ID
      li    r8, 0x2
      sthbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_04
      ori    r3,r3,0x0004
      stwbrx    r3,0,r4
      li     r3,0x0006          # PCICMD
      li    r8, 0x0
      sthbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_06
      ori    r3,r3,0x0006
      stwbrx    r3,0,r4
      li     r3,0x00A0          # PCISTAT
      li    r8, 0x2
      sthbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_08
      ori    r3,r3,0x0008
      stwbrx    r3,0,r4
      li     r3,0x10            # REVID
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_09
      ori    r3,r3,0x0009
      stwbrx    r3,0,r4
      li     r3,0x00            # PROGIR
      stb    r3,0x1(r5)
      lis    r3,0x8000          # ADDR_0A
      ori    r3,r3,0x000A
      stwbrx    r3,0,r4
      li     r3,0x00            # SUBCCODE
      stb    r3,0x2(r5)
      lis    r3,0x8000          # ADDR_0B
      ori    r3,r3,0x000B
      stwbrx    r3,0,r4
      li     r3,0x06            # PBCCR
      stb    r3,0x3(r5)
      lis    r3,0x8000          # ADDR_0C
      ori    r3,r3,0x000C
      stwbrx    r3,0,r4
      li     r3,0x08            # PCLSR
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_0D
      ori    r3,r3,0x000D
      stwbrx    r3,0,r4
      li     r3,0x00            # PLTR
      stb    r3,0x1(r5)
      lis    r3,0x8000          # ADDR_0E
      ori    r3,r3,0x000E
      stwbrx    r3,0,r4
      li     r3,0x00            # HEADTYPE
      stb    r3,0x2(r5)
      lis    r3,0x8000          # ADDR_0F
      ori    r3,r3,0x000F
      stwbrx    r3,0,r4
      li     r3,0x00            # BISTCTRL
      stb    r3,0x3(r5)
      lis    r3,0x8000          # ADDR_10
      ori    r3,r3,0x0010
      stwbrx    r3,0,r4
      lis    r3,0x0000          # LMBAR
      ori    r3,r3,0x0008
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_14
      ori    r3,r3,0x0014
      stwbrx    r3,0,r4
      lis    r3,0xF000          # PCSRBAR
      ori    r3,r3,0x0000
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_3C
      ori    r3,r3,0x003C
      stwbrx    r3,0,r4
      li     r3,0x00            # ILR
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_3D
      ori    r3,r3,0x003D
      stwbrx    r3,0,r4
      li     r3,0x01            # INTPIN
      stb    r3,0x1(r5)
      lis    r3,0x8000          # ADDR_3E
      ori    r3,r3,0x003E
      stwbrx    r3,0,r4
      li     r3,0x00            # MIN_GNT
      stb    r3,0x2(r5)
      lis    r3,0x8000          # ADDR_3F
      ori    r3,r3,0x003F
      stwbrx    r3,0,r4
      li     r3,0x00            # MAX_LAT
      stb    r3,0x3(r5)
      lis    r3,0x8000          # ADDR_40
      ori    r3,r3,0x0040
      stwbrx    r3,0,r4
      li     r3,0x00            # BUSNB
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_41
      ori    r3,r3,0x0041
      stwbrx    r3,0,r4
      li     r3,0x00            # SBUSNB
      stb    r3,0x1(r5)
      lis    r3,0x8000          # ADDR_46
      ori    r3,r3,0x0046
      stwbrx    r3,0,r4
#      li     r3,0xE080          # PCIARB
      li     r3,-0x1F80          # PCIARB
      li    r8, 0x2
      sthbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_70
      ori    r3,r3,0x0070
      stwbrx    r3,0,r4
      li     r3,0x0000          # PMCR1
      li    r8, 0x0
      sthbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_72
      ori    r3,r3,0x0072
      stwbrx    r3,0,r4
      li     r3,0xC0            # PMCR2
      stb    r3,0x2(r5)
      lis    r3,0x8000          # ADDR_73
      ori    r3,r3,0x0073
      stwbrx    r3,0,r4
      li     r3,0xEF            # ODCR
      stb    r3,0x3(r5)
      lis    r3,0x8000          # ADDR_74
      ori    r3,r3,0x0074
      stwbrx    r3,0,r4
      li     r3,0x7D00          # CLKDCR
      li    r8, 0x0
      sthbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_76
      ori    r3,r3,0x0076
      stwbrx    r3,0,r4
      li     r3,0x00            # MDCR
      stb    r3,0x2(r5)
      lis    r6,0xFCE0
      ori    r6,r6,0x0000       # r6 is the EUMBAR Base Address
      lis    r3,0x8000          # ADDR_78
      ori    r3,r3,0x0078
      stwbrx    r3,0,r4
      lis    r3,0xFCE0          # EUMBBAR
      ori    r3,r3,0x0000
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_80
      ori    r3,r3,0x0080
      stwbrx    r3,0,r4
      lis    r3,0xFFFF          # MSADDR1
      ori    r3,r3,0x4000
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_84
      ori    r3,r3,0x0084
      stwbrx    r3,0,r4
      lis    r3,0xFFFF          # MSADDR2
      ori    r3,r3,0xFFFF
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_88
      ori    r3,r3,0x0088
      stwbrx    r3,0,r4
      lis    r3,0x0303          # EMSADDR1
      ori    r3,r3,0x0000
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_8C
      ori    r3,r3,0x008C
      stwbrx    r3,0,r4
      lis    r3,0x0303          # EMSADDR2
      ori    r3,r3,0x0303
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_90
      ori    r3,r3,0x0090
      stwbrx    r3,0,r4
      lis    r3,0xFFFF          # EMEADDR1
      ori    r3,r3,0x7F3F
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_94
      ori    r3,r3,0x0094
      stwbrx    r3,0,r4
      lis    r3,0xFFFF          # EMEADDR2
      ori    r3,r3,0xFFFF
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_98
      ori    r3,r3,0x0098
      stwbrx    r3,0,r4
      lis    r3,0x0303          # EXTEMEM1
      ori    r3,r3,0x0000
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_9C
      ori    r3,r3,0x009C
      stwbrx    r3,0,r4
      lis    r3,0x0303          # EXTEMEM2
      ori    r3,r3,0x0303
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_A0
      ori    r3,r3,0x00A0
      stwbrx    r3,0,r4
      li     r3,0x03            # MEMBNKEN
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_A3
      ori    r3,r3,0x00A3
      stwbrx    r3,0,r4
      li     r3,0x00            # MEMPMODE
      stb    r3,0x3(r5)
      lis    r3,0x8000          # ADDR_B8
      ori    r3,r3,0x00B8
      stwbrx    r3,0,r4
      li     r3,0x00            # ECCCNT
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_B9
      ori    r3,r3,0x00B9
      stwbrx    r3,0,r4
      li     r3,0x00            # ECCTRG
      stb    r3,0x1(r5)
      lis    r3,0x8000          # ADDR_C0
      ori    r3,r3,0x00C0
      stwbrx    r3,0,r4
      li     r3,0xFF            # ERRENR1
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_C1
      ori    r3,r3,0x00C1
      stwbrx    r3,0,r4
      li     r3,0x00            # ERRDR1
      stb    r3,0x1(r5)
      lis    r3,0x8000          # ADDR_C3
      ori    r3,r3,0x00C3
      stwbrx    r3,0,r4
      li     r3,0x50            # IPBESR
      stb    r3,0x3(r5)
      lis    r3,0x8000          # ADDR_C4
      ori    r3,r3,0x00C4
      stwbrx    r3,0,r4
      li     r3,0xBF            # ERRENR2
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_C5
      ori    r3,r3,0x00C5
      stwbrx    r3,0,r4
      li     r3,0x00            # ERRDR2
      stb    r3,0x1(r5)
      lis    r3,0x8000          # ADDR_C7
      ori    r3,r3,0x00C7
      stwbrx    r3,0,r4
      li     r3,0x00            # PCIBESR
      stb    r3,0x3(r5)
      lis    r3,0x8000          # ADDR_C8
      ori    r3,r3,0x00C8
      stwbrx    r3,0,r4
      lis    r3,0x0000          # BERRADDR
      ori    r3,r3,0xE0FE
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_E0
      ori    r3,r3,0x00E0
      stwbrx    r3,0,r4
      li     r3,0xC0            # AMBOR
      stb    r3,0x0(r5)
      lis    r3,0x8000          # ADDR_F4
      ori    r3,r3,0x00F4
      stwbrx    r3,0,r4
      lis    r3,0x0000          # MCCR2
      ori    r3,r3,0x020C
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_F8
      ori    r3,r3,0x00F8
      stwbrx    r3,0,r4
      lis    r3,0x0230          # MCCR3
      ori    r3,r3,0x0000
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_FC
      ori    r3,r3,0x00FC
      stwbrx    r3,0,r4
      lis    r3,0x2532          # MCCR4
      ori    r3,r3,0x2220
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_F0
      ori    r3,r3,0x00F0
      stwbrx    r3,0,r4
      lis    r3,0xFFC8          # MCCR1
      ori    r3,r3,0x0000
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_A8
      ori    r3,r3,0x00A8
      stwbrx    r3,0,r4
      lis    r3,0xFF14          # PICR1
      ori    r3,r3,0x1CC8
      li    r8, 0x0
      stwbrx    r3,r8,r5
      lis    r3,0x8000          # ADDR_AC
      ori    r3,r3,0x00AC
      stwbrx    r3,0,r4
      lis    r3,0x0000          # PICR2
      ori    r3,r3,0x0000
      li    r8, 0x0
      stwbrx    r3,r8,r5

      blr
