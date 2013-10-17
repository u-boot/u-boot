/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_IBMPC_H_
#define __ASM_IBMPC_H_ 1

/* misc ports in an ibm compatible pc */

#define MASTER_PIC      0x20
#define PIT_BASE	0x40
#define KBDDATA         0x60
#define SYSCTLB         0x62
#define KBDCMD          0x64
#define SYSCTLA         0x92
#define SLAVE_PIC       0xa0

#if 1
#define UART0_BASE     0x3f8
#define UART1_BASE     0x2f8
#else
/* FixMe: uarts swapped */
#define UART0_BASE     0x2f8
#define UART1_BASE     0x3f8
#endif


#endif
