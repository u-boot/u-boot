/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 * Public prototypes for SoC FPGA RSU console-helper functions.
 */
#ifndef _RSU_CONSOLE_H_
#define _RSU_CONSOLE_H_

int rsu_spt_cpb_list(int argc, char * const argv[]);
int rsu_update(int argc, char * const argv[]);
int rsu_dtb(int argc, char * const argv[]);

#endif /* _RSU_CONSOLE_H_ */
