/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _OPENRISC_EXC_H_
#define _OPENRISC_EXC_H_

#define EXC_RESET		0x01
#define EXC_BUS_ERROR		0x02
#define EXC_DATA_PAGE_FAULT	0x03
#define EXC_INSTR_PAGE_FAULT	0x04
#define EXC_TIMER		0x05
#define EXC_ALIGNMENT		0x06
#define EXC_ILLEGAL_INSTR	0x07
#define EXC_EXT_IRQ		0x08
#define EXC_DTLB_MISS		0x09
#define EXC_ITLB_MISS		0x0a
#define EXC_RANGE		0x0b
#define EXC_SYSCALL		0x0c
#define EXC_FLOAT_POINT		0x0d
#define EXC_TRAP		0x0e

void exception_install_handler(int exception, void (*handler)(void));
void exception_free_handler(int exception);

#endif
