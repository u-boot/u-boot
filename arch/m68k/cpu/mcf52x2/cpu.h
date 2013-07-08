/*
 *  cpu.h
 *
 *  Copyright (c) 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CPU_H_
#define _CPU_H_

#include <command.h>

/* Use this to create board specific reset functions */
void board_reset(void) __attribute__((__weak__));

#endif /* _CPU_H_ */
