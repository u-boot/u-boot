/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019 Xilinx, Inc.
 * Siva Durga Prasad Paladugu <siva.durga.paladugu@xilinx.com>
 *
 */

#ifndef __FRU_H
#define __FRU_H

int fru_display(void);
int fru_capture(unsigned long addr);

#endif /* FRU_H */
